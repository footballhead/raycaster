#pragma once

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

namespace sdl_app {

mycolor::color get_surface_pixel(SDL_Surface* surf, mymath::point2f const& uv);

/// Use nearest neighbour filtering
mycolor::color get_surface_pixel_nn(
    SDL_Surface* surf, mymath::point2f const& uv);

} // namespace sdl_app
