#include "camera.hpp"
#include "level.hpp"
#include "pipeline.hpp"
#include "raycaster_app.hpp"
#include "texture_cache.hpp"

#include <lua_raii/lua_raii.hpp>
#include <mymath/mymath.hpp>
#include <sdl_application/asset_store.hpp>
#include <sdl_application/sdl_mymath.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace mymath;
using namespace raycaster;
using namespace sdl_app;

int main(int argc, char** argv)
{
    SDL_Log("Starting application main...");
    auto sdl = std::make_shared<sdl::sdl_init>();

    // Text input, for some reason, might be active right after init. Bad! Turn
    // off!
    if (SDL_IsTextInputActive()) {
        SDL_StopTextInput();
        SDL_Log("Text input was active on init, turn off");
    }

    auto const window_title = "Raycaster";
    SDL_Point const window_bounds{640, 360};
    auto window = sdl::make_window(window_title, window_bounds);

    // Create asset manager and preload assets
    auto assets = std::make_unique<asset_store>("../assets");
    assets->get_asset(common_assets::wall_texture);
    assets->get_asset(common_assets::stone_texture);
    assets->get_asset(common_assets::floor);
    assets->get_asset(common_assets::floor2);
    assets->get_asset(common_assets::ceiling);
    assets->get_asset(common_assets::column);
    assets->get_asset(common_assets::font);
    assets->get_asset(common_assets::barrel);
    assets->get_asset(common_assets::barrel_explode);
    assets->get_asset(common_assets::bat);

    auto L = lua::make_state();

    auto level_file = "../assets/levels/test_level.tmx.lua";
    if (argc >= 2) {
        level_file = argv[1];
    }
    auto test_level = load_level(level_file, L.get());

    camera cam{
        test_level.player_start,
        0.f, // rotation
        0.01f, // near
        8.f, // far
        0.01f, // right
    };

    auto input = std::make_unique<sdl_app::input_buffer>();

    auto pipeline = std::make_unique<raycaster::render_pipeline>(
        make_texture_cache(*assets));

    if (luaL_dofile(L.get(), "../assets/lua/main.lua")) {
        SDL_Log("Failed to do main.lua!");
        throw std::runtime_error{lua_tostring(L.get(), -1)};
    }

    SDL_Log("Creating raycaster_app...");
    raycaster_app app{std::move(sdl), std::move(window), std::move(input),
        std::move(assets), std::move(pipeline), std::move(L), test_level, cam};
    SDL_Log("Running app...");
    try {
        app.exec();
    } catch (const std::exception& e) {
        SDL_Log("EXCEPTION: %s", e.what());
    }

    return 0;
}
