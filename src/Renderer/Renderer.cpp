#include "Renderer.h"
#include "Scene.h"
#include <iostream>
#include <glad/glad.h>
#include <map>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

void Renderer::Render(RenderingContext &context)
{
    glm::mat4 view = context.CurrentCamera.GetViewMatrix();

    int lightIndex = 0;
    glStencilMask(0xff);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    
    for (auto sceneObjectPtr: context.CurrentScene.SceneObjects)
    {
        //glActiveTexture(0);
       
        auto sceneObject = *sceneObjectPtr;

        //@Performance: do this only when necessary, not on every frame.
        auto model = sceneObject.ComputeModel();

        for (auto objMesh : sceneObject.SceneObjectElements)
        {
            auto shader = *objMesh.shader;
            shader.Bind();

            glBindTexture(GL_TEXTURE_2D, 0);
            
            objMesh.vertexArray.Bind();
            objMesh.vertexArray.AddBufferAndBind(objMesh.vertexBuffer);
            objMesh.indexBuffer.Bind();
            if (shader.IsDefault)
            {
                int numLights = 0;
                for (auto lightObject :  context.CurrentScene.SceneObjects)
                {
                    if (lightObject->Light.IsDirectional)
                    {
                        std::string indexed = "directionalLights[" + std::to_string(numLights) + "]";
                        shader.SetUniformVec3(indexed + ".ambient", lightObject->Light.Ambient);
                        shader.SetUniformVec3(indexed + ".diffuse", lightObject->Light.Diffuse);
                        shader.SetUniformVec3(indexed + ".specular", lightObject->Light.Specular);
                        shader.SetUniformVec3(indexed + ".position", lightObject->Translation);
                        shader.SetUniform1f(indexed + ".constant", 0);
                        shader.SetUniform1f(indexed + ".linear", 0);
                        shader.SetUniform1f(indexed + ".quadratic", 0);
                        numLights++;
                    }
                }
                shader.SetUniform1i("numDirLights", numLights);
            }
            
            if (shader.IsDefault)
            {
                int numLights = 0;
                for (auto lightObject :  context.CurrentScene.SceneObjects)
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
            if (shader.IsDefault) {
                shader.SetUniformVec3("cameraPosition", context.CurrentCamera.Position);

                //shader.SetUniformVec3("material.ambient", objMesh.material.ambient);
               // shader.SetUniformVec3("material.diffuse", objMesh.material.diffuse);
                shader.SetUniformVec3("material.specular", objMesh.material.specular);
                shader.SetUniform1f("material.shininess", objMesh.material.shininess);
            }
            
            if (shader.IsDefault) {

                if (objMesh.texture.Valid)
                {
                    objMesh.texture.Bind(0);
                    if (shader.IsDefault)
                        shader.SetUniform2f("u_colorSource", 0.0, 1.0);
                }
                else
                {
                    if (shader.IsDefault)
                        shader.SetUniform2f("u_colorSource", 1.0, 0.0);
                }

                if (objMesh.specularHighlight.Valid)
                    objMesh.specularHighlight.Bind(1);

                if (objMesh.normalMap.Valid)
                    objMesh.normalMap.Bind(2);
                
                if (objMesh.reflectionMap.Valid)
                    objMesh.reflectionMap.Bind(3);
                
                context.ShadowMap->GetTexture().Bind(4);
            }

            skybox.Cubemap.Bind(5);

            if (shader.IsDefault) {
                shader.SetUniform1f("reflectivity", objMesh.reflectivity);
                shader.SetUniform1f("refractivity", objMesh.refractivity);
                shader.SetUniform1f("refractionRatio", objMesh.refractionRatio);
            }

            shader.SetUniformMat4f("u_projection", context.Projection);
            shader.SetUniformMat4f("u_view", view);
            shader.SetUniformMat4f("u_model", model);
            shader.SetUniformMat4f("u_lightSpace", context.LightSpace);

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
                this->OutlineShader->SetUniformMat4f("u_projection", context.Projection);
                this->OutlineShader->SetUniformMat4f("u_view", view);
                auto upscaledModel = glm::scale(model, glm::vec3(1.1, 1.1, 1.1));
                this->OutlineShader->SetUniformMat4f("u_model", upscaledModel);
                this->OutlineShader->SetUniformMat4f("u_lightSpace", context.LightSpace);

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
        }
        //unbind all textures
        glActiveTexture(GL_TEXTURE0 + 0);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0 + 1);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0 + 2);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0 + 3);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0 + 4);
        glBindTexture(GL_TEXTURE_2D, 0);

        glActiveTexture(GL_TEXTURE0 + 5);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
    this->skybox.Render(context.CurrentCamera,  context.Projection);
}

void Renderer::Clear()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
