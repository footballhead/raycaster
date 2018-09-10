#include "surface_manipulation.hpp"

#include <SDL.h>

#include <stdexcept>

using namespace mycolor;
using namespace mymath;

namespace {

constexpr auto desired_bpp = 3;
constexpr auto desired_format = SDL_PIXELFORMAT_BGR24;

bool is_surface_of_desired_format(
    SDL_Surface* surf, int want_bpp, Uint32 want_format)
{
    return surf->format->BytesPerPixel == want_bpp
        && surf->format->format == want_format;
}

color get_surface_pixel(SDL_Surface* surf, point2i p)
{
    if (!is_surface_of_desired_format(surf, desired_bpp, desired_format)) {
        SDL_Log("Invalid surface format: 0x%x", surf->format->format);
        throw std::runtime_error{"Got invalid bitmap surface! See log"};
    }

    if (p.x < 0) {
        p.x += surf->w;
    } else if (p.x >= surf->w) {
        p.x -= surf->w;
    }

    if (p.y < 0) {
        p.y += surf->h;
    } else if (p.y >= surf->h) {
        p.y -= surf->h;
    }

    // Assuming BGR24
    auto const index = p.y * surf->pitch + p.x * surf->format->BytesPerPixel;
    auto const pixel = static_cast<Uint8*>(surf->pixels) + index;
    return color{pixel[2], pixel[1], pixel[0]};
}

} // namespace

namespace sdl_app {

color get_surface_pixel(SDL_Surface* surf, point2f const& uv)
{
    auto const surface_extents = make_point<float>(surf->w, surf->h);

    auto const tex_coord = surface_extents * uv - point2f{0.5f, 0.5f};
    auto const tex_coord_remainder = remainder(tex_coord);

    auto const top_left = point_cast<int>(floor(tex_coord));
    auto const top_left_pixel = ::get_surface_pixel(surf, top_left);

    auto const bottom_right = point_cast<int>(floor(tex_coord)) + point2i{1, 1};
    auto const bottom_right_pixel = ::get_surface_pixel(surf, bottom_right);

    auto const top_right = point2i{bottom_right.x, top_left.y};
    auto const top_right_pixel = ::get_surface_pixel(surf, top_right);

    auto const bottom_left = point2i{top_left.x, bottom_right.y};
    auto const bottom_left_pixel = ::get_surface_pixel(surf, bottom_left);

    auto const color_lerp_col1 = linear_interpolate(
        top_left_pixel, bottom_left_pixel, tex_coord_remainder.y);
    auto const color_lerp_col2 = linear_interpolate(
        top_right_pixel, bottom_right_pixel, tex_coord_remainder.y);
    return linear_interpolate(
        color_lerp_col1, color_lerp_col2, tex_coord_remainder.x);
}

color get_surface_pixel_nn(SDL_Surface* surf, point2f const& uv)
{
    auto const surface_extents = make_point<float>(surf->w, surf->h);

    auto const lo = point2f{0.f, 0.f};
    auto const hi = surface_extents - point2f{1.f, 1.f};
    auto const tex_coord = clamp(surface_extents * uv, lo, hi);

    return ::get_surface_pixel(surf, make_point<int>(tex_coord.x, tex_coord.y));
}

} // namespace sdl_app