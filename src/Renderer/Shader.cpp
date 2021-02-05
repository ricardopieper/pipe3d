#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>

struct ShaderSource {
    std::string Vertex;
    std::string Fragment;
};

static ShaderSource ParseShader(const std::string& filepath) {
    std::ifstream stream(filepath);
    std::string line;
    std::stringstream ss[2];
    unsigned int type = -1;
    while (getline(stream, line)) {
        if(line.find("#shader") != std::string::npos) {
            if (line.find("vertex") != std::string::npos) {
                type = 0;
            } else if (line.find("fragment") != std::string::npos) {
                type = 1;
            }
        }
        else {
            ss[type] << line << '\n';
        }
    }

    ShaderSource sources;
    sources.Vertex = ss[0].str();
    sources.Fragment = ss[1].str();
    return sources;
}

unsigned int RunCompileShader(unsigned int type, const std::string& source) {
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);

    if (result == GL_FALSE) {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = new char[length];
        glGetShaderInfoLog(id, length, &length, message);
        std::cout << "Failed to compile: " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        delete message;
        return 0;
    }

    return id;
}

int CreateShader(unsigned int program, const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int vs = RunCompileShader(GL_VERTEX_SHADER, vertexShader);
    unsigned int fs = RunCompileShader(GL_FRAGMENT_SHADER, fragmentShader);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

Shader::Shader(const std::string& filepath) : filepath(filepath), IsDefault(false) {
    rendererId = glCreateProgram();
}

Shader::~Shader() {
}

void Shader::Bind() const {
    glUseProgram(rendererId);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::CompileShader() {
    auto shaders = ParseShader(filepath);
    CreateShader(rendererId, shaders.Vertex, shaders.Fragment);
}

void Shader::SetUniform1i(const std::string& name, int v0) {
    int location = GetUniformLocation(name);
    glUniform1i(location, v0);
}

void Shader::SetUniform1f(const std::string& name, float v0) {
    int location = GetUniformLocation(name);
    glUniform1f(location, v0);
}

void Shader::SetUniform4i(const std::string& name, int v0, int v1, int v2, int v3){
    int location = GetUniformLocation(name);
    glUniform4i(location, v0, v1, v2, v3);
}

void Shader::SetUniform2f(const std::string& name, float v0, float v1) {
    int location = GetUniformLocation(name);
    glUniform2f(location, v0, v1);
}

void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2) {
    int location = GetUniformLocation(name);
    glUniform3f(location, v0, v1, v2);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3) {
    int location = GetUniformLocation(name);
    glUniform4f(location, v0, v1, v2, v3);
}

void Shader::SetUniformMat4f(const std::string& name, glm::mat4& matrix) {
    int location = GetUniformLocation(name);
    glUniformMatrix4fv(location, 1, GL_FALSE, &matrix[0][0]);
}

unsigned int Shader::GetUniformLocation(const std::string& name) {
    if (uniformCache.find(name) == uniformCache.end()) {
        int location = glGetUniformLocation(rendererId, name.c_str());
        if (location == -1) {
            std::cout << "Warning: Did not find uniform with name "<<name<<std::endl;
            return -1;
        } else {
            uniformCache[name] = location;
        }
    } 
    return uniformCache[name];
}

