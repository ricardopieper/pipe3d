#pragma once
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "VertexBuffer.h"
#include "IndexBuffer.h"
#include <limits>


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

struct BoundingBox {
public:
    glm::vec3 min;
    glm::vec3 max;
    

    static BoundingBox MinMaxBox() {
        constexpr float min = std::numeric_limits<float>().lowest();
        constexpr float max = std::numeric_limits<float>().max();
        BoundingBox box;
        box.min = glm::vec3(max);
        box.max = glm::vec3(min);
        return box;
    };


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

    static Geometry FromBoundingBox(BoundingBox& box) {
      
        Geometry geom;

        glm::vec3 red(1, 0, 0);
        glm::vec3 normal(1, 0, 0);
        glm::vec2 uv(1, 0);

        glm::vec3 front1 = box.max;
        glm::vec3 front2 = glm::vec3(box.max.x, box.min.y, box.max.z);
        glm::vec3 front3 = glm::vec3(box.min.x, box.min.y, box.max.z);
        glm::vec3 front4 = glm::vec3(box.min.x, box.max.y, box.max.z);


        glm::vec3 back1 = glm::vec3(box.min.x, box.max.y, box.min.z);
        glm::vec3 back2 = box.min;
        glm::vec3 back3 = glm::vec3(box.max.x, box.min.y, box.min.z);
        glm::vec3 back4 = glm::vec3(box.max.x, box.max.y, box.min.z);


        //Front Face
        //triangle 1, max point
        geom.VertexData.emplace_back(front1, red, normal, uv);
        //triangle 1, below max at same level as min
        geom.VertexData.emplace_back(front2, red, normal, uv);
        //triangle 1, besides previous one
        geom.VertexData.emplace_back(front3, red, normal, uv);

        //triangle 2, same as previous one
        geom.VertexData.emplace_back(front3, red, normal, uv);
        //triangle 2, right above previous one
        geom.VertexData.emplace_back(front4, red, normal, uv);
        //triangle 2, same as the first one
        geom.VertexData.emplace_back(front1, red, normal, uv);


        //Top Face
        //triangle 1, behind max
        geom.VertexData.emplace_back(back4, red, normal, uv);
        //triangle 1, same as max
        geom.VertexData.emplace_back(front1, red, normal, uv);
        //triangle 1, besides max
        geom.VertexData.emplace_back(front4, red, normal, uv);

        //triangle 2, same as previous one
        geom.VertexData.emplace_back(front4, red, normal, uv);
        //triangle 2, behind previous one (above min)
        geom.VertexData.emplace_back(back1, red, normal, uv);
        //triangle 2, same as triangle 1 first
        geom.VertexData.emplace_back(back4, red, normal, uv);



        //Back face
        //triangle 1, above min
        geom.VertexData.emplace_back(back1, red, normal, uv);
        //triangle 1, min
        geom.VertexData.emplace_back(back2, red, normal, uv);
        //triangle 1, besides min
        geom.VertexData.emplace_back(back3, red, normal, uv);

        //triangle 2, same as previous one
        geom.VertexData.emplace_back(back3, red, normal, uv);
        //triangle 2, above previous one
        geom.VertexData.emplace_back(back4, red, normal, uv);
        //triangle 2, same as triangle 1 first
        geom.VertexData.emplace_back(back1, red, normal, uv);


        //Bottom face
        //triangle 1, below max
        geom.VertexData.emplace_back(front2, red, normal, uv);
        //triangle 1, behind previous
        geom.VertexData.emplace_back(back3, red, normal, uv);
        //triangle 1, min
        geom.VertexData.emplace_back(back2, red, normal, uv);

        //triangle 2, same as previous one
        geom.VertexData.emplace_back(back2, red, normal, uv);
        //triangle 2, above previous one
        geom.VertexData.emplace_back(front3, red, normal, uv);
        //triangle 2, same as triangle 1 first
        geom.VertexData.emplace_back(front2, red, normal, uv);



        //Left face
        //triangle 1, behind max
        geom.VertexData.emplace_back(back4, red, normal, uv);
        //triangle 1, below previous
        geom.VertexData.emplace_back(back3, red, normal, uv);
        //triangle 1, besides previous
        geom.VertexData.emplace_back(front2, red, normal, uv);

        //triangle 2, same as previous one
        geom.VertexData.emplace_back(front2, red, normal, uv);
        //triangle 2, above previous one
        geom.VertexData.emplace_back(front1, red, normal, uv);
        //triangle 2, same as triangle 1 first
        geom.VertexData.emplace_back(back4, red, normal, uv);


        //Right face
        //triangle 1, besides max
        geom.VertexData.emplace_back(front4, red, normal, uv);
        //triangle 1, below previous
        geom.VertexData.emplace_back(front3, red, normal, uv);
        //triangle 1, besides previous (min)
        geom.VertexData.emplace_back(back2, red, normal, uv);

        //triangle 2, same as previous one
        geom.VertexData.emplace_back(back2, red, normal, uv);
        //triangle 2, above previous one
        geom.VertexData.emplace_back(back1, red, normal, uv);
        //triangle 2, same as triangle 1 first
        geom.VertexData.emplace_back(front4, red, normal, uv);


        for (int i = 0; i < 36; i++) {
            geom.Indices.push_back(i);
        }

        return geom;
        

    };
};


