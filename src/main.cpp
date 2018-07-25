#include "my_app.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdlib>
#include <iostream>
#include <stdexcept>

int main(int argc, char** argv)
{
	sdl_app sdl;

	const auto window_title = "Raycaster";
	const SDL_Point window_bounds = {640, 480};
	const auto window = make_SDL_Window(window_title, window_bounds);

	auto renderer = make_SDL_Renderer(window.get());
	my_app app{std::move(renderer)};
	return app.exec();
}
