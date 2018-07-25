#pragma once

#include "sdl_app.hpp"

class my_app {
public:
	explicit my_app(SDL_Renderer_ptr renderer);

	int exec();

private:
	void update();
	void render();

	SDL_Renderer_ptr _renderer = nullptr;

	bool _running = false;
};
