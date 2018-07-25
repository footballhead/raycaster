#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdlib>
#include <iostream>

int main(int argc, char** argv)
{
	sdl_app sdl;
	std::cout << "Hello world" << std::endl;
	SDL_Delay(3000);
	return EXIT_SUCCESS;
}
