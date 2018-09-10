#include <sdl_application/sdl_application.hpp>

#include <sdl_raii/sdl_raii.hpp>

#include <SDL.h>

#include <stdexcept>

namespace sdl_app {

sdl_application::sdl_application(std::shared_ptr<sdl::sdl_init> sdl,
    sdl::window window, std::unique_ptr<input_buffer> input_buffer,
    std::unique_ptr<asset_store> assets)
: _sdl{std::move(sdl)}
, _window{std::move(window)}
, _input_buffer{std::move(input_buffer)}
, _asset_store{std::move(assets)}
{
    _framebuffer = sdl::make_surface(SDL_GetWindowSurface(_window.get()));
}

int sdl_application::exec()
{
    if (_running) {
        SDL_Log("Already running");
        throw std::runtime_error("Already running");
    }

    _running = true;
    while (_running) {
        _input_buffer->poll_events(
            [this](SDL_Event const& event) { unhandled_event(event); });
        update();

        render();
        SDL_CHECK(SDL_UpdateWindowSurface(_window.get()) == 0);

        // Yield to OS, don't hog the CPU.
        SDL_Delay(1);
    }

    return 0;
}

void sdl_application::quit() { _running = false; }

SDL_Window* sdl_application::get_window() { return _window.get(); }

SDL_Surface* sdl_application::get_framebuffer() { return _framebuffer.get(); }

input_buffer& sdl_application::get_input_buffer() { return *_input_buffer; }

asset_store& sdl_application::get_asset_store() { return *_asset_store; }

} // namespace sdl_app
