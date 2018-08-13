#include "asset_store.hpp"

namespace {

sdl::texture load_image(SDL_Renderer* renderer, std::string const& path)
{
    auto surface = sdl::make_surface(SDL_LoadBMP(path.c_str()));
    return sdl::make_texture(
        SDL_CreateTextureFromSurface(renderer, surface.get()));
}

} // namespace

namespace raycaster {

asset_store::asset_store(sdl::renderer renderer, std::string base_dir)
: _renderer{std::move(renderer)}
, _base_dir{std::move(base_dir)}
{
}

sdl::texture const& asset_store::get_asset(std::string const& path)
{
    auto const full_path = _base_dir + "/" + path;
    if (_asset_map.find(full_path) == _asset_map.end()) {
        _asset_map[full_path] = load_image(_renderer.get(), full_path);
    }
    return _asset_map[full_path];
}

} // namespace raycaster
