#pragma once 
#include <glad/glad.h>
#include "stb_image.h"
#include <vector>
#include <string>
#include <iostream>

class CubemapTexture {
private:
    unsigned int textureId;
public:

    CubemapTexture(std::vector<std::string> faces) {
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
        //stbi_set_flip_vertically_on_load(1);
        for (unsigned int i = 0; i < faces.size(); i++) {
            int width, height, channels;
            unsigned char* data = 
                stbi_load(faces[i].c_str(), &width, &height,&channels, 0);
            if (data) {
                glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                    0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
                stbi_image_free(data);
            } else {
                std::cout << "Cubemap tex failed to load at path "<<faces[i] << std::endl;
                stbi_image_free(data);
            }
        }
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    }

    void Bind(int slot) const {
        glActiveTexture(GL_TEXTURE0 + slot);
        glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);
    }

    void Unbind() const {
        glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    }
};