#include "camera.hpp"
#include "level.hpp"
#include "raycaster_app.hpp"

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

    level test_level = {
        std::vector<wall>{
            // walls
            wall{line2f{point2f{1.f, 1.f}, point2f{2.f, 1.f}}, 1},
            wall{line2f{point2f{2.f, 1.f}, point2f{2.f, 2.f}}, 2},
            wall{line2f{point2f{2.f, 2.f}, point2f{3.f, 2.f}}, 2},
            wall{line2f{point2f{3.f, 2.f}, point2f{3.f, 1.f}}, 2},
            wall{line2f{point2f{3.f, 1.f}, point2f{7.f, 1.f}}, 1},
            wall{line2f{point2f{7.f, 1.f}, point2f{7.f, 5.f}}, 1},
            wall{line2f{point2f{7.f, 5.f}, point2f{4.f, 5.f}}, 2},
            wall{line2f{point2f{4.f, 5.f}, point2f{4.f, 6.f}}, 2},
            wall{line2f{point2f{4.f, 6.f}, point2f{7.f, 6.f}}, 2},
            wall{line2f{point2f{7.f, 6.f}, point2f{7.f, 7.f}}, 1},
            wall{line2f{point2f{7.f, 7.f}, point2f{2.f, 7.f}}, 1},
            wall{line2f{point2f{2.f, 7.f}, point2f{2.f, 6.f}}, 2},
            wall{line2f{point2f{2.f, 6.f}, point2f{1.f, 6.f}}, 2},
            wall{line2f{point2f{1.f, 6.f}, point2f{1.f, 1.f}}, 1},
            // island
            wall{line2f{point2f{5.f, 2.f}, point2f{6.f, 2.f}}, 2},
            wall{line2f{point2f{6.f, 2.f}, point2f{6.f, 4.f}}, 2},
            wall{line2f{point2f{6.f, 4.f}, point2f{5.f, 4.f}}, 2},
            wall{line2f{point2f{5.f, 4.f}, point2f{5.f, 2.f}}, 2},
        },
        std::vector<sprite>{
            sprite{point2f{3.5f, 3.5f}, 4},
            sprite{point2f{1.5f, 1.5f}, 4},
        },
    };

    camera cam{
        point2f{
            3.5f, // x
            3.5f, // y
        },
        0.f, // rotation
        0.01f, // near
        8.f, // far
        0.01f, // right
    };

    auto input = std::make_unique<sdl_app::input_buffer>();

    SDL_Log("Creating raycaster_app...");
    raycaster_app app{std::move(sdl), std::move(window), std::move(input),
        std::move(assets), test_level, cam};
    SDL_Log("Running app...");
    try {
        app.exec();
    } catch (const std::exception& e) {
        SDL_Log("EXCEPTION: %s", e.what());
    }

    return 0;
}
