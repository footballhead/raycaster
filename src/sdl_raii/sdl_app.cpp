#include "sdl_app.hpp"

#include <sdl_raii/sdl_assert.hpp>

#include <SDL.h>

namespace {

unsigned& get_init_reference_count()
{
    static unsigned ref_count = 0;
    return ref_count;
}

} // namespace

namespace sdl {

sdl_app::sdl_app()
{
    if (get_init_reference_count() == 0) {
        SDL_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    }
    ++get_init_reference_count();
}

sdl_app::~sdl_app()
{
    --get_init_reference_count();
    if (get_init_reference_count() == 0) {
        SDL_Quit();
    }
}

} // namespace sdl
