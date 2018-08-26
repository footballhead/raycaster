#include "line_drawer_app.hpp"

#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

namespace {

SDL_Point get_renderer_logical_size(SDL_Renderer* ren)
{
    SDL_Point size{0, 0};
    SDL_RenderGetLogicalSize(ren, &size.x, &size.y);
    return size;
}

SDL_Point operator/(SDL_Point const& a, int scalar)
{
    return {a.x / scalar, a.y / scalar};
}

void draw_line(SDL_Renderer* ren, SDL_Point const& a, SDL_Point const& b)
{
    SDL_RenderDrawLine(ren, a.x, a.y, b.x, b.y);
}

} // namespace

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

void line_drawer_app::render()
{
    auto renderer = get_renderer();
    auto input = get_input_buffer();

    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 255);

    auto const midpoint = get_renderer_logical_size(renderer) / 2;
    auto const mouse_position = input.get_mouse_position();
    draw_line(renderer, midpoint, mouse_position);
}

} // namespace line_drawer
