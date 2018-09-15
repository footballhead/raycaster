#pragma once

#include <mymath/mymath.hpp>

#include <vector>

namespace raycaster {

struct wall {
    mymath::line2f data;
    unsigned int texture;
};

struct sprite {
    mymath::point2f data;
    unsigned int texture;
};

struct level {
    std::vector<wall> walls;
    std::vector<sprite> sprites;
};

} // namespace raycaster
