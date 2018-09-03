#pragma once

#include <mymath/mymath.hpp>

#include <SDL.h>

#include <array>
#include <functional>

namespace sdl_app {

class input_buffer {
public:
    void poll_events(
        std::function<void(SDL_Event const&)> unhandled_event = {});

    bool is_pressed(SDL_Scancode scancode) const;

    bool is_hit(SDL_Scancode scancode);

    bool is_quit() const;

    mymath::point2i get_mouse_position() const;

private:
    std::array<bool, SDL_NUM_SCANCODES> _key_pressed{};
    bool _quit;
};

} // namespace sdl_app
