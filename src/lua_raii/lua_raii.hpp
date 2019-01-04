#pragma once

// Since Lua is compiled as C, these public headers must be treated like C.
extern "C" {
#include <lauxlib.h>
#include <lua.h>
#include <lualib.h>
}

#include <memory>
#include <stdexcept>
#include <string>

namespace lua {

namespace detail {

struct state_deleter {
    void operator()(lua_State* L) const { lua_close(L); }
};

} // namespace detail

using state = std::unique_ptr<lua_State, detail::state_deleter>;

//
// window
//

/// Wrap an existing SDL_Window.
inline state make_state(lua_State* L)
{
    if (!L) {
        throw std::runtime_error{"L is nullptr!"};
    }
    return lua::state{L, detail::state_deleter{}};
}

/// Convenience helper with that uses lauxlib to newstate and openlibs
inline state make_state()
{
    auto L = luaL_newstate();
    if (!L) {
        throw std::runtime_error{"luaL_newstate failed!"};
    }
    luaL_openlibs(L);
    return lua::state{L, detail::state_deleter{}};
}

/// The base template only exists to be specialized... but works well for
/// userdata (pointers)
template <typename T> inline T to(lua_State* L, int pos = -1)
{
    auto type = lua_type(L, pos);
    if (type != LUA_TLIGHTUSERDATA && type != LUA_TUSERDATA) {
        throw std::runtime_error{"Value on stack is not userdata!"};
    }
    return static_cast<T>(lua_touserdata(L, pos));
}

template <> inline int to<int>(lua_State* L, int pos)
{
    if (lua_type(L, pos) != LUA_TNUMBER) {
        throw std::runtime_error{"Value on stack is not a number!"};
    }
    return static_cast<int>(lua_tointeger(L, pos));
}

template <> inline unsigned to<unsigned>(lua_State* L, int pos)
{
    if (lua_type(L, pos) != LUA_TNUMBER) {
        throw std::runtime_error{"Value on stack is not a number!"};
    }
    return static_cast<int>(lua_tointeger(L, pos));
}

template <> inline float to<float>(lua_State* L, int pos)
{
    if (lua_type(L, pos) != LUA_TNUMBER) {
        throw std::runtime_error{"Value on stack is not a number!"};
    }
    return static_cast<float>(lua_tonumber(L, pos));
}

template <> inline std::string to<std::string>(lua_State* L, int pos)
{
    if (lua_type(L, pos) != LUA_TSTRING) {
        throw std::runtime_error{"Value on stack is not a string!"};
    }

    auto cstr = lua_tostring(L, pos);
    if (!cstr) {
        throw std::runtime_error{"lua_tostring returned nullptr!"};
    }

    return std::string{cstr};
}

template <> inline char const* to<char const*>(lua_State* L, int pos)
{
    if (lua_type(L, pos) != LUA_TSTRING) {
        throw std::runtime_error{"Value on stack is not a string!"};
    }

    return lua_tostring(L, pos);
}

} // namespace lua
