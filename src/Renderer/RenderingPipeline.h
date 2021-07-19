#pragma once
#include "Framebuffer.h"
#include "Camera3D.h"
#include "Scene.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <memory>
#include <unordered_map>

struct RenderingContext {
    Camera3D& CurrentCamera;
    Scene& CurrentScene;
    glm::mat4 Projection;
    std::shared_ptr<Framebuffer> ShadowMap;
    glm::mat4 LightSpace;
    float DeltaTime;
    BoundingBox SceneBoundingBox;

};

class RenderingPipelineStage {
public:
    virtual void Initialize(int width, int height) = 0;
    virtual std::shared_ptr<Framebuffer> RunStage(RenderingContext& context, std::shared_ptr<Framebuffer> framebuffer) = 0;
    virtual void OnResizeWindow(int width, int height) { };
};

class RenderingPipeline {
private:
    std::vector<std::shared_ptr<RenderingPipelineStage>> RenderingStages;
    int Width = 0;
    int Height = 0;
public:

    RenderingPipeline(int width, int height): Width(width), Height(height) {}

    void AddStage(std::shared_ptr<RenderingPipelineStage> stage) {
        stage->Initialize(this->Width, this->Height);
        RenderingStages.push_back(stage);
    }

    void Render(RenderingContext& context) {
        std::shared_ptr<Framebuffer> currentFramebuffer;
        for (auto& stage: RenderingStages) {
            currentFramebuffer = stage->RunStage(context, currentFramebuffer);
        }
    }

    void OnResizeWindow(int width, int height) {
        this->Width = width;
        this->Height = height;
        for (auto& stage: RenderingStages) {
            stage->OnResizeWindow(width, height);
        }
    }

};