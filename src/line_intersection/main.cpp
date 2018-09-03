#include <SDL.h>

#include <mymath/mymath.hpp>
#include <sdl_application/asset_store.hpp>
#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <memory>

using namespace sdl_app;

class line_intersection_app : public sdl_application {
public:
    explicit line_intersection_app(std::shared_ptr<sdl::sdl_init> sdl,
        sdl::window window, sdl::shared_renderer renderer,
        std::unique_ptr<sdl_app::input_buffer> input)
    : sdl_application(std::move(sdl), std::move(window), std::move(renderer),
          std::move(input))
    {
    }

protected:
    void unhandled_event(SDL_Event const&) override {}
    void update() override
    {
        auto& input = get_input_buffer();
        if (input.is_quit() || input.is_hit(SDL_SCANCODE_ESCAPE)) {
            quit();
        }
    }
    void render() override {}
};

int main(int argc, char** argv)
{
    auto sdl = std::make_shared<sdl::sdl_init>();

    auto const window_title = "Line Intersection";
    SDL_Point const window_bounds{800, 600};
    auto window
        = sdl::make_window(window_title, window_bounds, SDL_WINDOW_RESIZABLE);

    auto renderer = sdl::shared_renderer{sdl::make_renderer(window.get())};

    auto input = std::make_unique<sdl_app::input_buffer>();

    line_intersection_app app{std::move(sdl), std::move(window),
        std::move(renderer), std::move(input)};
    try {
        return app.exec();
    } catch (const std::exception& e) {
        SDL_Log("EXCEPTION: %s", e.what());
    }

    return 1;
}