#include "raycaster_app.hpp"

#include "camera.hpp"

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
static auto s_use_bilinear = true;
static auto s_use_dither_fog = false;

using namespace raycaster;

constexpr auto step_size = 1.f / 64.f;
constexpr auto PI_OVER_2 = M_PI / 2.0;

SDL_Surface* get_wall_texture(asset_store& assets, unsigned int i)
{
    static const std::vector<std::string> texture_table{
        common_assets::wall_texture, common_assets::wall_texture,
        common_assets::stone_texture};
    if (i >= texture_table.size()) {
        throw std::runtime_error{"Invalid wall-texutre index"};
    }
    return assets.get_asset(texture_table.at(i));
}

/// The result of a single ray casting operation
struct collision_result {
    /// The distance to the point of collision. If < 0 then no collision
    float distance;
    /// The point of collision. If distance < 0 then this is not valid
    point2f position;
};

collision_result find_collision(level const& lvl, point2f const& origin,
    float direction, float max_distance)
{
    auto const no_result = collision_result{-1.f, {-1.f, -1.f}};

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
            return no_result;
        }

        auto const index = int_y * lvl.bounds.w + int_x;
        if (lvl.data[index] > 0) {
            return collision_result{distance, accum};
        }

        distance += step_size;
    }

    return no_result;
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
}

void raycaster_app::render()
{
    auto renderer = get_renderer();

    auto const fog_color = _camera.get_fog_color();
    auto const max_distance = _camera.get_far();
    auto const projection_plane = _camera.get_projection_plane();

    auto const logical_size = get_renderer_logical_size(renderer);
    auto const half_height = logical_size.h / 2;

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

        auto const rounded_collision = point_cast<int>(collision.position);

        // Determine which part of the texture to use by looking at the
        // remainder then trying to figure out which axis is more accurate
        auto const v_x = std::abs(collision.position.x - rounded_collision.x);
        auto const v_y = std::abs(collision.position.y - rounded_collision.y);
        auto const tolerance = step_size;
        auto const ray_v
            = v_x < tolerance || v_x > (1.f - tolerance) ? v_y : v_x;

        // Figure out which texture to use
        auto const index
            = rounded_collision.y * _level.bounds.w + rounded_collision.x;
        auto tex = get_wall_texture(*_asset_store, _level.data[index]);

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
                auto const pixel = s_use_bilinear
                    ? get_surface_pixel(tex, uv)
                    : get_surface_pixel_nn(tex, uv);

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
