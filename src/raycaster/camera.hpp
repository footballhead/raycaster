#pragma once

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

namespace raycaster {

/// A view into 3D space. Has a position, rotation, and projection plane.
///
/// FOV is `atan(near / right)`, so if near == right then FOV is 45 deg.
class camera {
public:
    explicit camera(mymath::point2f position, float rotation, float near,
        float far, float right);

    void rotate(float yaw_delta);

    void move(mymath::vector2f const& vec);
    void set_position(mymath::point2f const& pos);

    mymath::point2f const& get_position() const;
    float get_rotation() const;
    float get_near() const;
    float get_far() const;
    float get_right() const;
    float get_left() const;
    float get_fov() const;

    mymath::line2f get_projection_plane() const;

private:
    mymath::point2f _position;
    /// Camera rotation on the XY plane
    float _yaw; // (radians)
    /// The distance of the projection plane from the camera.
    float const _near;
    /// Objects that exceed this distance are not drawn.
    float const _far;
    /// The size of half the projection plane.
    float const _right;
    /// FOV is `atan(near / right)`
    float const _fov;
};

} // namespace raycaster
