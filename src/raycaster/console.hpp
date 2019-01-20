#pragma once

#include <sdl_application/input_buffer.hpp>

#include <functional>
#include <string>
#include <vector>

namespace raycaster {

class console {
public:
    using callback_fn = std::function<void(std::string const&)>;

    void set_callback(callback_fn callback);

    void open();
    void close();
    void toggle_open();
    bool is_open() const;

    void log(std::string const& msg);
    void do_command(std::string const& cmd);

    void handle_event(SDL_Event const& event);
    void update(sdl_app::input_buffer& input_buffer);

    void draw(SDL_Surface& framebuffer, SDL_Surface& font);

private:
    std::string _keyboard_string;
    std::vector<std::string> _history;
    std::vector<std::string> _log;
    callback_fn _callback;

    bool _opened = false;
    int _history_top_offset = 0;
};

}; // namespace raycaster