#include "sdl_app.hpp"

#include <SDL.h>

#include <stdexcept>

unsigned sdl_app::m_init_ref = 0;

sdl_app::sdl_app()
{
    if (m_init_ref == 0) {
        if (SDL_Init(SDL_INIT_EVERYTHING) != 0) {
            throw std::runtime_error{SDL_GetError()};
        }
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
