#include <SDL.h>

#include <mycolor/mycolor.hpp>
#include <mymath/mymath.hpp>
#include <sdl_application/sdl_application.hpp>
#include <sdl_raii/sdl_raii.hpp>

#include <memory>

using namespace mycolor;
using namespace mymath;
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

        _mouse_line.end = input.get_mouse_position();
    }

    void render() override
    {
        auto renderer = get_renderer();

        point2i cross_point{0, 0};
        auto const did_cross
            = find_intersection(_mouse_line, _line, cross_point);

        // Draw the static line
        SDL_CHECK(draw_line(
            renderer, _line, [](point2i const&) { return constants::yellow; }));

        // Draw the line that interacts with the mouse. It's blue if they cross,
        // yellow otherwise.
        SDL_CHECK(
            draw_line(renderer, _mouse_line, [&did_cross](point2i const&) {
                return did_cross ? constants::cyan : constants::yellow;
            }));

        // Draw a red cross at the intersection point
        if (did_cross) {
            SDL_CHECK(set_renderer_draw_color(renderer, constants::red));
            SDL_CHECK(draw_point(get_renderer(), cross_point));
            SDL_CHECK(draw_point(get_renderer(), cross_point + point2i{1, 1}));
            SDL_CHECK(
                draw_point(get_renderer(), cross_point + point2i{-1, -1}));
            SDL_CHECK(draw_point(get_renderer(), cross_point + point2i{1, -1}));
            SDL_CHECK(draw_point(get_renderer(), cross_point + point2i{-1, 1}));
        }
    }

private:
    line2i _line{point2i{160, 160}, point2i{200, 320}};
    line2i _mouse_line{point2i{100, 200}, point2i{400, 300}};
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