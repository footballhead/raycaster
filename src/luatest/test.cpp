#include <lua_raii/lua_raii.hpp>

#include <stdexcept>
#include <string>

int main(int /*argc*/, char** /*argv*/)
{
    auto L = lua::make_state();

    if (luaL_dostring(L.get(), R"(print("hello world"))")) {
        auto const lua_error
            = std::string{lua_tostring(L.get(), lua_gettop(L.get()))};
        throw std::runtime_error{lua_error};
    }

    return 0;
}
