#include "raycaster_app.hpp"

#include "camera.hpp"
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
constexpr float PI_FLOAT = M_PI;

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
    std::unique_ptr<asset_store> assets,
    std::unique_ptr<render_pipeline> pipeline, level lvl, camera cam)
: sdl_application(
      std::move(sdl), std::move(window), std::move(input), std::move(assets))
, _pipeline{std::move(pipeline)}
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
        throw std::runtime_error{"invalid surface format! See log"};
    }

    _font_texture = get_asset_store().get_asset(common_assets::font);
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
        try_to_move_camera({_camera.get_rotation() + PI_FLOAT, move_step});
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
        auto const ray_line = line2f{
            _camera.get_position(),
            _camera.get_position()
                + vector2f{_camera.get_rotation(), _camera.get_far()},
        };
        std::vector<sprite*> barrel_hits;
        for (auto& sprite : _level.sprites) {
            auto const sprite_plane = line2f{sprite.data
                    + vector2f{_camera.get_rotation() + PI_OVER_2, 0.5f},
                sprite.data
                    + vector2f{_camera.get_rotation() - PI_OVER_2, 0.5f}};
            point2f cross_point{0.f, 0.f};
            float t = 0.f;
            if (find_intersection(ray_line, sprite_plane, cross_point, t)) {
                if (sprite.texture == 8) {
                    barrel_hits.push_back(&sprite);
                }
            }
        }

        std::sort(
            begin(barrel_hits), end(barrel_hits), [this](sprite* a, sprite* b) {
                return line2f{_camera.get_position(), a->data}.length()
                    < line2f{_camera.get_position(), b->data}.length();
            });

        for (auto barrel : barrel_hits) {
            barrel->texture = 9;
            break;
        }
    }

    if (input_buffer.is_hit(SDL_SCANCODE_TAB)) {
        _screenshot_queued = true;
    }

    if (input_buffer.is_hit(SDL_SCANCODE_1)) {
        _debug_noclip = !_debug_noclip;
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

    _pipeline->render(_level, _camera, *framebuffer);

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

    if (_debug_noclip) {
        return;
    }

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

void raycaster_app::draw_hud()
{
    auto* framebuffer = get_framebuffer();
    auto* font = _font_texture;

    auto onOrOff = [](bool b) { return b ? "ON"s : "OFF"s; };

    SDL_CHECK(draw_string(
        "FPS: "s + std::to_string(_fps), point2i{0, 0}, font, framebuffer));
    SDL_CHECK(draw_string("1: Noclip "s + onOrOff(_debug_noclip),
        point2i{0, 10}, font, framebuffer));
    // SDL_CHECK(draw_string("2: Texture "s + onOrOff(!_debug_no_textures),
    //     point2i{0, 20}, font, framebuffer));
    // SDL_CHECK(draw_string("3: Floor "s + onOrOff(!_debug_no_floor),
    //     point2i{0, 30}, font, framebuffer));
    SDL_CHECK(draw_string("4: HUD "s + onOrOff(!_debug_no_hud), point2i{0, 40},
        font, framebuffer));
    SDL_CHECK(draw_string("# threads: "s + std::to_string(detail::num_threads),
        point2i{0, 50}, font, framebuffer));
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
