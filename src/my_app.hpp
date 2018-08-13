#pragma once

#include "asset_store.hpp"
#include "sdl_app.hpp"

#include <cstdint>
#include <vector>

namespace raycaster {

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
    explicit my_app(sdl::renderer renderer, std::unique_ptr<asset_store> assets,
        level lvl, camera cam);

    int exec();

private:
    void update();
    void render();

    void keydown(SDL_Keycode key);

    sdl::renderer _renderer = nullptr;
    std::unique_ptr<asset_store> _asset_store = nullptr;

    level _level;
    camera _camera;

    bool _running = false;
    uint64_t _ticks = 0u;
};

} // namespace raycaster
