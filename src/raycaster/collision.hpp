#pragma once

#include "level.hpp"

#include <mymath/mymath.hpp>

namespace raycaster {

/// The result of a ray intersecting with world geometry at a specific location.
struct ray_hit {
    /// The distance to the point of collision. Starts out in euclidean distance
    /// but, by the time you can access it, it's already in distance from the
    /// camera's projection plane. Should never be < 0.
    float distance;
    /// The point of collision in the world.
    mymath::point2f position;
    /// The ID of the texture to use when drawing the hit.
    unsigned int texture;
    /// Texture U coordinate (V is generated on-the-fly).
    float u;
};

inline bool operator<(ray_hit const& a, ray_hit const& b)
{
    return a.distance < b.distance;
}

/// A group of ray_hits for a given ray, depth-sorted from nearest to farthest.
struct render_candidates {
    /// The angle of the ray
    float angle;
    /// All intersections along the ray
    std::vector<ray_hit> hits;
};

/// @returns a depth-sorted (nearest to farthest) list of places the ray
/// intersected and associated rendering information
render_candidates find_collision(level const& lvl,
    mymath::point2f const& origin, float direction, float reference_direction,
    float max_distance);

} // namespace raycaster
