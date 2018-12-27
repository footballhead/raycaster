#include "intersection.hpp"

#include <SDL.h>

using namespace mymath;

namespace raycaster {

bool find_intersection(line2f const& a, line2f const& b, point2f& out, float& t)
{
    // There are a bunch of special cases which we could account for up at the
    // top before we do any math, but they make this function harder to follow.
    // For the interim, make this one function really ugly.

    auto candidate = point2f{0.f, 0.f};

    // This entire function is based on using y=mx+b to find the intersection.
    // Since this requires a slope, there are a bunch of exceptions for vertical
    // lines (infinite slope) and horizontal lines (no slope).

    if (a.is_vertical()) {
        if (b.is_vertical()) {
            // Both have infinite slope, which means 0 or infinite crossings
            return false;
        }

        if (b.is_horizontal()) {
            // This is probably the easiest case. If there is a crossing, it can
            // only be in one spot since `a`, the vertical line, has constant
            // `x` and `b`, the horizontal line, has constant `y`.
            candidate.x = static_cast<float>(a.start.x);
            candidate.y = static_cast<float>(b.start.y);
        } else {
            // Since `a` is vertical, that means `x` remains constant. If there
            // is an intersection, there is only one x value at which it can
            // happen. Then apply y=mx+b
            candidate.x = static_cast<float>(a.start.x);
            candidate.y = b.slope() * candidate.x + b.y_intercept();
        }
    } else if (b.is_vertical()) {
        // We know `a` is NOT vertical due to our place in the if statement

        // Like before, but swap `a` and `b`
        if (a.is_horizontal()) {
            candidate.x = static_cast<float>(b.start.x);
            candidate.y = static_cast<float>(a.start.y);
        } else {
            candidate.x = static_cast<float>(b.start.x);
            candidate.y = a.slope() * candidate.x + a.y_intercept();
        }
    } else if (a.is_horizontal()) {
        // We know `b` is NOT vertical due to our place in the if statement
        if (b.is_horizontal()) {
            // Both have same slope, which means 0 or infinite crossings
            return false;
        }

        // Since `a` is horizontal, then `y` is fixed. Work backwards from
        // `y=mx+b` to get `x=(y-b)/m`
        candidate.y = static_cast<float>(a.start.y);
        candidate.x = (candidate.y - b.y_intercept()) / b.slope();
    } else if (b.is_horizontal()) {
        // We know `a` is neither horizontal nor vertical due to the position
        // of this clause in the larger statement.

        // Like above, but reverse `a` and `b`.
        candidate.y = static_cast<float>(b.start.y);
        candidate.x = (candidate.y - a.y_intercept()) / a.slope();
    } else {
        // Since we assume both lines cross, that means they must share a `y`
        // and `x`. With `y=m_1x+b_1` and `y=m_2x+b_2`, this implies
        // `m_1x+b_1=m_2x+b_2`. Rearrange for `x`.
        candidate.x
            = (b.y_intercept() - a.y_intercept()) / (a.slope() - b.slope());
        candidate.y = a.slope() * candidate.x + a.y_intercept();
    }

    // We were assuming up to this point that the two lines did cross. Here, we
    // must validate that assumption. The point must lie within both bounding
    // boxes.
    auto const a_bb = a.get_bounding_box();
    auto const b_bb = b.get_bounding_box();
    if (!a_bb.contains(candidate) || !b_bb.contains(candidate)) {
        return false;
    }

    out = candidate;
    t = line2f{b.start, candidate}.length() / b.length();
    return true;
}

} // namespace raycaster
