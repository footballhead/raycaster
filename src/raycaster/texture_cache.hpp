#pragma once

#include <sdl_application/asset_store.hpp>

#include <array>

namespace raycaster {

enum class common_assets {
    brick,
    stone,
    floor,
    ceiling,
    column,
    barrel,
    barrel_explode,
    bat,
};

constexpr char const* filename_from_id(common_assets id)
{
    switch (id) {
    case common_assets::brick:
        return "wall.bmp";
    case common_assets::stone:
        return "stone.bmp";
    case common_assets::floor:
        return "floor.bmp";
    case common_assets::ceiling:
        return "ceil.bmp";
    case common_assets::column:
        return "column.bmp";
    case common_assets::barrel:
        return "barrel.bmp";
    case common_assets::barrel_explode:
        return "barrel_explode.bmp";
    case common_assets::bat:
        return "bat.bmp";
    }

    return nullptr;
}

/// I made an asset_manager which has a map with string indicies which means
/// it's too slow in practice. Use a flat array instead.
using texture_cache = std::array<SDL_Surface*, 11>;

inline texture_cache make_texture_cache(sdl_app::asset_store& assets)
{
    return {{
        nullptr,
        assets.get_asset(filename_from_id(common_assets::brick)),
        assets.get_asset(filename_from_id(common_assets::stone)),
        assets.get_asset(filename_from_id(common_assets::floor)),
        assets.get_asset(filename_from_id(common_assets::column)),
        nullptr,
        assets.get_asset(filename_from_id(common_assets::ceiling)),
        nullptr,
        assets.get_asset(filename_from_id(common_assets::barrel)),
        assets.get_asset(filename_from_id(common_assets::barrel_explode)),
        assets.get_asset(filename_from_id(common_assets::bat)),
    }};
}

} // namespace raycaster
