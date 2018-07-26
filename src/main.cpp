#include "my_app.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

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

	auto scaling_factor = 4;
	SDL_CHECK(SDL_RenderSetLogicalSize(renderer.get(),
		window_bounds.x/scaling_factor,
		window_bounds.y/scaling_factor) == 0);

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

	camera cam = {
		3.5f,// x
		3.5f,// y
		M_PI / 4.f, // yaw
	};

	my_app app{std::move(renderer), test_level, cam};
	try {
		app.exec();
	} catch (const std::exception& e) {
		SDL_Log("EXCEPTION: %s", e.what());
	}

	return 0;
}
