#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Shader.h"
#include <glad/glad.h>
#include <memory>

class Framebuffer {
private:
    bool empty = false;
    unsigned int renderBuffer;

    std::unique_ptr<VertexBuffer> quadVertexBuffer;
    VertexArray quadVertexArray;
    BufferLayout framebufferQuadLayout;
    std::shared_ptr<Shader> framebufferShader;
    
    int width;
    int height;

    static Framebuffer Empty() {
        auto f = Framebuffer(nullptr);
        f.empty = true;
        return f;
    }

    void RenderTo(bool toScreen, Framebuffer* framebuffer);

public:
    unsigned int FramebufferId;
    unsigned int TextureId;
    Framebuffer(std::shared_ptr<Shader> shader);
    Framebuffer() { };
    void MakeFramebuffer(int width, int height);
    void Bind();
    void Unbind();
    void BindDefaultShader();
    void RenderToSelf();
    void RenderToFramebuffer(Framebuffer& framebuffer);
    void RenderToScreen();
    
    void Dispose();
};