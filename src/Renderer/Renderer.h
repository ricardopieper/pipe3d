#pragma once
#include "VertexArray.h"
#include "IndexBuffer.h"
#include "Shader.h"
#include "Scene.h"
#include "Camera3D.h"
class Renderer {
public:

    Camera3D* Camera;

    void Clear();
    void Render(Scene& scene, Camera3D& camera, glm::mat4 projectionMatrix);
};