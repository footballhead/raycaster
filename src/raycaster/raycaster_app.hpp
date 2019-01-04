#pragma once

#include "camera.hpp"
#include "level.hpp"
#include "pipeline.hpp"

#include <lua_raii/lua_raii.hpp>
#include <mymath/mymath.hpp>
#include <sdl_application/asset_store.hpp>
#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <array>
#include <cstdint>
#include <vector>

namespace raycaster {

class raycaster_app : public sdl_app::sdl_application {
public:
    raycaster_app(std::shared_ptr<sdl::sdl_init> sdl, sdl::window window,
        std::unique_ptr<sdl_app::input_buffer> input,
        std::unique_ptr<sdl_app::asset_store> assets,
        std::unique_ptr<render_pipeline> pipeline, lua::state L, camera cam);

    void change_level(std::unique_ptr<level> level);

protected:
    void unhandled_event(SDL_Event const& event) override;
    void update() override;
    void render() override;

private:
    void try_to_move_camera(mymath::vector2f const& vec);
    void draw_hud();
    void on_window_event(SDL_WindowEvent const& event);

    std::unique_ptr<render_pipeline> _pipeline;
    lua::state _L;
    std::unique_ptr<level> _level;
    camera _camera;

    Uint32 _fps_interval_start = 0u;
    Uint32 _fps_interval_frames = 0u;
    Uint32 _fps = 0u;

    bool _debug_no_textures = false;
    bool _debug_no_floor = false;
    bool _debug_no_hud = false;
    bool _debug_noclip = false;
    bool _console_open = false;

    bool _screenshot_queued = false;

    std::string _console_input_buffer;
    std::vector<std::string> _console_history;

    SDL_Surface* _font_texture = nullptr;
};

} // namespace raycaster
