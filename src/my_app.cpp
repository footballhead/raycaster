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

struct point2f {
	float x;
	float y;
};

constexpr auto max_distance = 4.f;
constexpr auto step_size = 1.f / 32.f;

/// @returns The distance, >= 0 if collision (sets out_result)
float find_collision(const level& lvl, const point2f& origin, float direction,
	point2f& out_result)
{
	auto distance = step_size;
	while (distance < max_distance) {
		auto const march_x =
			static_cast<float>(origin.x + cos(direction) * distance);
		auto const march_y =
			static_cast<float>(origin.y + sin(direction) * distance);

		auto const int_x = static_cast<int>(march_x);
		auto const int_y = static_cast<int>(march_y);

		if (int_x < 0 || int_x > lvl.width-1 || int_y < 0
			|| int_y > lvl.height-1)
		{
			return -1.f;
		}

		auto const index = int_y * lvl.width + int_x;
		if (lvl.data[index] == 1) {
			out_result = point2f{march_x, march_y};
			return distance;
		}

		distance += step_size;
	}

	return -1.f;
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
		auto const local_ray_radians = (i - half_width) /
			static_cast<float>(logical_size.width) * fov;
		auto const camera_ray_radians = local_ray_radians - _camera.yaw;

		float ray_u = 0.f;
		point2f collision{0.f, 0.f};
		auto distance = find_collision(_level, {_camera.x, _camera.y},
			camera_ray_radians, collision);

		if (distance > 0) {
			SDL_Point const point{
				static_cast<int>(collision.x),
				static_cast<int>(collision.y)
			};

			auto const u = fabs(collision.x - point.x);
			auto const v = fabs(collision.y - point.y);
			auto const tolerance = 0.03125f; // TODO set to step size?
			ray_u = u < tolerance || u > (1.f - tolerance) ? v : u;
		} else {
			// Set to max distance so the color lerp will use the fog color
			distance = max_distance;
		}

		auto const interp = linear_interpolate(white_color, fog_color,
			distance / max_distance);
		SDL_CHECK(SDL_SetTextureColorMod(_wall_tex.get(), interp.r, interp.g,
			interp.b) == 0);

		distance *= sin(M_PI / 2.f - fabs(local_ray_radians));

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
