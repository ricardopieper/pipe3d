#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h> 
#include <cassert>
#include <map>
#include "Renderer/VertexBuffer.h"
#include "Renderer/IndexBuffer.h"
#include "Renderer/BufferLayout.h"
#include "Renderer/VertexArray.h"
#include "Renderer/OrthoProjection.h"
#include "Renderer/PerspectiveProjection.h"
#include "Renderer/Camera3D.h"
#include "Renderer/Shader.h"
#include "Renderer/Renderer.h"
#include "Renderer/Geometry.h"
#include "Renderer/Scene.h"
#include "Renderer/WavefrontMeshLoader.h"

static void GLFWErrorCallback(int error, const char* description)
{
    std::cout << "GLFW Error " << error << " " << description << std::endl;
}

void APIENTRY glDebugOutput(GLenum source, 
                            GLenum type, 
                            unsigned int id, 
                            GLenum severity, 
                            GLsizei length, 
                            const char *message, 
                            const void *userParam)
{
    // ignore non-significant error/warning codes
    if(id == 131169 || id == 131185 || id == 131218 || id == 131204) return; 

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " <<  message << std::endl;

    switch (source)
    {
        case GL_DEBUG_SOURCE_API:             std::cout << "Source: API"; break;
        case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   std::cout << "Source: Window System"; break;
        case GL_DEBUG_SOURCE_SHADER_COMPILER: std::cout << "Source: Shader Compiler"; break;
        case GL_DEBUG_SOURCE_THIRD_PARTY:     std::cout << "Source: Third Party"; break;
        case GL_DEBUG_SOURCE_APPLICATION:     std::cout << "Source: Application"; break;
        case GL_DEBUG_SOURCE_OTHER:           std::cout << "Source: Other"; break;
    } std::cout << std::endl;

    switch (type)
    {
        case GL_DEBUG_TYPE_ERROR:               std::cout << "Type: Error"; break;
        case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: std::cout << "Type: Deprecated Behaviour"; break;
        case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  std::cout << "Type: Undefined Behaviour"; break; 
        case GL_DEBUG_TYPE_PORTABILITY:         std::cout << "Type: Portability"; break;
        case GL_DEBUG_TYPE_PERFORMANCE:         std::cout << "Type: Performance"; break;
        case GL_DEBUG_TYPE_MARKER:              std::cout << "Type: Marker"; break;
        case GL_DEBUG_TYPE_PUSH_GROUP:          std::cout << "Type: Push Group"; break;
        case GL_DEBUG_TYPE_POP_GROUP:           std::cout << "Type: Pop Group"; break;
        case GL_DEBUG_TYPE_OTHER:               std::cout << "Type: Other"; break;
    } std::cout << std::endl;
    
    switch (severity)
    {
        case GL_DEBUG_SEVERITY_HIGH:         std::cout << "Severity: high"; break;
        case GL_DEBUG_SEVERITY_MEDIUM:       std::cout << "Severity: medium"; break;
        case GL_DEBUG_SEVERITY_LOW:          std::cout << "Severity: low"; break;
        case GL_DEBUG_SEVERITY_NOTIFICATION: std::cout << "Severity: notification"; break;
    } std::cout << std::endl;
    std::cout << std::endl;
}
glm::vec3 position = glm::vec3( 0 , 0, 5 );

