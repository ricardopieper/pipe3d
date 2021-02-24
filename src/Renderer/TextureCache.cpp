#include <iostream>
#include "TextureCache.h"
#include "Texture.h"


Texture TextureCache::GetTexture(std::string path) {
    if (CachedTextures.find(path) == CachedTextures.end()) {
        if (path == "") {
            CachedTextures.insert(std::pair<std::string, Texture>(path, Texture("")));
        } else {
            Texture tex(path);
            if (tex.width == 0) {
                std::cout << "failed to load "<< path << std::endl;
            }
            CachedTextures.insert(std::pair<std::string, Texture>(path, tex));
        }
    }
    Texture tex = CachedTextures[path];
    return tex;
}