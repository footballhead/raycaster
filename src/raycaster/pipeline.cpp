#include "pipeline.hpp"

#include "camera.hpp"
#include "intersection.hpp"
#include "level.hpp"

#include <mycolor/mycolor.hpp>
#include <sdl_application/surface_manipulation.hpp>

#include <SDL.h>

#include <chrono>

using namespace mymath;
using namespace sdl_app;
using namespace std::chrono_literals;

namespace {

constexpr float F_PI = static_cast<float>(M_PI);
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
    for (int i = 0; i < detail::num_threads; ++i) {
        auto& context = _contexts[i];
        auto& thread = _threads[i];

        context.id = i;
        thread = std::thread(
            [this](rendering_context* context) {
                while (true) {
                    // Spin, because I'm too lazy to figure out a CV solution
                    while (!context->can_start.exchange(false)) {
                        std::this_thread::sleep_for(1ms);
                    }

                    context->work(context->id);

                    context->done.store(true);
                }
            },
            &context);
        thread.detach();
    }
}

void render_pipeline::render(
    level const& lvl, camera const& cam, SDL_Surface& framebuffer)
{
    for (auto& context : _contexts) {
        // tell what the thread should do
        context.work = [&lvl, &cam, &framebuffer, this](
                           unsigned id) { do_work(id, lvl, cam, framebuffer); };

        // start thread
        context.can_start.store(true);
    }

    // wait for results before returning
    for (auto& context : _contexts) {
        // Spin, because I'm too lazy to figure out a CV solution
        while (!context.done.exchange(false)) {
            std::this_thread::sleep_for(1ms);
        }
    }
}

