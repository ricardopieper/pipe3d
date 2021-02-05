#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexBuffer.h"
#include "IndexBuffer.h"



struct Vertex {
    float x, y, z; //position
    float r, g, b; //color
    float nx, ny, nz; //normal
    float tex_u, tex_v; //uv
    float specularStrength;     
    Vertex(glm::vec3 xyz, glm::vec3 rgb, glm::vec3 normal,  
           glm::vec2 uv, float specularStrength): 
        x(xyz.x), y(xyz.y), z(xyz.z), 
        r(rgb.r), g(rgb.g), b(rgb.b),
        nx(normal.x), ny(normal.y), nz(normal.z),
        tex_u(uv.x), tex_v(uv.y),
        specularStrength(specularStrength) {}
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