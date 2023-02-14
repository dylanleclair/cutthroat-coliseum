#version 410 core
layout (location = 0) in vec3 vertPos;
//layout (location = 1) in vec3 vertCol;
//layout (location = 2) in vec2 texCoord;
layout (location = 3) in vec3 vertNormal;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
//uniform mat3 normalMat;
//uniform uint selector;

//out vec3 n;
out vec3 fragCol;
//out vec3 fragPos;
//out vec2 tc;

void main() {
	gl_Position = P * V * M * vec4(vertPos, 1.0);
	fragCol = vec3(0);
}
