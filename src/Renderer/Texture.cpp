#include "Texture.h"
#include <glad/glad.h>
#include "stb_image.h"

Texture::Texture(): rendererId(0), Width(0), Height(0), Valid(false) { }

Texture::Texture(int width, int height, int textureId): 
    Width(width), Height(height), rendererId(textureId), Valid(true) { }

Texture::Texture(const std::string& path, bool srgb) {
    
    Width = 0;
    Height = 0;

    if (path == "") {
        Valid = false;
        return;
    }
    int bpp;
    stbi_set_flip_vertically_on_load(1);
    unsigned char* localBuffer = stbi_load(path.c_str(), &Width, &Height, &bpp, 4);
    if (Width == 0 && Height == 0) {
         if (localBuffer) {
            stbi_image_free(localBuffer);
        }
        glBindTexture(GL_TEXTURE_2D, 0);
        Valid = false;
        return;
    }
    glGenTextures(1, &rendererId);
    glBindTexture(GL_TEXTURE_2D, rendererId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int format =  srgb? GL_SRGB_ALPHA: GL_RGBA;
    glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    if (localBuffer) {
        stbi_image_free(localBuffer);
    }
    Valid = true;
    glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::Dispose() const {
    glDeleteTextures(1, &rendererId);
}

void Texture::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, rendererId);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
