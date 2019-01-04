#include "level.hpp"

#include <lua_raii/lua_raii.hpp>

#include <SDL.h>

#include <fstream>
#include <sstream>
#include <stdexcept>

using namespace mymath;

namespace raycaster {

std::unique_ptr<level> load_level(std::string const& filename, lua_State* L)
{
    if (luaL_dofile(L, filename.c_str())) {
        throw std::runtime_error{lua::to<std::string>(L)};
    }

    auto new_level = std::make_unique<level>();

    //
    // player_start
    //

    if (lua_getfield(L, 1, "player_start") != LUA_TTABLE
        || lua_getfield(L, 2, "x") != LUA_TNUMBER
        || lua_getfield(L, 2, "y") != LUA_TNUMBER) {
        throw std::runtime_error{"Bad or missing player_start"};
    }
    new_level->player_start
        = point2f{lua::to<float>(L, -2), lua::to<float>(L, -1)};
    lua_pop(L, 3); // y, x, player_start

    //
    // walls
    //

    if (lua_getfield(L, 1, "walls") != LUA_TTABLE) {
        throw std::runtime_error{"Bad or missing walls"};
    }
    auto const walls_length = luaL_len(L, 2);
    for (auto i = 1; i <= walls_length; ++i) {
        if (lua_geti(L, 2, i) != LUA_TTABLE
            || lua_getfield(L, 3, "x1") != LUA_TNUMBER
            || lua_getfield(L, 3, "y1") != LUA_TNUMBER
            || lua_getfield(L, 3, "x2") != LUA_TNUMBER
            || lua_getfield(L, 3, "y2") != LUA_TNUMBER
            || lua_getfield(L, 3, "texid") != LUA_TNUMBER) {
            throw std::runtime_error{"Bad or missing wall entry"};
        }

        new_level->walls.push_back(wall{
            line2f{
                {lua::to<float>(L, -5), lua::to<float>(L, -4)},
                {lua::to<float>(L, -3), lua::to<float>(L, -2)},
            },
            lua::to<unsigned>(L, -1),
        });
        lua_pop(L, 6); // texid, y2, x2, y2, y1, walls[i]
    }
    lua_pop(L, 1); // walls

    //
    // sprites
    //

    if (lua_getfield(L, 1, "sprites") != LUA_TTABLE) {
        throw std::runtime_error{"Bad or missing sprites"};
    }
    auto const spritres_length = luaL_len(L, 2);
    for (auto i = 1; i <= spritres_length; ++i) {
        if (lua_geti(L, 2, i) != LUA_TTABLE
            || lua_getfield(L, 3, "x") != LUA_TNUMBER
            || lua_getfield(L, 3, "y") != LUA_TNUMBER
            || lua_getfield(L, 3, "texid") != LUA_TNUMBER) {
            throw std::runtime_error{"Bad or missing sprites entry"};
        }

        new_level->sprites.push_back(sprite{
            {lua::to<float>(L, -3), lua::to<float>(L, -2)},
            lua::to<unsigned>(L, -1),
        });
        lua_pop(L, 4); // texid, y, x, sprites[i]
    }
    lua_pop(L, 1); // sprites

    lua_pop(L, 1); // from dofile

    return new_level;
}

} // namespace raycaster