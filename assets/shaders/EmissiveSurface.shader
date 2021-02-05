#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 fragmentColor;
out vec3 fragmentNormal;
out vec3 worldspaceVertex;

void main() {
    worldspaceVertex = vec3(u_model * vec4(position, 1));
    gl_Position = u_projection * u_view * u_model * vec4(position, 1);
    fragmentColor = vertexColor;
    fragmentNormal = vertexNormal;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;

uniform vec2 u_colorSource;
in vec3 fragmentColor;
in vec3 fragmentNormal;
in vec3 worldspaceVertex;
        
void main() {
    color = vec4(1,1,1,1);
}