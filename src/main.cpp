#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv)
{
	sdl_app sdl;

	const auto window_title = "Raycaster";
	const SDL_Rect window_bounds =
		{SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 640, 480};
	auto window = make_SDL_Window(window_title, window_bounds, 0);

	auto renderer = make_SDL_Renderer(
		SDL_CreateRenderer(window.get(), -1, SDL_RENDERER_SOFTWARE));

	SDL_RenderClear(renderer.get());
	SDL_RenderPresent(renderer.get());

	std::cout << "Hello world" << std::endl;
	SDL_Delay(3000);
	return EXIT_SUCCESS;
}
