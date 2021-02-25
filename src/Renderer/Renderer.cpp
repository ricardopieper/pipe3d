#include "Renderer.h"
#include "Scene.h"
#include <iostream>
#include <glad/glad.h>
#include <map>

void Renderer::Render(Scene& scene, Camera3D& camera, glm::mat4 projection) {
    glm::mat4 view = camera.GetViewMatrix();
    std::shared_ptr<SceneObject> light;
    for (auto sceneObjectPtr: scene.SceneObjects) {
        if (sceneObjectPtr->Light.Enabled) {
            light = sceneObjectPtr;
            break;
        }
    }
    glStencilMask(0xff);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    std::map<float, decltype(scene.SceneObjects[0])> sorted;
    
    for (auto sceneObjectPtr: scene.SceneObjects) {
        auto sceneObject = *sceneObjectPtr;
        //@Performance: do this only when necessary, not on every frame. 
        auto model = 
            glm::scale(
                glm::translate(glm::mat4(1.0f), sceneObject.Translation), 
                    sceneObject.Scale);
        auto cameraPos = camera.Position;
        

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

       

        for (auto objMesh: sceneObject.SceneObjectBuffers) {
            objMesh.vertexArray.Bind();
            objMesh.vertexArray.AddBufferAndBind(objMesh.vertexBuffer);
            objMesh.indexBuffer.Bind();
            objMesh.shader.Bind();
 
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

 
            objMesh.shader.SetUniformMat4f("u_projection", projection);
            objMesh.shader.SetUniformMat4f("u_view", view);
            objMesh.shader.SetUniformMat4f("u_model", model);
            
            //When an object must be outlined, we do it by using the stencil buffer
            //@TODO: Outlined objects have to be rendered first for the code below to work... or last?
            if (sceneObject.Outlined) {
                //The strategy is to use a single-color shader to render the object slightly upscaled, 
                //on top of the currently rendered object with the normal shader.
                //however, if we do this, we will remove the original object from the screen. It will just show a white box
                //or whatever the chosen color is.

                //Therefore, first we mark the section of the screen we want to preserve as-is
                //using the stencil buffer.

                //Wherever the object was draw (passed stencil and depth test), we REPLACE the stencil buffer 
                //with the value set in glStencilFunc (1) 
                glEnable(GL_DEPTH_TEST);
                glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE); 
                
                glStencilFunc(GL_ALWAYS, 1, 0xFF); //for now, just make all fragments pass the stencil test and write 1 to it
                 //store value as-is
                glStencilMask(0xFF);
                
                //draw the object (using the currently bound shader)
                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
                 if (debugRendering) {
                    glfwSwapBuffers(this->window);
                }
                //by this point, the stencil buffer will have 1's wherever the object is rendered
                //but we do not want to render *over* the places where we have 1s
                //or rather, we want to render over the places where the stencil is not equal 1
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF); 
                glStencilMask(0xFF);

                //we also don't care about depth test, we want to draw the outline even if we look it through a wall                                
                glDisable(GL_DEPTH_TEST); //this makes all depth test pass

                //the area we draw again will be increased
                glStencilOp(GL_KEEP, GL_KEEP, GL_INCR); 

                objMesh.shader.Unbind();
                this->OutlineShader.Bind();
                //Render the object as red (or rather, the outline)
                this->OutlineShader.SetUniform3f("u_overridenColor", 1, 0, 0);
                this->OutlineShader.SetUniformMat4f("u_projection", projection);
                this->OutlineShader.SetUniformMat4f("u_view", view);
                auto upscaledModel = glm::scale(model, glm::vec3(1.1, 1.1, 1.1));
                this->OutlineShader.SetUniformMat4f("u_model", upscaledModel);
                
                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
                if (debugRendering) {
                    glfwSwapBuffers(this->window);
                }
                this->OutlineShader.SetUniform3f("u_overridenColor", 0, 0, 0);
                this->OutlineShader.Unbind();
                
                //clean up
               
                glEnable(GL_DEPTH_TEST);
                
            } else {
                //Draw normally and do not affect stencil buffer 
                glStencilMask(0x00);
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF); 
                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
                if (debugRendering) {
                    glfwSwapBuffers(this->window);
                }
            }

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
