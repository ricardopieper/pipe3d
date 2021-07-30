#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Tracy.hpp>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <stdio.h>
#include <cassert>
#include <map>
#include <memory>
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
#include "Renderer/Framebuffer.h"
#include "Renderer/WindowData.h"
#include "Renderer/RenderingPipeline.h"
#include "Renderer/RenderingStages/MainStage.h"
#include "Renderer/RenderingStages/MultisampledMainStage.h"
#include "Renderer/RenderingStages/PostProcessingEffect.h"
#include "Renderer/RenderingStages/FramebufferToScreen.h"
#include "Renderer/RenderingStages/DirectionalShadowMapGeneration.h"
#include "Renderer/RenderingStages/ShadowMapDebug.h"
#include "Renderer/ShaderManager.h"
#include "Renderer/CubemapTexture.h"
#include "Renderer/MultisampledFramebuffer.h"
#include "Renderer/Skybox.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"

#include <string>
#include <iostream>

static void GLFWErrorCallback(int error, const char *description)
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
    if (id == 131169 || id == 131185 || id == 131218 || id == 131204)
        return;

    std::cout << "---------------" << std::endl;
    std::cout << "Debug message (" << id << "): " << message << std::endl;

    switch (source)
    {
    case GL_DEBUG_SOURCE_API:
        std::cout << "Source: API";
        break;
    case GL_DEBUG_SOURCE_WINDOW_SYSTEM:
        std::cout << "Source: Window System";
        break;
    case GL_DEBUG_SOURCE_SHADER_COMPILER:
        std::cout << "Source: Shader Compiler";
        break;
    case GL_DEBUG_SOURCE_THIRD_PARTY:
        std::cout << "Source: Third Party";
        break;
    case GL_DEBUG_SOURCE_APPLICATION:
        std::cout << "Source: Application";
        break;
    case GL_DEBUG_SOURCE_OTHER:
        std::cout << "Source: Other";
        break;
    }
    std::cout << std::endl;

    switch (type)
    {
    case GL_DEBUG_TYPE_ERROR:
        std::cout << "Type: Error";
        break;
    case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR:
        std::cout << "Type: Deprecated Behaviour";
        break;
    case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:
        std::cout << "Type: Undefined Behaviour";
        break;
    case GL_DEBUG_TYPE_PORTABILITY:
        std::cout << "Type: Portability";
        break;
    case GL_DEBUG_TYPE_PERFORMANCE:
        std::cout << "Type: Performance";
        break;
    case GL_DEBUG_TYPE_MARKER:
        std::cout << "Type: Marker";
        break;
    case GL_DEBUG_TYPE_PUSH_GROUP:
        std::cout << "Type: Push Group";
        break;
    case GL_DEBUG_TYPE_POP_GROUP:
        std::cout << "Type: Pop Group";
        break;
    case GL_DEBUG_TYPE_OTHER:
        std::cout << "Type: Other";
        break;
    }
    std::cout << std::endl;

    switch (severity)
    {
    case GL_DEBUG_SEVERITY_HIGH:
        std::cout << "Severity: high";
        break;
    case GL_DEBUG_SEVERITY_MEDIUM:
        std::cout << "Severity: medium";
        break;
    case GL_DEBUG_SEVERITY_LOW:
        std::cout << "Severity: low";
        break;
    case GL_DEBUG_SEVERITY_NOTIFICATION:
        std::cout << "Severity: notification";
        break;
    }
    std::cout << std::endl;
    std::cout << std::endl;
}
glm::vec3 position = glm::vec3(0, 0, 5);

static void quit(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
}

std::string ReplaceString(std::string subject, const std::string &search,
                          const std::string &replace)
{
    size_t pos = 0;
    while ((pos = subject.find(search, pos)) != std::string::npos)
    {
        subject.replace(pos, search.length(), replace);
        pos += replace.length();
    }
    return subject;
}


