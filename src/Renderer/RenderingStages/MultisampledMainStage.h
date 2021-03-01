#pragma once

#include "../Renderer.h"
#include "../RenderingPipeline.h"
#include "../MultisampledFramebuffer.h"
#include <memory>

class MultisampledMainStage : public RenderingPipelineStage
{
private:
    std::shared_ptr<MultisampledFramebuffer> mainFramebuffer;
    std::shared_ptr<Framebuffer> framebuffer;
    std::shared_ptr<Renderer> renderer;

public:
    MultisampledMainStage(std::shared_ptr<MultisampledFramebuffer> mainFramebuffer, std::shared_ptr<Renderer> renderer): mainFramebuffer(mainFramebuffer), renderer(renderer) {
    }

    void Initialize(int width, int height) override {
        framebuffer = std::make_shared<Framebuffer>();
        framebuffer->MakeFramebuffer(width, height);
    }

    std::shared_ptr<Framebuffer> RunStage(RenderingContext &context, std::shared_ptr<Framebuffer> ignore) override
    {
        glViewport(0, 0, framebuffer->Width, framebuffer->Height);
        mainFramebuffer->Bind();
        renderer->Clear();
        //renders to the framebuffer
        renderer->Render(context);
        //shader that just copies to the texture
        mainFramebuffer->BindDefaultShader();
        mainFramebuffer->RenderToFramebuffer(*framebuffer);
        return framebuffer;
    };

    void OnResizeWindow(int width, int height)
    {
        mainFramebuffer->Dispose();
        mainFramebuffer->MakeFramebuffer(width, height);
    
        framebuffer->Dispose();
        framebuffer->MakeFramebuffer(width, height);
    }
};
