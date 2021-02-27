#include "Framebuffer.h"
#include <memory>
#include <stdio.h>

Framebuffer::Framebuffer(std::shared_ptr<Shader> framebufferShader): framebufferShader(framebufferShader) {}

void Framebuffer::MakeFramebuffer(int width, int height)
{
    this->width = width;
    this->height = height;
    glGenFramebuffers(1, &FramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, FramebufferId);

    glGenTextures(1, &TextureId);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, TextureId, 0);
    glBindTexture(GL_TEXTURE_2D, 0);

    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
        GL_RENDERBUFFER, renderBuffer);

    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete: %x\n", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //setup quad for rendering

    this->framebufferQuadLayout = BufferLayout();
    this->framebufferQuadLayout.PushFloat(2); //position
    this->framebufferQuadLayout.PushFloat(2); //tex coords

    //fullscreen quad
    float quadData[] = {
        -1, 1, 0, 1,
        -1, -1, 0, 0,
        1, -1, 1, 0,
        1, -1, 1, 0,
        1, 1, 1, 1,
        -1, 1, 0, 1};

    this->quadVertexBuffer = std::make_unique<VertexBuffer>(&quadData, sizeof(float) * 6 * 4);
    this->quadVertexArray = VertexArray();
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
        glClearColor(0,0,0,1);
        glClear(GL_COLOR_BUFFER_BIT);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->FramebufferId);
    }
 
    quadVertexArray.AddBufferAndBind(*quadVertexBuffer, framebufferQuadLayout);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, TextureId);
    glDrawArrays(GL_TRIANGLES, 0, 6);
  
}

void Framebuffer::Dispose()
{
    glDeleteFramebuffers(1, &FramebufferId);
    glDeleteFramebuffers(1, &TextureId);
    glDeleteRenderbuffers(1, &renderBuffer);
}