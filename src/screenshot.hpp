#pragma once

#include <SDL.h>

#include <string>

namespace sdl {

/// @brief Take a screenshot and save it to disk.
/// @param renderer The SDL renderer to take a screenshot of.
/// @param filename The file to save to.
/// @returns true if screenshot is successfully saved
/// @returns false on error
bool save_screenshot(SDL_Renderer* renderer, const char* filename);

} // namespace sdl
