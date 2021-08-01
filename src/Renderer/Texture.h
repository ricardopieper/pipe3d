#pragma once

#include <string>
#include "Image.h"

class Texture {
    unsigned int rendererId;
public:
    int Width, Height;
    bool Valid;
    Texture();
    Texture(Image& img, bool srgb);
    Texture(int width, int height, int textureId);
    void Dispose() const;
    void Bind(unsigned int slot) const;
    void Unbind() const;
};