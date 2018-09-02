#include "screenshot.hpp"

#include "sdl_mymath.hpp"

#include <sdl_raii/sdl_raii.hpp>

namespace raycaster {

bool save_screenshot(SDL_Renderer* renderer, const char* filename)
{
    auto renderer_size = get_renderer_output_size(renderer);
    if (renderer_size.w == -1 && renderer_size.h == -1) {
        return false;
    }

    auto const format = SDL_PIXELFORMAT_BGR888;
    auto const r_mask = 0x000000FF;
    auto const g_mask = 0x0000FF00;
    auto const b_mask = 0x00FF0000;
    auto const a_mask = 0;

    auto temp_surface = SDL_CreateRGBSurface(0, renderer_size.w,
        renderer_size.h, 32, r_mask, g_mask, b_mask, a_mask);
    if (!temp_surface) {
        SDL_Log("save_screenshot: SDL_CreateRGBSurfaceWithFormat failed: %s",
            SDL_GetError());
        return false;
    }
    auto surface = sdl::make_surface(temp_surface);

    SDL_Rect entire_screen = {0, 0, renderer_size.w, renderer_size.h};
    auto err = SDL_RenderReadPixels(
        renderer, &entire_screen, format, surface->pixels, surface->pitch);
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

} // namespace raycaster
