#include "line_drawer_app.hpp"

#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <cmath>
#include <iostream>

namespace {

template <typename T> int sgn(T val) { return val >= 0 ? 1 : -1; }

SDL_Point get_renderer_logical_size(SDL_Renderer* ren)
{
    SDL_Point size{0, 0};
    SDL_RenderGetLogicalSize(ren, &size.x, &size.y);
    return size;
}

std::ostream& operator<<(std::ostream& os, SDL_Point const& p)
{
    return os << '{' << p.x << '.' << p.y << '}';
}

SDL_Point operator/(SDL_Point const& a, int scalar)
{
    return {a.x / scalar, a.y / scalar};
}

SDL_Point operator+(SDL_Point const& lhs, SDL_Point const& rhs)
{
    return {lhs.x + rhs.x, lhs.y + rhs.y};
}

SDL_Point operator-(SDL_Point const& lhs, SDL_Point const& rhs)
{
    return {lhs.x - rhs.x, lhs.y - rhs.y};
}

bool operator==(SDL_Point const& lhs, SDL_Point const& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

SDL_Point round_to_point(float x, float y)
{
    return {static_cast<int>(std::round(x)), static_cast<int>(std::round(y))};
}

void draw_point(SDL_Renderer* ren, SDL_Point const& p)
{
    SDL_RenderDrawPoint(ren, p.x, p.y);
}

void draw_line(SDL_Renderer* ren, SDL_Point const& src, SDL_Point const& dst)
{
    auto const delta = dst - src;

    auto x_inc = 0.f;
    auto y_inc = 0.f;

    if (delta.x == 0) {
        x_inc = 0.f;
        y_inc = 1.f;
    } else if (delta.y == 0) {
        x_inc = 1.f;
        y_inc = 0.f;
    } else {
        if (std::abs(delta.y) < std::abs(delta.x)) {
            auto const slope = std::abs(delta.y / static_cast<float>(delta.x));
            x_inc = 1.f;
            y_inc = slope;
        } else {
            auto const inverse_slope
                = std::abs(delta.x / static_cast<float>(delta.y));
            x_inc = inverse_slope;
            y_inc = 1.f;
        }
    }

    x_inc *= sgn(delta.x);
    y_inc *= sgn(delta.y);

    // Put an arbitrary limit in case this goes into infinite loop
    auto const debug_limit = 320;
    for (int i = 0; i < debug_limit; ++i) {
        // The rounding is key to ensuring this algo halts!
        auto const accum = round_to_point(x_inc * i, y_inc * i);
        auto const interp = src + accum;
        draw_point(ren, interp);

        if (interp == dst) {
            break;
        }
    }
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
