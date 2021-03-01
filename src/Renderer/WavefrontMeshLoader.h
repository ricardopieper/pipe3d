#pragma once

#include <vector>
#include <glad/glad.h>
#include <assert.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <stdio.h>
#include <cstring>
#include <string>
#include <map>
#include "Geometry.h"

struct WavefrontMesh
{
    std::string meshName;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec2> uvs;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> vertexIndices;
    glm::vec3 ambientRgb;
    glm::vec3 diffuseRgb;
    glm::vec3 specularRgb;
    float specularStrength;
    int illuminationModel;
    //map_Ka
    std::string reflectivityTexturePath;
    //map_Kd
    std::string diffuseTexturePath;
    //map_Ks
    std::string specularTexturePath;
    //map_d
    std::string alphaTexturePath;
    //map_bump
    std::string bumpTexturePath;

    std::string materialName;

    Geometry ConvertToGeometry() {
        Geometry geom;
        
        for (int i = 0; i < vertices.size(); i++) {
            auto vertex = vertices[i];
            auto normal = normals[i];
            auto uv = uvs[i];
            Vertex v(vertex, diffuseRgb, normal, uv);
            geom.VertexData.push_back(v);
        }

        geom.Indices = vertexIndices;
        
        return geom;
    }
    Material GetMaterial() {
        //900 seems to be blender max value
        
        Material m;
        m.ambient = this->ambientRgb;
        m.diffuse = this->diffuseRgb;
        m.specular = this->specularRgb;
       // if (illuminationModel == 1) {
       //     m.shininess = 0;
       // } else {
            m.shininess = specularStrength;
        //}
        return m;
    }
};

struct WavefrontMaterial
{
    glm::vec3 ambientRgb;
    glm::vec3 diffuseRgb;
    glm::vec3 specularRgb;
    float specularStrength;
    int illuminationModel;
    std::string materialName;
     //map_Ka
    std::string reflectivityTexturePath;
    //map_Kd
    std::string diffuseTexturePath;
    //map_Ks
    std::string specularTexturePath;
    //map_d
    std::string alphaTexturePath;
    //map_bump
    std::string bumpTexturePath;
};

struct OpenglVector
{
    glm::vec3 vertex;
    glm::vec2 uv;
    glm::vec3 normal;
    bool operator<(const OpenglVector that) const
    {
        return memcmp((void *)this, (void *)&that, sizeof(OpenglVector)) > 0;
    };
};

