#version 410 core
layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec2 texCoord;

uniform mat4 P;
uniform mat4 V;
uniform vec3 lockingAxis;
uniform vec2 scale;
uniform vec3 centrePos;
uniform vec3 cameraPos;

out vec2 tc;

void main()
{
    vec3 cameraDir = normalize(centrePos - cameraPos);
    vec3 cameraRight = normalize(cross(vec3(0,1,0), cameraDir));
    vec3 cameraUp = normalize(cross(cameraDir, cameraRight));

    if(length(lockingAxis) > 0) {
        cameraRight = normalize(cross(cameraDir, lockingAxis));
        cameraUp = lockingAxis;
    }

    vec3 vertexPosition_worldspace = centrePos + cameraRight * vertPos.x * scale.x + cameraUp * vertPos.y * scale.y;
    gl_Position = P * V * vec4(vertexPosition_worldspace, 1);
    tc = texCoord;
}