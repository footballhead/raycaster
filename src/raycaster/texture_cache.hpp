#pragma once

#include <sdl_application/asset_store.hpp>

#include <array>

namespace raycaster {

namespace common_assets {
constexpr auto wall_texture = "wall.bmp";
constexpr auto stone_texture = "stone.bmp";
constexpr auto floor = "floor.bmp";
constexpr auto floor2 = "floor2.bmp";
constexpr auto ceiling = "ceil.bmp";
constexpr auto column = "column.bmp";
constexpr auto font = "6x8-terminal-mspaint.bmp";
constexpr auto barrel = "barrel.bmp";
constexpr auto barrel_explode = "barrel_explode.bmp";
constexpr auto bat = "bat.bmp";
} // namespace common_assets

constexpr char const* get_wall_texture(unsigned int i)
{
    switch (i) {
    case 0:
    case 1:
        return common_assets::wall_texture;
    case 2:
        return common_assets::stone_texture;
    case 3:
        return common_assets::floor;
    case 4:
        return common_assets::column;
    case 5:
        return common_assets::floor2;
    case 6:
        return common_assets::ceiling;
    case 7:
        return common_assets::font;
    case 8:
        return common_assets::barrel;
    case 9:
        return common_assets::barrel_explode;
    case 10:
        return common_assets::bat;
    default:
        return common_assets::wall_texture;
    }
}

/// I made an asset_manager which has a map with string indicies which means
/// it's too slow in practice. Use a flat array instead.
using texture_cache = std::array<SDL_Surface*, 11>;

inline texture_cache make_texture_cache(sdl_app::asset_store& assets)
{
    return {{
        assets.get_asset(get_wall_texture(0)),
        assets.get_asset(get_wall_texture(1)),
        assets.get_asset(get_wall_texture(2)),
        assets.get_asset(get_wall_texture(3)),
        assets.get_asset(get_wall_texture(4)),
        assets.get_asset(get_wall_texture(5)),
        assets.get_asset(get_wall_texture(6)),
        assets.get_asset(get_wall_texture(7)),
        assets.get_asset(get_wall_texture(8)),
        assets.get_asset(get_wall_texture(9)),
        assets.get_asset(get_wall_texture(10)),
    }};
}

} // namespace raycaster
