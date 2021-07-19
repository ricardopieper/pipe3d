#pragma once
#include <vector>
#include "Geometry.h"
#include "VertexArray.h"
#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include "Texture.h"
#include <memory>
#include <map>


struct SceneObjectElement {
    VertexArray vertexArray;
    VertexBuffer vertexBuffer;
    IndexBuffer indexBuffer;
    std::shared_ptr<Shader> shader;
    Texture texture;
    Texture specularHighlight;
    Texture normalMap;
    Texture reflectionMap;
    Material material;
    float reflectivity;
    float refractivity;
    float refractionRatio;
    BoundingBox boundingBox;
    SceneObjectElement(VertexArray vertexArray, VertexBuffer vertexBuffer, IndexBuffer indexBuffer, 
                      std::shared_ptr<Shader> shader, Texture texture, Texture specularHighlight, Texture normalMap,
                      Texture reflectionMap, Material material, BoundingBox boundingBox, 
                      float reflectivity, float refractivity, float refractionRatio):
        vertexArray(vertexArray), vertexBuffer(vertexBuffer), indexBuffer(indexBuffer), shader(shader),
        texture(texture), specularHighlight(specularHighlight),
        normalMap(normalMap), reflectionMap(reflectionMap), material(material), boundingBox(boundingBox),
        reflectivity(reflectivity), refractivity(refractivity), refractionRatio(refractionRatio) { }
};

struct ModelToRender {
    
    Geometry geometry;
    Material material;
    std::shared_ptr<Shader> shader;
    Texture texture;
    Texture normalMap;
    Texture specularHighlight;
    Texture reflectionMap;

    float reflectivity;
    float refractivity;
    float refractionRatio; 

    ModelToRender(Geometry geometry, Material material, std::shared_ptr<Shader> shader, 
                  Texture texture, Texture specularHighlight, Texture normalMap, 
                  Texture reflectionMap):
        geometry(geometry), texture(texture), shader(shader), material(material), 
        specularHighlight(specularHighlight), normalMap(normalMap), reflectionMap(reflectionMap) { 
        reflectivity = 0;
        refractivity = 0;
        refractionRatio = 0;
    }
};

struct LightProperties {
    glm::vec3 Ambient;
    glm::vec3 Diffuse;
    glm::vec3 Specular;
    float Constant;
    float Linear;
    float Quadratic;
    bool IsDirectional;
    bool IsPoint;
};

class SceneObject {
public:
    std::string Name = "Object";
    std::vector<SceneObjectElement> SceneObjectElements;
    LightProperties Light;

    glm::vec3 Translation;
    glm::vec3 Scale;
    glm::vec3 Rotation;

    bool Outlined;
    bool Enabled = true;
    bool IsDebugHelper = false;

    SceneObject(std::vector<SceneObjectElement> SceneObjectElements, LightProperties light): 
        SceneObjectElements(SceneObjectElements), Light(light)
    {
        Translation = glm::vec3(0.0f);
        Scale = glm::vec3(1.0f);
        Rotation = glm::vec3(0.0f);
        Outlined = false;
    }

    glm::mat4 ComputeModel() {
        auto translated = glm::translate(glm::mat4(1.0f), this->Translation);
        auto scaled = glm::scale(translated, this->Scale);
        auto model = scaled;
        if (this->Rotation.x > 0.0001 || this->Rotation.x < 0.0001)
        {
            model = glm::rotate(model, this->Rotation.x, glm::vec3(1.0, 0.0, 0.0));
        }
        if (this->Rotation.y > 0.0001 || this->Rotation.y < 0.0001)
        {
            model = glm::rotate(model, this->Rotation.y, glm::vec3(0.0, 1.0, 0.0));
        }
        if (this->Rotation.z > 0.0001 || this->Rotation.z < 0.0001)
        {
            model = glm::rotate(model, this->Rotation.z, glm::vec3(0.0, 0.0, 1.0));
        }
        return model;
    }

    void UpdateGeometryVertices(Geometry geometry) {
        if (SceneObjectElements.size() > 0) {
            SceneObjectElements[0].vertexBuffer.Update(geometry.VertexData.data(), sizeof(Vertex) * geometry.VertexData.size());
        }
    }

};

class Scene {
public:

    std::vector<std::shared_ptr<SceneObject>> SceneObjects;

    std::shared_ptr<SceneObject> FromGeometry(Geometry geo, Material material, std::shared_ptr<Shader> shader, 
        Texture texture, Texture specularHighlights, Texture normalMap, Texture reflectionMap) {
        BufferLayout layout;
        layout.PushFloat(3); //position
        layout.PushFloat(3); //color
        layout.PushFloat(3); //normal
        layout.PushFloat(2); //uv
        VertexArray va(layout);
        VertexBuffer vb = geo.GetVertexBuffer();
        IndexBuffer ib = geo.GetIndexBuffer();

        std::vector<SceneObjectElement> geometry;
        auto obj = SceneObjectElement(
            va, vb, ib, shader, texture, specularHighlights, 
            normalMap, reflectionMap, material, Geometry::ComputeBoundingBox(geo.VertexData), 0,0,0
        );

        geometry.push_back(obj);
        LightProperties light;
        light.IsDirectional = false;
        light.IsPoint = false;
        auto ptr = std::make_shared<SceneObject>(geometry, light);
        SceneObjects.push_back(ptr);
        return ptr;
    }

    std::shared_ptr<SceneObject> FromMeshes(std::vector<ModelToRender> renderableObjects) {
        std::vector<SceneObjectElement> allMeshes;
        BufferLayout layout;
        layout.PushFloat(3); //position
        layout.PushFloat(3); //color
        layout.PushFloat(3); //normal
        layout.PushFloat(2); //uv

        for (auto obj: renderableObjects) {
            VertexArray va(layout);
            VertexBuffer vb = obj.geometry.GetVertexBuffer();
            IndexBuffer ib = obj.geometry.GetIndexBuffer();
        
            auto mesh = SceneObjectElement(
                va, vb, ib, obj.shader, obj.texture, obj.specularHighlight,
                obj.normalMap, obj.reflectionMap, obj.material, Geometry::ComputeBoundingBox(obj.geometry.VertexData),
                obj.reflectivity, obj.refractivity, obj.refractionRatio
            );
            allMeshes.push_back(mesh);
        }  
        
        LightProperties light;
        light.IsDirectional = false;
        light.IsPoint = false;
        
        auto ptr = std::make_shared<SceneObject>(allMeshes, light);
        SceneObjects.push_back(ptr);
        return ptr;
    }

    ~Scene() {
        for (auto objptr : SceneObjects) {
            for (auto renderable: objptr->SceneObjectElements) {
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