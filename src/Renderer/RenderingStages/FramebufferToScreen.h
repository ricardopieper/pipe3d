#pragma once

#include "../RenderingPipeline.h"
#include <memory>

class FramebufferToScreen: public RenderingPipelineStage {
private:
    std::shared_ptr<Framebuffer> framebuffer;
public:
    FramebufferToScreen(int width, int height) {
        framebuffer = std::make_shared<Framebuffer>();
        framebuffer->MakeFramebuffer(width, height);
    }

    std::shared_ptr<Framebuffer> RunStage(RenderingContext& context, std::shared_ptr<Framebuffer> previousFramebuffer) override {
        previousFramebuffer->RenderToFramebuffer(*framebuffer);
        framebuffer->RenderToScreen();
        return framebuffer;
    };
    
    void OnResizeWindow(int width, int height) { 
        framebuffer->Dispose();
        framebuffer->MakeFramebuffer(width, height);
    };
};
