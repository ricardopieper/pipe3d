#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Scene.h"
#include "Camera3D.h"
#include <GLFW/glfw3.h>

class Renderer {
public:

    Camera3D* Camera;
    Shader& OutlineShader;
    GLFWwindow* window;
    bool debugRendering;

    Renderer(GLFWwindow* window, Shader& outlineShader): window(window), OutlineShader(outlineShader) {
        debugRendering = false;
    }

    void Clear();
    void Render(Scene& scene, Camera3D& camera, glm::mat4 projectionMatrix);
};