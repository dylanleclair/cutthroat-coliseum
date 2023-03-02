#version 410 core
out vec4 color;

in vec2 tc;
in vec3 n;
in vec3 fragPos;

uniform sampler2D tex;
uniform vec3 light;
uniform vec3 viewPos;
uniform float ambiantStr;
uniform float specularStrength;
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

	//determine the lighting
	vec3 lightDir = normalize(light - fragPos);
	vec3 normal = normalize(n);
	vec3 diff = max(dot(lightDir, normal),0.0) * vec3(1);

	//calculate specular
	vec3 viewDir = normalize(viewPos-fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir),0.0f),32);
	vec3 specular = specularStrength * spec * vec3(1.0f);
	
	//calculate ambiant
	vec3 ambiant = sampleCol * ambiantStr;

	//calculate final color
	//color = vec4(n.x,n.y,n.z,1);
	//if(dot(n, viewDir) < 0.2)
		//color = vec4(0,0,0,1);
	//else
		color = vec4((diff + ambiant + specular) * sampleCol, 1.0f);

}