#include "Renderer.h"
#include "Scene.h"
#include <iostream>
#include <glad/glad.h>


void Renderer::Render(Scene& scene, Camera3D& camera, glm::mat4 projection) {
    glm::mat4 view = camera.GetViewMatrix();
    std::shared_ptr<SceneObject> light;
    for (auto sceneObjectPtr: scene.SceneObjects) {
        if (sceneObjectPtr->Light.Enabled) {
            light = sceneObjectPtr;
            break;
        }
    }

    for (auto sceneObjectPtr: scene.SceneObjects) {
        auto sceneObject = *sceneObjectPtr;

        //@Performance: do this only when necessary, not on every frame. 
        auto model = 
            glm::scale(
                glm::translate(glm::mat4(1.0f), sceneObject.Translation), 
                    sceneObject.Scale);
        if (sceneObject.Rotation.x > 0.0001 || sceneObject.Rotation.x < 0.0001) {
            model = glm::rotate(model, sceneObject.Rotation.x, glm::vec3(1.0, 0.0, 0.0));
        }
        if (sceneObject.Rotation.y > 0.0001 || sceneObject.Rotation.y < 0.0001) {
            model = glm::rotate(model, sceneObject.Rotation.y, glm::vec3(0.0, 1.0, 0.0));
        }
        if (sceneObject.Rotation.z > 0.0001 || sceneObject.Rotation.z < 0.0001) {
            model = glm::rotate(model, sceneObject.Rotation.z, glm::vec3(0.0, 0.0, 1.0));
        }

        //find the light first
     


        for (auto objMesh: sceneObject.SceneObjectBuffers) {
            objMesh.vertexArray.Bind();
            objMesh.vertexArray.AddBufferAndBind(objMesh.vertexBuffer);
            objMesh.indexBuffer.Bind();
            objMesh.shader.Bind();
            objMesh.shader.SetUniformMat4f("u_projection", projection);
            objMesh.shader.SetUniformMat4f("u_view", view);
            objMesh.shader.SetUniformMat4f("u_model", model);
            objMesh.shader.SetUniformVec3("material.ambient", objMesh.material.ambient);
            objMesh.shader.SetUniformVec3("material.diffuse", objMesh.material.diffuse);
            objMesh.shader.SetUniformVec3("material.specular", objMesh.material.specular);
            objMesh.shader.SetUniform1f("material.shininess", objMesh.material.shininess);

            if (light != nullptr && light->Light.Enabled) {
                objMesh.shader.SetUniformVec3("light.ambient", light->Light.Ambient);
                objMesh.shader.SetUniformVec3("light.diffuse", light->Light.Diffuse);
                objMesh.shader.SetUniformVec3("light.specular", light->Light.Specular);
                objMesh.shader.SetUniformVec3("light.direction", light->Translation);
            }

            objMesh.shader.SetUniformMat4f("u_view", view);
            objMesh.shader.SetUniformMat4f("u_model", model);
            if (objMesh.texture.valid) {
                objMesh.shader.SetUniform1i("textureSampler", 0);
                objMesh.texture.Bind(0);
                if (objMesh.shader.IsDefault) {
                    objMesh.shader.SetUniform2f("u_colorSource", 0.0, 1.0);
                }
            } else {
                if (objMesh.shader.IsDefault) {
                    objMesh.shader.SetUniform2f("u_colorSource", 1.0, 0.0);
                }
            }

            if (objMesh.specularHighlight.valid) {
                objMesh.shader.SetUniform1i("specularSampler", 1);
                objMesh.specularHighlight.Bind(1);
            }

            if (objMesh.normalMap.valid) {
                objMesh.shader.SetUniform1i("normalSampler", 2);
                objMesh.normalMap.Bind(2);
            }

            glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);

            if (objMesh.texture.valid) {
                objMesh.texture.Unbind();
            }
            if (objMesh.specularHighlight.valid) {
                objMesh.specularHighlight.Unbind();
            }
            if (objMesh.normalMap.valid) {
                objMesh.normalMap.Unbind();
            }
        }

       
    }
}

void Renderer::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
