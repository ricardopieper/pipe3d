#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Texture.h"
#include "Shader.h"
#include <glad/glad.h>
#include <memory>
#include <stdexcept>

enum FramebufferType {
    Srgb, Depth
};


class Framebuffer {
private:
    bool empty = false;
    unsigned int renderBuffer;

    std::unique_ptr<VertexBuffer> quadVertexBuffer;
    VertexArray quadVertexArray;
    std::shared_ptr<Shader> framebufferShader;
    
    void RenderTo(bool toScreen, Framebuffer* framebuffer);

public:

    int Width;
    int Height;

    unsigned int FramebufferId;
    unsigned int TextureId;
    FramebufferType Type;
    Framebuffer(std::shared_ptr<Shader> shader);
    Framebuffer(std::shared_ptr<Shader> shader, FramebufferType type);
    Framebuffer() = default;
    void MakeFramebuffer(int width, int height);
    void Bind();
    void Unbind();
    void BindDefaultShader();
    void RenderToSelf();
    void RenderToFramebuffer(Framebuffer& framebuffer);
    void RenderToScreen();
    Texture GetTexture();
    void Dispose();
};

class FramebufferBuilder {
private:
    std::shared_ptr<Shader> shader = nullptr;
    int width = -1;
    int height = -1;
    FramebufferType type = FramebufferType::Srgb;
public:
    static FramebufferBuilder New() {
        return FramebufferBuilder();
    }

    FramebufferBuilder& WithSize(int width, int height) {
        this->width = width;
        this->height = height;
        return *this;
    }

    FramebufferBuilder& WithShader(std::shared_ptr<Shader> shader) {
        this->shader = shader;
        return *this;
    }
    
    FramebufferBuilder& AsDepth() {
        this->type = FramebufferType::Depth;
        return *this;
    }

    FramebufferBuilder& AsSrgb() {
        this->type = FramebufferType::Srgb;
        return *this;
    }

    std::shared_ptr<Framebuffer> Build() {
        if (width == -1 || height == -1) {
            throw std::invalid_argument("FramebufferBuilder error: No size set");
        }
       
        auto fb = std::make_shared<Framebuffer>(shader, type);
        fb->MakeFramebuffer(width, height);
        return fb;
    }
};
