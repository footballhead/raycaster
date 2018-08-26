#include "line_drawer_app.hpp"

#include <mymath/mymath.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cstdint>
#include <iostream>
#include <stdexcept>
#include <vector>

using namespace mymath;
using namespace line_drawer;

int main(int argc, char** argv)
{
    auto sdl = std::make_shared<sdl::sdl_init>();

    auto const window_title = "Line Drawer";
    SDL_Point const window_bounds{640, 480};
    auto window = sdl::make_window(window_title, window_bounds);

    auto renderer = sdl::shared_renderer{sdl::make_renderer(window.get())};

    SDL_CHECK(SDL_RenderSetLogicalSize(
                  renderer.get(), window_bounds.x, window_bounds.y)
        == 0);

    auto input = std::make_unique<sdl_app::input_buffer>();

    line_drawer_app app{std::move(sdl), std::move(window), std::move(renderer),
        std::move(input)};
    try {
        app.exec();
    } catch (const std::exception& e) {
        SDL_Log("EXCEPTION: %s", e.what());
    }

    return 0;
}
