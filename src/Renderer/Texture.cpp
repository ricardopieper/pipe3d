#include "Texture.h"
#include <glad/glad.h>
#include "stb_image.h"

Texture::Texture(): rendererId(0), Width(0), Height(0), Valid(false) { }

Texture::Texture(int width, int height, int textureId): 
    Width(width), Height(height), rendererId(textureId), Valid(true) { }

Texture::Texture(Image& img, bool srgb) {
    Width = img.Width;
    Height = img.Height;
    glGenTextures(1, &rendererId);
    glBindTexture(GL_TEXTURE_2D, rendererId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int format = img.GetOpenglFormat(srgb);
    int openglFormat = img.GetOpenglFormat(false);

    glTexImage2D(GL_TEXTURE_2D, 0, format, Width, Height, 0, openglFormat, GL_UNSIGNED_BYTE, (void*)img.Data);
    glGenerateMipmap(GL_TEXTURE_2D);
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
