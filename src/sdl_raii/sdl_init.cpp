#include <sdl_raii/sdl_init.hpp>

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

sdl_init::sdl_init()
{
    if (get_init_reference_count() == 0) {
        SDL_CHECK(SDL_Init(SDL_INIT_EVERYTHING) == 0);
    }
    ++get_init_reference_count();
}

sdl_init::~sdl_init()
{
    --get_init_reference_count();
    if (get_init_reference_count() == 0) {
        SDL_Quit();
    }
}

} // namespace sdl
