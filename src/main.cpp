#include "asset_store.hpp"
#include "camera.hpp"
#include "my_app.hpp"
#include "mymath.hpp"
#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace raycaster;

int main(int argc, char** argv)
{
    sdl::sdl_app sdl;

    const auto window_title = "Raycaster";
    const SDL_Point window_bounds = {640, 480};
    const auto window = sdl::make_window(window_title, window_bounds);

    auto renderer = sdl::make_renderer(window.get());

    auto assets = std::make_unique<asset_store>(renderer, "assets");
    assets->get_asset(common_assets::wall_texture);
    assets->get_asset(common_assets::stone_texture);

    auto scaling_factor = 4;
    SDL_CHECK(
        SDL_RenderSetLogicalSize(renderer.get(),
            window_bounds.x / scaling_factor, window_bounds.y / scaling_factor)
        == 0);

    // clang-format off
	level test_level = {
		8,	// width
		8,	// height
		{	// data
			1, 1, 1, 1, 1, 1, 1, 1,
			1, 0, 2, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 0, 2, 0, 1,
			1, 0, 0, 0, 0, 2, 0, 1,
			1, 0, 0, 0, 0, 0, 0, 1,
			1, 0, 0, 0, 2, 2, 2, 1,
			1, 2, 0, 0, 0, 0, 0, 1,
			1, 1, 1, 1, 1, 1, 1, 1,
		},
	};
    // clang-format on

    camera cam{
        point2f{
            3.5f, // x
            3.5f, // y
        },
        0.f, // yaw
        0.5f,
        0.5f,
    };

    my_app app{std::move(renderer), std::move(assets), test_level, cam};
    try {
        app.exec();
    } catch (const std::exception& e) {
        SDL_Log("EXCEPTION: %s", e.what());
    }

    return 0;
}
