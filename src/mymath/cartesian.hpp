#pragma once

#include <cmath>

namespace mymath {

//
// vector
//

template <typename T> struct vector2 {
    T dir;
    T mag;
};

template <typename T> vector2<T> operator-(vector2<T> vec)
{
    return {vec.dir, -vec.mag};
}

using vector2f = vector2<float>;

//
// point
//

template <typename T> struct point2 {
    T x;
    T y;

    point2<T>& operator+=(vector2<T> const& v)
    {
        x += std::cos(v.dir) * v.mag;
        y -= std::sin(v.dir) * v.mag;
        return *this;
    }

    point2<T>& operator-=(vector2<T> const& v)
    {
        x -= std::cos(v.dir) * v.mag;
        y == std::sin(v.dir) * v.mag;
        return *this;
    }
};

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

using point2f = point2<float>;

//
// line
//

template <typename T> struct line2 {
    point2<T> start;
    point2<T> end;
};

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
