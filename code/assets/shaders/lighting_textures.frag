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

void main()
{             
    //determine the fragment color
	vec3 sampleCol = vec3(texture(tex, tc));

	//determine the lighting
	//calculate diffuse lighting
	vec3 lightDir = normalize(light - fragPos);
	vec3 normal = normalize(n);
	vec3 diff = max(dot(lightDir, normal),0.0) * vec3(1);

	//calculate specular lighting
	vec3 viewDir = normalize(viewPos-fragPos);
	vec3 reflectDir = reflect(-lightDir, normal);
	float spec = pow(max(dot(viewDir, reflectDir),0.0f),32);
	vec3 specular = specularStrength * spec * vec3(1.0f);
		
	//calculate ambiant lighting
	vec3 ambiant = sampleCol * ambiantStr;

	//calculate final ligthing
	color = vec4((diff + ambiant + specular) * sampleCol, 1.0f);
}