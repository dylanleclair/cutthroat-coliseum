#version 410 core
layout (location = 0) in vec3 vertPos;
layout (location = 1) in vec3 vertCol;
layout (location = 2) in vec2 texCoord;

uniform mat4 M;
uniform mat4 V;
uniform mat4 P;

out vec3 fragCol;
out vec2 tc;

void main() {
	fragCol = vertCol;
	tc = texCoord;
	gl_Position = P * V * M * vec4(vertPos, 1.0);
}
