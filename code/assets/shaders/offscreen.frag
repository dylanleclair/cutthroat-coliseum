#version 410 core
layout(location = 0) out vec3 color;
//layout(location = 1) out vec3 normal;

in vec2 tc;
in vec3 n;

uniform sampler2D tex;
uniform uint shaderState;
uniform vec3 userColor;


void main()
{             
    //determine the fragment color
	vec3 sampleCol = vec3(1);
	if(shaderState == 1) {
		sampleCol = vec3(texture(tex, tc));
	} else {
		sampleCol = userColor;
	}

	//calculate final color
	color = vec3(0,0,1);//sampleCol;
	//normal = vec3(1,0,0);//normalize(n);
}