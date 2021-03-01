#pragma once

#include <string>

class Texture {
    unsigned int rendererId;
public:
    int Width, Height;
    bool Valid;
    Texture();
    Texture(int width, int height, int textureId);
    Texture(const std::string& path, bool srgb = true);
    void Dispose() const;
    void Bind(unsigned int slot) const;
    void Unbind() const;
};