#pragma once

#include <string>
#include <unordered_map>

#include <sdl_raii/sdl_app.hpp>

namespace raycaster {

/// Manages asset lifetime.
class asset_store {
public:
    explicit asset_store(sdl::renderer renderer, std::string base_dir);

    sdl::texture const& get_asset(std::string const& path);

private:
    using asset_map = std::unordered_map<std::string, sdl::texture>;

    sdl::renderer _renderer;
    std::string _base_dir;
    asset_map _asset_map;
};

namespace common_assets {
constexpr auto wall_texture = "wall.bmp";
constexpr auto stone_texture = "stone.bmp";
} // namespace common_assets

} // namespace raycaster