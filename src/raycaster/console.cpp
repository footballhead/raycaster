#include "console.hpp"

#include <mymath/mymath.hpp>
#include <sdl_raii/sdl_assert.hpp>

using namespace mymath;

namespace {

constexpr auto font_size = extent2i{6, 8}; // TODO unhardcode this

// TODO put in common code, own header?
inline bool draw_string(
    std::string const& str, point2i pos, SDL_Surface& font, SDL_Surface& dest)
{
    for (auto const& c : str) {
        if (c == '\0') {
            break;
        }

        auto src_rect = SDL_Rect{c * font_size.w, 0, font_size.w, font_size.h};
        auto dst_rect = SDL_Rect{pos.x, pos.y, font_size.w, font_size.h};
        if (SDL_BlitSurface(&font, &src_rect, &dest, &dst_rect) != 0) {
            return false;
        }

        pos.x += font_size.w;
    }

    return true;
}

constexpr auto log_draw_limit = 10;
constexpr auto text_spacing_px = 10;

} // namespace

namespace raycaster {

void console::set_callback(callback_fn callback)
{
    _callback = std::move(callback);
}

void console::open()
{
    if (_opened) {
        return;
    }

    _opened = true;
    SDL_StartTextInput();
    _keyboard_string.clear();
}

void console::close()
{
    if (!_opened) {
        return;
    }

    _opened = false;
    SDL_StopTextInput();
    _keyboard_string.clear();
    _history_top_offset = 0;
}

void console::toggle_open()
{
    if (_opened) {
        close();
    } else {
        open();
    }
}

bool console::is_open() const { return _opened; }

void console::log(std::string const& msg) { _log.push_back(msg); }

void console::handle_event(SDL_Event const& event)
{
    switch (event.type) {
    case SDL_TEXTINPUT:
        _keyboard_string += event.text.text;
        break;
    }
}

void console::update(sdl_app::input_buffer& input_buffer)
{
    using namespace std::string_literals;
    if (input_buffer.is_hit(SDL_SCANCODE_RETURN)) {
        _history.push_back(_keyboard_string);
        _log.push_back(">> "s + _keyboard_string);

        _callback(_keyboard_string);

        // reset state but allow the user to enter mroe commands
        _keyboard_string.clear();
        _history_top_offset = 0;
    }
    if (input_buffer.is_hit(SDL_SCANCODE_BACKSPACE)) {
        if (!_keyboard_string.empty()) {
            _keyboard_string.pop_back();
        }
    }
    if (input_buffer.is_hit(SDL_SCANCODE_UP)) {
        if (_history_top_offset < static_cast<int>(_history.size())) {
            ++_history_top_offset;
            _keyboard_string = *(end(_history) - _history_top_offset);
        }
    }
    if (input_buffer.is_hit(SDL_SCANCODE_DOWN)) {
        --_history_top_offset;

        if (_history_top_offset <= 0) {
            _keyboard_string.clear();
            _history_top_offset = 0;
        } else {
            _keyboard_string = *(end(_history) - _history_top_offset);
        }
    }
}

void console::draw(SDL_Surface& framebuffer, SDL_Surface& font)
{
    using namespace std::string_literals;

    SDL_CHECK(draw_string(
        "> "s + _keyboard_string + "_", point2i{0, 0}, font, framebuffer));

    auto i = 0;
    for (auto log_rev_iter = rbegin(_log); log_rev_iter != rend(_log);
         log_rev_iter++) {
        if (i >= log_draw_limit) {
            break;
        }

        SDL_CHECK(draw_string(*log_rev_iter,
            point2i{0, (i + 1) * text_spacing_px}, font,
            framebuffer)); // TODO magic number

        ++i;
    }
}

} // namespace raycaster