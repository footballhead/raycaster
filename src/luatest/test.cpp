#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#include <stdexcept>

int main(int /*argc*/, char** /*argv*/)
{
    auto L = luaL_newstate();
    if (!L) {
        throw std::runtime_error{"luaL_newstate: not enough memory"};
    }

    luaL_openlibs(L);

    if (!luaL_dostring(L, "io.stdout:write(5)")) {
        lua_close(L);
        throw std::runtime_error{"luaL_dostring: failed!"};
    }

    lua_close(L);
    return 0;
}
