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
        glCullFace(GL_FRONT);
        //Get the scene's XZ bounding plane
        glm::vec3 min;
        glm::vec3 max;
        min.x = std::numeric_limits<float>::max(); 
        min.y = std::numeric_limits<float>::max();
        min.z = std::numeric_limits<float>::max();
        max.x = std::numeric_limits<float>::min(); 
        max.y = std::numeric_limits<float>::min(); 
        max.z = std::numeric_limits<float>::min();

        glm::mat4 lightMatrix;
        for (auto& obj: context.CurrentScene.SceneObjects) {
            if (obj->Light.IsDirectional) {
                lightMatrix = glm::lookAt(
                    obj->Translation,
                    glm::vec3(0.0),
                    glm::vec3(0.0f, 0.0f, 1.0f)
                );
                break;
            }
        }
            
        for (auto& obj: context.CurrentScene.SceneObjects) {
            
            //if (obj->Light.IsDirectional) continue;
            auto model = obj->ComputeModel();
            for (auto& objMesh: obj->SceneObjectElements) {
                
                //apply transformations on the bounding box
                auto objMin =  model * glm::vec4(objMesh.boundingBox.min, 1);
                auto objMax =  model * glm::vec4(objMesh.boundingBox.max, 1);
                min.x = std::min(min.x, objMin.x);
                min.y = std::min(min.y, objMin.y);
                min.z = std::min(min.z, objMin.z);
                max.x = std::max(max.x, objMax.x);
                max.y = std::max(max.y, objMax.y);
                max.z = std::max(max.z, objMax.z);
            }
        }

       

        for (auto& obj: context.CurrentScene.SceneObjects) {
            auto sceneObject = *obj;
            auto model = obj->ComputeModel();
            depthShader->SetUniformMat4f("model", model);
            
            for (auto& light: context.CurrentScene.SceneObjects) {
                if (obj->Light.IsDirectional) {
                    
                    //pretend the light has a camera attached to it
                    //looking at the center of the scene
                    Camera3D camera;
                    camera.Position = obj->Translation;
                    
                    auto lmin = glm::vec4(min, 1);
                    auto lmax = glm::vec4(max, 1);
                    float nearPlane = 0.00001;
                    float xDistance = lmax.x - lmin.x;
                    float zDistance = lmax.z - lmin.z;
                    float aspect = width / height;

                    xDistance *= 1;
                    zDistance *= 1;
                   /*glm::mat4 lightProjection = glm::ortho(
                        -xDistance, +xDistance,
                        -zDistance, +zDistance, 
                        nearPlane, 100.0f);*/
                    glm::mat4 lightProjection = glm::ortho(
                        min.x, max.x ,
                        min.z, max.z , 
                        min.y*2, max.y*2);
                    /*glm::mat4 lightProjection = glm::perspective(
                        glm::radians(90.0f),
                        (float)this->depthShadowMap->Width / (float)this->depthShadowMap->Height,
                        0.01f, 10000.0f);*/
                    glm::mat4 lightSpace = lightProjection * lightMatrix;
                    context.LightSpace = lightSpace;
                    depthShader->SetUniformMat4f("lightSpace", lightSpace);
                }
            }
            
            for (auto objMesh: sceneObject.SceneObjectElements) {
                objMesh.vertexArray.Bind();
                objMesh.vertexArray.AddBufferAndBind(objMesh.vertexBuffer);
                objMesh.indexBuffer.Bind();
                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount() , GL_UNSIGNED_INT, nullptr);                
            }
        }
        
        context.ShadowMap = depthShadowMap;
        depthShadowMap->RenderToSelf();
        glCullFace(GL_FRONT);
        return depthShadowMap;
    };
    
    void OnResizeWindow(int width, int height) { 
        depthShadowMap->Dispose();
        depthShadowMap->MakeFramebuffer(width * mapSizeMultiplier, height * mapSizeMultiplier);
    };
};
