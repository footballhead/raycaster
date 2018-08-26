#include "line_drawer_app.hpp"

#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

using namespace sdl_app;

namespace line_drawer {

line_drawer_app::line_drawer_app(std::shared_ptr<sdl::sdl_init> sdl,
    sdl::window window, sdl::shared_renderer renderer,
    std::unique_ptr<input_buffer> input)
: sdl_application(
      std::move(sdl), std::move(window), std::move(renderer), std::move(input))
{
}

void line_drawer_app::update()
{
    auto& input_buffer = get_input_buffer();

    if (input_buffer.is_quit()
        || input_buffer.is_pressed(SDL_SCANCODE_ESCAPE)) {
        quit();
        return;
    }
}

void line_drawer_app::render() {}

} // namespace line_drawer
