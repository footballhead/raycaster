#include "my_app.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdlib>
#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

int main(int argc, char** argv)
{
	sdl_app sdl;

	const auto window_title = "Raycaster";
	const SDL_Point window_bounds = {640, 480};
	const auto window = make_SDL_Window(window_title, window_bounds);

	auto renderer = make_SDL_Renderer(window.get());
	SDL_CHECK(SDL_RenderSetLogicalSize(renderer.get(), window_bounds.x,
		window_bounds.y) == 0);

	level test_level = {
		8,	// width
		8,	// height
		{	// data
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 0, 1, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 0, 0, 0, 1, 0, 1,
			1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 1, 1, 1, 1,
			1, 1, 0, 0, 0, 0, 0, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
		}
	};

	my_app app{std::move(renderer), test_level};
	return app.exec();
}
