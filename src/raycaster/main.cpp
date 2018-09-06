#include "camera.hpp"
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
    auto sdl = std::make_shared<sdl::sdl_init>();

    auto const window_title = "Raycaster";
    SDL_Point const window_bounds{1280, 720};
    auto window
        = sdl::make_window(window_title, window_bounds, SDL_WINDOW_RESIZABLE);

    auto renderer = sdl::shared_renderer{sdl::make_renderer(window.get())};

    auto assets = std::make_unique<asset_store>(renderer, "../assets");
    assets->get_asset(common_assets::wall_texture);
    assets->get_asset(common_assets::stone_texture);

    auto const renderer_size = extent2i{320, 180};
    SDL_CHECK(set_renderer_logical_size(renderer.get(), renderer_size));

    level test_level = {std::vector<wall>{
        wall{line2f{point2f{0.f, 0.f}, point2f{0.f, 4.f}}, 1},
        wall{line2f{point2f{0.f, 4.f}, point2f{1.f, 5.f}}, 2},
        wall{line2f{point2f{1.f, 5.f}, point2f{5.f, 5.f}}, 1},
        wall{line2f{point2f{5.f, 5.f}, point2f{5.f, 0.f}}, 1},
        wall{line2f{point2f{5.f, 0.f}, point2f{0.f, 0.f}}, 1},
        wall{line2f{point2f{1.f, 4.f}, point2f{2.f, 5.f}}, 1},
    }};

    camera cam{
        point2f{
            3.5f, // x
            3.5f, // y
        },
        0.f, // rotation
        0.01f, // near
        4.f, // far
        0.01f, // right
    };

    auto input = std::make_unique<sdl_app::input_buffer>();

    raycaster_app app{std::move(sdl), std::move(window), std::move(renderer),
        std::move(input), std::move(assets), test_level, cam};
    try {
        app.exec();
    } catch (const std::exception& e) {
        SDL_Log("EXCEPTION: %s", e.what());
    }

    return 0;
}
