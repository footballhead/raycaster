#include "screenshot.hpp"

namespace {

struct extent {
    int w;
    int h;
};

/// @returns {-1, -1} on error
extent get_renderer_output_size(SDL_Renderer* renderer)
{
    int width = -1, height = -1;
    if (SDL_GetRendererOutputSize(renderer, &width, &height) != 0) {
        SDL_Log("SDL_GetRendererOutputSize failed: %s", SDL_GetError());
        return extent{-1, -1};
    }
    return extent{width, height};
}

//
// SDL_Surface_ptr
//

struct SDL_Surface_deleter {
    void operator() (SDL_Surface* other) const {
        SDL_FreeSurface(other);
    }
};

using SDL_Surface_ptr = std::unique_ptr<SDL_Surface, SDL_Surface_deleter>;

/// Wrap an existing SDL_Surface
SDL_Surface_ptr make_SDL_Surface(SDL_Surface* surface)
{
    return SDL_Surface_ptr{surface, SDL_Surface_deleter{}};
}

} // namespace

namespace sdl {

bool save_screenshot(SDL_Renderer* renderer, const char* filename)
{
    auto renderer_size = get_renderer_output_size(renderer);
    if (renderer_size.w == -1 && renderer_size.h == -1) {
        return false;
    }

    const auto format = SDL_PIXELFORMAT_BGR888;

    auto temp_surface = SDL_CreateRGBSurfaceWithFormat(0, renderer_size.w,
        renderer_size.h, 32, format);
    if (!temp_surface) {
        SDL_Log("save_screenshot: SDL_CreateRGBSurfaceWithFormat failed: %s",
            SDL_GetError());
        return false;
    }
    auto surface = make_SDL_Surface(temp_surface);

    SDL_Rect entire_screen = {0, 0, renderer_size.w, renderer_size.h};
    auto err = SDL_RenderReadPixels(renderer, &entire_screen, format,
        surface->pixels, surface->pitch);
    if (err != 0) {
        SDL_Log("save_screenshot: SDL_RendererReadPixels failed: %s",
            SDL_GetError());
        return false;
    }

    err = SDL_SaveBMP(surface.get(), filename);
    if (err != 0) {
        SDL_Log("save_screenshot: SDL_SaveBitmap failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

} // namespace sdl
