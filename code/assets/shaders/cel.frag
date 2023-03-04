#version 410 core
out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gDepth;

//variable uniforms
uniform float normalDiffWeight;
uniform float depthDiffWeight;

uniform vec3 lightDir;
uniform float ambiantStr;
uniform float diffuseWeight;
uniform int numQuantizedSplits;

in vec2 tc;

float near = 2.0; 
float far  = 1000.0; 
float hstep = 1.0/1200.0;
float vstep = 1.0/800.0;
  
float LinearizeDepth(float depth) 
{
	return depth;
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
	vec3 tposition = texture(gPosition, tc).xyz;
	vec3 tnormal = texture(gNormal, tc).xyz;
	vec3 tcolor = texture(gColor, tc).xyz;
	float tdepth = LinearizeDepth(texture(gDepth, tc).x);

	//determine the lighting
	vec3 diff = max(dot(lightDir, tnormal), 0.0) * vec3(1,0.97,0.94) * diffuseWeight;
	
	//calculate ambiant
	vec3 ambiant = tcolor * ambiantStr;

	
	//calculate edges
	float depthDiff = 0.0;
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(hstep, 0)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(-hstep, 0)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(0, vstep)).x));
	depthDiff += abs(tdepth - LinearizeDepth(texture(gDepth, tc + vec2(0, vstep)).x));

	float normalDiff = 0.0;
	normalDiff += distance(tnormal, texture(gNormal, tc + vec2(hstep, 0)).xyz);
	normalDiff += distance(tnormal, texture(gNormal, tc + vec2(-hstep, 0)).xyz);
	normalDiff += distance(tnormal, texture(gNormal, tc + vec2(0, vstep)).xyz);
	normalDiff += distance(tnormal, texture(gNormal, tc + vec2(0, -vstep)).xyz);

	float outline = (depthDiff * depthDiffWeight + normalDiff * normalDiffWeight);

	//quantize the color
	vec3 calculatedCol = (diff + ambiant) * tcolor;
	vec3 quantized = (ceil(calculatedCol * numQuantizedSplits) - 1)/(numQuantizedSplits - 1);     
	
	//calculate final color
	color = mix(vec4(quantized, 1),vec4(0,0,0,1),outline);
	//color = mix(vec4(tcolor, 1), vec4(0,0,0,1), outline);
}