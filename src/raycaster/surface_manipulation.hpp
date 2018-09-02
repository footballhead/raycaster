#pragma once

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

namespace raycaster {

mycolor::color get_surface_pixel(SDL_Surface* surf, mymath::point2f const& uv);

} // namespace raycaster

