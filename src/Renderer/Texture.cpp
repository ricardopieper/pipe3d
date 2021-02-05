#include "Texture.h"
#include <glad/glad.h>
#include "stb_image.h"

Texture::Texture(): rendererId(0), filepath(""), localBuffer(nullptr),
    width(0), height(0), bpp(0), valid(false) { }

Texture::Texture(const std::string& path)
    : rendererId(0), filepath(path), localBuffer(nullptr),
    width(0), height(0), bpp(0), valid(false) {

    if (path == "") {
        valid = false;
        return;
    }

    stbi_set_flip_vertically_on_load(1);
    localBuffer = stbi_load(path.c_str(), &width, &height, &bpp, 4);

    glGenTextures(1, &rendererId);
    glBindTexture(GL_TEXTURE_2D, rendererId);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, localBuffer);
    glGenerateMipmap(GL_TEXTURE_2D);
    if (localBuffer) {
        stbi_image_free(localBuffer);
    }
    valid = true;
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
