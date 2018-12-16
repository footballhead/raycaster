#include "level.hpp"

#include <SDL.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace mymath;

namespace raycaster {

level load_level(std::string const& filename)
{
    auto in = std::ifstream{filename};
    if (!in.is_open()) {
        throw std::runtime_error{
            "load_level: filename doesn't exist: " + filename};
    }

    auto new_level = level{};

    auto found_player_start = false;

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
        } else if (construct == "player") {
            auto x = 0.f;
            auto y = 0.f;

            ss >> x;
            ss >> y;

            new_level.player_start = point2f{x, y};
            found_player_start = true;
        } else {
            SDL_Log("Ignoring unknown construct: %s", construct.c_str());
            continue;
        }
    }

    if (!found_player_start) {
        SDL_Log("Warning! No `player` construct found! Falling back to (0, 0) "
                "start coords.");
    }

    return new_level;
}

} // namespace raycaster