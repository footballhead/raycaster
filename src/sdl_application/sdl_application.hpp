#pragma once

#include <sdl_application/input_buffer.hpp>

#include <sdl_raii/sdl_raii.hpp>

#include <memory>

namespace sdl_app {

class sdl_application {
public:
    explicit sdl_application(std::shared_ptr<sdl::sdl_init> sdl,
        sdl::window window, sdl::shared_renderer renderer,
        std::unique_ptr<input_buffer> input_buffer);

    sdl_application(sdl_application const& other) = delete;
    sdl_application(sdl_application&& other) = delete;
    sdl_application& operator=(sdl_application& other) = delete;
    sdl_application& operator=(sdl_application&& other) = delete;

    int exec();

    void quit();

protected:
    virtual void update() = 0;
    virtual void render() = 0;

    SDL_Renderer* get_renderer();
    input_buffer& get_input_buffer();

private:
    std::shared_ptr<sdl::sdl_init> _sdl;
    sdl::window _window;
    sdl::shared_renderer _renderer;
    std::unique_ptr<input_buffer> _input_buffer;

    bool _running = false;
};

} // namespace sdl_app
