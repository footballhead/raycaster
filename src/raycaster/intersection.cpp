#include "intersection.hpp"

using namespace mymath;

namespace {

bool find_intersection_vertical(
    line2f const& vertical, line2f const& b, point2f& out)
{
    auto const b_low_x = std::min(b.start.x, b.end.x);
    auto const b_hi_x = std::max(b.start.x, b.end.x);
    if (vertical.start.x < b_low_x || vertical.start.x > b_hi_x) {
        return false;
    }

    auto const x = static_cast<float>(vertical.start.x);
    auto const y = b.slope() * x + b.y_intercept();

    auto const candidate = point2f{x, y};

    // The point must lie within box bounding boxes to be on the line segment
    auto const a_bb = vertical.get_bounding_box();
    auto const b_bb = b.get_bounding_box();
    if (!a_bb.contains(candidate) || !b_bb.contains(candidate)) {
        return false;
    }

    out = candidate;
    return true;
}

bool find_intersection_horizontal(
    line2f const& horizontal, line2f const& b, point2f& out)
{
    auto const b_low_y = std::min(b.start.y, b.end.y);
    auto const b_hi_y = std::max(b.start.y, b.end.y);
    if (horizontal.start.y < b_low_y || horizontal.start.y > b_hi_y) {
        return false;
    }

    auto const y = static_cast<float>(horizontal.start.y);
    auto const x = (y - b.y_intercept()) / b.slope();

    auto const candidate = point2f{x, y};

    // The point must lie within box bounding boxes to be on the line segment
    auto const a_bb = horizontal.get_bounding_box();
    auto const b_bb = b.get_bounding_box();
    if (!a_bb.contains(candidate) || !b_bb.contains(candidate)) {
        return false;
    }

    out = candidate;
    return true;
}

} // namespace

namespace raycaster {

bool find_intersection(line2f const& a, line2f const& b, point2f& out)
{
    if (a.is_vertical() && b.is_vertical()) {
        return false;
    }

    if (a.is_vertical()) {
        return find_intersection_vertical(a, b, out);
    } else if (b.is_vertical()) {
        return find_intersection_vertical(b, a, out);
    }

    if (a.is_horizontal()) {
        return find_intersection_horizontal(a, b, out);
    } else if (b.is_horizontal()) {
        return find_intersection_horizontal(b, a, out);
    }

    if (close_enough(a.slope(), b.slope())) {
        // Either 0 or infinitely many
        return false;
    }

    // ... else neither are vertical so proceed as usual

    auto const x
        = (b.y_intercept() - a.y_intercept()) / (a.slope() - b.slope());
    auto const y = a.slope() * x + a.y_intercept();

    auto const candidate = point2f{x, y};

    // The point must lie within box bounding boxes to be on the line segment
    auto const a_bb = a.get_bounding_box();
    auto const b_bb = b.get_bounding_box();
    if (!a_bb.contains(candidate) || !b_bb.contains(candidate)) {
        return false;
    }

    out = candidate;
    return true;
}

} // namespace raycaster
