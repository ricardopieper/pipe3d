#pragma once 

#include <glad/glad.h>
#include <vector>
#include <string>
#include <iostream>
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "CubemapTexture.h"
#include "RenderingPipeline.h"
#include "Shader.h"
#include <memory>

class Skybox {
private:
    VertexBuffer vb;
    VertexArray va;
    
    std::shared_ptr<Shader> skyboxShader;
   
    float skyboxVertices[108] = {
        // positions          
        -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,

        -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f, -1.0f,  1.0f,
        -1.0f, -1.0f,  1.0f,

        -1.0f,  1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f,  1.0f,  1.0f,
        1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,

        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,
        1.0f, -1.0f,  1.0f
    };
public:
    CubemapTexture& Cubemap;
    const int vertices = 36;
    const int vertexSize = 3;
    
    Skybox(CubemapTexture& cubemap, std::shared_ptr<Shader> shader)
        :Cubemap(cubemap), skyboxShader(shader) {
        BufferLayout bufferLayout;
        bufferLayout.PushFloat(3);

        const int size = sizeof(float) * vertexSize * vertices;

        vb = VertexBuffer(&skyboxVertices, size);
        va = VertexArray(bufferLayout);
    };

    void Render(Camera3D& camera, glm::mat4& projection) {
        //remove translations (moving around), the box will always be in the same place
        glm::mat4 view = glm::mat4(glm::mat3(camera.GetViewMatrix())); 
        
        //disable writing to the depth buffer, it will always be rendered 
        //behind all objects
        glDepthMask(GL_FALSE);
        glDepthFunc(GL_LEQUAL);
        skyboxShader->Bind();
        skyboxShader->SetUniformMat4f("view", view);
        skyboxShader->SetUniformMat4f("projection", projection);
        va.AddBufferAndBind(vb);
        Cubemap.Bind(0);
        skyboxShader->SetUniform1i("skybox", 0);
        glDrawArrays(GL_TRIANGLES, 0, 36);
        //return to normal
        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
    }
};

