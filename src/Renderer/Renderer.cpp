#include "Renderer.h"
#include "Scene.h"

#include <glad/glad.h>


void Renderer::Render(Scene& scene, Camera3D& camera, glm::mat4 projection) {
    glm::mat4 view = camera.GetViewMatrix();

    for (auto sceneObjectPtr: scene.SceneObjects) {
        auto sceneObject = *sceneObjectPtr;
        auto model = 
            glm::scale(
                glm::translate(glm::mat4(1.0f), sceneObject.Translation), 
                    sceneObject.Scale);

        sceneObject._VertexBuffer.Bind();
        sceneObject._VertexArray.AddBufferAndBind(sceneObject._VertexBuffer);
        sceneObject._IndexBuffer.Bind();
        sceneObject._Shader.Bind();
        sceneObject._Shader.SetUniformMat4f("u_projection", projection);
        sceneObject._Shader.SetUniformMat4f("u_view", view);
        sceneObject._Shader.SetUniformMat4f("u_model", model);

        if (sceneObject._Texture.valid) {
            sceneObject._Texture.Bind();
            if (sceneObject._Shader.IsDefault) {
                sceneObject._Shader.SetUniform2f("u_colorSource", 0.0, 1.0);
            }
        } else {
            if (sceneObject._Shader.IsDefault) {
                sceneObject._Shader.SetUniform2f("u_colorSource", 1.0, 0.0);
            }
        }

        glDrawElements(GL_TRIANGLES, sceneObject._IndexBuffer.GetCount(), GL_UNSIGNED_INT, nullptr);
    }
}

void Renderer::Clear() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}
