#shader vertex
#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 vertexColor;
layout(location = 2) in vec3 vertexNormal;
layout(location = 3) in vec2 uv;
layout(location = 4) in float vertexSpecularStrength;

uniform mat4 u_projection;
uniform mat4 u_view;
uniform mat4 u_model;

out vec3 fragmentColor;
out vec3 fragmentNormal;
out vec3 worldspaceVertex;
out vec2 fragmentUv;
out float specularStrength;

void main() {
    worldspaceVertex = vec3(u_model * vec4(position, 1));
    gl_Position = u_projection * u_view * u_model * vec4(position, 1);
    fragmentColor = vertexColor;
    fragmentNormal = vertexNormal;
    fragmentUv = uv;
    specularStrength = vertexSpecularStrength;
}

#shader fragment
#version 330 core

layout(location = 0) out vec4 color;


in vec3 fragmentColor;
in vec3 fragmentNormal;
in vec3 worldspaceVertex;
in vec2 fragmentUv;
in float specularStrength;

uniform vec3 lampPosition;
uniform vec3 cameraPosition;
uniform float ambientLight;

uniform vec2 u_colorSource;
uniform sampler2D textureSampler;

void main() {
    //a vector pointing towards the location where the light hits
    
    float fromVertexColor = u_colorSource.x;
    float fromTextureColor = u_colorSource.y;

    vec4 fragVertexColor = fromVertexColor * vec4(fragmentColor, 1);
    vec4 fragTextureColor = fromTextureColor * texture(textureSampler, fragmentUv);


    //vec4 calculatedFragColor = vec4(fragmentColor, 1);
    vec4 calculatedFragColor = fragVertexColor + fragTextureColor;
   
    if (calculatedFragColor.a < 0.5) discard;

    vec3 lightToSurface = normalize(lampPosition - worldspaceVertex);
    vec3 normal = normalize(fragmentNormal); //ensure normal is actually normal
    float lightAngle = dot(normal, lightToSurface);
    float diffusion = max(lightAngle, 0.0);
    vec4 diffuse = diffusion * calculatedFragColor;

    float shininess = 32;
    //compute the vector that points from worldspaceVertex towards the camera
    vec3 cameraToSurface = normalize(cameraPosition - worldspaceVertex);

    //we need a vector that points towards the light source
    vec3 surfaceToLight = -lightToSurface;

    //imagine a ray of light hitting the surface at an angle.
    //Specifically, suppose the light is hitting at worldspaceVertex
    //(though worldspaceVertex is being interpolated as usual)
    //the normal is perpendicular to the surface. If we imagine
    //a surface with a face pointing upwards, the normal is a vector
    //that indicates the "pointing upwards" part (vec(0,1,0)). 
    //we reflect this ray light along the normal, so now we have
    //a ray pointing to the other side.
    //This is the R vector in this image: 
    //https://learnopengl.com/img/lighting/basic_lighting_specular_theory.png  
    vec3 lightReflection = reflect(surfaceToLight, normal);

    //the smaller the angle, the more light hits the camera
    float angleFromCameraToReflection = dot(cameraToSurface, lightReflection);
   
    //negatives have no meaning here
    angleFromCameraToReflection = max(angleFromCameraToReflection, 0);

    float specularFactor = pow(angleFromCameraToReflection, shininess);
    vec4 specular = specularStrength * specularFactor * vec4(1);
    
    vec4 colorResult = (ambientLight + diffuse + specular) * calculatedFragColor;

    //distance from light to point
    float dist = distance(lampPosition, worldspaceVertex) / 3;

    colorResult = colorResult / dist;

    
    color = colorResult;
}