#pragma once

#include <algorithm>
#include <cstdint>

namespace mycolor {

struct color {
    uint8_t r;
    uint8_t g;
    uint8_t b;
};

namespace constants {
constexpr color black{0, 0, 0};
constexpr color dark_gray{64, 64, 64};
constexpr color gray{128, 128, 128};
constexpr color light_gray{192, 192, 192};
constexpr color white{255, 255, 255};

constexpr color red{255, 0, 0};
}

constexpr color linear_interpolate(color const& a, color const& b, float t)
{
    t = std::max(std::min(t, 1.f), 0.f);
    return {
        static_cast<uint8_t>(a.r - (a.r * t) + (b.r * t)),
        static_cast<uint8_t>(a.g - (a.g * t) + (b.g * t)),
        static_cast<uint8_t>(a.b - (a.b * t) + (b.b * t)),
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

} // namespace mycolor
