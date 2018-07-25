#include "my_app.hpp"

#include "sdl_app.hpp"

#include <SDL.h>

#include <stdexcept>

namespace {

struct color {
	uint8_t r;
	uint8_t g;
	uint8_t b;
};

color hueToRgb(float hue) {
	auto sixth = 1.f/6.f;

	while (hue > 1.f) {
		hue -= 1.f;
	}

	if (hue < sixth) {
		return {
			255,
			static_cast<uint8_t>(hue / sixth * 255),
			0
		};
	} else if (hue < 2*sixth) {
		return {
			static_cast<uint8_t>(255 - ((hue-sixth) / sixth) * 255),
			255,
			0
		};
	} else if (hue < 3*sixth) {
		return {
			0,
			255,
			static_cast<uint8_t>((hue-2*sixth) / sixth * 255),
		};
	} else if (hue < 4*sixth) {
		return {
			0,
			static_cast<uint8_t>(255 - ((hue-3*sixth) / sixth) * 255),
			255
		};
	} else if (hue < 5*sixth) {
		return {
			static_cast<uint8_t>((hue-4*sixth) / sixth * 255),
			0,
			255
		};
	} else {
		return {
			255,
			0,
			static_cast<uint8_t>(255 - ((hue-5*sixth) / sixth) * 255),
		};
	}
}

} // namespace <anonymous>

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