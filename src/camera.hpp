#pragma once

#include "mymath.hpp"

namespace raycaster {

/// A view into 3D space. Has a position, rotation, and projection plane.
///
/// FOV is `atan(near / right)`, so if near == right then FOV is 45 deg.
struct camera {
    point2f pos;
    /// Camera rotation on the XY plane
    float yaw; // (radians)
    /// The distance of the projection plane from the camera.
    float near;
    /// The size of half the projection plane.
    float right;
};

} // namespace raycaster
