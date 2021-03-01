#pragma once

#include "../RenderingPipeline.h"
#include <memory>

class ShadowMapDebug: public RenderingPipelineStage {
private:
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<Shader> shadowMapDebug;
public:
    ShadowMapDebug(std::shared_ptr<Shader> shadowMapDebug): 
        shadowMapDebug(shadowMapDebug) { }

    void Initialize(int width, int height) override {
        framebuffer = std::make_shared<Framebuffer>();
        framebuffer->MakeFramebuffer(width, height);
    }

    std::shared_ptr<Framebuffer> RunStage(RenderingContext& context, std::shared_ptr<Framebuffer> previousFramebuffer) override {
        glViewport(0, 0, framebuffer->Width, framebuffer->Height);
        shadowMapDebug->Bind();
        shadowMapDebug->SetUniform1i("depthMap", 0);
        previousFramebuffer->RenderToFramebuffer(*framebuffer);
        framebuffer->RenderToScreen();
        return previousFramebuffer;
    };
    
    void OnResizeWindow(int width, int height) { 
        framebuffer->Dispose();
        framebuffer->MakeFramebuffer(width, height);
    };
};
