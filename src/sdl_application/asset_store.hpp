#pragma once

#include <string>
#include <unordered_map>

#include <sdl_raii/sdl_raii.hpp>

namespace sdl_app {

/// Manages asset lifetime.
class asset_store {
public:
    explicit asset_store(sdl::shared_renderer renderer, std::string base_dir);

    SDL_Surface* get_asset(std::string const& path);

private:
    using asset_map = std::unordered_map<std::string, sdl::surface>;

    sdl::shared_renderer _renderer;
    std::string _base_dir;
    asset_map _asset_map;
};

} // namespace raycaster
