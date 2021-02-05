#pragma once
#include <string>
#include <unordered_map>
#include <glm/glm.hpp>

class Shader {
private:
    unsigned int rendererId;
    const std::string filepath;
    std::unordered_map<std::string, unsigned int> uniformCache;
public:
    bool IsDefault = false;
    Shader(const std::string& filepath);
    ~Shader();

    void Bind() const;
    void Unbind() const;
    void SetUniform1i(const std::string& name, int v0);
    void SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3);
    void SetUniform1f(const std::string& name, float v0);
    void SetUniform2f(const std::string& name, float v0, float v1);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniformMat4f(const std::string& name, glm::mat4& matrix);
    unsigned int GetUniformLocation(const std::string& name);
    void CompileShader();
    
};