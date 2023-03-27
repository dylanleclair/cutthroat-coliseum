#version 410 core
layout (location = 0) in vec2 vertPos;
layout (location = 1) in vec4 transformation;   //first 3 are position, last is scale
layout (location = 2) in vec3 transformation2; //cos and sin of the matrix

uniform mat4 P;
uniform mat4 V;
uniform vec3 cameraPos;

out vec3 tc;

void main() {
    vec2 rotation = transformation2.xy;
    vec3 centrePos = transformation.xyz;
    vec3 cameraDir = centrePos - cameraPos;
    vec3 cameraRight = normalize(cross(vec3(0,1,0), cameraDir));
    vec3 cameraUp = normalize(cross(cameraDir, cameraRight));

    mat2 rotationMatrix = mat2(rotation.x, rotation.y, -rotation.y, rotation.x);
    vec2 newVertPos = rotationMatrix * vertPos;

    vec3 vertexPosition_worldspace = (centrePos + cameraRight * newVertPos.x * transformation.w + cameraUp * newVertPos.y * transformation.w);
    gl_Position = P * V * vec4(vertexPosition_worldspace, 1);

	tc = vec3(vertPos + 0.5f, transformation2.z);
}