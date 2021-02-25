#pragma once
#include <vector>
#include "Geometry.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include <memory>
#include <map>

struct SceneObjectBuffer {
    VertexArray vertexArray;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    Shader shader;
    Texture texture;
    Texture specularHighlight;
    Texture normalMap;
    Material material;
    SceneObjectBuffer(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, 
                      Shader shader, Texture texture, Texture specularHighlight,Texture normalMap,
                      Material material):
        vertexArray(vertexArray), vertexBuffer(vertexBuffer), indexBuffer(indexBuffer),
        shader(shader), texture(texture), specularHighlight(specularHighlight),
        normalMap(normalMap), material(material) { }
};

struct ModelToRender {
    Geometry geometry;
    Material material;
    Shader shader;
    Texture texture;
    Texture normalMap;
    Texture specularHighlight;
    ModelToRender(Geometry geometry, Material material, Shader shader, 
        Texture texture, Texture specularHighlight, Texture normalMap):
        geometry(geometry), shader(shader), texture(texture), 
        material(material), specularHighlight(specularHighlight), normalMap(normalMap) { }
};

struct LightProperties {
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
    bool Enabled;
};

class SceneObject {
public:

    std::vector<SceneObjectBuffer> SceneObjectBuffers;
    LightProperties Light;

    glm::vec3 Translation;
    glm::vec3 Scale;
    glm::vec3 Rotation;

    bool Outlined;

    SceneObject(std::vector<SceneObjectBuffer> sceneObjectBuffers, LightProperties light) : SceneObjectBuffers(sceneObjectBuffers), Light(light)
    {
        Translation = glm::vec3(0.0f);
        Scale = glm::vec3(1.0f);
        Rotation = glm::vec3(0.0f);
        Outlined = false;
    }
};

class Scene {
public:

    std::vector<std::shared_ptr<SceneObject>> SceneObjects;

    std::shared_ptr<SceneObject> FromGeometry(Geometry geo, Material material, Shader shader, 
        Texture texture, Texture specularHighlights, Texture normalMap) {
        VertexArray va;
        VertexBuffer vb = geo.GetVertexBuffer();
        IndexBuffer ib = geo.GetIndexBuffer();

        std::vector<SceneObjectBuffer> geometry;
        auto obj = SceneObjectBuffer(
            va, vb, ib, shader, texture, specularHighlights, normalMap, material
        );

        geometry.push_back(obj);
        LightProperties light;
        light.Enabled = false;
        auto ptr = std::make_shared<SceneObject>(geometry, light);
        SceneObjects.push_back(ptr);
        return ptr;
    }

    std::shared_ptr<SceneObject> FromMeshes(std::vector<ModelToRender> renderableObjects) {
        std::vector<SceneObjectBuffer> allMeshes;

        for (auto obj: renderableObjects) {
            VertexArray va;
            VertexBuffer vb = obj.geometry.GetVertexBuffer();
            IndexBuffer ib = obj.geometry.GetIndexBuffer();
        
            auto mesh = SceneObjectBuffer(
                va, vb, ib, obj.shader, obj.texture, obj.specularHighlight,
                obj.normalMap, obj.material
            );
            allMeshes.push_back(mesh);
        }  
        
        LightProperties light;
        light.Enabled = false;
        
        auto ptr = std::make_shared<SceneObject>(allMeshes, light);
        SceneObjects.push_back(ptr);
        return ptr;
    }

    ~Scene() {
        for (auto objptr : SceneObjects) {
            for (auto renderable: objptr->SceneObjectBuffers) {
                renderable.vertexArray.Unbind();
                renderable.vertexArray.Dispose();

                renderable.indexBuffer.Unbind();
                renderable.indexBuffer.Dispose();
                
                renderable.vertexBuffer.Unbind();
                renderable.vertexBuffer.Dispose();
            }
        }
    }
};