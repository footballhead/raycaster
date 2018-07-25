#pragma once

#include <SDL.h>

#include <memory>
#include <stdexcept>

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

inline auto make_SDL_Window(SDL_Window* window)
{
	if (!window) {
		throw std::runtime_error{SDL_GetError()};
	}
	return SDL_Window_ptr{window, SDL_Window_deleter{}};
}
