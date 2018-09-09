#include "raycaster_app.hpp"

#include "camera.hpp"
#include "intersection.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>
#include <sdl_application/screenshot.hpp>
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
static auto s_use_dither_fog = true;
static auto s_use_textures = true;
static auto s_draw_floor = true;

using namespace raycaster;

constexpr auto PI_OVER_2 = M_PI / 2.0;

/// The result of a single ray casting operation
struct collision_result {
    /// The distance to the point of collision. If < 0 then no collision
    float distance;
    /// The point of collision. If distance < 0 then this is not valid
    point2f position;
    /// The ID of the texture to use
    unsigned int texture;
    /// Texture V coordinate
    float v;
};

collision_result find_collision(level const& lvl, point2f const& origin,
    float direction, float max_distance)
{
    auto const no_result = collision_result{-1.f, {-1.f, -1.f}, 0u, 0.f};

    auto const march_vector = vector2f{direction, max_distance};
    auto const ray_line = line2f{origin, origin + march_vector};

    std::vector<collision_result> intersections;

    for (auto const& wall : lvl.walls) {
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, wall.data, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            intersections.push_back(collision_result{
                exact_line.length(), cross_point, wall.texture, t});
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
    sdl::window window, sdl::shared_renderer renderer,
    std::unique_ptr<input_buffer> input, std::unique_ptr<asset_store> assets,
    level lvl, camera cam)
: sdl_application(
      std::move(sdl), std::move(window), std::move(renderer), std::move(input))
, _asset_store{std::move(assets)}
, _level{lvl}
, _camera{cam}
{
}

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

    if (input_buffer.is_hit(SDL_SCANCODE_SPACE)) {
        if (!save_screenshot(get_renderer(), "screenshot.bmp")) {
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
    if (input_buffer.is_hit(SDL_SCANCODE_3)) {
        s_use_dither_fog = !s_use_dither_fog;
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

    auto renderer = get_renderer();

    auto const fog_color = _camera.get_fog_color();
    auto const max_distance = _camera.get_far();
    auto const projection_plane = _camera.get_projection_plane();

    auto const logical_size = get_renderer_logical_size(renderer);
    auto const half_height = logical_size.h / 2;

    if (s_draw_floor) {
        // draw ceiling (top-down)
        color const ceiling_color{64, 0, 0};
        for (int i = 0; i < half_height; ++i) {
            auto const t_scale
                = max_distance / static_cast<float>(max_distance - 1);
            auto const interp = linear_interpolate(ceiling_color, fog_color,
                i / static_cast<float>(half_height) * t_scale);
            SDL_CHECK(draw_line(renderer, {0, i}, {logical_size.w, i},
                [&interp](point2i const&) { return interp; }));
        }

        // draw floor (bottom-up)
        color const floor_color{64, 128, 255};
        for (int i = 0; i < half_height; ++i) {
            auto const t_scale
                = max_distance / static_cast<float>(max_distance - 1);
            auto const interp = linear_interpolate(floor_color, fog_color,
                (half_height - i) / static_cast<float>(half_height) * t_scale);
            auto const draw_y = i + half_height;
            SDL_CHECK(draw_line(renderer, {0, draw_y}, {logical_size.w, draw_y},
                [&interp](point2i const&) { return interp; }));
        }
    }

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
        // No collision means don't draw anything. The fog effect will be taken
        // care of by floor/ceiling gradient.
        if (collision.distance < 0) {
            continue;
        }

        auto const wall_size
            = static_cast<int>(half_height / collision.distance);

        auto const ray_v = collision.v;

        // Figure out which texture to use
        auto tex = _asset_store->get_asset(get_wall_texture(collision.texture));

        // Color the texture to apply the fog effect. The "fog scale factor" is
        // used to account for the draw cutoff being determined by euclidean
        // distance but the render color being determined by the proejcted
        // distance.
        auto const fog_scale_factor = 0.75f;
        auto const fog_distance = max_distance * fog_scale_factor;
        auto const fog_t = collision.distance / fog_distance;

        auto const line_start = point2i{column, half_height - wall_size};
        auto const line_end = point2i{column, half_height + wall_size};
        SDL_CHECK(draw_line(renderer, line_start, line_end,
            [&line_start, &line_end, tex, &ray_v, fog_t, &fog_color](
                point2i const& draw_pos) {
                auto const y_percent = (draw_pos.y - line_start.y)
                    / static_cast<float>(line_end.y - line_start.y);

                auto const uv = point2f{ray_v, y_percent};
                auto const pixel = s_use_textures
                    ? (s_use_bilinear ? get_surface_pixel(tex, uv)
                                      : get_surface_pixel_nn(tex, uv))
                    : constants::white;

                if (!s_use_fog) {
                    return pixel;
                }

                auto t = 0.f;
                if (s_use_dither_fog) {
                    auto const dither_steps = 4;

                    auto const num_bands = 8;
                    auto const band_index = static_cast<int>(fog_t * num_bands);
                    auto const band_t
                        = static_cast<int>(
                              (fog_t * num_bands - band_index) * dither_steps)
                        + 1;

                    auto const color_grade
                        = std::floor(fog_t * num_bands) / num_bands;
                    auto const alt_color_grade
                        = std::max(0.f, color_grade - 1.f / num_bands);

                    auto const even_pixel
                        = ((draw_pos.x + draw_pos.y) % band_t) != 0;

                    t = (even_pixel ? color_grade : alt_color_grade);
                } else {
                    t = fog_t;
                }

                return linear_interpolate(pixel, fog_color, t);
            }));
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
