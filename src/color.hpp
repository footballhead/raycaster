#pragma once

#include "sdl_app.hpp"

#include <SDL.h>

#include <cstdint>

namespace raycaster {

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

inline void set_render_draw_color(SDL_Renderer* renderer, const color& c)
{
    SDL_CHECK(SDL_SetRenderDrawColor(renderer, c.r, c.g, c.b, 255) == 0);
}

constexpr color linear_interpolate(color a, color b, float t)
{
    t = std::max(std::min(t, 1.f), 0.f);
    return {
        static_cast<uint8_t>(a.r * (1.f - t) + b.r * t),
        static_cast<uint8_t>(a.g * (1.f - t) + b.g * t),
        static_cast<uint8_t>(a.b * (1.f - t) + b.b * t),
    };
}

constexpr color hue_to_rgb(float hue)
{
    auto sixth = 1.f / 6.f;

    while (hue > 1.f) {
        hue -= 1.f;
    }

    if (hue < sixth) {
        return {
            255,
            static_cast<uint8_t>(hue / sixth * 255),
            0,
        };
    } else if (hue < 2 * sixth) {
        return {
            static_cast<uint8_t>(255 - ((hue - sixth) / sixth) * 255),
            255,
            0,
        };
    } else if (hue < 3 * sixth) {
        return {
            0,
            255,
            static_cast<uint8_t>((hue - 2 * sixth) / sixth * 255),
        };
    } else if (hue < 4 * sixth) {
        return {
            0,
            static_cast<uint8_t>(255 - ((hue - 3 * sixth) / sixth) * 255),
            255,
        };
    } else if (hue < 5 * sixth) {
        return {
            static_cast<uint8_t>((hue - 4 * sixth) / sixth * 255),
            0,
            255,
        };
    } else {
        return {
            255,
            0,
            static_cast<uint8_t>(255 - ((hue - 5 * sixth) / sixth) * 255),
        };
    }
}

} // namespace raycaster
