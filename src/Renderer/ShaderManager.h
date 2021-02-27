#include <vector>
#include <memory>
#include "Shader.h"

//Monitors and stores shaders, updates them as well. Must be called on the opengl thread
class ShaderManager {
private:
    std::vector<std::shared_ptr<Shader>> Shaders;

public:
    std::shared_ptr<Shader> MakeNew(std::string vertexShaderPath, std::string fragmentShaderPath) {
        std::shared_ptr<Shader> shader = std::make_shared<Shader>(vertexShaderPath, fragmentShaderPath);
        shader->CompileShader();
        shader->ListenChanges();        
        Shaders.push_back(shader);
        return shader;
    }

    void CheckForChangesAndRecompile() {
        for (auto shaderPtr: Shaders) {
            Shader& shader = *shaderPtr;
            if (shader.Changed) {
                shader.Unbind();
                shader.Reload();
                shader.Changed = false;
            }
        }   
    }
};