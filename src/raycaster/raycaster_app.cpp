#include "raycaster_app.hpp"

#include "camera.hpp"
#include "intersection.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>
#include <sdl_application/sdl_mymath.hpp>
#include <sdl_application/surface_manipulation.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cmath>
#include <stdexcept>
#include <utility>

using namespace mycolor;
using namespace mymath;
using namespace sdl_app;

namespace {

static auto s_use_fog = true;
static auto s_use_bilinear = false;
static auto s_use_textures = true;
static auto s_draw_floor = true;

using namespace raycaster;

constexpr float PI_OVER_2 = M_PI / 2.f;

// This is what works in my VBox setup
constexpr Uint32 desired_framebuffer_formats[]
    = {SDL_PIXELFORMAT_ARGB8888, SDL_PIXELFORMAT_RGB888};

bool save_screenshot(SDL_Surface* framebuffer, const char* filename)
{
    // Try a straight dump
    auto const err = SDL_SaveBMP(framebuffer, filename);
    if (err != 0) {
        SDL_Log("save_screenshot: SDL_SaveBitmap failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

char const* pixel_type_to_string(Uint32 val)
{
    switch (val) {
    case SDL_PIXELTYPE_PACKED32:
        return "SDL_PIXELTYPE_PACKED32";
    default:
        return "???";
    }
}

char const* packed_order_to_string(Uint32 val)
{
    switch (val) {
    case SDL_PACKEDORDER_XRGB:
        return "SDL_PACKEDORDER_XRGB";
    case SDL_PACKEDORDER_ARGB:
        return "SDL_PACKEDORDER_ARGB";
    default:
        return "???";
    }
}

char const* packed_layout_to_string(Uint32 val)
{
    switch (val) {
    case SDL_PACKEDLAYOUT_8888:
        return "SDL_PACKEDLAYOUT_8888";
    default:
        return "???";
    }
}

void print_pixel_format(Uint32 fmt)
{
    SDL_Log("format=0x%x", fmt);
    SDL_Log("pixel type=%s", pixel_type_to_string(SDL_PIXELTYPE(fmt)));
    SDL_Log("pixel order=%s (%u)", packed_order_to_string(SDL_PIXELORDER(fmt)),
        SDL_PIXELORDER(fmt));
    SDL_Log("pixel layout=%s", packed_layout_to_string(SDL_PIXELLAYOUT(fmt)));
    SDL_Log("BITS per pixel=%u", SDL_BITSPERPIXEL(fmt));
    SDL_Log("BYTES per pixel=%u", SDL_BYTESPERPIXEL(fmt));
    SDL_Log("is indexed? %x", SDL_ISPIXELFORMAT_INDEXED(fmt));
    SDL_Log("is alpha? %x", SDL_ISPIXELFORMAT_ALPHA(fmt));
    SDL_Log("is fourcc? %x", SDL_ISPIXELFORMAT_FOURCC(fmt));
}

void set_surface_pixel(SDL_Surface* surf, int x, int y, color const& c)
{
    // Assuming BGR24
    auto const index = y * surf->pitch + x * surf->format->BytesPerPixel;
    auto const pixel = static_cast<Uint8*>(surf->pixels) + index;
    pixel[2] = c.r;
    pixel[1] = c.g;
    pixel[0] = c.b;
}

/// The result of a single ray casting operation
struct collision_result {
    /// The distance to the point of collision. If < 0 then no collision
    float distance;
    /// The point of collision. If distance < 0 then this is not valid
    point2f position;
    /// The ID of the texture to use
    unsigned int texture;
    /// Texture U coordinate (V is generated on-the-fly)
    float u;
    /// Ray anfle in camera space
    float angle;
};

collision_result find_collision(level const& lvl, point2f const& origin,
    float direction, float max_distance)
{
    auto const no_result
        = collision_result{-1.f, {-1.f, -1.f}, 0u, 0.f, direction};

    auto const march_vector = vector2f{direction, max_distance};
    auto const ray_line = line2f{origin, origin + march_vector};

    std::vector<collision_result> intersections;

    for (auto const& wall : lvl.walls) {
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, wall.data, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            intersections.push_back(collision_result{
                exact_line.length(), cross_point, wall.texture, t, direction});
        }
    }

    if (intersections.empty()) {
        return no_result;
    }

    auto closest_I = intersections.cbegin();
    for (auto I = intersections.cbegin(); I < intersections.cend(); I++) {
        auto const& collision = *I;
        auto const& closest_so_far = *closest_I;
        if (collision.distance < closest_so_far.distance) {
            closest_I = I;
        }
    }

    return *closest_I;
}

} // namespace

namespace raycaster {

raycaster_app::raycaster_app(std::shared_ptr<sdl::sdl_init> sdl,
    sdl::window window, std::unique_ptr<input_buffer> input,
    std::unique_ptr<asset_store> assets, level lvl, camera cam)
: sdl_application(
      std::move(sdl), std::move(window), std::move(input), std::move(assets))
, _level{lvl}
, _camera{cam}
{
    auto framebuffer = get_framebuffer();
    auto found_format = false;
    for (auto const& want_format : desired_framebuffer_formats) {
        if (framebuffer->format->format == want_format) {
            found_format = true;
            break;
        }
    }

    if (!found_format) {
        SDL_Log("Invalid surface format!");
        SDL_Log("GOT:");
        print_pixel_format(framebuffer->format->format);
        throw std::runtime_error{
            "set_surface_pixel: invalid surface format! See log"};
    }
} // namespace raycaster

void raycaster_app::unhandled_event(SDL_Event const& event)
{
    switch (event.type) {
    case SDL_WINDOWEVENT:
        on_window_event(event.window);
        break;
    }
}

void raycaster_app::update()
{
    auto& input_buffer = get_input_buffer();

    if (input_buffer.is_quit()
        || input_buffer.is_pressed(SDL_SCANCODE_ESCAPE)) {
        quit();
        return;
    }

    auto const yaw_step = 0.05f;
    auto const move_step = 0.05f;

    if (input_buffer.is_pressed(SDL_SCANCODE_W)) {
        _camera.move({_camera.get_rotation(), move_step});
    }
    if (input_buffer.is_pressed(SDL_SCANCODE_S)) {
        _camera.move({_camera.get_rotation(), -move_step});
    }

    if (input_buffer.is_pressed(SDL_SCANCODE_A)) {
        _camera.rotate(yaw_step);
    }
    if (input_buffer.is_pressed(SDL_SCANCODE_D)) {
        _camera.rotate(-yaw_step);
    }

    if (input_buffer.is_pressed(SDL_SCANCODE_Q)) {
        _camera.move({_camera.get_rotation() + PI_OVER_2, move_step});
    }
    if (input_buffer.is_pressed(SDL_SCANCODE_E)) {
        _camera.move({_camera.get_rotation() + PI_OVER_2, -move_step});
    }

    if (input_buffer.is_hit(SDL_SCANCODE_SPACE)) {
        if (!save_screenshot(get_framebuffer(), "screenshot.bmp")) {
            SDL_Log("save_screenshot failed!");
        } else {
            SDL_Log("saved screenshot to screenshot.bmp");
        }
    }

    if (input_buffer.is_hit(SDL_SCANCODE_1)) {
        s_use_fog = !s_use_fog;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_2)) {
        s_use_bilinear = !s_use_bilinear;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_4)) {
        s_use_textures = !s_use_textures;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_5)) {
        s_draw_floor = !s_draw_floor;
    }
}