class WavefrontMeshLoader
{
private:
    static void buildIndex(WavefrontMesh &mesh)
    {

        std::vector<OpenglVector> vectors;
        std::vector<unsigned int> openglIndices;

        for (int i = 0; i < mesh.vertices.size(); i++)
        {
            OpenglVector vec = {
                mesh.vertices[i],
                mesh.uvs[i],
                mesh.normals[i],
            };
            vectors.push_back(vec);
        }

        std::map<OpenglVector, unsigned int> indexedVectors;
        std::vector<glm::vec3> vertices;
        std::vector<glm::vec3> normals;
        std::vector<glm::vec2> uvs;
        int reused = 0;
        for (int i = 0; i < vectors.size(); i++)
        {
            OpenglVector vec = vectors[i];
            auto it = indexedVectors.find(vec);
            if (it != indexedVectors.end())
            {
                reused++;
                openglIndices.push_back(it->second);
            }
            else
            {
                vertices.push_back(mesh.vertices[i]);
                normals.push_back(mesh.normals[i]);
                uvs.push_back(mesh.uvs[i]);
                int newIndex = vertices.size() - 1;
                openglIndices.push_back(newIndex);
                indexedVectors[vec] = newIndex;
            }
        }
        printf("Reused %i vertices out of %lu\n", reused, vectors.size());

        mesh.vertexIndices = openglIndices;
        mesh.vertices = vertices;
        mesh.normals = normals;
        mesh.uvs = uvs;
    }

public:
    static std::vector<WavefrontMesh> Load(const char *path, const char *material)
    {
        FILE *file = fopen(path, "r");
        if (file == NULL)
        {
            printf("Impossible to open the file !\n");
            return std::vector<WavefrontMesh>();
        }
        char line[1024];
        std::vector<WavefrontMesh> meshes;
        {
            WavefrontMesh current;

            std::vector<glm::vec3> vertices;
            std::vector<glm::vec3> normals;
            std::vector<glm::vec2> uvs;

            int tmp_verticesUntilNow = 0;
            int tmp_normalsUntilNow = 0;
            int tmp_uvUntilNow = 0;

            int verticesUntilNow = 0;
            int normalsUntilNow = 0;
            int uvUntilNow = 0;

            while (true)
            {
                int res = fscanf(file, "%s", line);
                if (res == EOF)
                {
                    if (current.vertices.size() > 0)
                    {
                        meshes.push_back(current);
                    }
                    break;
                }
                else if (strcmp(line, "o") == 0 || strcmp(line, "g") == 0)
                {
                    if (current.vertices.size() > 0)
                    {
                        meshes.push_back(current);
                        current = WavefrontMesh();
                        if (strcmp(line, "o") == 0) {
                            verticesUntilNow = tmp_verticesUntilNow;
                            normalsUntilNow = tmp_normalsUntilNow;
                            uvUntilNow = tmp_uvUntilNow;
                            vertices = std::vector<glm::vec3>();
                            normals = std::vector<glm::vec3>();
                            uvs = std::vector<glm::vec2>();
                        }
                    }
                    char name[512];
                    fscanf(file, "%s\n", (char*)name);
                    std::string buffer(name);
                    current.meshName = buffer;
                }
                else if (strcmp(line, "usemtl") == 0)
                {
                    char name[512];
                    fscanf(file, "%s\n", (char*)name);
                    std::string buffer(name);
                    current.materialName = buffer;
                }
                else if (strcmp(line, "v") == 0)
                {
                    tmp_verticesUntilNow += 1;
                    glm::vec3 vertex;
                    fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
                    vertices.push_back(vertex);
                }
                else if (strcmp(line, "vn") == 0)
                {
                    tmp_normalsUntilNow += 1;
                    glm::vec3 normal;
                    fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
                    normals.push_back(normal);
                }
                else if (strcmp(line, "vt") == 0)
                {
                    tmp_uvUntilNow += 1;
                    glm::vec3 uvTextureCoord;
                    int matches = fscanf(file, "%f %f %f\n",
                           &uvTextureCoord.x, &uvTextureCoord.y, &uvTextureCoord.z);

                    uvs.push_back(glm::vec2(uvTextureCoord.x, uvTextureCoord.y));
                }
                else if (strcmp(line, "f") == 0)
                {
                    unsigned int vertexIndices[4];
                    unsigned int uvIndices[4];
                    unsigned int normalIndices[4];

                    int matches2 = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d %d/%d/%d\n",
                           &vertexIndices[0], &uvIndices[0], &normalIndices[0],
                           &vertexIndices[1], &uvIndices[1], &normalIndices[1],
                           &vertexIndices[2], &uvIndices[2], &normalIndices[2],
                           &vertexIndices[3], &uvIndices[3], &normalIndices[3]);

                    if (matches2 == 9) {
                        for (int i = 0; i < 3; i++)
                        {
                            int vertexIndex = (vertexIndices[i] - 1) - verticesUntilNow;
                            int uvIndex = (uvIndices[i] - 1) - uvUntilNow;
                            int normalIndex = (normalIndices[i] - 1) - normalsUntilNow;

                            current.vertices.push_back(vertices[vertexIndex]);
                            current.normals.push_back(normals[normalIndex]);
                            current.uvs.push_back(uvs[uvIndex]);
                        }
                    } else if (matches2 == 12) {
                        int winding[6] = {0,1,2,0,2,3};
                        for (int i = 0; i < 6; i++)
                        {
                            int index = winding[i];
                            int vertexIndex = (vertexIndices[index] - 1) - verticesUntilNow;
                            int uvIndex = (uvIndices[index] - 1) - uvUntilNow;
                            int normalIndex = (normalIndices[index] - 1) - normalsUntilNow;

                            current.vertices.push_back(vertices[vertexIndex]);
                            current.normals.push_back(normals[normalIndex]);
                            current.uvs.push_back(uvs[uvIndex]);
                        }
                    }                   
                }
                else
                {
                    fgets(line, 1024, file);
                }
            }
            
        }
       

        for (auto& mesh: meshes) {
            buildIndex(mesh);
        }

        if (material == nullptr)
        {
            return meshes;
        }

