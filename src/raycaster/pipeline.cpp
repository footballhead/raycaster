#include "pipeline.hpp"

#include "camera.hpp"
#include "intersection.hpp"
#include "level.hpp"

#include <mycolor/mycolor.hpp>
#include <sdl_application/surface_manipulation.hpp>

#include <SDL.h>

using namespace mymath;
using namespace sdl_app;

namespace {

constexpr float PI_OVER_2 = M_PI / 2.f;

using namespace mycolor;

// This function is only confirmed to work if the surface has a pixel format
// that is in desired_framebuffer_formats.
void set_surface_pixel(SDL_Surface& surf, int x, int y, color const& c)
{
    auto const index = y * surf.pitch + x * surf.format->BytesPerPixel;
    auto const pixel = static_cast<Uint8*>(surf.pixels) + index;
    pixel[2] = c.r;
    pixel[1] = c.g;
    pixel[0] = c.b;
}

} // namespace

namespace raycaster {

render_pipeline::render_pipeline(texture_cache cache)
: _texture_cache{std::move(cache)}
{
}

void render_pipeline::render(
    level const& lvl, camera const& cam, SDL_Surface& framebuffer)
{
    rasterize(cast_rays(framebuffer.w, lvl, cam), framebuffer, cam);
}

// First "stage" of the pipeline:
auto render_pipeline::cast_rays(
    int num_rays, level const& lvl, camera const& cam) -> candidate_buffer
{
    candidate_buffer buffer;
    buffer.reserve(num_rays);

    // Fire a ray for each column on the screen and save the result.
    for (int i = 0; i < num_rays; ++i) {
        auto const width_percent = i / static_cast<float>(num_rays);
        auto const proj_point_interp
            = linear_interpolate(cam.get_projection_plane(), width_percent);

        auto const diff = proj_point_interp - cam.get_position();
        auto const camera_ray_radians = atan2(diff.y, diff.x);

        buffer.emplace_back(find_collision(lvl, proj_point_interp,
            camera_ray_radians, cam.get_rotation(), cam.get_far()));

        // Fix fish eye distortion by changing distance from euclidean to
        // projected on the projection plane using basic trig.
        auto& candidates = buffer.back();
        for (auto& hit : candidates.hits) {
            auto const local_ray_radians
                = cam.get_rotation() - camera_ray_radians;
            hit.distance *= sin(PI_OVER_2 - std::abs(local_ray_radians));

            if (hit.distance < 0) {
                SDL_Log("Invalid ray hit distance!");
                throw std::runtime_error{"Invalid ray hit distance!"};
            }
        }
    }

    return buffer;
}

auto render_pipeline::find_collision(level const& lvl, point2f const& origin,
    float direction, float reference_direction, float max_distance)
    -> render_candidates
{
    auto const march_vector = vector2f{direction, max_distance};
    auto const ray_line = line2f{origin, origin + march_vector};

    render_candidates candidates{direction, {}};
    auto& hits = candidates.hits;

    for (auto const& wall : lvl.walls) {
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, wall.data, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            // HACK! For walls, we want the texture to repeat across the length,
            // but the `t` we get normalizes across the line. So correct for
            // that here.
            t *= wall.data.length();
            t -= std::floor(t);
            hits.push_back(
                ray_hit{exact_line.length(), cross_point, wall.texture, t});
        }
    }

    for (auto const& sprite : lvl.sprites) {
        auto const sprite_plane = line2f{
            sprite.data + vector2f{reference_direction + PI_OVER_2, 0.5f},
            sprite.data + vector2f{reference_direction - PI_OVER_2, 0.5f}};
        point2f cross_point{0.f, 0.f};
        float t = 0.f;
        if (find_intersection(ray_line, sprite_plane, cross_point, t)) {
            auto const exact_line = line2f{origin, cross_point};
            hits.push_back(
                ray_hit{exact_line.length(), cross_point, sprite.texture, t});
        }
    }

    std::sort(hits.begin(), hits.end());
    return candidates;
}

void render_pipeline::rasterize(
    candidate_buffer const& buffer, SDL_Surface& framebuffer, camera const& cam)
{
    int column = -1;
    for (auto const& candidates : buffer) {
        ++column;
        draw_column(column, candidates, framebuffer, cam);
    }
}

void render_pipeline::draw_column(int column,
    render_candidates const& candidates, SDL_Surface& framebuffer,
    camera const& cam)
{
    auto& hits = candidates.hits;

    auto const floor_texture = _texture_cache[3];
    auto const ceiling_texture = _texture_cache[6];

    auto const half_height = framebuffer.h / 2;

    for (auto row = 0; row < framebuffer.h; ++row) {
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
                = cam.get_rotation() - candidates.angle;
            auto const floor_distance = static_cast<float>(half_height)
                / mymath::abs(half_height - row)
                / std::sin(PI_OVER_2 - std::abs(local_ray_radians));

            auto const floor_local_coord = point2f{0.f, 0.f}
                + vector2f{static_cast<float>(M_PI) - candidates.angle,
                      floor_distance};
            auto floor_coord = cam.get_position()
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

} // namespace raycaster
