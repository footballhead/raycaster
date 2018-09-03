#pragma once

#include "camera.hpp"

#include <mymath/mymath.hpp>
#include <sdl_application/asset_store.hpp>
#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <cstdint>
#include <vector>

namespace raycaster {

struct level {
    mymath::extent2i bounds;
    std::vector<uint8_t> data;
};

class raycaster_app : public sdl_app::sdl_application {
public:
    explicit raycaster_app(std::shared_ptr<sdl::sdl_init> sdl,
        sdl::window window, sdl::shared_renderer renderer,
        std::unique_ptr<sdl_app::input_buffer> input,
        std::unique_ptr<sdl_app::asset_store> assets, level lvl, camera cam);

protected:
    void unhandled_event(SDL_Event const& event) override;
    void update() override;
    void render() override;

private:
    void on_window_event(SDL_WindowEvent const& event);

    std::unique_ptr<sdl_app::asset_store> _asset_store = nullptr;

    level _level;
    camera _camera;
};

} // namespace raycaster
