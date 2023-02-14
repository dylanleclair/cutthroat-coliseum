#version 410 core
out vec4 color;

in vec3 fragCol;
//in vec2 tc;
//in vec3 n;
in vec3 fragPos;

//uniform sampler2D tex;
//uniform uint selector;
//uniform vec3 light;
//uniform vec3 viewPos;
//uniform float ambiantStr;
//uniform float specularStrength;
//uniform vec3 userColor;

void main()
{             
	color = vec4(fragCol, 1);
}