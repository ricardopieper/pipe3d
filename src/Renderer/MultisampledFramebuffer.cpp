#include "MultisampledFramebuffer.h"
#include <memory>
#include <stdio.h>

MultisampledFramebuffer::MultisampledFramebuffer(std::shared_ptr<Shader> framebufferShader): framebufferShader(framebufferShader) {}

void MultisampledFramebuffer::MakeFramebuffer(int width, int height)
{
    this->width = width;
    this->height = height;
    int samples = 4;
    glGenFramebuffers(1, &multisampledFramebufferId);
    glBindFramebuffer(GL_FRAMEBUFFER, multisampledFramebufferId);

    glGenTextures(1, &multisampledTextureId);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, multisampledTextureId);
    glTexImage2DMultisample(GL_TEXTURE_2D_MULTISAMPLE, samples, GL_SRGB, width, height, GL_TRUE);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D_MULTISAMPLE, multisampledTextureId, 0);
    glBindTexture(GL_TEXTURE_2D_MULTISAMPLE, 0);


    glGenRenderbuffers(1, &renderBuffer);
    glBindRenderbuffer(GL_RENDERBUFFER, renderBuffer);
    glRenderbufferStorageMultisample(GL_RENDERBUFFER, samples, GL_DEPTH24_STENCIL8, width, height);
    glBindRenderbuffer(GL_RENDERBUFFER, 0);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                              GL_RENDERBUFFER, renderBuffer);

   
    int status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete: %x\n", status);
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glGenFramebuffers(1, &intermediateFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, intermediateFbo);
    glGenTextures(1, &screenTexture);
    glBindTexture(GL_TEXTURE_2D, screenTexture);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_SRGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, screenTexture, 0);	

    status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        printf("ERROR: Framebuffer is not complete: %x\n", status);
    }

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

void MultisampledFramebuffer::Bind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, multisampledFramebufferId);
}

void MultisampledFramebuffer::Unbind()
{
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

void MultisampledFramebuffer::BindDefaultShader()
{ 
    framebufferShader->Bind();
}

void MultisampledFramebuffer::RenderToScreen() {
    this->Render(true);
}

void MultisampledFramebuffer::RenderToFramebuffer() {
    this->Render(false);
}

void MultisampledFramebuffer::Render(bool toScreen)
{
    glBindFramebuffer(GL_READ_FRAMEBUFFER, multisampledFramebufferId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, intermediateFbo);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height, GL_COLOR_BUFFER_BIT, GL_NEAREST);
    
    if (toScreen) {
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    } else {
        glBindFramebuffer(GL_FRAMEBUFFER, multisampledFramebufferId);
    } 
    glClearColor(0,0,0,1);
    glClear(GL_COLOR_BUFFER_BIT);
    quadVertexArray.AddBufferAndBind(*quadVertexBuffer, framebufferQuadLayout);
    glDisable(GL_DEPTH_TEST);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, screenTexture);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

void MultisampledFramebuffer::Dispose()
{
    glDeleteFramebuffers(1, &multisampledFramebufferId);
    glDeleteFramebuffers(1, &intermediateFbo);
    glDeleteRenderbuffers(1, &renderBuffer);
    glDeleteTextures(1, &multisampledTextureId);
    glDeleteTextures(1, &screenTexture);
}