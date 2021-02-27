#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Scene.h"
#include "Camera3D.h"
#include <GLFW/glfw3.h>

struct GlobalSettings {
    bool debugMode = false;
};

static GlobalSettings RenderingGlobalSettings;

class Renderer {
public:

    std::shared_ptr<Shader> OutlineShader;
    BufferLayout vertexBufferLayout;
    GLFWwindow* window;

    Renderer(GLFWwindow* window, std::shared_ptr<Shader> outlineShader): window(window), OutlineShader(outlineShader) {
        BufferLayout layout;
        layout.PushFloat(3); //position
        layout.PushFloat(3); //color
        layout.PushFloat(3); //normal
        layout.PushFloat(2); //uv
        this->vertexBufferLayout = layout;
    }

    void Clear();
    void Render(Scene& scene, Camera3D& camera, glm::mat4& projectionMatrix);
};