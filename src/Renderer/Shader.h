#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <thread>
#include <vector>
class Shader {
private:
    unsigned int rendererId;
   
    std::unordered_map<std::string, unsigned int> uniformCache;
    std::vector<std::thread*> fileChangeListener;
public:
    const std::string fragmentPath;
    const std::string vertexPath;
    bool Changed = false;
    bool IsDefault = false;
    Shader(const std::string& vertex, const std::string& fragment);
    ~Shader();

    void Bind() const;
    void Unbind() const;
    void SetUniform1i(const std::string& name, int v0);
    void SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3);
    void SetUniform1f(const std::string& name, float v0);
    void SetUniform2f(const std::string& name, float v0, float v1);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniformVec3(const std::string& name, glm::vec3 vec3);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(const std::string& name, glm::mat4& matrix);
    unsigned int GetUniformLocation(const std::string& name);
    void CompileShader();
    void Reload();
    void ListenChanges();
};