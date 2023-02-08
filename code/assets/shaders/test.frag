#version 410 core
out vec4 color;

in vec3 fragCol;
in vec2 tc;

uniform sampler2D tex;
uniform int selector;

void main() {
	if(selector == 0) {
		color = vec4(fragCol, 1.0f);
	} else {
		vec3 texCol = vec3(texture(tex, tc));
		color = vec4(texCol, 1.0f);
	}
} 
