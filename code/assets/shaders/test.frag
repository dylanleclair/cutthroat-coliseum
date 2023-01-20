#version 410 core
out vec4 color;

in vec3 fragCol;

void main() {
	color = vec4(fragCol, 1.0f);
} 
