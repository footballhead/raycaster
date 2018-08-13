#pragma once

#include "sdl_app.hpp"

#include <cstdint>
#include <vector>

struct level {
    int width;
    int height;
    std::vector<uint8_t> data;
};

struct camera {
    float x;
    float y;
    float yaw; // (radians)
};

class my_app {
public:
    explicit my_app(SDL_Renderer_ptr renderer, level lvl, camera cam);

    int exec();

private:
    void update();
    void render();

    void keydown(SDL_Keycode key);

    SDL_Renderer_ptr _renderer = nullptr;
    SDL_Texture_ptr _wall_tex = nullptr;

    level _level;
    camera _camera;

    bool _running = false;
    uint64_t _ticks = 0u;
};
