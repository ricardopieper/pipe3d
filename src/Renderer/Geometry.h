#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexBuffer.h"
#include "IndexBuffer.h"

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
};