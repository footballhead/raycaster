#pragma once

#include <mymath/mymath.hpp>

#include <string>
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
    mymath::point2f player_start;
};

// Doesn't go through the asset manager yet
level load_level(std::string const& filename);

} // namespace raycaster
