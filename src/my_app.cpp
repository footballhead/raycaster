#include "my_app.hpp"

#include <SDL.h>

#include <stdexcept>

my_app::my_app(SDL_Renderer_ptr renderer)
: _renderer{std::move(renderer)}
{
}

int my_app::exec()
{
	if (_running) {
		throw std::runtime_error("Already running");
	}

	_running = true;
	while (_running) {
		update();

		if (SDL_RenderClear(_renderer.get()) != 0) {
			throw std::runtime_error{SDL_GetError()};
		}
		render();
		SDL_RenderPresent(_renderer.get());
	}

	return 0;
}

void my_app::update()
{
	SDL_Event evt;
	while (SDL_PollEvent(&evt)) {
		switch (evt.type) {
		case SDL_QUIT:
			_running = false;
			break;
		}
	}
}

void my_app::render()
{

}