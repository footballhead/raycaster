#pragma once

#include "camera.hpp"
#include "level.hpp"

#include <mymath/mymath.hpp>
#include <sdl_application/asset_store.hpp>
#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cstdint>
#include <vector>

namespace raycaster {

class collision_result;

namespace common_assets {
constexpr auto wall_texture = "wall.bmp";
constexpr auto stone_texture = "stone.bmp";
constexpr auto floor = "floor.bmp";
constexpr auto floor2 = "floor2.bmp";
constexpr auto ceiling = "ceil.bmp";
constexpr auto column = "column.bmp";
constexpr auto font = "6x8-terminal-mspaint.bmp";
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
    void rasterize(
        std::vector<std::vector<collision_result>> const& ray_collisions);
    void draw_column(int column, collision_result const& collision);
    void draw_hud();
    void on_window_event(SDL_WindowEvent const& event);

    level _level;
    camera _camera;

    Uint32 _fps_interval_start = 0u;
    Uint32 _fps_interval_frames = 0u;
    Uint32 _fps = 0u;

    bool _debug_no_fog = false;
    bool _debug_no_textures = false;
    bool _debug_no_floor = false;

    bool _screenshot_queued = false;
};

} // namespace raycaster
