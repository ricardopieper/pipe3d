#pragma once

#include "../RenderingPipeline.h"
#include <memory>
#include <limits>

class DirectionalShadowMapGeneration: public RenderingPipelineStage {
private:
    std::shared_ptr<Framebuffer> depthShadowMap;
    std::shared_ptr<Shader> depthShader;
    BufferLayout bufferLayout;
    int mapSizeMultiplier = 8;
public:
    DirectionalShadowMapGeneration(std::shared_ptr<Shader> depthShader) {
        this->depthShader = depthShader;
        bufferLayout.PushFloat(3);
    }

    void Initialize(int width, int height) override {
        this->depthShadowMap = FramebufferBuilder().AsDepth()
            .WithSize(width * mapSizeMultiplier, height * mapSizeMultiplier).Build();
    }

    std::shared_ptr<Framebuffer> RunStage(RenderingContext& context, std::shared_ptr<Framebuffer> previousFramebuffer) override {
        depthShadowMap->Bind();
        depthShader->Bind();
        glEnable(GL_DEPTH_TEST);
        float width = depthShadowMap->Width;
        float height = depthShadowMap->Height;
        glViewport(0, 0, width, height);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);
        //Get the scene's XZ bounding plane
        glm::vec3 min;
        glm::vec3 max;
        min.x = std::numeric_limits<float>::max(); 
        min.y = std::numeric_limits<float>::max();
        min.z = std::numeric_limits<float>::max();
        max.x = std::numeric_limits<float>::lowest(); 
        max.y = std::numeric_limits<float>::lowest();
        max.z = std::numeric_limits<float>::lowest();

        for (auto& obj : context.CurrentScene.SceneObjects) {
            if (!obj->Enabled) continue;
            //Debug helping objects do not cast shadows
            if (obj->IsDebugHelper) continue;
            //sun itself does not cast shadow
            if (obj->Light.IsDirectional) continue;
            auto model = obj->ComputeModel();
            for (auto& objMesh : obj->SceneObjectElements) {

                //apply transformations on the bounding box
                auto objMin = model * glm::vec4(objMesh.boundingBox.min, 1);
                auto objMax = model * glm::vec4(objMesh.boundingBox.max, 1);
                min.x = std::min(min.x, std::min(objMin.x, objMax.x));
                min.y = std::min(min.y, std::min(objMin.y, objMax.y));
                min.z = std::min(min.z, std::min(objMin.z, objMax.z));
                max.x = std::max(max.x, std::max(objMin.x, objMax.x));
                max.y = std::max(max.y, std::max(objMin.y, objMax.y));
                max.z = std::max(max.z, std::max(objMin.z, objMax.z));
                /*for (auto& item : objMesh.geometry.VertexData) {
                    auto objpos = model * glm::vec4(item.position, 1);
                    min.x = std::min(min.x, objpos.x);
                    min.y = std::min(min.y, objpos.y);
                    min.z = std::min(min.z, objpos.z);
                    max.x = std::max(max.x, objpos.x);
                    max.y = std::max(max.y, objpos.y);
                    max.z = std::max(max.z, objpos.z);
                }*/

                //return VertexBuffer(VertexData.data(), sizeof(Vertex) * VertexData.size());
                //for (auto& vertex: objMesh.vertexBuffer)

            }
        }
        context.SceneBoundingBox.min = min;
        context.SceneBoundingBox.max = max;

        //std::cout << "SceneBoundingBox: Min(" << min.x << " " << min.y << " " << min.z << "), Max(" << max.x << " " << max.y << " " << max.z << ")\n";

        glm::vec3 midScene = glm::vec3(
            (min.x + max.x) / 2.0,
            0.0,
            (min.z + max.z) / 2.0
        );
        //std::cout << "MidScene: (" << midScene.x << " " << midScene.y << " " << midScene.z << ")\n";

        glm::mat4 lightMatrix;
        for (auto& obj: context.CurrentScene.SceneObjects) {
            if (obj->Light.IsDirectional) {
                lightMatrix = glm::lookAt(
                    midScene + glm::vec3(obj->Translation.x, max.y, obj->Translation.z),
                    midScene,
                    glm::vec3(0.0f, 0.0f, 1.0f)
                );
                break;
            }
        }

      
        for (auto& obj: context.CurrentScene.SceneObjects) {
            if (!obj->Enabled) continue;
            if (obj->IsDebugHelper) continue;
            auto sceneObject = *obj;
            auto model = obj->ComputeModel();
            depthShader->SetUniformMat4f("model", model);
            
            for (auto& light: context.CurrentScene.SceneObjects) {
                if (light->Light.IsDirectional) {

                    float sceneWidth = abs(max.x - min.x);
                    float sceneLength = abs(max.z - min.z);
                    float sceneHeight = abs(max.y - min.y);

                    glm::mat4 lightProjection = glm::ortho(
                        -sceneWidth/2.0f, +sceneWidth / 2.0f,
                        -sceneLength/2.0f, +sceneLength / 2.0f,
                        -sceneHeight * 2, +sceneHeight * 2);
              
                    glm::mat4 lightSpace = lightProjection * lightMatrix;
                    context.LightSpace = lightSpace;
                    depthShader->SetUniformMat4f("lightSpace", lightSpace);
                }
            }
            
            for (auto objMesh: sceneObject.SceneObjectElements) {
                if (obj->Light.IsDirectional) {
                    continue;
                }
                if (!obj->Enabled) continue;
                if (obj->IsDebugHelper) continue;

                objMesh.vertexArray.Bind();
                objMesh.vertexArray.AddBufferAndBind(objMesh.vertexBuffer);
                objMesh.indexBuffer.Bind();
                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount() , GL_UNSIGNED_INT, nullptr);                
            }
        }
        
        context.ShadowMap = depthShadowMap;
        depthShadowMap->RenderToSelf();
        
        return depthShadowMap;
    };
    
    void OnResizeWindow(int width, int height) { 
        depthShadowMap->Dispose();
        depthShadowMap->MakeFramebuffer(width * mapSizeMultiplier, height * mapSizeMultiplier);
    };
};
