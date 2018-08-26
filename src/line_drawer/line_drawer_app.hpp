#pragma once

#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

namespace line_drawer {

class line_drawer_app : public sdl_app::sdl_application {
public:
    explicit line_drawer_app(std::shared_ptr<sdl::sdl_init> sdl,
        sdl::window window, sdl::shared_renderer renderer,
        std::unique_ptr<sdl_app::input_buffer> input);

protected:
    void update() override;
    void render() override;
};

} // namespace line_drawer
