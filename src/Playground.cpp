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
#include "Renderer/TextureCache.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

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
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);  
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
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
    int glfwCursorState = GLFW_CURSOR_NORMAL;

    glfwSetInputMode(window, GLFW_CURSOR, glfwCursorState);
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

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO &io = ImGui::GetIO();
        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init();
        // Setup Dear ImGui style
        ImGui::StyleColorsDark();



        PerspectiveProjection perspectiveProjection;
        perspectiveProjection.SetCutoff(0.01, 10000);
        
        Camera3D camera;
        camera.Position = glm::vec3(0, 0, 5);

        Shader defaultShader("./assets/shaders/DefaultShader.vertex", 
            "./assets/shaders/DefaultShader.fragment");
        defaultShader.IsDefault = true;
        defaultShader.CompileShader();
        defaultShader.ListenChanges();

        Shader lightBlockShader("./assets/shaders/White.vertex", "./assets/shaders/White.fragment");
        lightBlockShader.CompileShader();

        Scene scene;

        auto oneMeterCubeModel = WavefrontMeshLoader::Load(
            "./assets/models/1mcube/1mcube.obj", 
            "./assets/models/1mcube/1mcube.mtl");
        auto oneMeterCube = scene.FromGeometry(
            oneMeterCubeModel[0].ConvertToGeometry(), 
            oneMeterCubeModel[0].GetMaterial(),
            defaultShader, 
            Texture(""),
            Texture(""), 
            Texture(""));
        

        auto wavefrontCube = WavefrontMeshLoader::Load(
            "./assets/models/lightcube/lightcube.obj", 
            "./assets/models/lightcube/lightcube.mtl");
       
        Geometry cubeGeometry = wavefrontCube[0].ConvertToGeometry();
        auto lightSource = scene.FromGeometry(cubeGeometry, Material::DefaultMaterial(), 
            lightBlockShader, Texture(""), Texture(""), Texture(""));
        lightSource->Translation = glm::vec3(0.0f);
        lightSource->Scale = glm::vec3(0.1);
        lightSource->Light.Enabled = true;
        lightSource->Light.Ambient = glm::vec3(0.05);
        lightSource->Light.Diffuse = glm::vec3(0.3);
        lightSource->Light.Specular = glm::vec3(1.0);

        auto sponzaWavefront = WavefrontMeshLoader::Load(
            "./assets/models/sponza/sponza.obj", 
            "./assets/models/sponza/sponza.mtl");

        TextureCache textureCache;
        std::vector<ModelToRender> sponzaModel;
        for (auto sponzaObj : sponzaWavefront) {
            if (sponzaObj.meshName == "Cube_Cube_Material") continue;
            std::cout << "Loading object "<<sponzaObj.meshName << std::endl;
            Geometry geom = sponzaObj.ConvertToGeometry();
            Material material = sponzaObj.GetMaterial();
            Texture tex = textureCache.GetTexture(sponzaObj.diffuseTexturePath);
            Texture specularHighlights = textureCache.GetTexture(sponzaObj.specularTexturePath);
            Texture normalMap = textureCache.GetTexture(sponzaObj.bumpTexturePath);
            sponzaModel.emplace_back(geom, material, defaultShader, tex, specularHighlights, normalMap);
        }

        auto sponza = scene.FromMeshes(sponzaModel);
        sponza->Translation = glm::vec3(0.0f);
        sponza->Scale = glm::vec3(2.72f);

        auto chloeWavefront = WavefrontMeshLoader::Load(
            "./assets/models/chloe-lis/0.obj", 
            "./assets/models/chloe-lis/0.mtl");

        std::vector<ModelToRender> chloeModel;
        for (auto chloeObj : chloeWavefront) {
            std::cout << "Loading object "<<chloeObj.meshName << std::endl;
            Geometry geom = chloeObj.ConvertToGeometry();
            Material material = chloeObj.GetMaterial();

            std::string base = "./assets/models/chloe-lis/";
            std::string texPath = base + chloeObj.diffuseTexturePath;
            texPath = ReplaceString(texPath, "\\", "/");

            Texture tex = textureCache.GetTexture(texPath);
            chloeModel.emplace_back(geom, material, defaultShader, tex, Texture(""), Texture(""));
        }
        
        auto chloe = scene.FromMeshes(chloeModel);
        chloe->Translation = glm::vec3(-1.0, 0.0, 0.0);
        chloe->Scale = glm::vec3(1.16);
        chloe->Rotation = glm::vec3(0, glm::radians(90.0f), 0);

        auto donut = WavefrontMeshLoader::Load(
            "./assets/models/donut/donut3.obj", 
            "./assets/models/donut/donut3.mtl");
        
        for (auto wavefrontObj : donut) {
            if (wavefrontObj.meshName == "Cube") continue;
            Geometry geom = wavefrontObj.ConvertToGeometry();
            auto obj = scene.FromGeometry(geom, wavefrontObj.GetMaterial(), defaultShader, 
                Texture(""), Texture(""), Texture(""));
            obj->Translation = glm::vec3(0.3, 0.0, 0.3);
        }

       

        //glFrontFace(GL_CW);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        
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
        bool freeCameraMovementMouse = false;
        float cameraFlyMovement = 4;
        float pi = 3.1415;
        float alphaDiscard = 0.01;
        while (!glfwWindowShouldClose(window)) {
            double currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            io.DeltaTime = deltaTime;

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            
            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int window_width, window_height;
            glfwGetWindowSize(window, &window_width, &window_height);

            if (freeCameraMovementMouse || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS) {
                float moveSpeed = deltaTime;
                if (!freeCameraMovementMouse) {
                    moveSpeed *= 5; 
                }
                camera.ChangeHorizontalAngle(mouseSpeed * moveSpeed * float(lastXpos - xpos));
                camera.ChangeVerticalAngle( mouseSpeed * moveSpeed * float(lastYpos - ypos));
            }

            lastXpos = xpos;
            lastYpos = ypos;

            float movement = deltaTime * speed;
            movement = movement * cameraFlyMovement;
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
             if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS){
                defaultShader.Reload();
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
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS) {
                freeCameraMovementMouse = !freeCameraMovementMouse;
            }

            if (defaultShader.Changed) {
                printf("Reloading shader\n");
                defaultShader.Reload();
                defaultShader.Changed = false;
            }

            glm::mat4 projection = perspectiveProjection.GetProjectionMatrix(
                window_width, window_height);   

            renderer.Clear();

            renderer.Render(scene, camera, projection);
            defaultShader.SetUniform1f("ambientLight", 0.1);
            defaultShader.SetUniform1f("alphaDiscard", alphaDiscard);
            defaultShader.SetUniform3f("lampPosition", lightSource->Translation.x, lightSource->Translation.y, lightSource->Translation.z);
            defaultShader.SetUniform3f("cameraPosition", camera.Position.x, camera.Position.y, camera.Position.z);

        // feed inputs to dear imgui, start new frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Chloe model");
            ImGui::DragFloat3("Translation", (float*)&chloe->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float*)&chloe->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float*)&chloe->Scale, 0.01, -20, 20);
            ImGui::End();

            ImGui::Begin("Sponza model");
            ImGui::DragFloat3("Translation", (float*)&sponza->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float*)&sponza->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float*)&sponza->Scale, 0.01, -20, 20);
            ImGui::End();

            ImGui::Begin("One meter cube model");
            ImGui::DragFloat3("Translation", (float*)&oneMeterCube->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float*)&oneMeterCube->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float*)&oneMeterCube->Scale, 0.01, -20, 20);
            ImGui::End();

            ImGui::Begin("Light cube");
            ImGui::DragFloat3("Translation", (float*)&lightSource->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float*)&lightSource->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float*)&lightSource->Scale, 0.01, -20, 20);
            ImGui::DragFloat3("Ambient Light", (float*)&lightSource->Light.Ambient, 0.01, 0, 1);
            ImGui::DragFloat3("Diffuse Light", (float*)&lightSource->Light.Diffuse, 0.01, 0, 1);
            ImGui::DragFloat3("Specular Light", (float*)&lightSource->Light.Specular, 0.01, 0, 1);
            ImGui::Checkbox("Specular Light", (bool*)&lightSource->Light.Enabled);
            ImGui::End();

            ImGui::Begin("Camera and Editor");
            ImGui::DragFloat("Alpha Discard", (float*)&alphaDiscard, 0.0001, 0.0f, 0.5f);
            ImGui::Checkbox("FPS mouse movement", &freeCameraMovementMouse);
            ImGui::SliderFloat("Fly speed", &cameraFlyMovement, 0, 100);
            ImGui::RadioButton("Show cursor", &glfwCursorState, GLFW_CURSOR_NORMAL);
            ImGui::RadioButton("Hidden cursor", &glfwCursorState, GLFW_CURSOR_HIDDEN);
            ImGui::RadioButton("Disable cursor", &glfwCursorState, GLFW_CURSOR_DISABLED);
            ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSetInputMode(window, GLFW_CURSOR, glfwCursorState);
            
            lastTime = glfwGetTime();    
            
            glfwSwapBuffers(window);
            glfwPollEvents();

           //// printf("Camera position: %f %f %f\n",
           //     camera.Position.x,camera.Position.y, camera.Position.z );
           // printf("Camera angle: h %f v %f\n",
           //     camera.horizontalAngle, camera.verticalAngle);
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}