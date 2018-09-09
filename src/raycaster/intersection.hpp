#pragma once

#include <mymath/mymath.hpp>

namespace raycaster {

bool find_intersection(mymath::line2f const& a, mymath::line2f const& b,
    mymath::point2f& out, float& t);

} // namespace raycaster
