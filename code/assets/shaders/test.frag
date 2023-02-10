#version 410 core
out vec4 color;

in vec3 fragCol;
in vec2 tc;
in vec3 n;
in vec3 fragPos;

uniform sampler2D tex;
uniform uint selector;
uniform vec3 light;
uniform vec3 viewPos;
uniform float ambiantStr;
uniform float specularStrength;

void main() {
	//determine the fragment color
	vec3 sampleCol = vec3(1.0f);
	if((selector & 1) != 0) {
		sampleCol = fragCol;
	} else if((selector & 2) != 0) {
		sampleCol = vec3(texture(tex, tc));
	} else {
		sampleCol = vec3(1.0f);
	}	

	//determine the lighting
	if((selector & 4) != 0) {
		vec3 lightDir = normalize(light - fragPos);
		vec3 normal = normalize(n);
		float diff = max(dot(lightDir, normal), 0.0);
		vec3 specular = vec3(0.0f);
		if((selector & 8) != 0) {
			vec3 viewDir = normalize(viewPos-fragPos);
			vec3 reflectDir = reflect(-lightDir, normal);
			float spec = pow(max(dot(viewDir, reflectDir),0.0f),32);
			specular = specularStrength * spec * vec3(1.0f);
		}
		vec3 ambiant = sampleCol * ambiantStr;
		color = vec4((ambiant + diff + specular) * sampleCol, 1.0f);
	} else {
		color = vec4(sampleCol, 1.0f);
	}

} 
