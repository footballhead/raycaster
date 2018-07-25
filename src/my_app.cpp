#include "my_app.hpp"

#include "color.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cmath>
#include <stdexcept>

my_app::my_app(SDL_Renderer_ptr renderer, level lvl, camera cam)
: _renderer{std::move(renderer)}
, _level{lvl}
, _camera{cam}
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

		set_render_draw_color(_renderer.get(), {0, 0, 0});
		SDL_CHECK(SDL_RenderClear(_renderer.get()) == 0);
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
		case SDL_KEYDOWN:
			keydown(evt.key.keysym.sym);
			break;
		}
	}
}

void my_app::render()
{
	const auto fov = M_PI / 2.f;
	const auto max_distance = 8;

	const auto step_size = 0.05f;

	int width = 0, height = 0;
	SDL_RenderGetLogicalSize(_renderer.get(), &width, &height);

	auto half_width = width/2;
	auto half_height = height/2;

	for (int i = 0; i < width; ++i) {
		auto local_radians = (i-half_width) / static_cast<float>(width) * fov;
		auto camera_radians = local_radians - _camera.yaw;
		auto ray_radians = camera_radians;

		auto step = step_size;
		while (step_size < max_distance) {
			SDL_Point point{
				_camera.x + cos(ray_radians)*step,
				_camera.y + sin(ray_radians)*step
			};

			int index = point.y * _level.width + point.x;
			if (_level.data[index] == 1) {
				break;
			}

			step += step_size;
		}

		color ray_color{0, 0, 0};
		ray_color.r = ray_color.g = ray_color.b =
					static_cast<uint8_t>(255 - (256 / max_distance) * step);
		auto wall_size = static_cast<int>(half_height - step*step*step);

		set_render_draw_color(_renderer.get(), ray_color);
		SDL_CHECK(SDL_RenderDrawLine(_renderer.get(), i, half_height - wall_size,
			i, half_height + wall_size) == 0);
	}
}

void my_app::keydown(SDL_Keycode key)
{
	const auto yaw_step = 0.01f;
	switch (key) {
	case SDLK_a:
		_camera.yaw += yaw_step;
		break;
	case SDLK_d:
		_camera.yaw -= yaw_step;
		break;
	}
}
