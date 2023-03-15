#version 410 core
out vec4 color;

in vec2 tc;

uniform sampler2D tex;

void main()
{             
	color = texture(tex, tc);
}