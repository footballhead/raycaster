#pragma once

#include <SDL.h>

#include <cmath>

namespace raycaster {

template <typename T> struct vector2 {
    T dir;
    T mag;
};

using vector2f = vector2<float>;

template <typename T> struct point2 {
    T x;
    T y;

    point2<T> operator+(vector2<T> const& v)
    {
        return {x + std::cos(v.dir) * v.mag, y - std::sin(v.dir) * v.mag};
    }

    point2<T> operator-(vector2<T> const& v)
    {
        return {x - std::cos(v.dir) * v.mag, y + std::sin(v.dir) * v.mag};
    }
};

using point2f = point2<float>;

template <typename T> struct extent2 {
    T w;
    T h;
};

using extent2i = extent2<int>;

} // namespace raycaster

namespace sdl {

/// @returns {0, 0} if logical size is not set
inline raycaster::extent2i get_renderer_logical_size(SDL_Renderer* renderer)
{
    int width = 0, height = 0;
    SDL_RenderGetLogicalSize(renderer, &width, &height);
    return {width, height};
}

inline raycaster::extent2i get_texture_size(SDL_Texture* texture)
{
    int width = 0, height = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        SDL_Log("SDL_QueryTexture failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

/// @returns {-1, -1} on error
inline raycaster::extent2i get_renderer_output_size(SDL_Renderer* renderer)
{
    int width = -1, height = -1;
    if (SDL_GetRendererOutputSize(renderer, &width, &height) != 0) {
        SDL_Log("SDL_GetRendererOutputSize failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

} // namespace sdl
