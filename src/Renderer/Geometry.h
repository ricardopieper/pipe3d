#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <limits>

struct BoundingBox {
    glm::vec3 min;
    glm::vec3 max;

    static BoundingBox MinMaxBox() {
        float min = std::numeric_limits<float>().min();
        float max = std::numeric_limits<float>().max();
        BoundingBox box;
        box.min = glm::vec3(max);
        box.max = glm::vec3(min);
        return box;
    }
};


struct Material {
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;

    static Material DefaultMaterial() {
        return DefaultColoredMaterial(glm::vec3(1.0));
    }

   static Material DefaultColoredMaterial(glm::vec3 rgbColor) {
        Material m;
        m.ambient = rgbColor;
        m.diffuse = rgbColor;
        m.specular = glm::vec3(0.0f);
        m.shininess = 0;
        return m;
    }

};

struct Vertex {
    glm::vec3 position; //position
    glm::vec3 color; //color
    glm::vec3 normal; //position
    glm::vec2 uv; //uv  
    Vertex(glm::vec3 position, glm::vec3 color, glm::vec3 normal,  
           glm::vec2 uv): 
        position(position), color(color), normal(normal),
        uv(uv) {}
};

struct Geometry {
public:
    std::vector<Vertex> VertexData;
    std::vector<unsigned int> Indices;

    VertexBuffer GetVertexBuffer() {
        return VertexBuffer(VertexData.data(), sizeof(Vertex) * VertexData.size());
    }

    IndexBuffer GetIndexBuffer() {
        return IndexBuffer(Indices.data(), Indices.size());
    }

    static BoundingBox ComputeBoundingBox(const std::vector<Vertex>& vertices) {
        BoundingBox box = BoundingBox::MinMaxBox();
        
        for (auto& vertex: vertices) {
            box.min.x = std::min(box.min.x, vertex.position.x);
            box.min.y = std::min(box.min.y, vertex.position.y);
            box.min.z = std::min(box.min.z, vertex.position.z);
            box.max.x = std::max(box.max.x, vertex.position.x);
            box.max.y = std::max(box.max.y, vertex.position.y);
            box.max.z = std::max(box.max.z, vertex.position.z);
        }
        return box;
    }   
};