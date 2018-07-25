#pragma once

#include "sdl_app.hpp"

#include <cstdint>
#include <vector>

struct level {
	int width;
	int height;
	std::vector<uint8_t> data;
};

class my_app {
public:
	explicit my_app(SDL_Renderer_ptr renderer, level lvl);

	int exec();

private:
	void update();
	void render();

	SDL_Renderer_ptr _renderer = nullptr;

	level _level;

	bool _running = false;
	uint64_t _ticks = 0u;
};
