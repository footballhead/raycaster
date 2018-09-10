#pragma once

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

namespace sdl_app {

/// Query an image pixel based on UV coordinates.
mycolor::color get_surface_pixel(SDL_Surface* surf, mymath::point2f const& uv);

/// Query an image pixel based on image coordinates.
mycolor::color get_surface_pixel(SDL_Surface* surf, mymath::point2i p);

} // namespace sdl_app
