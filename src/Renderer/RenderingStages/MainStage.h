#pragma once

#include "../Renderer.h"
#include "../RenderingPipeline.h"
#include <memory>

class MainRenderingStage : public RenderingPipelineStage
{
private:
    std::shared_ptr<Framebuffer> mainFramebuffer;
    std::shared_ptr<Renderer> renderer;

public:
    MainRenderingStage(std::shared_ptr<Framebuffer> mainFramebuffer, std::shared_ptr<Renderer> renderer) : mainFramebuffer(mainFramebuffer), renderer(renderer) {}

    std::shared_ptr<Framebuffer> RunStage(RenderingContext &context, std::shared_ptr<Framebuffer> ignore) override
    {
        mainFramebuffer->Bind();
        renderer->Clear();
        //renders to the framebuffer
        renderer->Render(context.CurrentScene, context.CurrentCamera, context.Projection);
        //shader that just copies to the texture
        mainFramebuffer->BindDefaultShader();
        mainFramebuffer->RenderToSelf();
        return mainFramebuffer;
    };

    void OnResizeWindow(int width, int height)
    {
        mainFramebuffer->Dispose();
        mainFramebuffer->MakeFramebuffer(width, height);
    }
};
