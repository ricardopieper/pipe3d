#include "Shader.h"
#include <glad/glad.h>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>
#include <glm/glm.hpp>
#include <sys/types.h>

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <thread>

static std::string LoadFile(const std::string& path) {
    std::ifstream t(path);
    std::stringstream buffer;
    buffer << t.rdbuf();
    return buffer.str();
}

unsigned int RunCompileShader(unsigned int type, const std::string& path, const std::string& source) {
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
        std::cout << "Failed to compile "<<path<<": " << (type == GL_VERTEX_SHADER ? "vertex" : "fragment") << std::endl;
        std::cout << message << std::endl;
        glDeleteShader(id);
        delete[] message;
        return 0;
    }

    return id;
}

int CreateShader(unsigned int program, Shader& shader, const std::string& vertexShader, const std::string& fragmentShader) {
    unsigned int vs = RunCompileShader(GL_VERTEX_SHADER, shader.vertexPath, vertexShader);
    unsigned int fs = RunCompileShader(GL_FRAGMENT_SHADER, shader.fragmentPath, fragmentShader);
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

Shader::Shader(const std::string& vertex, const std::string& fragment): vertexPath(vertex), fragmentPath(fragment), IsDefault(false) {
    rendererId = glCreateProgram();
}

void Shader::Reload() {
    uniformCache.clear();
    Unbind();
    glDeleteProgram(rendererId);
    rendererId = glCreateProgram();
    CompileShader();
}

#ifdef _WIN32

#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <Shlwapi.h>

void Shader::ListenChanges() {


    fileChangeListener.push_back(new std::thread([this]() {
        
        char buffer[256];
        TCHAR** lppPart = { NULL };

        GetFullPathName(this->fragmentPath.c_str(),
            256,
            buffer,
            lppPart);

        std::string filename(buffer);
        std::string directory;
        const size_t last_slash_idx = filename.rfind('\\');
        if (std::string::npos != last_slash_idx)
        {
            directory = filename.substr(0, last_slash_idx);
        }
        while (true) {
            HANDLE hDir = CreateFile(
                directory.c_str(), FILE_LIST_DIRECTORY,
                FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, NULL,
                OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS | FILE_FLAG_OVERLAPPED, NULL);

            if (hDir == INVALID_HANDLE_VALUE) {
                std::cout << "DEU RUIM " << std::endl;
                break;
            }

            /*
            return ReadDirectoryChangesW(
			pWatch->mDirHandle, pWatch->mBuffer, sizeof(pWatch->mBuffer),
            pWatch->mIsRecursive,
			pWatch->mNotifyFilter, NULL, &pWatch->mOverlapped, _clear ? 0 : WatchCallback) != 0;
            */
            BYTE buffer[32 * 1024];
            DWORD BytesReturned;

            while (ReadDirectoryChangesW(
                hDir, /* handle to directory */
                &buffer, /* read results buffer */
                sizeof(buffer), /* length of buffer */
                TRUE, /* recursive */
                FILE_NOTIFY_CHANGE_LAST_WRITE, /* filter conditions */
                &BytesReturned, /* bytes returned */
                NULL, /* overlapped buffer */
                NULL)) {
               
                this->Changed = true;
            }
        }
    }));

}
#else
#include <sys/inotify.h>
#include <unistd.h>
void Shader::ListenChanges() {
    fileChangeListener.push_back(new std::thread([this]() {
        auto fd = inotify_init();
        const int buflen = 1024 * (sizeof(inotify_event) + 16);
        char buffer[buflen];
        auto wd = inotify_add_watch(fd, this->vertexPath.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
        while (true) {
            int length = read(fd, buffer, buflen);
            printf("Vertex shader program change detected\n");
            this->Changed = true;
        }
        (void)inotify_rm_watch(fd, wd);
        (void)close(fd);
        }));
    fileChangeListener.push_back(new std::thread([this]() {
        auto fd = inotify_init();
        const int buflen = 1024 * (sizeof(inotify_event) + 16);
        char buffer[buflen];
        auto wd = inotify_add_watch(fd, this->fragmentPath.c_str(), IN_MODIFY | IN_CREATE | IN_DELETE);
        while (true) {
            int length = read(fd, buffer, buflen);
            printf("Fragment shader program change detected\n");
            this->Changed = true;
        }
        (void)inotify_rm_watch(fd, wd);
        (void)close(fd);
        }));
}
#endif

void Shader::Bind() const {
    glUseProgram(rendererId);
}

void Shader::Unbind() const {
    glUseProgram(0);
}

void Shader::CompileShader() {
    auto vertex = LoadFile(vertexPath);
    auto fragment = LoadFile(fragmentPath);
    CreateShader(rendererId, *this, vertex, fragment);
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

void Shader::SetUniformVec3(const std::string& name, glm::vec3 vec3) {
    int location = GetUniformLocation(name);
    glUniform3f(location, vec3.x, vec3.y, vec3.z);
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
            //std::cout << "Warning: Did not find uniform with name "<<name<<std::endl;
            return -1;
        } else {
            std::cout << "caching uniform " << name << std::endl;
            uniformCache[name] = location;
        }
    } 
    return uniformCache[name];
}

