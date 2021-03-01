#pragma once 
#include <map>
#include <string>
#include "Texture.h"

class TextureCache {
private:
    std::map<std::string, Texture> CachedTextures;
public:
    Texture GetTexture(std::string path, bool srgb = true);
};