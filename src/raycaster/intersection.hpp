/// @file intersection.hpp
/// @brief This is an extension of mymath that operates on certain template
/// specializations.

#pragma once

#include <mymath/mymath.hpp>

namespace raycaster {

/// Find the intersection point of two lines.
///
/// @param a A 2D line
/// @param b Another 2D line
/// @param out If an intersection is found, this will be modified to contain
/// that collision point
/// @param t If an intersection is found, this will be modified to contain the
/// interpolating factor of the collision point along line b
/// @return true if an intersection exists, false otherwise
bool find_intersection(mymath::line2f const& a, mymath::line2f const& b,
    mymath::point2f& out, float& t);

} // namespace raycaster
