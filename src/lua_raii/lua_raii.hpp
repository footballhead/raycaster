#pragma once

// Since Lua is compiled as C, these public headers must be treated like C.
extern "C" {
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
}

#include <memory>
#include <stdexcept>

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

} // namespace sdl
