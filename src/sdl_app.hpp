#pragma once

#include <SDL.h>

#include <memory>
#include <stdexcept>
#include <string>

#define SDL_CHECK(pred)                                                        \
    if (!(pred)) {                                                             \
        SDL_Log("%s - %s", __func__, SDL_GetError());                          \
        throw std::runtime_error(SDL_GetError());                              \
    }

namespace sdl {

//
// sdl_app
//

/// RAII wrapper around static SDL initialization.
class sdl_app {
public:
    /// Initialize SDL (if not done already)
    sdl_app();

    /// Shut down SDL (if no one else is using it)
    ~sdl_app();

private:
    static unsigned m_init_ref;
};

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

using renderer = std::unique_ptr<SDL_Renderer, detail::renderer_deleter>;

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
inline window make_window(const std::string& title, const SDL_Point& extents)
{
    auto win = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED, extents.x, extents.y, 0);
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