        FILE *materialFile = fopen(material, "r");
        if (materialFile == NULL)
        {
            printf("Impossible to open the file !\n");
            return std::vector<WavefrontMesh>();
        }

        std::vector<WavefrontMaterial> materials;
        WavefrontMaterial currentMaterial;

        while (true)
        {
            int res = fscanf(materialFile, "%s", line);
            if (res == EOF)
            {
                if (currentMaterial.materialName.size() > 0)
                {
                    materials.push_back(currentMaterial);
                }
                break;
            }

            if (strcmp(line, "newmtl") == 0)
            {
                if (currentMaterial.materialName.size() > 0)
                {
                    materials.push_back(currentMaterial);
                    currentMaterial = WavefrontMaterial();
                }
                char name[512];
                fscanf(materialFile, "%s\n", (char*)name);
                std::string buffer(name);
                currentMaterial.materialName = buffer;
            }
            else if (strcmp(line, "Ka") == 0)
            {
                glm::vec3 ambientRgb;
                fscanf(materialFile, "%f %f %f\n", &ambientRgb.r, &ambientRgb.g, &ambientRgb.b);
                currentMaterial.ambientRgb = ambientRgb;
            }
            else if (strcmp(line, "Kd") == 0)
            {
                glm::vec3 diffuseRgb;
                fscanf(materialFile, "%f %f %f\n", &diffuseRgb.r, &diffuseRgb.g, &diffuseRgb.b);
                currentMaterial.diffuseRgb = diffuseRgb;
            }
            else if (strcmp(line, "Ks") == 0)
            {
                glm::vec3 specularRgb;
                fscanf(materialFile, "%f %f %f\n", &specularRgb.r, &specularRgb.g, &specularRgb.b);
                currentMaterial.specularRgb = specularRgb;
            }
            else if (strcmp(line, "Ns") == 0)
            {
                float specularStrength;
                fscanf(materialFile, "%f\n", &specularStrength);
                currentMaterial.specularStrength = specularStrength;
            }
             else if (strcmp(line, "map_Ka") == 0)
            {
                char name[512];
                fscanf(materialFile, "%s\n", (char*)name);
                std::string buffer(name);
                currentMaterial.reflectivityTexturePath = name;
            }
             else if (strcmp(line, "map_Kd") == 0)
            {
                char name[512];
                fscanf(materialFile, "%s\n", (char*)name);
                std::string buffer(name);
                currentMaterial.diffuseTexturePath = name;
            }
             else if (strcmp(line, "map_Ks") == 0)
            {
                char name[512];
                fscanf(materialFile, "%s\n", (char*)name);
                std::string buffer(name);
                currentMaterial.specularTexturePath = name;
            }
            else if (strcmp(line, "map_d") == 0)
            {
                char name[512];
                fscanf(materialFile, "%s\n", (char*)name);
                std::string buffer(name);
                currentMaterial.alphaTexturePath = name;
            }
             else if (strcmp(line, "map_Bump") == 0)
            {
                char name[512];
                fscanf(materialFile, "%s\n", (char*)name);
                std::string buffer(name);
                currentMaterial.bumpTexturePath = name;
            }
            else if (strcmp(line, "illum") == 0)
            {
                int illuminationModel;
                fscanf(materialFile, "%i\n", &illuminationModel);
                currentMaterial.illuminationModel = illuminationModel;
            }
            else
            {
                fgets(line, 1024, materialFile);
            }
        }

        for (auto &mesh : meshes)
        {
            for (auto &mat : materials)
            {
                if (mesh.materialName == mat.materialName)
                {
                    mesh.ambientRgb = mat.ambientRgb;
                    mesh.diffuseRgb = mat.diffuseRgb;
                    mesh.specularRgb = mat.specularRgb;

                    mesh.specularStrength = mat.specularStrength;
                    mesh.illuminationModel = mat.illuminationModel;

                    mesh.reflectivityTexturePath = mat.reflectivityTexturePath;
                    mesh.diffuseTexturePath = mat.diffuseTexturePath;
                    mesh.specularTexturePath = mat.specularTexturePath;
                    mesh.alphaTexturePath = mat.alphaTexturePath;
                    mesh.bumpTexturePath = mat.bumpTexturePath;
                  
                    break;
                }
            }
        }

        return meshes;
    }
};
