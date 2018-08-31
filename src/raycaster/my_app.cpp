#include "my_app.hpp"

#include "screenshot.hpp"
#include "sdl_mymath.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cmath>
#include <stdexcept>

using namespace mycolor;
using namespace mymath;
using namespace sdl_app;

namespace {

using namespace raycaster;

constexpr auto step_size = 1.f / 128.f;
constexpr auto PI_OVER_2 = M_PI / 2.0;

/// @returns The distance, >= 0 if collision (sets out_result)
float find_collision(level const& lvl, point2f const& origin, float direction,
    float max_distance, point2f& out_result)
{
    auto const unit_vector = vector2f{direction, step_size};

    // Convert vector into a point (to avoid trig)
    auto const vector_as_point = point2f{0.f, 0.f} + unit_vector;
    auto const& march = vector_as_point;
    // auto const slope = vector_as_point.y / vector_as_point.x;

    auto accum = origin + march;

    auto distance = step_size;
    while (distance < max_distance) {
        accum += march;

        auto const int_x = static_cast<int>(accum.x);
        auto const int_y = static_cast<int>(accum.y);

        if (int_x < 0 || int_x > lvl.bounds.w - 1 || int_y < 0
            || int_y > lvl.bounds.h - 1) {
            return -1.f;
        }

        auto const index = int_y * lvl.bounds.w + int_x;
        if (lvl.data[index] > 0) {
            out_result = accum;
            return distance;
        }

        distance += step_size;
    }

    return -1.f;
}

} // namespace

namespace raycaster {

my_app::my_app(std::shared_ptr<sdl::sdl_init> sdl, sdl::window window,
    sdl::shared_renderer renderer, std::unique_ptr<input_buffer> input,
    std::unique_ptr<asset_store> assets, level lvl, camera cam)
: sdl_application(
      std::move(sdl), std::move(window), std::move(renderer), std::move(input))
, _asset_store{std::move(assets)}
, _level{lvl}
, _camera{cam}
{
}

void my_app::unhandled_event(SDL_Event const& event)
{
    switch (event.type) {
    case SDL_WINDOWEVENT:
        on_window_event(event.window);
        break;
    }
}

void my_app::update()
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
}

void my_app::render()
{
    auto renderer = get_renderer();

    auto const fog_color = _camera.get_fog_color();
    auto const max_distance = _camera.get_far();
    auto const projection_plane = _camera.get_projection_plane();

    auto const logical_size = get_renderer_logical_size(renderer);
    auto const half_width = logical_size.w / 2;
    auto const half_height = logical_size.h / 2;

    // draw ceiling (top-down)
    color const ceiling_color{64, 0, 0};
    for (int i = 0; i < half_height; ++i) {
        auto const t_scale
            = max_distance / static_cast<float>(max_distance - 1);
        auto const interp = linear_interpolate(ceiling_color, fog_color,
            i / static_cast<float>(half_height) * t_scale);
        SDL_CHECK(set_render_draw_color(renderer, interp));
        SDL_CHECK(SDL_RenderDrawLine(renderer, 0, i, logical_size.w, i) == 0);
    }

    // draw floor (bottom-up)
    color const floor_color{64, 128, 255};
    for (int i = 0; i < half_height; ++i) {
        auto const t_scale
            = max_distance / static_cast<float>(max_distance - 1);
        auto const interp = linear_interpolate(floor_color, fog_color,
            (half_height - i) / static_cast<float>(half_height) * t_scale);
        SDL_CHECK(set_render_draw_color(renderer, interp));

        auto const draw_y = i + half_height;
        SDL_CHECK(
            SDL_RenderDrawLine(renderer, 0, draw_y, logical_size.w, draw_y)
            == 0);
    }

    SDL_CHECK(set_render_draw_color(renderer, white_color));

    // Fire a ray for each column on the screen.
    for (int i = 0; i < logical_size.w; ++i) {
        auto const width_percent = i / static_cast<float>(logical_size.w);
        auto const proj_point_interp
            = linear_interpolate(projection_plane, width_percent);

        auto const diff = proj_point_interp - _camera.get_position();
        auto const camera_ray_radians = atan2(diff.y, diff.x);
        auto const local_ray_radians
            = _camera.get_rotation() - camera_ray_radians;

        point2f collision{0.f, 0.f};
        auto distance = find_collision(_level, proj_point_interp,
            camera_ray_radians, max_distance, collision);

        // No collision means don't draw anything. The fog effect will be taken
        // care of by floor/ceiling gradient.
        if (distance < 0) {
            continue;
        }

        // Fix fish eye distortion by changing distance from euclidean to
        // projection plane using basic trig.
        auto const projected_distance
            = distance * sin(PI_OVER_2 - std::abs(local_ray_radians));

        SDL_Point const point{
            static_cast<int>(collision.x), static_cast<int>(collision.y)};

        auto const u = fabs(collision.x - point.x);
        auto const v = fabs(collision.y - point.y);
        auto const tolerance = step_size; // TODO set to step size?
        auto const ray_u = u < tolerance || u > (1.f - tolerance) ? v : u;

        SDL_Texture* tex = nullptr;
        auto const index = point.y * _level.bounds.w + point.x;
        if (_level.data[index] == 1) {
            tex = _asset_store->get_asset(common_assets::wall_texture).get();
        } else {
            tex = _asset_store->get_asset(common_assets::stone_texture).get();
        }
        auto texture_size = get_texture_size(tex);

        auto const fog_scale_factor = 0.75f;
        auto const fog_distance = max_distance * fog_scale_factor;
        auto const interp = linear_interpolate(
            white_color, fog_color, projected_distance / fog_distance);
        SDL_CHECK(
            SDL_SetTextureColorMod(tex, interp.r, interp.g, interp.b) == 0);

        auto const wall_size
            = static_cast<int>(half_height / projected_distance);

        auto const image_column = static_cast<int>(ray_u * texture_size.w);
        SDL_Rect src{image_column, 0, 1, texture_size.h};
        SDL_Rect dst{i, half_height - wall_size, 1, wall_size * 2};

        SDL_CHECK(SDL_RenderCopy(renderer, tex, &src, &dst) == 0);
    }
}

void my_app::on_window_event(SDL_WindowEvent const& event)
{
    switch (event.event) {
    case SDL_WINDOWEVENT_RESIZED:
        SDL_CHECK(
            SDL_RenderSetLogicalSize(get_renderer(), event.data1, event.data2)
            == 0);
        break;
    }
}

} // namespace raycaster
