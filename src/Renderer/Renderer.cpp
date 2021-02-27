#include "Renderer.h"
#include "Scene.h"
#include <iostream>
#include <glad/glad.h>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Renderer::Render(Scene &scene, Camera3D &camera, glm::mat4 &projection)
{
    glm::mat4 view = camera.GetViewMatrix();

    int lightIndex = 0;
    glStencilMask(0xff);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    for (auto sceneObjectPtr : scene.SceneObjects)
    {
        auto sceneObject = *sceneObjectPtr;

        //@Performance: do this only when necessary, not on every frame.
        auto model =
            glm::scale(
                glm::translate(glm::mat4(1.0f), sceneObject.Translation),
                sceneObject.Scale);
        if (sceneObject.Rotation.x > 0.0001 || sceneObject.Rotation.x < 0.0001)
        {
            model = glm::rotate(model, sceneObject.Rotation.x, glm::vec3(1.0, 0.0, 0.0));
        }
        if (sceneObject.Rotation.y > 0.0001 || sceneObject.Rotation.y < 0.0001)
        {
            model = glm::rotate(model, sceneObject.Rotation.y, glm::vec3(0.0, 1.0, 0.0));
        }
        if (sceneObject.Rotation.z > 0.0001 || sceneObject.Rotation.z < 0.0001)
        {
            model = glm::rotate(model, sceneObject.Rotation.z, glm::vec3(0.0, 0.0, 1.0));
        }

        for (auto objMesh : sceneObject.SceneObjectBuffers)
        {
            auto shader = *objMesh.shader;
            shader.Bind();
            objMesh.vertexArray.Bind();
            objMesh.vertexArray.AddBufferAndBind(objMesh.vertexBuffer, this->vertexBufferLayout);
            objMesh.indexBuffer.Bind();
            {
                int numLights = 0;
                for (auto lightObject : scene.SceneObjects)
                {
                    if (lightObject->Light.IsDirectional)
                    {
                        std::string indexed = "directionalLights[" + std::to_string(numLights) + "]";
                        shader.SetUniformVec3(indexed + ".ambient", lightObject->Light.Ambient);
                        shader.SetUniformVec3(indexed + ".diffuse", lightObject->Light.Diffuse);
                        shader.SetUniformVec3(indexed + ".specular", lightObject->Light.Specular);
                        shader.SetUniformVec3(indexed + ".position", lightObject->Translation);
                        numLights++;
                    }
                }
                shader.SetUniform1i("numDirLights", numLights);
            }
            
            {
                int numLights = 0;
                for (auto lightObject : scene.SceneObjects)
                {
                    if (lightObject->Light.IsPoint)
                    {
                        std::string indexed = "pointLights[" + std::to_string(numLights) + "]";
                        shader.SetUniformVec3(indexed + ".ambient", lightObject->Light.Ambient);
                        shader.SetUniformVec3(indexed + ".diffuse", lightObject->Light.Diffuse);
                        shader.SetUniformVec3(indexed + ".specular", lightObject->Light.Specular);
                        shader.SetUniformVec3(indexed + ".position", lightObject->Translation);
                        shader.SetUniform1f(indexed + ".constant", lightObject->Light.Constant);
                        shader.SetUniform1f(indexed + ".linear", lightObject->Light.Linear);
                        shader.SetUniform1f(indexed + ".quadratic", lightObject->Light.Quadratic);
                        numLights++;
                    }
                }
                shader.SetUniform1i("numPointLights", numLights);
            }

            shader.SetUniformVec3("material.ambient", objMesh.material.ambient);
            shader.SetUniformVec3("material.diffuse", objMesh.material.diffuse);
            shader.SetUniformVec3("material.specular", objMesh.material.specular);
            shader.SetUniform1f("material.shininess", objMesh.material.shininess);

            if (objMesh.texture.valid)
            {
                shader.SetUniform1i("textureSampler", 0);
                objMesh.texture.Bind(0);
                if (shader.IsDefault)
                {
                    shader.SetUniform2f("u_colorSource", 0.0, 1.0);
                }
            }
            else
            {
                if (shader.IsDefault)
                {
                    shader.SetUniform2f("u_colorSource", 1.0, 0.0);
                }
            }

            if (objMesh.specularHighlight.valid)
            {
                shader.SetUniform1i("specularSampler", 1);
                objMesh.specularHighlight.Bind(1);
            }

            if (objMesh.normalMap.valid)
            {
                shader.SetUniform1i("normalSampler", 2);
                objMesh.normalMap.Bind(2);
            }

            shader.SetUniformMat4f("u_projection", projection);
            shader.SetUniformMat4f("u_view", view);
            shader.SetUniformMat4f("u_model", model);

            //When an object must be outlined, we do it by using the stencil buffer
            //@TODO: Outlined objects have to be rendered first for the code below to work... or last?
            if (sceneObject.Outlined)
            {
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
                if (RenderingGlobalSettings.debugMode)
                {
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
                glStencilOp(GL_INCR, GL_INCR, GL_INCR);

                shader.Unbind();
                this->OutlineShader->Bind();
                //Render the object as red (or rather, the outline)
                this->OutlineShader->SetUniform3f("u_overridenColor", 1, 0, 0);
                this->OutlineShader->SetUniformMat4f("u_projection", projection);
                this->OutlineShader->SetUniformMat4f("u_view", view);
                auto upscaledModel = glm::scale(model, glm::vec3(1.1, 1.1, 1.1));
                this->OutlineShader->SetUniformMat4f("u_model", upscaledModel);

                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
                if (RenderingGlobalSettings.debugMode)
                {
                    glfwSwapBuffers(this->window);
                }
                this->OutlineShader->SetUniform3f("u_overridenColor", 0, 0, 0);
                this->OutlineShader->Unbind();

                //clean up

                glEnable(GL_DEPTH_TEST);
            }
            else
            {
                //Draw normally and do not affect stencil buffer
                glStencilMask(0x00);
                glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
                glDrawElements(GL_TRIANGLES, objMesh.indexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
                if (RenderingGlobalSettings.debugMode)
                {
                    glfwSwapBuffers(this->window);
                }
            }

            shader.Unbind();
            if (objMesh.texture.valid)
            {
                objMesh.texture.Unbind();
            }
            if (objMesh.specularHighlight.valid)
            {
                objMesh.specularHighlight.Unbind();
            }
            if (objMesh.normalMap.valid)
            {
                objMesh.normalMap.Unbind();
            }
        }
    }
}

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
