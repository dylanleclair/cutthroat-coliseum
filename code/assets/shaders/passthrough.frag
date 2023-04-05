#version 410 core
out vec4 color;

uniform sampler2D inCol;

in vec2 tc;

void main() {
	color = vec4(texture(inCol, tc));
}