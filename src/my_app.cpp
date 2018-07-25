#include "my_app.hpp"

#include "color.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cmath>
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
		SDL_Delay(1);
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
	const auto fov = 90;

	int width = 0, height = 0;
	SDL_RenderGetLogicalSize(_renderer.get(), &width, &height);

	for (int i = 0; i < width; ++i) {
		auto ray_angle = i / static_cast<float>(width) * fov;
		auto ray_radians = ray_angle / 180.f * M_PI;

		auto const ray_color = hueToRgb(ray_radians / (M_PI / 2.f));
		set_render_draw_color(_renderer.get(), ray_color);
		SDL_CHECK(SDL_RenderDrawLine(_renderer.get(), i, 0, i, height) == 0);
	}
}
