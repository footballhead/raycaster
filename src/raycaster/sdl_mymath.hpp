#pragma once

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

namespace raycaster {

/// @returns {0, 0} if logical size is not set
mymath::extent2i get_renderer_logical_size(SDL_Renderer* renderer);

mymath::extent2i get_texture_size(SDL_Texture* texture);

/// @returns {-1, -1} on error
mymath::extent2i get_renderer_output_size(SDL_Renderer* renderer);

bool set_render_draw_color(SDL_Renderer* renderer, mycolor::color const& c);

} // namespace raycaster
