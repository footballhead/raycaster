#pragma once

#include <lua_raii/lua_raii.hpp>
#include <mymath/mymath.hpp>

#include <memory>
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
std::unique_ptr<level> load_level(std::string const& filename, lua_State* L);

} // namespace raycaster
