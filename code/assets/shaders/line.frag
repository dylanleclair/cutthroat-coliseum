#version 410 core
out vec4 color;

uniform vec3 userColor;

void main()
{             
	color = vec4(userColor, 1);
}