#include "Framebuffer.h"
#include <memory>
#include <stdio.h>

Framebuffer::Framebuffer(std::shared_ptr<Shader> framebufferShader): framebufferShader(framebufferShader) {
    Type = FramebufferType::Srgb;
}
Framebuffer::Framebuffer(std::shared_ptr<Shader> framebufferShader, FramebufferType type): framebufferShader(framebufferShader), Type(type) {}

void Framebuffer::MakeFramebuffer(int width, int height)
{
    this->Width = width;
    this->Height = height;
    glGenFramebuffers(1, &FramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferId);

    glGenTextures(1, &TextureId);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    if (Type == FramebufferType::Srgb) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureId, 0);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glGenRenderbuffers(1, &renderBuffer);
        glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, renderBuffer);
    } else if (Type == FramebufferType::Depth) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32F, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);  
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, TextureId, 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        renderBuffer = 0;
    } else {
        throw std::invalid_argument("Framebuffer type not implemented");
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
   

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete: %x\n", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //setup quad for rendering

    auto framebufferQuadLayout = BufferLayout();
    framebufferQuadLayout.PushFloat(2); //position
    framebufferQuadLayout.PushFloat(2); //tex coords

    //fullscreen quad, pos and tex coord
    float quadData[] = {
        -1, 1, 0, 1,
        -1, -1, 0, 0,
        1, -1, 1, 0,
        1, -1, 1, 0,
        1, 1, 1, 1,
        -1, 1, 0, 1};

    this->quadVertexBuffer = std::make_unique<VertexBuffer>(&quadData, sizeof(float) * 6 * 4);
    this->quadVertexArray = VertexArray(framebufferQuadLayout);
}

void Framebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferId);
}

void Framebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Framebuffer::BindDefaultShader()
{ 
    if (framebufferShader == nullptr) {
        printf("Error binding default framebuffer shader: Framebuffer created without a default shader\n");
    } else {
        framebufferShader->Bind();
    }
}

void Framebuffer::RenderToFramebuffer(Framebuffer& framebuffer) {
    this->RenderTo(false, &framebuffer);
}

void Framebuffer::RenderToSelf() {
    this->RenderTo(false, this);
}

void Framebuffer::RenderToScreen() {
    this->RenderTo(true, nullptr);
}

void Framebuffer::RenderTo(bool toScreen, Framebuffer* framebuffer)
{
    glMemoryBarrier(GL_FRAMEBUFFER_BARRIER_BIT);
    if (toScreen){
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FramebufferId);
    }
 
    //glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    quadVertexArray.AddBufferAndBind(*quadVertexBuffer);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

Texture Framebuffer::GetTexture() {
    return Texture(this->Width, this->Height, this->TextureId);
}

void Framebuffer::Dispose()
{
    glDeleteFramebuffers(1, &FramebufferId);
    glDeleteFramebuffers(1, &TextureId);
    glDeleteRenderbuffers(1, &renderBuffer);
}