void raycaster_app::render()
{
    static auto start_time = SDL_GetTicks();
    static auto frame_number = 0;

    auto framebuffer = get_framebuffer();

    auto const fog_color = _camera.get_fog_color();
    auto const max_distance = _camera.get_far();
    auto const projection_plane = _camera.get_projection_plane();

    auto const logical_size = get_surface_size(framebuffer);
    auto const half_height = logical_size.h / 2;

    auto const num_rays = logical_size.w;
    std::vector<collision_result> collision_buffer;
    collision_buffer.reserve(num_rays);

    // Fire a ray for each column on the screen and save the result.
    for (int i = 0; i < num_rays; ++i) {
        auto const width_percent = i / static_cast<float>(num_rays);
        auto const proj_point_interp
            = linear_interpolate(projection_plane, width_percent);

        auto const diff = proj_point_interp - _camera.get_position();
        auto const camera_ray_radians = atan2(diff.y, diff.x);

        collision_buffer.push_back(find_collision(
            _level, proj_point_interp, camera_ray_radians, max_distance));

        auto& last_result = collision_buffer.back();
        if (last_result.distance < 0) {
            continue;
        }

        // Fix fish eye distortion by changing distance from euclidean to
        // projected on the projection plane using basic trig.
        auto const local_ray_radians
            = _camera.get_rotation() - camera_ray_radians;
        last_result.distance *= sin(PI_OVER_2 - std::abs(local_ray_radians));
    }

    // Draw the rays to the screen
    int column = -1;
    for (auto const& collision : collision_buffer) {
        ++column;

        auto const wall_size = collision.distance <= 0.f
            ? 0
            : static_cast<int>(half_height / collision.distance);

        auto tex
            = get_asset_store().get_asset(get_wall_texture(collision.texture));

        // Color the texture to apply the fog effect. The "fog scale factor" is
        // used to account for the draw cutoff being determined by euclidean
        // distance but the render color being determined by the projected
        // distance.
        auto const fog_scale_factor = 0.75f;
        auto const fog_distance = max_distance * fog_scale_factor;
        auto const fog_t = collision.distance / fog_distance;

        auto const wall_start = half_height - wall_size;
        auto const wall_end = half_height + wall_size;

        for (auto row = 0; row < framebuffer->h; ++row) {
            // Draw wall/ceiling
            if (row < wall_start || row >= wall_end) {
                if (!s_draw_floor) {
                    set_surface_pixel(framebuffer, column, row, fog_color);
                    continue;
                }

                auto const local_ray_radians
                    = _camera.get_rotation() - collision.angle;
                auto const floor_distance = static_cast<float>(half_height)
                    / mymath::abs(half_height - row)
                    / std::sin(PI_OVER_2 - std::abs(local_ray_radians));
                auto const floor_local_coord = point2f{0.f, 0.f}
                    + vector2f{static_cast<float>(M_PI) - collision.angle,
                          floor_distance};
                auto const floor_coord = point_cast<int>(_camera.get_position()
                    + point2f{-floor_local_coord.x, floor_local_coord.y});
                auto const is_even = (floor_coord.x + floor_coord.y) % 2 == 0;
                auto const tile_color
                    = (is_even) ? constants::white : constants::black;
                auto const foggy_color
                    = linear_interpolate(tile_color, fog_color, floor_distance / fog_distance);
                set_surface_pixel(framebuffer, column, row, foggy_color);
                continue;
            }

            auto const v = (row - wall_start)
                / static_cast<float>(wall_end - wall_start);

            auto const uv = point2f{collision.u, v};
            auto const texel = s_use_textures
                ? (s_use_bilinear ? get_surface_pixel(tex, uv)
                                  : get_surface_pixel_nn(tex, uv))
                : constants::white;

            if (!s_use_fog) {
                set_surface_pixel(framebuffer, column, row, texel);
                continue;
            }

            auto const texel_after_fog
                = linear_interpolate(texel, fog_color, fog_t);
            set_surface_pixel(framebuffer, column, row, texel_after_fog);
        }
    }

    ++frame_number;

    const auto end_time = SDL_GetTicks();
    if (end_time >= start_time + 1000) {
        start_time = end_time;
        SDL_Log("FPS: %d", frame_number);
        frame_number = 0;
    }
}

void raycaster_app::on_window_event(SDL_WindowEvent const& event)
{
    switch (event.event) {
    case SDL_WINDOWEVENT_RESIZED:
        // Used to do something, now doesn't
        break;
    }
}

} // namespace raycaster
