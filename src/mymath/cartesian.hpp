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

using point2f = point2<float>;

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
