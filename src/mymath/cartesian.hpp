#pragma once

#include <mymath/linear_algebra.hpp>

#include <cmath>
#include <stdexcept>

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

    float slope_inverse() const
    {
        if (is_horizontal()) {
            throw std::runtime_error("Horiontal line has no inverse slopte");
        }
        return (end.x - start.x) / static_cast<float>(end.y - start.y);
    }

    float x_intercept() const
    {
        if (is_horizontal()) {
            throw std::runtime_error("Horiontal line has no x-intercept!");
        }
        // Since both start and end define the line, either can be used
        return end.y - slope() * end.x;
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
};

template <typename T>
point2<T> linear_interpolate(line2<T> const& line, float t)
{
    return linear_interpolate(line.start, line.end, t);
}

using line2f = line2<float>;
using line2i = line2<int>;

namespace detail {

template <typename T>
bool find_intersection_vertical(line2<T> const& vertical, line2<T> const& b, point2<T>& out)
{
    auto const b_low_x = std::min(b.start.x, b.end.x);
    auto const b_hi_x = std::max(b.start.x, b.end.x);
    if (vertical.start.x < b_low_x || vertical.start.x > b_hi_x) {
        return false;
    }

    auto const x = static_cast<float>(vertical.start.x);
    auto const y = b.slope() * x + b.y_intercept();

    auto const candidate = make_point<T>(x, y);

    // The point must lie within box bounding boxes to be on the line segment
    auto const a_bb = vertical.get_bounding_box();
    auto const b_bb = b.get_bounding_box();
    if (!a_bb.contains(candidate) || !b_bb.contains(candidate)) {
        return false;
    }

    out = candidate;
    return true;
}

}

template <typename T>
bool find_intersection(line2<T> const& a, line2<T> const& b, point2<T>& out)
{
    if (a.is_vertical() && b.is_vertical()) {
        return false;
    }

    if (a.is_vertical()) {
        return detail::find_intersection_vertical(a, b, out);
    } else if (b.is_vertical()) {
        return detail::find_intersection_vertical(b, a, out);
    }

    if (close_enough(a.slope(), b.slope())) {
        // Either 0 or infinitely many
        return false;
    }

    // ... else neither are vertical so proceed as usual

    auto const x
        = (b.y_intercept() - a.y_intercept()) / (a.slope() - b.slope());
    auto const y = a.slope() * x + a.y_intercept();

    auto const candidate = make_point<T>(x, y);

    // The point must lie within box bounding boxes to be on the line segment
    auto const a_bb = a.get_bounding_box();
    auto const b_bb = b.get_bounding_box();
    if (!a_bb.contains(candidate) || !b_bb.contains(candidate)) {
        return false;
    }

    out = candidate;
    return true;
}

//
// extent
//

template <typename T> struct extent2 {
    T w;
    T h;
};

using extent2i = extent2<int>;

} // namespace mymath
