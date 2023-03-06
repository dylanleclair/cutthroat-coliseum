#version 410 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gColor;

in vec2 tc;
in vec3 n;
in vec3 fragPos;

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
	gColor = sampleCol;
	gNormal = normalize(n);
	gPosition = fragPos;
}