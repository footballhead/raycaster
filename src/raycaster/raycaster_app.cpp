#include "raycaster_app.hpp"

#include "camera.hpp"
#include "collision.hpp"
#include "pipeline.hpp"
#include "pixel_format_debug.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>
#include <sdl_application/sdl_mymath.hpp>
#include <sdl_application/surface_manipulation.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cmath>
#include <stdexcept>
#include <string>
#include <utility>

using namespace mycolor;
using namespace mymath;
using namespace sdl_app;

using namespace std::string_literals;

namespace {

using namespace raycaster;

constexpr float PI_OVER_2 = M_PI / 2.f;

// My framebuffer set pixel operation has only been tested on the following
constexpr Uint32 desired_framebuffer_formats[] = {
    // macOS 10.12
    SDL_PIXELFORMAT_ARGB8888,
    // Xubuntu 18.04 VBox guest in Windows 10 host
    SDL_PIXELFORMAT_RGB888,
};

bool save_screenshot(SDL_Surface* framebuffer, const char* filename)
{
    // Try a straight dump (note: gives undesired results if format includes
    // alpha)
    auto const err = SDL_SaveBMP(framebuffer, filename);
    if (err != 0) {
        SDL_Log("save_screenshot: SDL_SaveBitmap failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

// This function is only confirmed to work if the surface has a pixel format
// that is in desired_framebuffer_formats.
void set_surface_pixel(SDL_Surface* surf, int x, int y, color const& c)
{
    auto const index = y * surf->pitch + x * surf->format->BytesPerPixel;
    auto const pixel = static_cast<Uint8*>(surf->pixels) + index;
    pixel[2] = c.r;
    pixel[1] = c.g;
    pixel[0] = c.b;
}

bool draw_string(
    std::string const& str, point2i pos, SDL_Surface* font, SDL_Surface* dest)
{
    auto const font_size = extent2i{6, 8};

    for (auto const& c : str) {
        if (c == '\0') {
            break;
        }

        auto src_rect = SDL_Rect{c * font_size.w, 0, font_size.w, font_size.h};
        auto dst_rect = SDL_Rect{pos.x, pos.y, font_size.w, font_size.h};
        if (SDL_BlitSurface(font, &src_rect, dest, &dst_rect) != 0) {
            return false;
        }

        pos.x += font_size.w;
    }

    return true;
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
        _screenshot_queued = true;
    }

    if (input_buffer.is_hit(SDL_SCANCODE_1)) {
        _debug_no_fog = !_debug_no_fog;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_2)) {
        _debug_no_textures = !_debug_no_textures;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_3)) {
        _debug_no_floor = !_debug_no_floor;
    }
}

void raycaster_app::render()
{
    auto* framebuffer = get_framebuffer();

    rasterize(cast_rays(framebuffer->w, _level, _camera));

    if (_screenshot_queued) {
        if (!save_screenshot(framebuffer, "screenshot.bmp")) {
            SDL_Log("save_screenshot failed!");
        } else {
            SDL_Log("saved screenshot to screenshot.bmp");
        }
        _screenshot_queued = false;
    } else {
        draw_hud();
    }

    ++_fps_interval_frames;

    const auto end_time = SDL_GetTicks();
    if (end_time >= _fps_interval_start + 1000) {
        _fps_interval_start = end_time;
        _fps = _fps_interval_frames;
        _fps_interval_frames = 0;
    }
}

void raycaster_app::rasterize(
    std::vector<std::vector<collision_result>> const& ray_collisions)
{
    auto* framebuffer = get_framebuffer();

    if (ray_collisions.size() != static_cast<size_t>(framebuffer->w)) {
        SDL_Log("Not enough ray collisions!");
        throw std::runtime_error{"Not enough ray collisions!"};
    }

    int column = -1;
    for (auto const& intersections : ray_collisions) {
        ++column;
        // TODO intersections may be empty in the future!
        auto const& closest_intersection = intersections.at(0);
        draw_column(column, closest_intersection);
    }
}

void raycaster_app::draw_column(int column, collision_result const& collision)
{
    auto* framebuffer = get_framebuffer();
    auto& asset_store = get_asset_store();

    auto const fog_color = _camera.get_fog_color();

    auto const wall_tex
        = asset_store.get_asset(get_wall_texture(collision.texture));
    auto const floor_texture = asset_store.get_asset(common_assets::floor);
    auto const floor_texture2 = asset_store.get_asset(common_assets::floor2);
    auto const ceiling_texture = asset_store.get_asset(common_assets::ceiling);

    auto const half_height = framebuffer->h / 2;
    auto const wall_size = collision.distance <= 0.f
        ? 0
        : static_cast<int>(half_height / collision.distance);
    auto const wall_start = half_height - wall_size;
    auto const wall_end = half_height + wall_size;

    // Color the texture to apply the fog effect. The "fog scale factor" is
    // used to account for the draw cutoff being determined by euclidean
    // distance but the render color being determined by the projected
    // distance.
    auto const fog_scale_factor = 0.75f;
    auto const fog_distance = _camera.get_far() * fog_scale_factor;
    auto const fog_t = collision.distance / fog_distance;

    for (auto row = 0; row < framebuffer->h; ++row) {
        // Draw wall/ceiling
        if (row < wall_start || row >= wall_end) {
            if (_debug_no_floor) {
                set_surface_pixel(framebuffer, column, row, fog_color);
                continue;
            }

            // Reverse project each pixel into a world coordinate
            auto const local_ray_radians
                = _camera.get_rotation() - collision.angle;
            auto const floor_distance = static_cast<float>(half_height)
                / mymath::abs(half_height - row)
                / std::sin(PI_OVER_2 - std::abs(local_ray_radians));
            auto const floor_local_coord = point2f{0.f, 0.f}
                + vector2f{static_cast<float>(M_PI) - collision.angle,
                      floor_distance};
            auto floor_coord = _camera.get_position()
                + point2f{-floor_local_coord.x, floor_local_coord.y};

            auto const floor_fog_t = floor_distance / fog_distance;

            if (floor_fog_t >= 1.f) {
                set_surface_pixel(framebuffer, column, row, fog_color);
                continue;
            }

            auto is_ceiling = row < half_height;

            auto const floored_coord = point_cast<int>(floor_coord);
            auto const is_even = ((floored_coord.x + floored_coord.y) % 2) == 0;

            // wrap the coordinate between [0,1] before querying the texture
            while (floor_coord.x < 0.f) {
                floor_coord.x += 1.f;
            }
            while (floor_coord.x > 1.f) {
                floor_coord.x -= 1.f;
            }
            while (floor_coord.y < 0.f) {
                floor_coord.y += 1.f;
            }
            while (floor_coord.y > 1.f) {
                floor_coord.y -= 1.f;
            }

            auto const tile_color = get_surface_pixel(is_ceiling
                    ? ceiling_texture
                    : (is_even ? floor_texture : floor_texture2),
                floor_coord);
            auto const foggy_tile_color = _debug_no_fog
                ? tile_color
                : linear_interpolate(tile_color, fog_color, floor_fog_t);

            set_surface_pixel(framebuffer, column, row, foggy_tile_color);
            continue;
        }

        auto const v
            = (row - wall_start) / static_cast<float>(wall_end - wall_start);

        auto const uv = point2f{collision.u, v};
        auto const texel = _debug_no_textures ? constants::white
                                              : get_surface_pixel(wall_tex, uv);

        auto const texel_after_fog = _debug_no_fog
            ? texel
            : linear_interpolate(texel, fog_color, fog_t);
        set_surface_pixel(framebuffer, column, row, texel_after_fog);
    }
}

void raycaster_app::draw_hud()
{
    auto* framebuffer = get_framebuffer();

    auto& asset_store = get_asset_store();
    auto* font = asset_store.get_asset(common_assets::font);

    SDL_CHECK(draw_string(
        "FPS: "s + std::to_string(_fps), point2i{0, 0}, font, framebuffer));
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
