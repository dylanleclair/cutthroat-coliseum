#version 410 core
out vec4 color;

uniform sampler2D tex;

in vec2 tc;

void main()
{             
	color = texture(tex, tc);
	if(color.a == 0)
		discard;
}