#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdexcept>
#include <string>

int main(int /*argc*/, char** /*argv*/)
{
    auto L = luaL_newstate();
    if (!L) {
        throw std::runtime_error{"luaL_newstate: not enough memory"};
    }

    luaL_openlibs(L);

    if (luaL_dostring(L, R"(print("hello world"))")) {
        auto const lua_error = std::string{lua_tostring(L, lua_gettop(L))};
        lua_close(L);
        throw std::runtime_error{lua_error};
    }

    lua_close(L);
    return 0;
}
