#include "my_app.hpp"

#include "color.hpp"
#include "screenshot.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cmath>
#include <stdexcept>

namespace {

struct extent {
	int width;
	int height;
};

/// @returns {0, 0} if logical size is not set
extent get_renderer_logical_size(SDL_Renderer* renderer)
{
	int width = 0, height = 0;
	SDL_RenderGetLogicalSize(renderer, &width, &height);
	return {width, height};
}

} // namespace

my_app::my_app(SDL_Renderer_ptr renderer, level lvl, camera cam)
: _renderer{std::move(renderer)}
, _level{lvl}
, _camera{cam}
{
	auto surface = make_SDL_Surface(SDL_LoadBMP("assets/wall.bmp"));

	_wall_tex = make_SDL_Texture(
		SDL_CreateTextureFromSurface(_renderer.get(), surface.get()));
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
	auto const fov = M_PI / 2.f;
	auto const max_distance = 4;

	auto const step_size = 1.f / 32.f;

	auto const logical_size = get_renderer_logical_size(_renderer.get());

	auto const half_width = logical_size.width / 2;
	auto const half_height = logical_size.height / 2;

	color const fog_color{0, 0, 0};
	color const white_color{255, 255, 255};

	// draw ceiling (top-down)
	color const ceiling_color{64, 0, 0};
	for (int i = 0; i < half_height; ++i) {
		auto const t_scale = max_distance / static_cast<float>(max_distance-1);
		auto const interp = linear_interpolate(ceiling_color, fog_color,
			i / static_cast<float>(half_height) * t_scale);
		set_render_draw_color(_renderer.get(), interp);
		SDL_CHECK(SDL_RenderDrawLine(_renderer.get(), 0, i, logical_size.width,
			i) == 0);
	}

	// draw floor (bottom-up)
	color const floor_color{64, 128, 255};
	for (int i = 0; i < half_height; ++i) {
		auto const t_scale = max_distance / static_cast<float>(max_distance-1);
		auto const interp = linear_interpolate(floor_color, fog_color,
			(half_height-i) / static_cast<float>(half_height) * t_scale);
		set_render_draw_color(_renderer.get(), interp);

		auto const draw_y = i + half_height;
		SDL_CHECK(SDL_RenderDrawLine(_renderer.get(), 0, draw_y,
			logical_size.width, draw_y) == 0);
	}

	set_render_draw_color(_renderer.get(), {255, 255, 255});

	// draw geom
	for (int i = 0; i < logical_size.width; ++i) {
		auto const local_radians = (i - half_width) /
			static_cast<float>(logical_size.width) * fov;
		auto const camera_radians = local_radians - _camera.yaw;
		auto const ray_radians = camera_radians;

		auto distance = step_size;
		auto ray_u = 0.f;
		while (distance < max_distance) {
			auto const march_x = _camera.x + cos(ray_radians)*distance;
			auto const march_y = _camera.y + sin(ray_radians)*distance;

			SDL_Point const point{
				static_cast<int>(march_x),
				static_cast<int>(march_y)
			};

			auto const u = fabs(march_x - point.x);
			auto const v = fabs(march_y - point.y);
			auto const tolerance = 0.03125f;
			ray_u = u < tolerance || u > (1.f - tolerance) ? v : u;

			auto const index = point.y * _level.width + point.x;
			if (_level.data[index] == 1) {
				break;
			}

			distance += step_size;
		}

		auto const interp = linear_interpolate(white_color, fog_color,
			distance / max_distance);
		SDL_CHECK(SDL_SetTextureColorMod(_wall_tex.get(), interp.r, interp.g,
			interp.b) == 0);

		distance *= cos(sin(local_radians));

		auto const wall_size = static_cast<int>(half_height/distance);

		auto const image_column = static_cast<int>(ray_u * 16); // MAGIC NNMBER
		SDL_Rect src{image_column, 0, 1, 16}; // MAGIC NUMBER
		SDL_Rect dst{i, half_height - wall_size, 1, wall_size*2};

		SDL_CHECK(
			SDL_RenderCopy(_renderer.get(), _wall_tex.get(), &src, &dst) == 0);
	}
}

void my_app::keydown(SDL_Keycode key)
{
	const auto yaw_step = 0.1f;
	const auto move_step = 0.1f;

	switch (key) {
	case SDLK_a:
		_camera.yaw += yaw_step;
		break;
	case SDLK_d:
		_camera.yaw -= yaw_step;
		break;
	case SDLK_w:
		_camera.x += cos(_camera.yaw) * move_step;
		_camera.y -= sin(_camera.yaw) * move_step;
		break;
	case SDLK_s:
		_camera.x -= cos(_camera.yaw) * move_step;
		_camera.y += sin(_camera.yaw) * move_step;
		break;
	case SDLK_SPACE:
		if (!sdl::save_screenshot(_renderer.get(), "screenshot.bmp")) {
			SDL_Log("save_screenshot failed!");
		}
		break;
	case SDLK_ESCAPE:
		_running = false;
		break;
	}
}
