#pragma once

#include <string>

class Texture {
public:
    unsigned int rendererId;
    std::string filepath;
    unsigned char* localBuffer;
    int width, height, bpp;
    bool valid;

    Texture();

    Texture(const std::string& path);
    void Dispose() const;
    void Bind(unsigned int slot) const;
    void Unbind() const;

    inline int GetWidth() const {return width;}
    inline int GetHeight() const {return height;}
};