#version 410 core
out vec4 color;

uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gColor;
uniform sampler2D gDepth;
uniform sampler2D gShadow;

//variable uniforms
uniform float normalDiffWeight = 1;
uniform float depthDiffWeight = 1;
uniform vec3 goochCool = vec3(125/255.0, 249/255.0, 255/255.0);
uniform vec3 goochWarm = vec3(149/255.0, 53/255.0, 83/255.0);
uniform float goochWeight = 0;

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
	float shadow = texture(gShadow, tc).r; //get the alpha channel which holds the shadow data
	float tdepth = LinearizeDepth(texture(gDepth, tc).x);
	vec3 nlightDir = normalize(lightDir);

	//determine the lighting
	vec3 diff = max(dot(nlightDir, tnormal), 0.0) * vec3(1,0.97,0.94) * diffuseWeight;
	
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

	//calculate Gooch shading
	vec3 gooch = mix(goochCool, goochWarm, (1 + dot(nlightDir, tnormal))/2.0);

	//quantize the color
	vec3 calculatedCol = (diff + ambiant) * tcolor;
	vec3 quantized = (ceil(calculatedCol * numQuantizedSplits) - 1)/(numQuantizedSplits - 1);    
	
	//calculate final color
	//if shadow = 1 then the pixel is in shadow
	color = mix(vec4(mix(quantized, gooch, goochWeight), 1),vec4(0,0,0,1),outline) * ((1-shadow) + (shadow * 0.4));
	//color = vec4(tcolor, 1) * ((1-shadow) + (shadow * 0.8));
	//color = vec4(shadow, 0, 0, 1);

}