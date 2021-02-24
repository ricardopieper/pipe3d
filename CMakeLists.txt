# CMakeList.txt : Top-level CMake project file, do global configuration
# and include sub-projects here.
#
cmake_minimum_required (VERSION 3.8)

project ("Pipe3D")
set(CMAKE_BUILD_TYPE Debug)

# Include sub-projects.
set(SRC_DIR "src")
set(VENDOR_DIR "vendor")

file(GLOB pipe3d_renderer CONFIGURE_DEPENDS 
     "src/Renderer/*.cpp"
)

add_executable(Pipe3D_Playground
    "src/Playground.cpp"
	${pipe3d_renderer})
    
target_include_directories(Pipe3D_Playground PRIVATE ${SRC_DIR})
target_include_directories(Pipe3D_Playground PRIVATE "${VENDOR_DIR}/stb")
set_property(TARGET Pipe3D_Playground PROPERTY CXX_STANDARD 11)
set_property(TARGET Pipe3D_Playground PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "./")


#STB
set(STB_DIR "${VENDOR_DIR}/stb")
add_library("stb" "${STB_DIR}/stb_image.cpp")
target_include_directories("stb" PRIVATE "${STB_DIR}")
target_include_directories(Pipe3D_Playground PRIVATE "${STB_DIR}")
target_link_libraries(Pipe3D_Playground "stb" "${CMAKE_DL_LIBS}")

#GLFW
set(GLFW_DIR "${VENDOR_DIR}/glfw")
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
add_subdirectory("${GLFW_DIR}")
target_link_libraries(Pipe3D_Playground "glfw" "${GLFW_LIBRARIES}")
target_include_directories(Pipe3D_Playground PRIVATE "${GLFW_DIR}/include")
target_compile_definitions(Pipe3D_Playground PRIVATE "GLFW_INCLUDE_NONE")

#GLAD
set(GLAD_DIR "${VENDOR_DIR}/glad")
add_library("glad" "${GLAD_DIR}/src/glad.c")
target_include_directories("glad" PRIVATE "${GLAD_DIR}/include")
target_include_directories(Pipe3D_Playground PRIVATE "${GLAD_DIR}/include")
target_link_libraries(Pipe3D_Playground "glad" "${CMAKE_DL_LIBS}")

#IMGUI
set(IMGUI "${VENDOR_DIR}/imgui")
add_library("imgui" 
    "${IMGUI}/imgui.cpp" 
    "${IMGUI}/imgui_draw.cpp" 
    "${IMGUI}/imgui_tables.cpp" 
    "${IMGUI}/imgui_widgets.cpp" 
    "${IMGUI}/backends/imgui_impl_opengl3.cpp" 
    "${IMGUI}/backends/imgui_impl_glfw.cpp")
    
target_include_directories("imgui" PRIVATE "${GLAD_DIR}/include")
target_include_directories("imgui" PRIVATE "${GLFW_DIR}/include")
target_include_directories("imgui" PRIVATE "${IMGUI}")
target_include_directories(Pipe3D_Playground PRIVATE "${IMGUI}")
target_link_libraries(Pipe3D_Playground "imgui" "${CMAKE_DL_LIBS}")
#target_link_libraries("glfw" "imgui" "${CMAKE_DL_LIBS}")
#target_link_libraries("glad" "imgui" "${CMAKE_DL_LIBS}")
target_link_libraries("imgui" "glfw" "${CMAKE_DL_LIBS}")
target_link_libraries("imgui" "glad" "${CMAKE_DL_LIBS}")

#GLM
set(GLM_DIR "${VENDOR_DIR}/glm")
add_subdirectory("${GLM_DIR}")
target_include_directories(Pipe3D_Playground PRIVATE "${GLM_DIR}")
