#include "sdl_mymath.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

#include <stdexcept>

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

color get_surface_pixel(SDL_Surface* surf, point2f const& uv)
{
    if (surf->format->BytesPerPixel != 3) {
        throw std::runtime_error{"Got BMP texture that is not 3 BPP!"};
    }

    if (surf->pitch % 3 != 0) {
        throw std::runtime_error{"Got tex where pitch is not divisible by 3"};
    }

    auto const total_pixels = surf->w * surf->h * 3;

    auto const x_tex_coord = static_cast<int>(surf->w * uv.x);
    auto const y_tex_coord = static_cast<int>(surf->h * uv.y);

    auto const index = y_tex_coord * surf->h * 3 + x_tex_coord * 3;

    if (index >= total_pixels) {
        return constants::red;
    }

    auto const pixel = static_cast<Uint8*>(surf->pixels) + index;
    return color{pixel[2], pixel[1], pixel[0]};
}

} // namespace raycaster
