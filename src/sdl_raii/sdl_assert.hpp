#pragma once

#include <SDL.h>

#include <stdexcept>

#define SDL_CHECK(pred)                                                        \
    if (!(pred)) {                                                             \
        SDL_Log("%s - %s", __func__, SDL_GetError());                          \
        throw std::runtime_error(SDL_GetError());                              \
    }
