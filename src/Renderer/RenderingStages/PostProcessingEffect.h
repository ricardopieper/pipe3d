#pragma once

#include "../RenderingPipeline.h"
#include <memory>

class PostProcessingEffect: public RenderingPipelineStage {
private:
    std::shared_ptr<Shader> shader;
    std::shared_ptr<Framebuffer> framebuffer;
public:
    PostProcessingEffect(int width, int height, std::shared_ptr<Shader> shader): shader(shader) {
        framebuffer = std::make_shared<Framebuffer>(shader);
        framebuffer->MakeFramebuffer(width, height);
    }

    std::shared_ptr<Framebuffer> RunStage(RenderingContext& context, std::shared_ptr<Framebuffer> previousFramebuffer) override {
        shader->Bind();
        previousFramebuffer->RenderToFramebuffer(*framebuffer);
        return framebuffer;
    };

    void OnResizeWindow(int width, int height) { 
        framebuffer->Dispose();
        framebuffer->MakeFramebuffer(width, height);
    };
};