float mouseWheel = 0;
int main() {

    #ifdef TRACY_ENABLE
    std::cout<<"Tracy enabled!" << std::endl;
    #endif

    glfwInit();
    //glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 5);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, true);
    glfwWindowHint(GLFW_SAMPLES, 4);

    glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE);

    float width = 1024;
    float height = 768;

    GLFWwindow *window = glfwCreateWindow(width, height, "Pipe3D Playground", NULL, NULL);
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

    glfwSetKeyCallback(window, quit);

    glfwSetErrorCallback(GLFWErrorCallback);
    glfwSetInputMode(window, GLFW_STICKY_KEYS, GLFW_TRUE);
    int glfwCursorState = GLFW_CURSOR_NORMAL;

    glfwSetInputMode(window, GLFW_CURSOR, glfwCursorState);
    // glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
    glViewport(0, 0, width, height);

    glfwSetFramebufferSizeCallback(window, [](GLFWwindow *window, int resizeWidth, int resizeHeight) mutable {
        glViewport(0, 0, resizeWidth, resizeHeight);
        WindowData *windowData = (WindowData *)glfwGetWindowUserPointer(window);
        windowData->renderingPipeline->OnResizeWindow(resizeWidth, resizeHeight);
    });

    ShaderManager shaderManager;

    {
        int flags;
        glGetIntegerv(GL_CONTEXT_FLAGS, &flags);
        if (flags & GL_CONTEXT_FLAG_DEBUG_BIT)
        {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

            glDebugMessageCallback(glDebugOutput, (void *)new char[2]);

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
        camera.Position = glm::vec3(1.9613384, 1.83839011, 0.118167907);
        camera.horizontalAngle = 4.7262249;
        camera.verticalAngle = -0.542656064;

        auto framebufferShader = shaderManager.MakeNew("./assets/shaders/Framebuffer.vertex", "./assets/shaders/Framebuffer.fragment");
        auto singleColorShader = shaderManager.MakeNew("./assets/shaders/SingleColor.vertex", "./assets/shaders/SingleColor.fragment");
        auto defaultShader = shaderManager.MakeNew("./assets/shaders/DefaultShader.vertex", "./assets/shaders/DefaultShader.fragment");
        auto skyboxShader = shaderManager.MakeNew("./assets/shaders/Skybox.vertex", "./assets/shaders/Skybox.fragment");
        auto shadowMap = shaderManager.MakeNew("./assets/shaders/ShadowMap.vertex", "./assets/shaders/ShadowMap.fragment");
        auto shadowMapDebug = shaderManager.MakeNew("./assets/shaders/Framebuffer.vertex", "./assets/shaders/FramebufferShadowMapDebug.fragment");
        //auto envMappingTest = shaderManager.MakeNew("./assets/shaders/DefaultShader.vertex", "./assets/shaders/TestEnvMapping.fragment");
        defaultShader->IsDefault = true;


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
            Texture(""),
            Texture(""));
        oneMeterCube->Translation.x = 2.2;
        oneMeterCube->Translation.y = -0.07;
        oneMeterCube->Translation.z = 0;
        oneMeterCube->Enabled = false;
        for (auto& it: oneMeterCube->SceneObjectElements) { 
            it.reflectivity = 0.7;
        };

        /*CubemapTexture cubemapTex(std::vector<std::string> {
            "./assets/skyboxes/oceansky/right.jpg",
            "./assets/skyboxes/oceansky/left.jpg",
            "./assets/skyboxes/oceansky/top.jpg",
            "./assets/skyboxes/oceansky/bottom.jpg",
            "./assets/skyboxes/oceansky/front.jpg",
            "./assets/skyboxes/oceansky/back.jpg"
        });*/
    
        CubemapTexture cubemapTex(std::vector<std::string> {
            "./assets/skyboxes/oceansky/right.jpg",
            "./assets/skyboxes/oceansky/left.jpg",
            "./assets/skyboxes/oceansky/top.jpg",
            "./assets/skyboxes/oceansky/bottom.jpg",
            "./assets/skyboxes/oceansky/front.jpg",
            "./assets/skyboxes/oceansky/back.jpg"
            /*
            "./assets/skyboxes/Yokohama3/posx.jpg",
            "./assets/skyboxes/Yokohama3/negx.jpg",
            "./assets/skyboxes/Yokohama3/posy.jpg",
            "./assets/skyboxes/Yokohama3/negy.jpg",
            "./assets/skyboxes/Yokohama3/posz.jpg",
            "./assets/skyboxes/Yokohama3/negz.jpg"*/
        });

        Skybox skybox(cubemapTex, skyboxShader);

        auto wavefrontCube = WavefrontMeshLoader::Load(
            "./assets/models/lightcube/lightcube.obj",
            "./assets/models/lightcube/lightcube.mtl");

        Geometry cubeGeometry = wavefrontCube[0].ConvertToGeometry();
        auto sun = scene.FromGeometry(
            cubeGeometry, Material::DefaultMaterial(),
            singleColorShader, Texture(""), Texture(""), Texture(""),Texture(""));
        //sun->Translation = glm::vec3(4.8f, 6.3, 1.3);
        sun->Translation = glm::vec3(0.0f, 20.0f, 0.0);
        sun->Scale = glm::vec3(0.1);
        sun->Light.IsDirectional = true;
        sun->Light.Ambient = glm::vec3(0.085);
        sun->Light.Diffuse = glm::vec3(0.3);
        sun->Light.Specular = glm::vec3(0.1);
        sun->Name = "Sun";
       
       // pointLight->Outlined = true;

        auto pointLight = scene.FromGeometry(
            cubeGeometry, Material::DefaultMaterial(),
            singleColorShader, Texture(""), Texture(""), Texture(""), Texture(""));
        pointLight->Translation = glm::vec3(0.0f);
        pointLight->Scale = glm::vec3(0.1);
        pointLight->Light.IsPoint = true;
        pointLight->Light.Ambient = glm::vec3(0.3);
        pointLight->Light.Diffuse = glm::vec3(0.3);
        pointLight->Light.Specular = glm::vec3(0.3);
        pointLight->Light.Constant = 10;
        pointLight->Light.Linear = 0.18;
        pointLight->Light.Quadratic = 0.01;
        pointLight->Light.IsPoint = false;
        pointLight->Enabled = false;

        auto pointLight2 = scene.FromGeometry(
            cubeGeometry, Material::DefaultMaterial(),
            singleColorShader, Texture(""), Texture(""), Texture(""),Texture(""));
        pointLight2->Translation = glm::vec3(1.0f, 0.0f, 1.0f);
        pointLight2->Scale = glm::vec3(0.1);
        pointLight2->Light.IsPoint = true;
        pointLight2->Light.Ambient = glm::vec3(0.3);
        pointLight2->Light.Diffuse = glm::vec3(0.3);
        pointLight2->Light.Specular = glm::vec3(0.3);
        pointLight2->Light.Constant = 10;
        pointLight2->Light.Linear = 0.7;
        pointLight2->Light.Quadratic = 1.8;
        pointLight2->Light.IsPoint = false;
        pointLight2->Enabled = false;

        auto sponzaWavefront = WavefrontMeshLoader::Load(
            "./assets/models/sponza/sponza.obj",
            "./assets/models/sponza/sponza.mtl");

        TextureCache textureCache;
        std::vector<ModelToRender> sponzaModel;
        for (auto sponzaObj : sponzaWavefront)
        {
            if (sponzaObj.meshName == "Cube_Cube_Material")
                continue;
            std::cout << "Loading sponza object " << sponzaObj.meshName << std::endl;
            Geometry geom = sponzaObj.ConvertToGeometry();
            Material material = sponzaObj.GetMaterial();
            Texture tex = textureCache.GetTexture(sponzaObj.diffuseTexturePath);
            Texture specularHighlights = textureCache.GetTexture(sponzaObj.specularTexturePath, false);
            Texture normalMap = textureCache.GetTexture(sponzaObj.bumpTexturePath, false);
            sponzaModel.emplace_back(geom, material, defaultShader, tex, specularHighlights, normalMap, Texture(""));
        }

        auto sponza = scene.FromMeshes(sponzaModel);
        sponza->Translation = glm::vec3(0.0f, 0.0f, 0.0);
        sponza->Scale = glm::vec3(2.72f);
        // sponza->Enabled = false;

        auto crysisGuyWavefront = WavefrontMeshLoader::Load(
            "./assets/models/nanosuit_reflection/nanosuit.obj",
            "./assets/models/nanosuit_reflection/nanosuit.mtl");

        std::vector<ModelToRender> crysisGuyModel;
        for (auto cryGuyPart : crysisGuyWavefront)
        {
            std::cout << "Loading crysis guy object " << cryGuyPart.meshName << std::endl;
            Geometry geom = cryGuyPart.ConvertToGeometry();
            Material material = cryGuyPart.GetMaterial();
            Texture tex = textureCache.GetTexture("./assets/models/nanosuit_reflection/" + cryGuyPart.diffuseTexturePath);
            Texture refl = textureCache.GetTexture("./assets/models/nanosuit_reflection/" + cryGuyPart.reflectivityTexturePath, false);
            Texture specularHighlights = textureCache.GetTexture("./assets/models/nanosuit_reflection/" + cryGuyPart.specularTexturePath, false);
            Texture normalMap = textureCache.GetTexture("./assets/models/nanosuit_reflection/" + cryGuyPart.bumpTexturePath, false);
            crysisGuyModel.emplace_back(geom, material, defaultShader, tex, specularHighlights, normalMap, refl);
        }

        auto cryGuy = scene.FromMeshes(crysisGuyModel);
        cryGuy->Translation = glm::vec3(0.0f, 0.0f, -1.3f);
        cryGuy->Scale = glm::vec3(0.14);
        cryGuy->Enabled = false;

        auto chloeWavefront = WavefrontMeshLoader::Load(
            "./assets/models/chloe-lis/0.obj",
            "./assets/models/chloe-lis/0.mtl");

        std::vector<ModelToRender> chloeModel;
        for (auto chloeObj : chloeWavefront)
        {
            std::cout << "Loading chloe object " << chloeObj.meshName << std::endl;
            Geometry geom = chloeObj.ConvertToGeometry();
            Material material = chloeObj.GetMaterial();

            std::string base = "./assets/models/chloe-lis/";
            std::string texPath = base + chloeObj.diffuseTexturePath;
            texPath = ReplaceString(texPath, "\\", "/");

            Texture tex = textureCache.GetTexture(texPath);
            chloeModel.emplace_back(geom, material, defaultShader, tex, Texture(""), Texture(""),  Texture(""));
            //emplaced.reflectivity = 1.0;
        }

        auto chloe = scene.FromMeshes(chloeModel);
        chloe->Translation = glm::vec3(-1.0, 0.0, 0.0);
        chloe->Scale = glm::vec3(1.16);
        chloe->Rotation = glm::vec3(0, glm::radians(90.0f), 0);
        chloe->Enabled = true;
        chloe->Name = "Chloe";

        std::shared_ptr<SceneObject> sceneBoundingBoxRenderer;
        
       /* auto donut = WavefrontMeshLoader::Load(
            "./assets/models/donut/donut3.obj",
            "./assets/models/donut/donut3.mtl");

        for (auto wavefrontObj : donut)
        {
            if (wavefrontObj.meshName == "Cube")
                continue;
            Geometry geom = wavefrontObj.ConvertToGeometry();
            auto obj = scene.FromGeometry(geom, wavefrontObj.GetMaterial(), defaultShader,
                                          Texture(""), Texture(""), Texture(""));
            obj->Translation = glm::vec3(0.3, 0.0, 0.3);
        }*/

        //glFrontFace(GL_CW);
        glEnable(GL_FRAMEBUFFER_SRGB);
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); //wireframe or normal rendering

        //Blending
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        //Stencil test
        glEnable(GL_STENCIL_TEST);

        //Face culling
        glEnable(GL_CULL_FACE);
        glCullFace(GL_BACK);

        //MSAA
        glEnable(GL_MULTISAMPLE);


        float speed = 3.0f; // 3 units / second
        float mouseSpeed = 0.045f;
        double lastTime = glfwGetTime();

        auto framebuffer = std::make_shared<MultisampledFramebuffer>(framebufferShader);
        framebuffer->MakeFramebuffer(width, height);
        framebuffer->Unbind();

        auto renderer = std::make_shared<Renderer>(window, singleColorShader, skybox);

        RenderingPipeline renderingPipeline(width, height);
        auto invertColorEffect = shaderManager.MakeNew(
           "./assets/shaders/Framebuffer.vertex",
           "./assets/shaders/PostProcessingEffects/Inverter.fragment"
        );
        auto kernelEffect = shaderManager.MakeNew(
           "./assets/shaders/Framebuffer.vertex",
           "./assets/shaders/PostProcessingEffects/Kernel.fragment"
        );
        
        //renderingPipeline.AddStage(std::make_shared<MainRenderingStage>(framebuffer, renderer));
        renderingPipeline.AddStage(std::make_shared<DirectionalShadowMapGeneration>(shadowMap));
       // renderingPipeline.AddStage(std::make_shared<ShadowMapDebug>(shadowMapDebug));
        
        renderingPipeline.AddStage(std::make_shared<MultisampledMainStage>(framebuffer, renderer));
        renderingPipeline.AddStage(std::make_shared<PostProcessingEffect>(invertColorEffect));
        renderingPipeline.AddStage(std::make_shared<PostProcessingEffect>(kernelEffect));
        renderingPipeline.AddStage(std::make_shared<FramebufferToScreen>());
        

        WindowData windowData;
        windowData.perspectiveProjection = &perspectiveProjection;
        windowData.renderingPipeline = &renderingPipeline;

        glfwSetWindowUserPointer(window, &windowData);

        glfwSetScrollCallback(window, [](GLFWwindow *window, double offsetx, double offsety) mutable {
            WindowData *windowData = (WindowData *)glfwGetWindowUserPointer(window);
            PerspectiveProjection *perspectiveProjection = windowData->perspectiveProjection;
            perspectiveProjection->ChangeZoom(offsety);
        });

        double lastXpos, lastYpos;
        bool freeCameraMovementMouse = false;
        float cameraFlyMovement = 4;
        float pi = 3.1415;
        float alphaDiscard = 0.01;

        float targetFps = 60;
        bool prevCloack = false;
        bool cloackEngaged = false;

        float cloackRefractTarget = 1.33;
        float initialCloack = 0;
        
        int animationTime = 0.5 * targetFps;
        int currentAnimationTime = -1;

        while (!glfwWindowShouldClose(window))
        {

            FrameMarkStart("Frame");

            double currentTime = glfwGetTime();
            float deltaTime = currentTime - lastTime;
            io.DeltaTime = deltaTime;

            double xpos, ypos;
            glfwGetCursorPos(window, &xpos, &ypos);

            int window_width, window_height;
            glfwGetWindowSize(window, &window_width, &window_height);

            if (freeCameraMovementMouse || glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS)
            {
                float moveSpeed = deltaTime;
                if (!freeCameraMovementMouse)
                {
                    moveSpeed *= 5;
                }
                camera.ChangeHorizontalAngle(mouseSpeed * moveSpeed * float(lastXpos - xpos));
                camera.ChangeVerticalAngle(mouseSpeed * moveSpeed * float(lastYpos - ypos));
            }

            lastXpos = xpos;
            lastYpos = ypos;

            float movement = deltaTime * speed;
            movement = movement * cameraFlyMovement;
            if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
            {
                camera.MoveForward(movement);
            }
            // Move backward
            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            {
                camera.MoveBackward(movement);
            }
            // Strafe right
            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            {
                camera.MoveRight(movement);
            }
            // Strafe left
            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            {
                camera.MoveLeft(movement);
            }
            if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS)
            {
                camera.ResetAngle();
                camera.Position = glm::vec3(0, 0, 5);
            }
          
            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
            {
                pointLight->Translation.y += 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
            {
                pointLight->Translation.y -= 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
            {
                pointLight->Translation.x -= 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
            {
                pointLight->Translation.x += 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_KP_0) == GLFW_PRESS)
            {
                pointLight->Translation.z += 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_KP_1) == GLFW_PRESS)
            {
                pointLight->Translation.z -= 0.1;
            }
            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
            {
                freeCameraMovementMouse = !freeCameraMovementMouse;
            }

            shaderManager.CheckForChangesAndRecompile();

            glm::mat4 projection = perspectiveProjection.GetProjectionMatrix(
                window_width, window_height);

            RenderingContext renderingContext{camera, scene, projection, nullptr, glm::mat4(0), deltaTime, BoundingBox::MinMaxBox()};

            defaultShader->Bind();
            //defaultShader->SetUniform1f("alphaDiscard", alphaDiscard);
            
            renderingPipeline.Render(renderingContext);

          
            // feed inputs to dear imgui, start new frame
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            ImGui::Begin("Chloe model");
            ImGui::DragFloat3("Translation", (float *)&chloe->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&chloe->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&chloe->Scale, 0.01, -20, 20);
            ImGui::End();

            ImGui::Begin("Sponza model");
            ImGui::DragFloat3("Translation", (float *)&sponza->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&sponza->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&sponza->Scale, 0.01, -20, 20);
            ImGui::End();

            ImGui::Begin("Crysis Guy");
            ImGui::DragFloat3("Translation", (float *)&cryGuy->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&cryGuy->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&cryGuy->Scale, 0.01, -20, 20);
            ImGui::DragFloat("Cloack refract", &cloackRefractTarget, 0.01, 1, 5);
            ImGui::Checkbox("Engage Cloack", &cloackEngaged);
            ImGui::End();

            /*
                       float targetFps = 60;
                        bool prevCloack = false;
                        bool cloackEngaged = false;

                        float cloackRefractTarget = 1.0 / 1.33;
                        float initialCloack = 0;
                        
                        int animationTime = 1.5 * targetFps;
                        int currentAnimationTime = -1;
            
            */

            if (cloackEngaged != prevCloack) {
                printf("Animating...\n");
                currentAnimationTime++;
                float progress = (float(currentAnimationTime) / float(animationTime));
                if (cloackEngaged) { //cloacking
                     for (auto& cryGuyPart: cryGuy->SceneObjectElements) {
                        cryGuyPart.refractivity = pow(progress, 2);
                        cryGuyPart.refractionRatio = (1.0 / cloackRefractTarget) * pow(progress, 2);
                        printf("Cloack: %f\n", cryGuyPart.refractionRatio);
                    }
                }
                else { //uncloacking
                     for (auto& cryGuyPart: cryGuy->SceneObjectElements) {
                        cryGuyPart.refractivity = 1 - pow(progress, 2);
                        cryGuyPart.refractionRatio = (1.0 / cloackRefractTarget) * (1 - pow(progress, 2));
                         printf("Cloack: %f\n", cryGuyPart.refractionRatio);
                    }
                }

                if (currentAnimationTime == animationTime) {
                    printf("Animation finished!\n");
                    prevCloack = cloackEngaged;
                    currentAnimationTime = -1;
                }
            } else {
                  if (cloackEngaged) {
                    for (auto& cryGuyPart: cryGuy->SceneObjectElements) {
                        cryGuyPart.refractionRatio = (1.0 / cloackRefractTarget);
                    }
                  }else {
                    for (auto& cryGuyPart: cryGuy->SceneObjectElements) {
                        cryGuyPart.refractionRatio = (1.0 / cloackRefractTarget) * 2;
                    }
                  }
            }



            ImGui::Begin("1M cube");
            ImGui::DragFloat3("Translation", (float *)&oneMeterCube->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&oneMeterCube->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&oneMeterCube->Scale, 0.01, -20, 20);
            ImGui::End();

            ImGui::Begin("Light cube");
            ImGui::DragFloat3("Translation", (float *)&pointLight->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&pointLight->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&pointLight->Scale, 0.01, -20, 20);
            ImGui::DragFloat3("Ambient Light", (float *)&pointLight->Light.Ambient, 0.01, 0, 1);
            ImGui::DragFloat3("Diffuse Light", (float *)&pointLight->Light.Diffuse, 0.01, 0, 1);
            ImGui::DragFloat3("Specular Light", (float *)&pointLight->Light.Specular, 0.01, 0, 1);
            ImGui::DragFloat("Quadratic", (float *)&pointLight->Light.Quadratic, 0.01, 0, 100);
            ImGui::DragFloat("Linear", (float*)&pointLight->Light.Linear, 0.01, 0, 100);
            ImGui::DragFloat("Power", (float *)&pointLight->Light.Constant, 1, 0, 1000);
            ImGui::Checkbox("Enable light", (bool *)&pointLight->Light.IsPoint);
            ImGui::End();

            ImGui::Begin("Sun");
            ImGui::DragFloat3("Translation", (float *)&sun->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&sun->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&sun->Scale, 0.01, -20, 20);
            ImGui::DragFloat3("Ambient Light", (float *)&sun->Light.Ambient, 0.01, 0, 1);
            ImGui::DragFloat3("Diffuse Light", (float *)&sun->Light.Diffuse, 0.01, 0, 1);
            ImGui::DragFloat3("Specular Light", (float *)&sun->Light.Specular, 0.01, 0, 1);
            ImGui::End();

            ImGui::Begin("Light cube 2");
            ImGui::DragFloat3("Translation", (float *)&pointLight2->Translation, 0.1, -2000, 2000);
            ImGui::DragFloat3("Rotation", (float *)&pointLight2->Rotation, 0.01, -pi * 2, pi * 2);
            ImGui::DragFloat3("Scale", (float *)&pointLight2->Scale, 0.01, -20, 20);
            ImGui::DragFloat3("Ambient Light", (float *)&pointLight2->Light.Ambient, 0.01, 0, 1);
            ImGui::DragFloat3("Diffuse Light", (float *)&pointLight2->Light.Diffuse, 0.01, 0, 1);
            ImGui::DragFloat3("Specular Light", (float *)&pointLight2->Light.Specular, 0.01, 0, 1);
            ImGui::DragFloat("Quadratic", (float *)&pointLight2->Light.Quadratic, 0.01, 0, 100);
            ImGui::DragFloat("Linear", (float *)&pointLight2->Light.Linear, 0.01, 0, 100);
            ImGui::Checkbox("Enable light", (bool *)&pointLight2->Light.IsPoint);
            ImGui::End();


            ImGui::Begin("Camera and Editor");
            ImGui::DragFloat("Alpha Discard", (float *)&alphaDiscard, 0.0001, 0.0f, 0.999f);
            ImGui::Checkbox("FPS mouse movement", &freeCameraMovementMouse);
            ImGui::DragFloat("Fly speed", &cameraFlyMovement, 0.1, 0, 100);
            ImGui::RadioButton("Show cursor", &glfwCursorState, GLFW_CURSOR_NORMAL);
            ImGui::RadioButton("Hidden cursor", &glfwCursorState, GLFW_CURSOR_HIDDEN);
            ImGui::RadioButton("Disable cursor", &glfwCursorState, GLFW_CURSOR_DISABLED);
            ImGui::Checkbox("Debug rendering", &RenderingGlobalSettings.debugMode);
            ImGui::Text("Performance: %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();

            
            ImGui::Begin("Shadow Map");
            ImVec2 pos = ImGui::GetCursorScreenPos();
           
           
            ImGui::GetWindowDrawList()->AddImage(
                (void*)renderingContext.ShadowMap->TextureId, //user_texture_id
                ImVec2(ImGui::GetCursorScreenPos()), //p_min
                ImVec2(ImGui::GetCursorScreenPos().x + window_width / 4,
                       ImGui::GetCursorScreenPos().y + window_height / 4), //p_max
                ImVec2(0, 1), //uv_min
                ImVec2(1, 0)); //uv_max
            ImGui::End();
            


            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
            glfwSetInputMode(window, GLFW_CURSOR, glfwCursorState);

            lastTime = glfwGetTime();

            glfwSwapBuffers(window);
            glfwPollEvents();

           /* if (sceneBoundingBoxRenderer.get() == nullptr) {
                auto geom = Geometry::FromBoundingBox(renderingContext.SceneBoundingBox);
                sceneBoundingBoxRenderer = scene.FromGeometry(
                    geom, Material::DefaultColoredMaterial(glm::vec3(1, 0, 0)),
                    singleColorShader, Texture(""), Texture(""), Texture(""), Texture("")
                );
                sceneBoundingBoxRenderer->Enabled = true;
                sceneBoundingBoxRenderer->IsDebugHelper = true;
            }
            else {
                auto geom = Geometry::FromBoundingBox(renderingContext.SceneBoundingBox);
                sceneBoundingBoxRenderer->UpdateGeometryVertices(geom);
            }*/


           // printf("Camera position: %f %f %f\n",
              //   renderingContext.CurrentCamera.Position.x,
              //   renderingContext.CurrentCamera.Position.y, 
              //   renderingContext.CurrentCamera.Position.z );
            // printf("Camera angle: h %f v %f\n",
            //     camera.horizontalAngle, camera.verticalAngle);


            FrameMarkEnd("Frame");
        }
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}