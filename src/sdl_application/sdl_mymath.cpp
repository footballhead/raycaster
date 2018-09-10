#include "sdl_mymath.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

#include <stdexcept>

using namespace mycolor;
using namespace mymath;

namespace sdl_app {

extent2i get_renderer_logical_size(SDL_Renderer* renderer)
{
    int width = 0, height = 0;
    SDL_RenderGetLogicalSize(renderer, &width, &height);
    return {width, height};
}

bool set_renderer_logical_size(SDL_Renderer* renderer, extent2i const& size)
{
    return SDL_RenderSetLogicalSize(renderer, size.w, size.h) == 0;
}

extent2i get_texture_size(SDL_Texture* texture)
{
    int width = 0, height = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        SDL_Log("SDL_QueryTexture failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

extent2i get_surface_size(SDL_Surface* surface)
{
    return {surface->w, surface->h};
}

extent2i get_renderer_output_size(SDL_Renderer* renderer)
{
    int width = -1, height = -1;
    if (SDL_GetRendererOutputSize(renderer, &width, &height) != 0) {
        SDL_Log("SDL_GetRendererOutputSize failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

bool set_renderer_draw_color(SDL_Renderer* renderer, color const& c)
{
    return SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255) == 0;
}

} // namespace sdl_app
