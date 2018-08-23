#include "sdl_mymath.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

using namespace mycolor;
using namespace mymath;

namespace raycaster {

extent2i get_renderer_logical_size(SDL_Renderer* renderer)
{
    int width = 0, height = 0;
    SDL_RenderGetLogicalSize(renderer, &width, &height);
    return {width, height};
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

extent2i get_renderer_output_size(SDL_Renderer* renderer)
{
    int width = -1, height = -1;
    if (SDL_GetRendererOutputSize(renderer, &width, &height) != 0) {
        SDL_Log("SDL_GetRendererOutputSize failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

bool set_render_draw_color(SDL_Renderer* renderer, color const& c)
{
    return SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255) == 0;
}

} // namespace raycaster
