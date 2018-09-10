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

} // namespace

namespace sdl_app {

color get_surface_pixel(SDL_Surface* surf, point2f const& uv)
{
    return get_surface_pixel(
        surf, make_point<int>(uv.x * surf->w, uv.y * surf->h));
}

color get_surface_pixel(SDL_Surface* surf, point2i p)
{
    // Assuming BGR24
    auto const index = p.y * surf->pitch + p.x * surf->format->BytesPerPixel;
    auto const pixel = static_cast<Uint8*>(surf->pixels) + index;
    return color{pixel[2], pixel[1], pixel[0]};
}

} // namespace sdl_app
