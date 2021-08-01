#include <iostream>
#include "TextureCache.h"
#include "Texture.h"
#include "Image.h"


Texture TextureCache::GetTexture(std::string path, bool srgb) {
    if (CachedTextures.find(path) == CachedTextures.end()) {
        if (path == "") {
            CachedTextures.insert(std::pair<std::string, Texture>(path, Texture()));
        } else {
            Image image = Image::Load(path);
            
            if (image.Width == 0) {
                std::cout << "failed to load "<< path << std::endl;
                return Texture();
            }
            Texture tex(image, srgb);
            image.SaveCache();
            image.Dispose();
            CachedTextures.insert(std::pair<std::string, Texture>(path, tex));
        }
    }
    Texture tex = CachedTextures[path];
    return tex;
}