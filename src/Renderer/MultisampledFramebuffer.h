#pragma once

#include "VertexArray.h"
#include "VertexBuffer.h"
#include "Framebuffer.h"
#include "Shader.h"
#include <glad/glad.h>
#include <memory>

class MultisampledFramebuffer {
    unsigned int multisampledFramebufferId;
    unsigned int multisampledTextureId;
    unsigned int renderBuffer;

    unsigned int intermediateFbo;
    unsigned int screenTexture;

    std::unique_ptr<VertexBuffer> quadVertexBuffer;
    VertexArray quadVertexArray;
    std::shared_ptr<Shader> framebufferShader;
    
    int width;
    int height;
    void RenderTo(bool toScreen, unsigned int framebufferId);
public:
    MultisampledFramebuffer(std::shared_ptr<Shader> shader);
    void MakeFramebuffer(int width, int height);
    void Bind();
    void Unbind();
    void BindDefaultShader();
    void RenderToSelf();
    void RenderToFramebuffer(Framebuffer& framebuffer);
    void RenderToScreen();
    void Dispose();
};