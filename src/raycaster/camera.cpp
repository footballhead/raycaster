#include "camera.hpp"

#include <mymath/mymath.hpp>

#include <cmath>

using namespace mycolor;
using namespace mymath;

namespace raycaster {

camera::camera(point2f position, float rotation, float near, float far, float right)
: _position{std::move(position)}
, _yaw{rotation}
, _near{near}
, _far{far}
, _right{right}
, _fov{std::atan(near / right)}
{
}

void camera::rotate(float yaw_delta) { _yaw += yaw_delta; }

void camera::move(vector2f const& vec) { _position = _position + vec; }

point2f const& camera::get_position() const { return _position; }

float camera::get_rotation() const { return _yaw; }

float camera::get_near() const { return _near; }

float camera::get_far() const { return _far; }

float camera::get_right() const { return _right; }

float camera::get_left() const { return _right; }

float camera::get_fov() const { return _fov; }

line2f camera::get_projection_plane() const
{
    auto const midpoint = _position + vector2f{_yaw, _near};
    auto const start
        = midpoint + vector2f{static_cast<float>(_yaw + M_PI / 2), get_right()};
    auto const end
        = midpoint + vector2f{static_cast<float>(_yaw - M_PI / 2), get_left()};
    return {start, end};
}

} // namespace raycaster
