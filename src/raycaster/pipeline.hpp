#pragma once

#include "texture_cache.hpp"

#include <mymath/mymath.hpp>

#include <atomic>
#include <functional>
#include <thread>
#include <vector>

struct SDL_Surface;

namespace raycaster {

class camera;
struct level;

namespace detail {
constexpr auto num_threads = 6;
} // namespace detail

class render_pipeline {
public:
    render_pipeline(texture_cache cache);

    void render(level const& lvl, camera const& cam, SDL_Surface& framebuffer);

private:
    /// The result of a ray intersecting with world geometry at a specific
    /// location.
    struct ray_hit {
        /// The distance to the point of collision. Starts out in euclidean
        /// distance but, by the time you can access it, it's already in
        /// distance from the camera's projection plane. Should never be < 0.
        float distance;
        /// The point of collision in the world.
        mymath::point2f position;
        /// The ID of the texture to use when drawing the hit.
        unsigned int texture;
        /// Texture U coordinate (V is generated on-the-fly).
        float u;

        bool operator<(ray_hit const& other) const
        {
            return this->distance < other.distance;
        }
    };

    /// A group of ray_hits for a given ray, depth-sorted from nearest to
    /// farthest.
    struct render_candidates {
        /// The angle of the ray
        float angle;
        /// All intersections along the ray
        std::vector<ray_hit> hits;
    };

    /// All the hits for all rays which, together, gives enough information to
    /// fill the framebuffer with pixels.
    using candidate_buffer = std::vector<render_candidates>;

    /// Stage 1: cast a bunch of rays and find their intersections
    candidate_buffer cast_rays(int image_width, int start_ray_id,
        int end_ray_id, level const& lvl, camera const& cam);

    /// Stage 1 helper function
    render_candidates find_collision(level const& lvl,
        mymath::point2f const& origin, float direction,
        float reference_direction, float max_distance);

    /// Stage 2: rasterize
    void rasterize(candidate_buffer const& buffer, int start_column,
        SDL_Surface& framebuffer, camera const& cam);

    // Stage 2 helper function
    void draw_column(int column, render_candidates const& candidates,
        SDL_Surface& framebuffer, camera const& cam);

    texture_cache _texture_cache;

    using rendering_fn = std::function<void(unsigned)>;

    struct rendering_context {
        unsigned id;
        std::atomic<bool> can_start{false};
        std::atomic<bool> done{false};
        std::function<void(unsigned)> work;
    };

    std::array<rendering_context, detail::num_threads> _contexts;
    std::array<std::thread, detail::num_threads> _threads;

    bool _debug_no_textures = false;
    bool _debug_no_floor = false;
};

} // namespace raycaster
