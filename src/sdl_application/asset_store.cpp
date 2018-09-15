#include "asset_store.hpp"

using namespace std::string_literals;

namespace {

constexpr auto format_bmp_no_alpha = SDL_PIXELFORMAT_BGR24;
constexpr auto purple_b8g8r8 = 0x00FF00FF;

sdl::surface load_image(std::string const& path)
{
    auto surf = sdl::make_surface(SDL_LoadBMP(path.c_str()));

    if (surf->format->format != format_bmp_no_alpha) {
        auto const error_message
            = "Wrong format for image, need BGR24: "s + path;
        SDL_Log("%s", error_message.c_str());
        throw std::runtime_error{error_message};
    }

    if (SDL_SetColorKey(surf.get(), SDL_TRUE, purple_b8g8r8) != 0) {
        SDL_Log("Couldn't set color key on file: %s", path.c_str());
    }

    return surf;
}

} // namespace

namespace sdl_app {

asset_store::asset_store(std::string base_dir)
: _base_dir{std::move(base_dir)}
{
}

SDL_Surface* asset_store::get_asset(std::string const& path)
{
    auto const full_path = _base_dir + "/" + path;
    if (_asset_map.find(full_path) == _asset_map.end()) {
        SDL_Log("Loading asset: %s", full_path.c_str());
        _asset_map[full_path] = load_image(full_path);
    }

    return _asset_map[full_path].get();
}

} // namespace sdl_app
