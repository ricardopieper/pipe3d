#pragma once
#include "Framebuffer.h"
#include "Camera3D.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <memory>

struct RenderingContext {
    Camera3D& CurrentCamera;
    Scene& CurrentScene; 
    glm::mat4 Projection;
};

class RenderingPipelineStage {
public:
    virtual std::shared_ptr<Framebuffer> RunStage(RenderingContext& context, std::shared_ptr<Framebuffer> framebuffer) = 0;
    virtual void OnResizeWindow(int width, int height) { };
};

class RenderingPipeline {
private:
    std::vector<std::shared_ptr<RenderingPipelineStage>> RenderingStages;
public:
    void AddStage(std::shared_ptr<RenderingPipelineStage> stage) {
        RenderingStages.push_back(stage);
    }

    void Render(RenderingContext& context) {
        std::shared_ptr<Framebuffer> currentFramebuffer;
        for (auto& stage: RenderingStages) {
            currentFramebuffer = stage->RunStage(context, currentFramebuffer);
        }
    }

    void OnResizeWindow(int width, int height) {
        for (auto& stage: RenderingStages) {
            stage->OnResizeWindow(width, height);
        }
    }

};