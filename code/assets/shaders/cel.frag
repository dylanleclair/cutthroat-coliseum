#version 410 core
out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gDepth;

//uniform vec3 light;
//uniform float ambiantStr;

in vec2 tc;

void main()
{             
    //determine the fragment color
	//vec3 sampleCol = vec3(1);
	//if(shaderState == 1) {
	//	sampleCol = vec3(texture(tex, tc));
	//} else {
	//	sampleCol = userColor;
	//}

	//determine the lighting
	//vec3 lightDir = normalize(light - fragPos);
	//vec3 normal = normalize(n);
	//vec3 diff = max(dot(lightDir, normal),0.0) * vec3(1);
	
	//calculate ambiant
	//vec3 ambiant = sampleCol * ambiantStr;

	//calculate final color
	//color = vec4((diff + ambiant) * sampleCol, 1.0f);
	color = texture(gDepth, tc);
	//color = vec4(gl_FragCoord.x / 1200, gl_FragCoord.y / 800, 0, 1);
}