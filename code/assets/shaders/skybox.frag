#version 410 core
layout (location = 2) out vec3 gColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
    gColor = vec3(texture(skybox, TexCoords));
}