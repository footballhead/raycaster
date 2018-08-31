#include <sdl_application/input_buffer.hpp>

#include <SDL.h>

namespace sdl_app {

void input_buffer::poll_events(
    std::function<void(SDL_Event const&)> unhandled_event)
{
    SDL_Event evt;
    while (SDL_PollEvent(&evt)) {
        switch (evt.type) {
        case SDL_QUIT:
            _quit = true;
            break;
        case SDL_KEYDOWN:
            _key_pressed[evt.key.keysym.scancode] = true;
            break;
        case SDL_KEYUP:
            _key_pressed[evt.key.keysym.scancode] = false;
            break;
        default:
            if (unhandled_event) {
                unhandled_event(evt);
            }
            break;
        }
    }
}

bool input_buffer::is_pressed(SDL_Scancode scancode) const
{
    return _key_pressed[scancode];
}

bool input_buffer::is_hit(SDL_Scancode scancode)
{
    auto previous_state = _key_pressed[scancode];
    _key_pressed[scancode] = false;
    return previous_state;
}

bool input_buffer::is_quit() const { return _quit; }

} // namespace sdl_app
