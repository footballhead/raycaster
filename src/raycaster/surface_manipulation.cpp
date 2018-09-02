#include "surface_manipulation.hpp"

#include <stdexcept>

using namespace mycolor;
using namespace mymath;

namespace {

color get_surface_pixel(SDL_Surface* surf, point2i const& p)
{
    if (surf->format->BytesPerPixel != 3) {
        throw std::runtime_error{"Got BMP texture that is not 3 BPP!"};
    }

    if (surf->pitch % 3 != 0) {
        throw std::runtime_error{"Got tex where pitch is not divisible by 3"};
    }

    // Assuming BGR888
    auto const index = p.y * surf->pitch + p.x * surf->format->BytesPerPixel;
    auto const pixel = static_cast<Uint8*>(surf->pixels) + index;
    return color{pixel[2], pixel[1], pixel[0]};
}

} // namespace

namespace raycaster {

color get_surface_pixel(SDL_Surface* surf, point2f const& uv)
{
    auto const surface_extents = make_point<float>(surf->w, surf->h);

    auto const lo = point2f{0.f, 0.f};
    auto const hi = surface_extents - point2f{1.f, 1.f};
    auto const tex_coord = clamp(surface_extents * uv, lo, hi);

    return ::get_surface_pixel(
        surf, make_point<int>(tex_coord.x, tex_coord.y));
}

} // namespace raycaster

