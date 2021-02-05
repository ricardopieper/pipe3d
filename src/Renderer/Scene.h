#pragma once
#include <vector>
#include "Geometry.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include <memory>

class SceneObject {
public:
    VertexArray _VertexArray;
    VertexBuffer _VertexBuffer;
    IndexBuffer _IndexBuffer;
    Shader _Shader;
    Texture _Texture;

    glm::vec3 Translation;
    glm::vec3 Scale;

    SceneObject(VertexArray va, VertexBuffer vb, 
                IndexBuffer ib, Shader shader, Texture texture): 
        _VertexArray(va), _VertexBuffer(vb), _IndexBuffer(ib), 
        _Shader(shader), _Texture(texture) {
        Translation = glm::vec3(0.0f);
        Scale = glm::vec3(1,1,1);
    }
    
};

class Scene {
public:
    std::vector<std::shared_ptr<SceneObject>> SceneObjects;

    std::shared_ptr<SceneObject> FromGeometry(Geometry* geo, Shader shader, Texture texture) {
        VertexArray va;
        VertexBuffer vb = geo->GetVertexBuffer();
        IndexBuffer ib = geo->GetIndexBuffer();
        
        auto ptr = std::make_shared<SceneObject>(va, vb, ib, shader, texture);
        SceneObjects.push_back(ptr);
        return ptr;
    }

    ~Scene() {
        for (auto objptr : SceneObjects) {
            auto obj = *objptr;
            obj._VertexArray.Unbind();
            obj._VertexArray.Dispose();

            obj._IndexBuffer.Unbind();
            obj._IndexBuffer.Dispose();
            
            obj._VertexBuffer.Unbind();
            obj._VertexBuffer.Dispose();
        }
    }
};