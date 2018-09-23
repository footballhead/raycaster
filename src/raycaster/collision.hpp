#pragma once

#include "level.hpp"

#include <mymath/mymath.hpp>

namespace raycaster {

/// The result of a single ray casting operation
struct collision_result {
    /// The distance to the point of collision. If < 0 then no collision
    float distance;
    /// The point of collision. If distance < 0 then this is not valid
    mymath::point2f position;
    /// The ID of the texture to use
    unsigned int texture;
    /// Texture U coordinate (V is generated on-the-fly)
    float u;
    /// Ray anfle in camera space
    float angle;
};

inline bool operator<(collision_result const& a, collision_result const& b)
{
    return a.distance < b.distance;
}

/// @returns a depth-sorted (nearest to farthest) list of places the ray
/// intersected and what it intersected with
std::vector<collision_result> find_collision(level const& lvl,
    mymath::point2f const& origin, float direction, float reference_direction,
    float max_distance);

} // namespace raycaster
