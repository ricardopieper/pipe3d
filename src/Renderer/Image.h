#pragma once

#include <string>
#include <vector>
#include <iterator>

class Image {
public:
    unsigned char* Data = nullptr;
    std::string Path;
    int Width = 0, Height = 0;
    int Channels = 0;
    bool Valid = false;
    bool IsFromCache = false;
    void SaveCache();
    void Dispose();
    int GetOpenglFormat(bool srgb);
    static Image Load(std::string& path, bool loadInverted = true);
};

