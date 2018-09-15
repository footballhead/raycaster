#pragma once

#include <SDL.h>

namespace raycaster {

constexpr char const* pixel_type_to_string(Uint32 val)
{
    switch (val) {
    case SDL_PIXELTYPE_PACKED32:
        return "SDL_PIXELTYPE_PACKED32";
    default:
        return "???";
    }
}

constexpr char const* packed_order_to_string(Uint32 val)
{
    switch (val) {
    case SDL_PACKEDORDER_XRGB:
        return "SDL_PACKEDORDER_XRGB";
    case SDL_PACKEDORDER_ARGB:
        return "SDL_PACKEDORDER_ARGB";
    default:
        return "???";
    }
}

constexpr char const* packed_layout_to_string(Uint32 val)
{
    switch (val) {
    case SDL_PACKEDLAYOUT_8888:
        return "SDL_PACKEDLAYOUT_8888";
    default:
        return "???";
    }
}

inline void print_pixel_format(Uint32 fmt)
{
    SDL_Log("format=0x%x", fmt);
    SDL_Log("pixel type=%s", pixel_type_to_string(SDL_PIXELTYPE(fmt)));
    SDL_Log("pixel order=%s (%u)", packed_order_to_string(SDL_PIXELORDER(fmt)),
        SDL_PIXELORDER(fmt));
    SDL_Log("pixel layout=%s", packed_layout_to_string(SDL_PIXELLAYOUT(fmt)));
    SDL_Log("BITS per pixel=%u", SDL_BITSPERPIXEL(fmt));
    SDL_Log("BYTES per pixel=%u", SDL_BYTESPERPIXEL(fmt));
    SDL_Log("is indexed? %x", SDL_ISPIXELFORMAT_INDEXED(fmt));
    SDL_Log("is alpha? %x", SDL_ISPIXELFORMAT_ALPHA(fmt));
    SDL_Log("is fourcc? %x", SDL_ISPIXELFORMAT_FOURCC(fmt));
}

} // namespace raycaster