void render_pipeline::do_work(
    unsigned thread_id, level const& lvl, camera const& cam, SDL_Surface& fb)
{
    // Some helper vars.
    auto const half_height = fb.h / 2;

    // We can nicely partition the rectangular framebuffer into sets of
    // contiguous, non-overlapping columns; this thread will only be responsible
    // for rendering one of those sets (here called a workset)
    int start_column = thread_id * fb.w / detail::num_threads;
    int end_column = (thread_id + 1) * fb.w / detail::num_threads;
    for (auto column = start_column; column < end_column; ++column) {
        // This loop can be split roughly in two:
        //
        // 1. Figure out which things to draw
        // 2. Draw them
        //
        // It can be split like this because each step has different
        // dependencies.

        //
        // STEP 1: Figure out which things to draw
        //

        // We shoot rays through a projection plane, so find the plane point
        // corresponding to the screen column. (This is a transformation from
        // one space to another [framebuffer space to world space])
        auto const proj_point_ws = linear_interpolate(
            cam.get_projection_plane(), column / static_cast<float>(fb.w));

        // Determine the world space angle that this represents. The angle does
        // not increase linearly across the plane, hence why the trig is
        // necessary (although a sufficient approximation might speed this up)
        auto const diff = proj_point_ws - cam.get_position();
        auto const ray_radians_ws = atan2(diff.y, diff.x);

        // Now that we have a point and an angle, we can define a line to
        // represent the ray in worldspace.
        auto const ray_vector_ws = vector2f{ray_radians_ws, cam.get_far()};
        auto const ray_line_ws
            = line2f{proj_point_ws, proj_point_ws + ray_vector_ws};

        // Now that we have a ray, we can start testing it against level
        // geometry to find hits (which we will later render). We can't render
        // right now because we want to do depth-sorting and translucency
        // effects.
        struct ray_hit {
            float distance;
            mymath::point2f position;
            unsigned int texture;
            float u;

            // Used to depth-sort by comparing distances
            bool operator<(ray_hit const& other) const
            {
                return this->distance < other.distance;
            }
        };
        std::vector<ray_hit> candidates;

        for (auto const& wall : lvl.walls) {
            // Walls are lines in worldspace, and the ray is a line in
            // worldspace, so finding the candidate is as easy as finding the
            // algrebraic intersection between them
            point2f cross_point{0.f, 0.f};
            float t = 0.f;
            if (find_intersection(ray_line_ws, wall.data, cross_point, t)) {
                auto const exact_line = line2f{proj_point_ws, cross_point};
                // HACK! For walls, we want the texture to repeat across the
                // length, but the `t` we get normalizes across the line and
                // causes the texture to stretch. So correct for that here.
                t *= wall.data.length();
                t -= std::floor(t);
                candidates.push_back(
                    ray_hit{exact_line.length(), cross_point, wall.texture, t});
            }
        }

        for (auto const& sprite : lvl.sprites) {
            // Sprites, unlike lines, rotate to face the camera. As such, they
            // are modeled as points that we turn into lines in order to work
            // with. Sprites always take up 1 unit, which means 0.5 on either
            // side.
            auto const sprite_plane = line2f{
                sprite.data + vector2f{cam.get_rotation() + PI_OVER_2, 0.5f},
                sprite.data + vector2f{cam.get_rotation() - PI_OVER_2, 0.5f}};
            point2f cross_point{0.f, 0.f};
            float t = 0.f;
            if (find_intersection(ray_line_ws, sprite_plane, cross_point, t)) {
                auto const exact_line = line2f{proj_point_ws, cross_point};
                candidates.push_back(ray_hit{
                    exact_line.length(), cross_point, sprite.texture, t});
            }
        }

        // TODO: It would be nice if both for loops above could be combined into
        // one...

        // Depth-sort the hits. This way, we can traverse the vector from front
        // to back to make rendering simpler
        std::sort(candidates.begin(), candidates.end());

        // Fix fish eye distortion by changing distance from euclidean to
        // projected-on-the-projection-plane (using basic trig). Now we're
        // operating in viewspace.
        //
        // TODO: Might be better to do this later to prevent error accumulation.
        // This is a problem when rendering floors/ceilings...
        auto const ray_radians_vs = cam.get_rotation() - ray_radians_ws;
        for (auto& hit : candidates) {
            hit.distance *= sin(PI_OVER_2 - std::abs(ray_radians_vs));
        }

        //
        // STEP 2: Now draw them
        //

        // Now that we know what to render and in which order, start placing
        // pixels down a row! Worksets are rectangles with height of the
        // framebuffer, only the width is partitioned.
        for (auto row = 0; row < fb.h; ++row) {
            // Track whether something was drawn, if not then the floor/ceiling
            // should be rendered to fill in the space.
            bool drew_a_hit = false;

            for (auto const& hit : candidates) {
                // Sanity check: never try to render something with bad
                // distance
                if (hit.distance <= 0) {
                    SDL_Log("Invalid ray hit distance!");
                    throw std::runtime_error{"Invalid ray hit distance!"};
                }

                // Compute how much screen real estate the hit will take up. The
                // nice thing about raycasters is that everthing is the same
                // height in worldspace so this step is easy.
                auto const wall_size
                    = static_cast<int>(half_height / hit.distance);
                auto const wall_start = half_height - wall_size;
                auto const wall_end = half_height + wall_size;

                // Since everything is the same height, wall_size of farher away
                // objects should never be bigger than closer ones. We can
                // safely stop here.
                if (row < wall_start || row >= wall_end) {
                    break;
                }

                // Compute the vertical texture coordinate based on size.
                auto const v = (row - wall_start)
                    / static_cast<float>(wall_end - wall_start);
                auto const uv = point2f{hit.u, v};

                // Get the color of the pixel based on the wall texture.
                auto const texel
                    = get_surface_pixel(_texture_cache[hit.texture], uv);

                // If the pixel is transparent then don't render this hit. Keep
                // iterating through farther back hits to find a non transparent
                // pixel.
                //
                // HACK! Magenta is hardcoded as the translucent pixel.
                if (texel.r == 255 && texel.g == 0 && texel.b == 255) {
                    continue;
                }

                // Otherwise, place the pixel onto the framebuffer and move onto
                // the next row.
                set_surface_pixel(fb, column, row, texel);
                drew_a_hit = true;
                break;
            }

            // Since a pixel was placed, move onto the next row
            if (drew_a_hit) {
                continue;
            }

            // Otherwise: draw a floor/ceiling in its place.

            // Prevent division by 0 in reverse projection
            if (half_height == row) {
                set_surface_pixel(fb, column, row, mycolor::constants::black);
                continue;
            }

            // We want to transform each pixel from framebuffer space into world
            // space so that the resulting pixel is chosen with the right
            // perspective. Start by finding the distance in view space,
            // then use that to construct a point in world space.
            auto const floor_distance_vs = static_cast<float>(half_height)
                / mymath::abs(half_height - row)
                / std::sin(PI_OVER_2 - std::abs(ray_radians_vs));
            auto const floor_coord_local_ws = point2f{0.f, 0.f}
                + vector2f{static_cast<float>(M_PI) - ray_radians_ws,
                      floor_distance_vs};
            // TODO: Why are the signs weird here...
            auto floor_coord_ws = cam.get_position()
                + point2f{-floor_coord_local_ws.x, floor_coord_local_ws.y};

            // Figure out if we're rendering the floor or ceiling.
            auto is_ceiling = row < half_height;

            // Wrap the coordinate between [0,1] before querying the texture
            while (floor_coord_ws.x <= 0.f) {
                floor_coord_ws.x += 1.f;
            }
            while (floor_coord_ws.x >= 1.f) {
                floor_coord_ws.x -= 1.f;
            }
            while (floor_coord_ws.y <= 0.f) {
                floor_coord_ws.y += 1.f;
            }
            while (floor_coord_ws.y >= 1.f) {
                floor_coord_ws.y -= 1.f;
            }

            // Query the texture then place the pixel.
            auto const tile_color = get_surface_pixel(
                is_ceiling ? _texture_cache[6] : _texture_cache[3],
                floor_coord_ws);
            set_surface_pixel(fb, column, row, tile_color);
        }
    }
}

} // namespace raycaster
