#pragma once

#include <SDL.h>

#include <memory>
#include <stdexcept>

#define SDL_CHECK(pred) if (!(pred)) { \
							SDL_Log("%s", SDL_GetError()); \
							throw std::runtime_error(SDL_GetError()); \
						} 

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

//
// SDL_Window_ptr
//

struct SDL_Window_deleter {
	void operator() (SDL_Window* other) const {
		SDL_DestroyWindow(other);
	}
};

using SDL_Window_ptr = std::unique_ptr<SDL_Window, SDL_Window_deleter>;

/// Wrap an existing SDL_Window.
inline auto make_SDL_Window(SDL_Window* window)
{
	SDL_CHECK(window);
	return SDL_Window_ptr{window, SDL_Window_deleter{}};
}

/// Convenience helper with parameters that make more sense.
inline auto make_SDL_Window(const std::string& title, const SDL_Point& extents)
{
	auto window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, extents.x, extents.y, 0);
	SDL_CHECK(window);
	return SDL_Window_ptr{window, SDL_Window_deleter{}};
}

//
// SDL_Renderer_ptr
//

struct SDL_Renderer_deleter {
	void operator() (SDL_Renderer* other) const {
		SDL_DestroyRenderer(other);
	}
};

using SDL_Renderer_ptr = std::unique_ptr<SDL_Renderer, SDL_Renderer_deleter>;

/// Wrap an existing SDL_Renderer
inline auto make_SDL_Renderer(SDL_Renderer* renderer)
{
	SDL_CHECK(renderer);
	return SDL_Renderer_ptr{renderer, SDL_Renderer_deleter{}};
}

/// Convenience helper
inline auto make_SDL_Renderer(SDL_Window* window)
{
	auto renderer = SDL_CreateRenderer(window, -1, 0);
	SDL_CHECK(renderer);
	return SDL_Renderer_ptr{renderer, SDL_Renderer_deleter{}};
}
