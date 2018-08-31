#include <sdl_application/sdl_application.hpp>

#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <stdexcept>

namespace sdl_app {

sdl_application::sdl_application(std::shared_ptr<sdl::sdl_init> sdl,
    sdl::window window, sdl::shared_renderer renderer,
    std::unique_ptr<input_buffer> input_buffer)
: _sdl{std::move(sdl)}
, _window{std::move(window)}
, _renderer{std::move(renderer)}
, _input_buffer{std::move(input_buffer)}
{
}

int sdl_application::exec()
{
    if (_running) {
        SDL_Log("Already running");
        throw std::runtime_error("Already running");
    }

    _running = true;
    while (_running) {
        _input_buffer->poll_events([this](SDL_Event const& event) {
            unhandled_event(event);
        });
        update();

        SDL_CHECK(SDL_SetRenderDrawColor(_renderer.get(), 0, 0, 0, 255) == 0);
        SDL_CHECK(SDL_RenderClear(_renderer.get()) == 0);
        render();
        SDL_RenderPresent(_renderer.get());

        // Yield to OS, don't hog the CPU.
        SDL_Delay(1);
    }

    return 0;
}

void sdl_application::quit() { _running = false; }

SDL_Renderer* sdl_application::get_renderer() { return _renderer.get(); }

input_buffer& sdl_application::get_input_buffer() { return *_input_buffer; }

} // namespace sdl_app
