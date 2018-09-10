#pragma once

#include <sdl_application/asset_store.hpp>
#include <sdl_application/input_buffer.hpp>
#include <sdl_application/screenshot.hpp>
#include <sdl_application/sdl_mymath.hpp>
#include <sdl_application/surface_manipulation.hpp>

#include <sdl_raii/sdl_raii.hpp>

#include <memory>

namespace sdl_app {

class sdl_application {
public:
    explicit sdl_application(std::shared_ptr<sdl::sdl_init> sdl,
        sdl::window window, std::unique_ptr<input_buffer> input_buffer,
        std::unique_ptr<asset_store> assets);

    sdl_application(sdl_application const& other) = delete;
    sdl_application(sdl_application&& other) = delete;
    sdl_application& operator=(sdl_application& other) = delete;
    sdl_application& operator=(sdl_application&& other) = delete;

    int exec();

    void quit();

protected:
    virtual void unhandled_event(SDL_Event const& event) = 0;
    virtual void update() = 0;
    virtual void render() = 0;

    SDL_Window* get_window();
    SDL_Surface* get_framebuffer();
    input_buffer& get_input_buffer();
    asset_store& get_asset_store();

private:
    std::shared_ptr<sdl::sdl_init> _sdl;
    sdl::window _window;
    sdl::surface _framebuffer;
    std::unique_ptr<input_buffer> _input_buffer;
    std::unique_ptr<asset_store> _asset_store;

    bool _running = false;
};

} // namespace sdl_app
