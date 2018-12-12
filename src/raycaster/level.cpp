#include "level.hpp"

#include <SDL.h>

#include <fstream>
#include <sstream>

using namespace mymath;

namespace raycaster {

level load_level(std::string const& filename)
{
    auto in = std::ifstream{filename};

    auto new_level = level{};

    std::string line;
    while (std::getline(in, line).good()) {
        if (line.empty()) {
            continue;
        }
        if (*line.begin() == '#') {
            continue;
        }

        auto ss = std::istringstream{line};

        std::string construct;
        ss >> construct;
        if (construct == "wall") {
            auto startx = 0.f;
            auto starty = 0.f;
            auto endx = 0.f;
            auto endy = 0.f;
            auto texid = 0u;

            ss >> startx;
            ss >> starty;
            ss >> endx;
            ss >> endy;
            ss >> texid;

            new_level.walls.push_back(
                wall{line2f{{startx, starty}, {endx, endy}}, texid});
        } else if (construct == "sprite") {
            auto x = 0.f;
            auto y = 0.f;
            auto texid = 0u;

            ss >> x;
            ss >> y;
            ss >> texid;

            new_level.sprites.push_back(sprite{point2f{x, y}, texid});
        } else {
            SDL_Log("Ignoring unknown construct: %s", construct.c_str());
            continue;
        }
    }

    return new_level;
}

} // namespace raycaster