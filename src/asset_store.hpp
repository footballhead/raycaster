#pragma once

#include <string>
#include <unordered_map>

#include "sdl_app.hpp"

/// Manages asset lifetime.
class asset_store {
public:
    explicit asset_store(std::string basedir);

    SDL_Texture_ptr get_asset(std::string path);

private:
    using AssetMap = std::unordered_map<std::string, SDL_Texture_ptr>;

    std::string _basedir;
    AssetMap _asset_map;
};
