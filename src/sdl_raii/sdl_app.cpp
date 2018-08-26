#include "sdl_app.hpp"

#include <sdl_raii/sdl_assert.hpp>

#include <SDL.h>

namespace sdl {

unsigned sdl_app::m_init_ref = 0;

sdl_app::sdl_app()
{
    if (m_init_ref == 0) {
        SDL_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    }
    ++m_init_ref;
}

sdl_app::~sdl_app()
{
    --m_init_ref;
    if (m_init_ref == 0) {
        SDL_Quit();
    }
}

} // namespace sdl
