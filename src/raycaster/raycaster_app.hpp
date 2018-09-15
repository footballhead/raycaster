#pragma once

#include "camera.hpp"
#include "level.hpp"

#include <mymath/mymath.hpp>
#include <sdl_application/asset_store.hpp>
#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <cstdint>
#include <vector>

namespace raycaster {

namespace common_assets {
constexpr auto wall_texture = "wall.bmp";
constexpr auto stone_texture = "stone.bmp";
constexpr auto floor = "floor.bmp";
constexpr auto floor2 = "floor2.bmp";
constexpr auto ceiling = "ceil.bmp";
constexpr auto column = "column.bmp";
} // namespace common_assets

inline char const* get_wall_texture(unsigned int i)
{
    switch (i) {
    case 2:
        return common_assets::stone_texture;
    case 4:
        return common_assets::column;
    case 0:
    case 1:
    default:
        return common_assets::wall_texture;
    }
}

class raycaster_app : public sdl_app::sdl_application {
public:
    explicit raycaster_app(std::shared_ptr<sdl::sdl_init> sdl,
        sdl::window window, std::unique_ptr<sdl_app::input_buffer> input,
        std::unique_ptr<sdl_app::asset_store> assets, level lvl, camera cam);

protected:
    void unhandled_event(SDL_Event const& event) override;
    void update() override;
    void render() override;

private:
    void on_window_event(SDL_WindowEvent const& event);

    level _level;
    camera _camera;
};

} // namespace raycaster
