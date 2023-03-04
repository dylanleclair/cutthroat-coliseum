#version 410 core
out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gDepth;

//uniform vec3 light;
//uniform float ambiantStr;

in vec2 tc;

float near = 2.0; 
float far  = 100.0; 
float hstep = 1.0/1200.0;
float vstep = 1.0/800.0;
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
	vec3 tposition = texture(gPosition, tc).xyz;
	vec3 tnormal = texture(gNormal, tc).xyz;
	vec3 tcolor = texture(gColor, tc).xyz;
	float tdepth = LinearizeDepth(texture(gDepth, tc).x);

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
	//color = vec4(tdepth, 0, 0, 1);

	float depthDiff = 0.0;
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(hstep, 0)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(-hstep, 0)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(0, vstep)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(0, vstep)).x));

	float normalDiff = 0.0;
	depthDiff += abs(tdepth - LinearizeDepth(texture(gNormal, tc + vec2(hstep, 0)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gNormal, tc + vec2(-hstep, 0)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gNormal, tc + vec2(0, vstep)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gNormal, tc + vec2(0, -vstep)).x));

	float outline = (depthDiff + normalDiff);
	//if(outline > 10)
		//color = vec4(vec3(outline), 1.0);
	//else
		color = vec4(vec3(tcolor),1);
}