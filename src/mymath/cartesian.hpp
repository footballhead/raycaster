#pragma once

#include <mymath/linear_algebra.hpp>

#include <cmath>
#include <stdexcept>
#include <type_traits>

namespace mymath {

//
// vector
//

template <typename T> struct vector2 {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value,
        "vector2 requires int/fp type.");

    T dir;
    T mag;
};

using vector2f = vector2<float>;

//
// point
//

template <typename T> struct point2 {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value,
        "point2 requires int/fp type.");

    T x;
    T y;

    //
    // point operations
    //

    point2<T>& operator+=(point2<T> const& rhs)
    {
        x += rhs.x;
        y += rhs.y;
        return *this;
    }

    point2<T>& operator-=(point2<T> const& rhs)
    {
        x -= rhs.x;
        y -= rhs.y;
        return *this;
    }

    point2<T>& operator*=(point2<T> const& rhs)
    {
        x *= rhs.x;
        y *= rhs.y;
        return *this;
    }

    //
    // vector operations
    //

    point2<T>& operator+=(vector2<T> const& rhs)
    {
        x += std::cos(rhs.dir) * rhs.mag;
        y += std::sin(rhs.dir) * rhs.mag;
        return *this;
    }

    point2<T>& operator-=(vector2<T> const& rhs)
    {
        x -= std::cos(rhs.dir) * rhs.mag;
        y -= std::sin(rhs.dir) * rhs.mag;
        return *this;
    }

    //
    // scalar operations
    //

    point2<T>& operator*=(float rhs)
    {
        x *= rhs;
        y *= rhs;
        return *this;
    }

    //
    // generic operations
    //

    float slope() const { return y / static_cast<float>(x); }
    float slope_inverse() const { return x / static_cast<float>(y); }
};

//
// point-point operations
//

template <typename T> point2<T> operator+(point2<T> lhs, point2<T> const& rhs)
{
    lhs += rhs;
    return lhs;
}

template <typename T> point2<T> operator-(point2<T> lhs, point2<T> const& rhs)
{
    lhs -= rhs;
    return lhs;
}

template <typename T> point2<T> operator*(point2<T> lhs, point2<T> const& rhs)
{
    lhs *= rhs;
    return lhs;
}

//
// point-vector operations
//

template <typename T> point2<T> operator+(point2<T> lhs, vector2<T> const& rhs)
{
    lhs += rhs;
    return lhs;
}

template <typename T> point2<T> operator-(point2<T> lhs, vector2<T> const& rhs)
{
    lhs -= rhs;
    return lhs;
}

//
// point-scalar operations
//

template <typename T> point2<T> operator*(point2<T> lhs, float rhs)
{
    lhs *= rhs;
    return lhs;
}

// point comparison operations

template <typename T>
bool operator==(point2<T> const& lhs, point2<T> const& rhs)
{
    return lhs.x == rhs.x && lhs.y == rhs.y;
}

//
// point generic operations
//

using point2f = point2<float>;
using point2i = point2<int>;

template <typename CastType, typename InputType>
point2<CastType> make_point(InputType a, InputType b)
{
    return point2<CastType>{static_cast<CastType>(a), static_cast<CastType>(b)};
}

template <typename CastType, typename InputType>
point2<CastType> point_cast(point2<InputType> a)
{
    return point2<CastType>{
        static_cast<CastType>(a.x), static_cast<CastType>(a.y)};
}

template <typename T> point2<T> abs(point2<T> const& p)
{
    return {std::abs(p.x), std::abs(p.y)};
}

// To disambiguate when T is int
constexpr point2<int> abs(point2<int> const& p)
{
    return {p.x < 0 ? -p.x : p.x, p.y < 0 ? -p.y : p.y};
}

template <typename T>
point2<T> clamp(point2<T> const& val, point2<T> const& lo, point2<T> const& hi)
{
    return {clamp(val.x, lo.x, hi.x), clamp(val.y, lo.y, hi.y)};
}

template <typename T> point2<T> floor(point2<T> const& p)
{
    return {std::floor(p.x), std::floor(p.y)};
}

template <typename T> point2<T> ceil(point2<T> const& p)
{
    return {std::ceil(p.x), std::ceil(p.y)};
}

template <typename T> point2<T> remainder(point2<T> const& p)
{
    return point_cast<T>(p - floor(p));
}

template <typename T>
point2<T> wrap(point2<T> const& p, point2<T> const& lo, point2<T> const& hi)
{
    return {wrap(p.x, lo.x, hi.x), wrap(p.y, lo.y, hi.y)};
}

//
// rectangle
//

template <typename T> struct rectangle2 {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value,
        "rectangle2 requires int/fp type.");

    point2<T> tl;
    point2<T> br;

    bool contains(point2<T> const& p) const
    {
        // This assumes tl is indeed top-left and br is indeed bottom-right
        return p.x >= tl.x && p.x <= br.x && p.y >= tl.y && p.y <= br.y;
    }
};

//
// line
//

template <typename T> struct line2 {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value,
        "line2 requires int/fp type.");

    point2<T> start;
    point2<T> end;

    bool is_vertical() const { return end.x == start.x; }

    bool is_horizontal() const { return end.y == start.y; }

    float slope() const
    {
        if (is_vertical()) {
            throw std::runtime_error("Vertical line has no slope!");
        }
        return (end.y - start.y) / static_cast<float>(end.x - start.x);
    }

    float y_intercept() const
    {
        if (is_vertical()) {
            throw std::runtime_error("Vertical line has no y-intercept!");
        }
        // Since both start and end define the line, either can be used
        return end.y - slope() * end.x;
    }

    rectangle2<T> get_bounding_box() const
    {
        // Point order for a line isn't guaranteed but we need a bounding box
        // where our comparisons will always work
        auto const bb_start_x = std::min(start.x, end.x);
        auto const bb_start_y = std::min(start.y, end.y);
        auto const bb_end_x = std::max(start.x, end.x);
        auto const bb_end_y = std::max(start.y, end.y);
        return {{bb_start_x, bb_start_y}, {bb_end_x, bb_end_y}};
    }

    float length() const
    {
        auto const a = start.x - end.x;
        auto const b = start.y - end.y;
        return std::sqrt(a * a + b * b);
    }
};

template <typename T>
point2<T> linear_interpolate(line2<T> const& line, float t)
{
    return linear_interpolate(line.start, line.end, t);
}

using line2f = line2<float>;
using line2i = line2<int>;


//
// extent
//

template <typename T> struct extent2 {
    static_assert(
        std::is_integral<T>::value || std::is_floating_point<T>::value,
        "extent2 requires int/fp type.");

    T w;
    T h;
};

using extent2i = extent2<int>;

} // namespace mymath
