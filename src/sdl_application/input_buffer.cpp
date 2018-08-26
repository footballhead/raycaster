#include <sdl_application/input_buffer.hpp>

#include <SDL.h>

namespace sdl_app {

void input_buffer::poll_events()
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

SDL_Point input_buffer::get_mouse_position() const
{
    SDL_Point mouse_pos{0, 0};
    SDL_GetMouseState(&mouse_pos.x, &mouse_pos.y);
    return mouse_pos;
}

} // namespace sdl_app
