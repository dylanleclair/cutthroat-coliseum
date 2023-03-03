#version 410 core
layout (location = 0) in vec3 vertPos;

out vec2 tc;

void main() {
	gl_Position = vec4(vertPos, 1.0);
	tc = (vertPos.xy + 1)/2;
}
