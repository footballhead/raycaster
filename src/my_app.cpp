#include "my_app.hpp"

#include "color.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <stdexcept>

my_app::my_app(SDL_Renderer_ptr renderer, level lvl)
: _renderer{std::move(renderer)}
, _level{lvl}
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

		render();
		SDL_RenderPresent(_renderer.get());

		++_ticks;
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
	auto col = hueToRgb(static_cast<float>(_ticks) / 100000.f);
	SDL_CHECK(
		SDL_SetRenderDrawColor(_renderer.get(), col.r, col.g, col.b, 255) == 0);
	SDL_CHECK(SDL_RenderClear(_renderer.get()) == 0);
}