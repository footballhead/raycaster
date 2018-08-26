#pragma once

#include <SDL.h>

namespace sdl {

//
// sdl_app
//

/// RAII wrapper around static SDL initialization.
class sdl_app {
public:
    /// Initialize SDL (if not done already)
    sdl_app();

    /// Shut down SDL (if no one else is using it)
    ~sdl_app();

private:
    static unsigned m_init_ref;
};

} // namespace sdl

