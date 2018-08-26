#pragma once

#include "asset_store.hpp"
#include "camera.hpp"
#include "input_buffer.hpp"

#include <mymath/mymath.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <cstdint>
#include <vector>

namespace raycaster {

struct level {
    mymath::extent2i bounds;
    std::vector<uint8_t> data;
};

class my_app {
public:
    explicit my_app(sdl::renderer renderer, std::unique_ptr<asset_store> assets,
        std::unique_ptr<input_buffer> input, level lvl, camera cam);

    int exec();

private:
    void update();
    void render();

    sdl::renderer _renderer = nullptr;
    std::unique_ptr<asset_store> _asset_store = nullptr;
    std::unique_ptr<input_buffer> _input_buffer = nullptr;

    level _level;
    camera _camera;

    bool _running = false;
    uint64_t _ticks = 0u;
};

} // namespace raycaster
