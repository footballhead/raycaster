#pragma once

#include <mymath/linear_algebra.hpp>

#include <cmath>

namespace mymath {

//
// vector
//

template <typename T> struct vector2 {
    T dir;
    T mag;
};

using vector2f = vector2<float>;

//
// point
//

template <typename T> struct point2 {
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

using point2f = point2<float>;
using point2i = point2<int>;

template <typename CastType, typename InputType>
point2<CastType> make_point(InputType a, InputType b)
{
    return point2<CastType>{static_cast<CastType>(a), static_cast<CastType>(b)};
}

//
// line
//

template <typename T> struct line2 {
    point2<T> start;
    point2<T> end;
};

template <typename T>
point2<T> linear_interpolate(line2<T> const& line, float t)
{
    return linear_interpolate(line.start, line.end, t);
}

using line2f = line2<float>;

//
// extent
//

template <typename T> struct extent2 {
    T w;
    T h;
};

using extent2i = extent2<int>;

} // namespace mymath
