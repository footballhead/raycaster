#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
	sdl_app sdl;

	const auto window_title = "Raycaster";
	const SDL_Rect window_bounds =
		{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480};
	auto window = make_SDL_Window(window_title, window_bounds, 0);

	auto renderer = make_SDL_Renderer(
		SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE));

	auto running = true;
	while (running) {
		SDL_Event evt;

		while (SDL_PollEvent(&evt)) {
			switch (evt.type) {
			case SDL_QUIT:
				running = false;
				break;
			}
		}

		if (SDL_RenderClear(renderer.get()) != 0) {
			throw std::runtime_error{SDL_GetError()};
		}
		SDL_RenderPresent(renderer.get());
	}

	return EXIT_SUCCESS;
}
