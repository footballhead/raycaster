#pragma once

#include <SDL.h>

namespace sdl {

/// RAII wrapper around static SDL initialization.
class sdl_init {
public:
    /// Initialize SDL (if not done already)
    sdl_init();

    /// Shut down SDL (if no one else is using it)
    ~sdl_init();
};

} // namespace sdl

