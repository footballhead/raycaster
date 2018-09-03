#include "sdl_mymath.hpp"

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>

#include <SDL.h>

#include <stdexcept>

using namespace mycolor;
using namespace mymath;

namespace {

point2i round_to_point(float x, float y)
{
    return make_point<int>(std::round(x), std::round(y));
}

} // namespace

namespace sdl_app {

extent2i get_renderer_logical_size(SDL_Renderer* renderer)
{
    int width = 0, height = 0;
    SDL_RenderGetLogicalSize(renderer, &width, &height);
    return {width, height};
}

bool set_renderer_logical_size(SDL_Renderer* renderer, extent2i const& size)
{
    return SDL_RenderSetLogicalSize(renderer, size.w, size.h) == 0;
}

extent2i get_texture_size(SDL_Texture* texture)
{
    int width = 0, height = 0;
    if (SDL_QueryTexture(texture, nullptr, nullptr, &width, &height) != 0) {
        SDL_Log("SDL_QueryTexture failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

extent2i get_renderer_output_size(SDL_Renderer* renderer)
{
    int width = -1, height = -1;
    if (SDL_GetRendererOutputSize(renderer, &width, &height) != 0) {
        SDL_Log("SDL_GetRendererOutputSize failed: %s", SDL_GetError());
        return {-1, -1};
    }
    return {width, height};
}

bool set_render_draw_color(SDL_Renderer* renderer, color const& c)
{
    return SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255) == 0;
}

bool draw_point(SDL_Renderer* ren, point2i const& p)
{
    return SDL_RenderDrawPoint(ren, p.x, p.y) == 0;
}

bool draw_line(SDL_Renderer* ren, point2i const& src, point2i const& dst,
    std::function<color(point2i const&)> get_color)
{
    auto const delta = dst - src;

    // Anticipate division by 0 and short circuit
    auto const zero_vector = point2i{0, 0};
    if (delta == zero_vector) {
        return set_render_draw_color(ren, get_color(dst))
            && draw_point(ren, dst);
    }

    // We can do all the math in absolutes then apply the sign later to get the
    // right result! This greatly simplfies the code
    auto const abs_delta = mymath::abs(delta);
    auto const use_unit_x = abs_delta.y < abs_delta.x;

    auto const x_inc
        = (use_unit_x ? 1.f : abs_delta.slope_inverse()) * sgn(delta.x);
    auto const y_inc = (use_unit_x ? abs_delta.slope() : 1.f) * sgn(delta.y);

    // Put an arbitrary limit in case this goes into infinite loop
    auto const debug_limit = 2048;
    for (int i = 0; i < debug_limit; ++i) {
        // The rounding is key to ensuring this algo halts!
        auto const iterated_step = round_to_point(x_inc * i, y_inc * i);
        auto const interp = src + iterated_step;

        if (!set_render_draw_color(ren, get_color(interp))) {
            return false;
        }

        if (!draw_point(ren, interp)) {
            return false;
        }

        if (interp == dst) {
            break;
        }
    }

    return true;
}

} // namespace raycaster
