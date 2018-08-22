#include "my_app.hpp"

#include "color.hpp"
#include "mymath.hpp"
#include "screenshot.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cmath>
#include <stdexcept>

namespace {

using namespace raycaster;

constexpr auto max_distance = 4.f;
constexpr auto step_size = 1.f / 32.f;

/// @returns The distance, >= 0 if collision (sets out_result)
float find_collision(level const& lvl, point2f const& origin, float direction,
    point2f& out_result)
{
    auto distance = step_size;
    while (distance < max_distance) {
        auto const march_x
            = static_cast<float>(origin.x + cos(direction) * distance);
        auto const march_y
            = static_cast<float>(origin.y + sin(direction) * distance);

        auto const int_x = static_cast<int>(march_x);
        auto const int_y = static_cast<int>(march_y);

        if (int_x < 0 || int_x > lvl.bounds.w - 1 || int_y < 0
            || int_y > lvl.bounds.h - 1) {
            return -1.f;
        }

        auto const index = int_y * lvl.bounds.w + int_x;
        if (lvl.data[index] > 0) {
            out_result = point2f{march_x, march_y};
            return distance;
        }

        distance += step_size;
    }

    return -1.f;
}

} // namespace

namespace raycaster {

my_app::my_app(sdl::renderer renderer, std::unique_ptr<asset_store> assets, std::unique_ptr<input_buffer> input,
    level lvl, camera cam)
: _renderer{std::move(renderer)}
, _asset_store{std::move(assets)}
, _input_buffer{std::move(input)}
, _level{lvl}
, _camera{cam}
{
}

int my_app::exec()
{
    if (_running) {
        throw std::runtime_error("Already running");
    }

    _running = true;
    while (_running) {
        _input_buffer->poll_events();
        update();

        sdl::set_render_draw_color(_renderer.get(), black_color);
        SDL_CHECK(SDL_RenderClear(_renderer.get()) == 0);
        render();
        SDL_RenderPresent(_renderer.get());

        ++_ticks;
        SDL_Delay(1);
    }

    return 0;
}

void my_app::update()
{
    if (_input_buffer->is_quit()) {
        _running = false;
        return;
    }

    auto const yaw_step = 0.01f;
    auto const move_step = 0.01f;

    if (_input_buffer->is_pressed(SDL_SCANCODE_W)) {
        _camera.pos = _camera.pos + vector2f{_camera.yaw, move_step};
    }
    if (_input_buffer->is_pressed(SDL_SCANCODE_S)) {
        _camera.pos = _camera.pos - vector2f{_camera.yaw, move_step};
    }
    if (_input_buffer->is_pressed(SDL_SCANCODE_A)) {
        _camera.yaw += yaw_step;
    }
    if (_input_buffer->is_pressed(SDL_SCANCODE_D)) {
        _camera.yaw -= yaw_step;
    }

    if (_input_buffer->is_hit(SDL_SCANCODE_SPACE)) {
        if (!save_screenshot(_renderer.get(), "screenshot.bmp")) {
            SDL_Log("save_screenshot failed!");
        }
    }
}

void my_app::render()
{
    auto const fov = 2 * atan(_camera.near / _camera.right);

    color constexpr fog_color = black_color;

    auto const logical_size = sdl::get_renderer_logical_size(_renderer.get());
    auto const half_width = logical_size.w / 2;
    auto const half_height = logical_size.h / 2;

    // draw ceiling (top-down)
    color const ceiling_color{64, 0, 0};
    for (int i = 0; i < half_height; ++i) {
        auto const t_scale
            = max_distance / static_cast<float>(max_distance - 1);
        auto const interp = linear_interpolate(ceiling_color, fog_color,
            i / static_cast<float>(half_height) * t_scale);
        sdl::set_render_draw_color(_renderer.get(), interp);
        SDL_CHECK(
            SDL_RenderDrawLine(_renderer.get(), 0, i, logical_size.w, i) == 0);
    }

    // draw floor (bottom-up)
    color const floor_color{64, 128, 255};
    for (int i = 0; i < half_height; ++i) {
        auto const t_scale
            = max_distance / static_cast<float>(max_distance - 1);
        auto const interp = linear_interpolate(floor_color, fog_color,
            (half_height - i) / static_cast<float>(half_height) * t_scale);
        sdl::set_render_draw_color(_renderer.get(), interp);

        auto const draw_y = i + half_height;
        SDL_CHECK(SDL_RenderDrawLine(
                      _renderer.get(), 0, draw_y, logical_size.w, draw_y)
            == 0);
    }

    sdl::set_render_draw_color(_renderer.get(), white_color);

    // Fire a ray for each column on the screen.
    for (int i = 0; i < logical_size.w; ++i) {
        auto const local_ray_radians
            = (i - half_width) / static_cast<float>(logical_size.w) * fov;
        auto const camera_ray_radians = local_ray_radians - _camera.yaw;

        point2f collision{0.f, 0.f};
        auto distance = find_collision(_level, {_camera.pos.x, _camera.pos.y},
            camera_ray_radians, collision);

        // No collision means don't draw anything. The fog effect will be taken
        // care of by floor/ceiling gradient.
        if (distance < 0) {
            continue;
        }

        // "Fix" fish eye distortion by changing distance from euclidean to
        // pseudo-plane projected using maths.
        distance *= sin(M_PI / 2.f - fabs(local_ray_radians));

        SDL_Point const point{
            static_cast<int>(collision.x), static_cast<int>(collision.y)};

        auto const u = fabs(collision.x - point.x);
        auto const v = fabs(collision.y - point.y);
        auto const tolerance = 0.03125f; // TODO set to step size?
        auto const ray_u = u < tolerance || u > (1.f - tolerance) ? v : u;

        SDL_Texture* tex = nullptr;
        auto const index = point.y * _level.bounds.w + point.x;
        if (_level.data[index] == 1) {
            tex = _asset_store->get_asset(common_assets::wall_texture).get();
        } else {
            tex = _asset_store->get_asset(common_assets::stone_texture).get();
        }
        auto texture_size = sdl::get_texture_size(tex);

        auto const fog_scale_factor = 0.75f;
        auto const fog_distance = max_distance * fog_scale_factor;
        auto const interp = linear_interpolate(
            white_color, fog_color, distance / fog_distance);
        SDL_CHECK(
            SDL_SetTextureColorMod(tex, interp.r, interp.g, interp.b) == 0);

        auto const wall_size = static_cast<int>(half_height / distance);

        auto const image_column = static_cast<int>(ray_u * texture_size.w);
        SDL_Rect src{image_column, 0, 1, texture_size.h};
        SDL_Rect dst{i, half_height - wall_size, 1, wall_size * 2};

        SDL_CHECK(SDL_RenderCopy(_renderer.get(), tex, &src, &dst) == 0);
    }
}

} // namespace raycaster
