#include "asset_store.hpp"

namespace {

sdl::surface load_image(std::string const& path)
{
    return sdl::make_surface(SDL_LoadBMP(path.c_str()));
}

} // namespace

namespace sdl_app {

asset_store::asset_store(sdl::shared_renderer renderer, std::string base_dir)
: _renderer{std::move(renderer)}
, _base_dir{std::move(base_dir)}
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

} // namespace raycaster
