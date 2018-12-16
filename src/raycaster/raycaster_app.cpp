#include "raycaster_app.hpp"

#include "camera.hpp"
#include "collision.hpp"
#include "intersection.hpp"
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

std::array<SDL_Surface*, 8> makeTextureCache(sdl_app::asset_store& assets)
{
    return {assets.get_asset(get_wall_texture(0)),
        assets.get_asset(get_wall_texture(1)),
        assets.get_asset(get_wall_texture(2)),
        assets.get_asset(get_wall_texture(3)),
        assets.get_asset(get_wall_texture(4)),
        assets.get_asset(get_wall_texture(5)),
        assets.get_asset(get_wall_texture(6)),
        assets.get_asset(get_wall_texture(7))};
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

    _texture_cache = makeTextureCache(get_asset_store());
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
        try_to_move_camera({_camera.get_rotation(), move_step});
    }
    if (input_buffer.is_pressed(SDL_SCANCODE_S)) {
        try_to_move_camera({_camera.get_rotation() + M_PI, move_step});
    }

    if (input_buffer.is_pressed(SDL_SCANCODE_A)) {
        _camera.rotate(yaw_step);
    }
    if (input_buffer.is_pressed(SDL_SCANCODE_D)) {
        _camera.rotate(-yaw_step);
    }

    if (input_buffer.is_pressed(SDL_SCANCODE_Q)) {
        try_to_move_camera({_camera.get_rotation() + PI_OVER_2, move_step});
    }
    if (input_buffer.is_pressed(SDL_SCANCODE_E)) {
        try_to_move_camera({_camera.get_rotation() - PI_OVER_2, move_step});
    }

    if (input_buffer.is_hit(SDL_SCANCODE_SPACE)) {
        _screenshot_queued = true;
    }

    if (input_buffer.is_hit(SDL_SCANCODE_2)) {
        _debug_no_textures = !_debug_no_textures;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_3)) {
        _debug_no_floor = !_debug_no_floor;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_4)) {
        _debug_no_hud = !_debug_no_hud;
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
    } else if (!_debug_no_hud) {
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

void raycaster_app::try_to_move_camera(mymath::vector2f const& vec)
{
    // Do the movement
    auto const old_pos = _camera.get_position();
    _camera.move(vec);
    auto const new_pos = _camera.get_position();

    auto const movement_line = line2f{old_pos, new_pos};

    // Then figure out if it's valid. If not, reverse it.
    for (auto const& wall : _level.walls) {
        auto t = 0.f;
        auto intersection = point2f{0.f, 0.f};
        if (find_intersection(movement_line, wall.data, intersection, t)) {
            _camera.set_position(old_pos);
            return;
        }
    }
}

void raycaster_app::rasterize(candidate_buffer const& buffer)
{
    auto* framebuffer = get_framebuffer();

    if (buffer.size() != static_cast<size_t>(framebuffer->w)) {
        SDL_Log("Not enough ray collisions!");
        throw std::runtime_error{"Not enough ray collisions!"};
    }

    int column = -1;
    for (auto const& candidates : buffer) {
        ++column;
        draw_column(column, candidates);
    }
}

void raycaster_app::draw_column(int column, render_candidates const& candidates)
{
    auto* framebuffer = get_framebuffer();

    auto& hits = candidates.hits;

    auto const floor_texture = _texture_cache[3];
    auto const ceiling_texture = _texture_cache[6];

    auto const half_height = framebuffer->h / 2;

    for (auto row = 0; row < framebuffer->h; ++row) {
        auto drew_a_hit = false;

        for (auto const hit : hits) {
            auto const wall_size = static_cast<int>(half_height / hit.distance);
            auto const wall_start = half_height - wall_size;
            auto const wall_end = half_height + wall_size;
            auto const v = (row - wall_start)
                / static_cast<float>(wall_end - wall_start);

            // Since everything is the same height, wall_size of farher away
            // objects should never be bigger than closer ones. We can safely
            // stop here.
            if (v < 0.f || v >= 1.f) {
                drew_a_hit = false;
                break;
            }

            auto const wall_tex = _texture_cache[hit.texture];

            auto const uv = point2f{hit.u, v};
            auto const texel = _debug_no_textures
                ? constants::white
                : get_surface_pixel(wall_tex, uv);

            if (texel.r == 255 && texel.g == 0 && texel.b == 255) {
                // for transparent pixels, differ to other walls to blend with
                continue;
            }

            set_surface_pixel(framebuffer, column, row, texel);
            drew_a_hit = true;

            break;
        }

        // If we didn't draw a hit (e.g. a wall) then it must be a floor or
        // ceiling
        if (!drew_a_hit) {
            if (_debug_no_floor) {
                set_surface_pixel(
                    framebuffer, column, row, mycolor::constants::black);
                continue;
            }

            // Prevent division by 0 in reverse projection
            if (half_height == row) {
                set_surface_pixel(
                    framebuffer, column, row, mycolor::constants::black);
                continue;
            }

            // Reverse project each pixel into a world coordinate
            auto const local_ray_radians
                = _camera.get_rotation() - candidates.angle;
            auto const floor_distance = static_cast<float>(half_height)
                / mymath::abs(half_height - row)
                / std::sin(PI_OVER_2 - std::abs(local_ray_radians));

            auto const floor_local_coord = point2f{0.f, 0.f}
                + vector2f{static_cast<float>(M_PI) - candidates.angle,
                      floor_distance};
            auto floor_coord = _camera.get_position()
                + point2f{-floor_local_coord.x, floor_local_coord.y};

            auto is_ceiling = row < half_height;

            // wrap the coordinate between [0,1] before querying the texture
            while (floor_coord.x <= 0.f) {
                floor_coord.x += 1.f;
            }
            while (floor_coord.x >= 1.f) {
                floor_coord.x -= 1.f;
            }
            while (floor_coord.y <= 0.f) {
                floor_coord.y += 1.f;
            }
            while (floor_coord.y >= 1.f) {
                floor_coord.y -= 1.f;
            }

            auto const tile_color = get_surface_pixel(
                is_ceiling ? ceiling_texture : floor_texture, floor_coord);

            set_surface_pixel(framebuffer, column, row, tile_color);
            continue;
        }
    }
}

void raycaster_app::draw_hud()
{
    auto* framebuffer = get_framebuffer();
    auto* font = _texture_cache[7];

    auto onOrOff = [](bool b) { return b ? "ON"s : "OFF"s; };

    SDL_CHECK(draw_string(
        "FPS: "s + std::to_string(_fps), point2i{0, 0}, font, framebuffer));
    SDL_CHECK(draw_string("2: Texture "s + onOrOff(!_debug_no_textures),
        point2i{0, 20}, font, framebuffer));
    SDL_CHECK(draw_string("3: Floor "s + onOrOff(!_debug_no_floor),
        point2i{0, 30}, font, framebuffer));
    SDL_CHECK(draw_string("4: HUD "s + onOrOff(!_debug_no_hud), point2i{0, 40},
        font, framebuffer));
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
