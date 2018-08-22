#include "camera.hpp"

#include <cmath>

namespace raycaster {

camera::camera(point2f position, float yaw, float near, float far, float right)
: _position{std::move(position)}
, _yaw{yaw}
, _near{near}
, _far{far}
, _right{right}
, _fov{2 * std::atan(near / right)}
, _fog_color{black_color}
{
}

void camera::rotate(float yaw_delta) { _yaw += yaw_delta; }

void camera::move(vector2f const& vec) { _position = _position + vec; }

point2f const& camera::get_position() const { return _position; }
float camera::get_yaw() const { return _yaw; }
float camera::get_near() const { return _near; }
float camera::get_far() const { return _far; }
float camera::get_right() const { return _right; }
float camera::get_fov() const { return _fov; }
color const& camera::get_fog_color() const { return _fog_color; };

} // namespace raycaster
