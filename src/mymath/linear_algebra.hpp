#pragma once

#include <algorithm>
#include <cmath>

namespace mymath {

template <class T>
constexpr const T& clamp(const T& v, const T& lo, const T& hi)
{
    return std::min(std::max(v, lo), hi);
}

/// Linearly interpolate between two values.
///
/// @tparam T The type of the values to interpolate, must support addition,
/// subtraction, and multiplication.
/// @param a The value when t=0
/// @param b The value when t=1
/// @param t Value between [0, 1], used as the interpolation factor
/// @return `a - (a * t) + (b * t)`
template <typename T>
inline T linear_interpolate(const T& a, const T& b, float t)
{
    t = clamp(t, 0.f, 1.f);
    return a - (a * t) + (b * t);
}

template <typename T> inline int sgn(T val) { return val >= 0 ? 1 : -1; }

template <class T> constexpr T wrap(T v, const T& lo, const T& hi)
{
    while (v < lo) {
        v += hi;
    }

    while (v > hi) {
        v -= hi;
    }

    return v;
}

} // namespace mymath
