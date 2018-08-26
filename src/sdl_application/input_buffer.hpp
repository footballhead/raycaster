#pragma once

#include <SDL.h>

#include <array>

namespace sdl_app {

class input_buffer {
public:
    void poll_events();

    bool is_pressed(SDL_Scancode scancode) const;

    bool is_hit(SDL_Scancode scancode);

    bool is_quit() const;

private:
    std::array<bool, SDL_NUM_SCANCODES> _key_pressed{};
    bool _quit;
};

} // namespace sdl_app
