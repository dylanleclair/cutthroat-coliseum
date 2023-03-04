#version 410 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertNormal;
layout (location = 2) in vec2 texCoord;


uniform mat4 M;
uniform mat4 V;
uniform mat4 P;
uniform mat3 normalMat;

out vec3 n;
out vec2 tc;
out vec3 fragPos;

void main() {
	gl_Position = P * V * M * vec4(vertPos, 1.0);
	n = normalMat * vertNormal;
	tc = texCoord;
	fragPos = vec3(M * vec4(vertPos, 1.0));
}