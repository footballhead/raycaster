#pragma once

#include <sdl_raii/sdl_assert.hpp>

#include <SDL.h>

#include <memory>
#include <string>

namespace sdl {

namespace detail {

struct window_deleter {
    void operator()(SDL_Window* other) const { SDL_DestroyWindow(other); }
};

struct renderer_deleter {
    void operator()(SDL_Renderer* other) const { SDL_DestroyRenderer(other); }
};

struct texture_deleter {
    void operator()(SDL_Texture* other) const { SDL_DestroyTexture(other); }
};

struct surface_deleter {
    void operator()(SDL_Surface* other) const { SDL_FreeSurface(other); }
};

} // namespace detail

using window = std::unique_ptr<SDL_Window, detail::window_deleter>;
using shared_window = std::shared_ptr<SDL_Window>;

using renderer = std::unique_ptr<SDL_Renderer, detail::renderer_deleter>;
using shared_renderer = std::shared_ptr<SDL_Renderer>;

using texture = std::unique_ptr<SDL_Texture, detail::texture_deleter>;

using surface = std::unique_ptr<SDL_Surface, detail::surface_deleter>;

//
// window
//

/// Wrap an existing SDL_Window.
inline window make_window(SDL_Window* win)
{
    SDL_CHECK(win);
    return sdl::window{win, detail::window_deleter{}};
}

/// Convenience helper with parameters that make more sense.
inline window make_window(
    char const* title, SDL_Point const& extents, Uint32 flags = 0)
{
    auto win = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, extents.x, extents.y, flags);
    SDL_CHECK(win);
    return sdl::window{win, detail::window_deleter{}};
}

//
// renderer
//

/// Wrap an existing SDL_Renderer
inline renderer make_renderer(SDL_Renderer* ren)
{
    SDL_CHECK(ren);
    return renderer{ren, detail::renderer_deleter{}};
}

/// Convenience helper
inline renderer make_renderer(SDL_Window* win)
{
    auto ren = SDL_CreateRenderer(win, -1, 0);
    SDL_CHECK(ren);
    return renderer{ren, detail::renderer_deleter{}};
}

//
// texture
//

/// Wrap an existing SDL_Texture
inline texture make_texture(SDL_Texture* tex)
{
    SDL_CHECK(tex);
    return texture{tex, detail::texture_deleter{}};
}

//
// surface
//

/// Wrap an existing SDL_Surface
inline surface make_surface(SDL_Surface* surf)
{
    return surface{surf, detail::surface_deleter{}};
}

} // namespace sdl
