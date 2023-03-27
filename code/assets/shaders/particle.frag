#version 410 core
out vec4 color;
uniform sampler2D tex;

in vec3 tc;

void main()
{             
	color = texture(tex, tc.xy);
	color.w *= tc.z;
	if(color.a == 0)
		discard;
}