static void quit(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

std::string ReplaceString(std::string subject, const std::string& search,
                          const std::string& replace) {
    size_t pos = 0;
    while((pos = subject.find(search, pos)) != std::string::npos) {
         subject.replace(pos, search.length(), replace);
         pos += replace.length();
    }
    return subject;
}

float mouseWheel = 0;
int main()
{
    glfwInit();

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);  
    glfwWindowHint(GLFW_SAMPLES, 4);

    //glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    float width = 1024;
    float height = 768;

    GLFWwindow* window = glfwCreateWindow(width, height, "Pipe3D Playground", NULL, NULL);
    glfwMakeContextCurrent(window);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetKeyCallback(window, quit);
    glfwSetErrorCallback(GLFWErrorCallback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
  // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    glfwSwapInterval(1);
    {
        int flags; glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS); 

            glDebugMessageCallback(glDebugOutput, (void*)new char[2]);

            glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, nullptr, GL_TRUE);
        } 
        PerspectiveProjection perspectiveProjection;
        perspectiveProjection.SetCutoff(0.01, 10000);
        
        Camera3D camera;
        camera.Position = glm::vec3(0, 0, 5);

        Shader defaultShader("./assets/shaders/DefaultShader.shader");
        defaultShader.IsDefault = true;
        defaultShader.CompileShader();
    
        Shader emissiveShader("./assets/shaders/EmissiveSurface.shader");
        emissiveShader.CompileShader();

        Scene scene;

        auto wavefrontCube = WavefrontMeshLoader::Load(
            "./assets/models/lightcube/lightcube.obj", 
            "./assets/models/lightcube/lightcube.mtl");
       
        Geometry cubeGeometry = wavefrontCube[0].ConvertToGeometry();
        auto lightSource = scene.FromGeometry(&cubeGeometry, emissiveShader, Texture(""));
        lightSource->Translation = glm::vec3(0.0f);

        auto sponza = WavefrontMeshLoader::Load(
            "./assets/models/sponza/sponza.obj", 
            "./assets/models/sponza/sponza.mtl");

        std::map<std::string, Texture> texture_map;

        for (auto sponzaObj : sponza) {
            if (sponzaObj.meshName == "Cube_Cube_Material") continue;
            std::cout << "Loading object "<<sponzaObj.meshName << std::endl;
            Geometry geom = sponzaObj.ConvertToGeometry();
            
            std::string base = "";
            std::string texPath = base + sponzaObj.diffuseTexturePath;
            texPath = ReplaceString(texPath, "\\", "/");

            if (texture_map.find(texPath) == texture_map.end()) {
                if (sponzaObj.diffuseTexturePath == "") {
                    texture_map.insert(std::pair<std::string, Texture>(texPath, Texture("")));
                } else {
                    Texture tex(texPath);
                    if (tex.width == 0) {
                        std::cout << "failed to load "<< texPath << std::endl;
                    }
                    texture_map.insert(std::pair<std::string, Texture>(texPath, tex));
                }
            }

            Texture tex = texture_map[texPath];
           
            auto obj = scene.FromGeometry(&geom, defaultShader, tex);
            obj->Translation = glm::vec3(0.0f);
            obj->Scale = glm::vec3(3,3,3);
        }
        
        auto donut = WavefrontMeshLoader::Load(
            "./assets/models/donut/donut3.obj", 
            "./assets/models/donut/donut3.mtl");
        
        for (auto wavefrontObj : donut) {
            if (wavefrontObj.meshName == "Cube") continue;
            Geometry geom = wavefrontObj.ConvertToGeometry();
            auto obj = scene.FromGeometry(&geom, defaultShader, Texture(""));
            obj->Translation = glm::vec3(1.0f);
            obj->Translation = glm::vec3(0.3, 0.0, 0.3);
        }

        //glFrontFace(GL_CW);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        glEnable(GL_CULL_FACE);  
        glCullFace(GL_BACK);

        glEnable(GL_MULTISAMPLE);  

        float speed = 3.0f; // 3 units / second
        float mouseSpeed = 0.045f;
        double lastTime = glfwGetTime();
  

        glfwSetWindowUserPointer(window, &perspectiveProjection);

        glfwSetScrollCallback(window, [](GLFWwindow* window, double offsetx, double offsety) mutable {
            PerspectiveProjection* perspectiveProjection = (PerspectiveProjection*)glfwGetWindowUserPointer(window);
            perspectiveProjection->ChangeZoom(offsety);
        });

        double lastXpos, lastYpos;
        
        Renderer renderer;
        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int window_width, window_height;
            glfwGetWindowSize(window, &window_width, &window_height);

            camera.ChangeHorizontalAngle(mouseSpeed * deltaTime * float(lastXpos - xpos));
            camera.ChangeVerticalAngle( mouseSpeed * deltaTime * float(lastYpos - ypos));

            lastXpos = xpos;
            lastYpos = ypos;

            float movement = deltaTime * speed;
            movement = movement * 4;
            if (glfwGetKey(window, GLFW_KEY_W ) == GLFW_PRESS) {
                camera.MoveForward(movement);
            }
            // Move backward
            if (glfwGetKey(window, GLFW_KEY_S ) == GLFW_PRESS){
                camera.MoveBackward(movement);
            }
            // Strafe right
            if (glfwGetKey(window, GLFW_KEY_D ) == GLFW_PRESS){
                camera.MoveRight(movement);
            }
            // Strafe left
            if (glfwGetKey(window, GLFW_KEY_A ) == GLFW_PRESS){
                 camera.MoveLeft(movement);
            }
            if (glfwGetKey(window, GLFW_KEY_C ) == GLFW_PRESS){
                camera.ResetAngle();
                camera.Position = glm::vec3(0, 0, 5);
            }
            if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS) {
                lastXpos = window_width / 2;
                lastYpos = window_height / 2;
                glfwSetCursorPos(window, lastXpos, lastYpos);
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);
            }
            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS){
                glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
            }
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS){
                 lightSource->Translation.y += 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS){
                 lightSource->Translation.y -= 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS){
                 lightSource->Translation.x -= 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS){
                 lightSource->Translation.x += 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS){
                 lightSource->Translation.z += 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS){
                 lightSource->Translation.z -= 0.1;
            }


            glm::mat4 projection = perspectiveProjection.GetProjectionMatrix(
                window_width, window_height);   

            renderer.Clear();
            renderer.Render(scene, camera, projection);
            defaultShader.SetUniform1f("ambientLight", 0.1);
            defaultShader.SetUniform3f("lampPosition", lightSource->Translation.x,  lightSource->Translation.y,  lightSource->Translation.z);
            defaultShader.SetUniform3f("cameraPosition", camera.Position.x, camera.Position.y, camera.Position.z);

            lastTime = glfwGetTime();    
            
            glfwSwapBuffers(window);
            glfwPollEvents();

           //// printf("Camera position: %f %f %f\n",
           //     camera.Position.x,camera.Position.y, camera.Position.z );
           // printf("Camera angle: h %f v %f\n",
           //     camera.horizontalAngle, camera.verticalAngle);
        }
    }

    glfwDestroyWindow(window);
    glfwTerminate();
}