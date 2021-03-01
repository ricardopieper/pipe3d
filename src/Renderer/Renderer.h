#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Scene.h"
#include "Camera3D.h"
#include "Skybox.h"
#include <GLFW/glfw3.h>

struct GlobalSettings {
    bool debugMode = false;
};

static GlobalSettings RenderingGlobalSettings;

class Renderer {
public:

    std::shared_ptr<Shader> OutlineShader;
    GLFWwindow* window;
    Skybox& skybox;

    Renderer(GLFWwindow* window, std::shared_ptr<Shader> outlineShader, Skybox& skybox): 
        window(window), skybox(skybox), OutlineShader(outlineShader) {
    }

    void Clear();
    void Render(RenderingContext &context);
};