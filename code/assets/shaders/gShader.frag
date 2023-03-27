#version 410 core
layout (location = 0) out vec3 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec3 gColor;
layout (location = 3) out float gShadow;

in vec2 tc;
in vec3 n;
in vec3 fragPos;
in vec4 fragPosLightSpace;

uniform sampler2D gShadowDepth;
uniform sampler2D diffTexture;
uniform uint shaderState;
uniform vec3 userColor;

float ShadowCalculation(vec4 fragPosLightSpace_in)
{
    // perform perspective divide
    vec3 projCoords = fragPosLightSpace_in.xyz / fragPosLightSpace_in.w;
	if(projCoords.z > 1)
		return 0;
    // transform to [0,1] range
    projCoords = projCoords * 0.5 + 0.5;
    // get closest depth value from light's perspective (using [0,1] range fragPosLight as coords)
    float closestDepth = texture(gShadowDepth, projCoords.xy).r; 
    // get depth of current fragment from light's perspective
    float currentDepth = projCoords.z;
    // check whether current frag pos is in shadow
	float bias = max(0.5 * (1.0 - dot(normalize(gNormal), normalize(vec3(1, -3, 1)))), 0.005);  
    float shadow = ((currentDepth - closestDepth) > bias)  ? 1.0 : 0.0;

    return shadow;
}  

void main()
{             
    //determine the fragment color
	vec3 sampleCol = vec3(1);
	if((shaderState & 1) != 0) {
		sampleCol = vec3(texture(diffTexture, tc));
	} else {
		sampleCol = userColor;
	}

	//determine if the fragment is in shadow
	float shadow = ShadowCalculation(fragPosLightSpace);


	//calculate final color
	gColor = sampleCol;
	gNormal = normalize(n);
	gPosition = fragPos;
	gShadow = shadow;
}