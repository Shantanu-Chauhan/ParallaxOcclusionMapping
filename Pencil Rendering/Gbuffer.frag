/////////////////////////////////////////////////////////////////////////
// Pixel shader for lighting
////////////////////////////////////////////////////////////////////////
#version 330

// These definitions agree with the ObjectIds enum in scene.h
const int     nullId	= 0;
const int     skyId	= 1;
const int     seaId	= 2;
const int     groundId	= 3;
const int     roomId	= 4;
const int     boxId	= 5;
const int     frameId	= 6;
const int     lPicId	= 7;
const int     rPicId	= 8;
const int     teapotId	= 9;
const int     spheresId	= 10;

in vec3 normalVec;
in vec4 FragPos;
in mat3 TBN;
in vec3 viewPosition;
in vec3 tanVec;
out vec4 FragColor;
uniform int objectId;
uniform vec3 diffuse;//Kd
uniform mat4 WorldInverse;

uniform vec3 specular;//Ks
in vec2 texCoord;//texcoord
uniform float shininess;//alpha exponent
uniform float heightPresent;
uniform float normalPresent;
uniform sampler2D TEXTURE;
uniform sampler2D NORMAL;
uniform sampler2D HEIGHT;

uniform int GBufferNum;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 Diffuse;


vec2 ParallaxMapping(vec2 uv, vec3 V)
{
	float height = texture(HEIGHT, uv).r;
	return uv - V.xy * (height * 0.1);
}

vec2 ParallaxOcclusionMapping(vec2 uv, vec3 V)
{
	const float minLayers = 0;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0, 0.0, 1.0), V)));

	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0;

	vec2 P = V.xy / V.z * 0.1;//0.1 is height scale
	vec2 deltaUV = P / numLayers;

	vec2 currentUV = uv;
	float currentDMV = texture(HEIGHT, currentUV).r;

	while(currentLayerDepth < currentDMV)
	{
		currentUV -= deltaUV;
		currentDMV = texture(HEIGHT, currentUV).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevUV = currentUV + deltaUV;

	float afterDepth = currentDMV - currentLayerDepth;
	float beforeDepth = texture(HEIGHT, prevUV).r - currentLayerDepth + layerDepth;

	float weight = afterDepth / (afterDepth - beforeDepth);
	vec2 finalUV = prevUV * weight + currentUV * (1.0 - weight);

	return finalUV;
}

void main()
{    
	// store the fragment position vector in the first gbuffer texture
	gPosition = FragPos;
	
	// also store the per-fragment normals into the gbuffer
	gNormal = normalize(normalVec);
	   
	// Assuming if a height map is present, a normal map is also present
	//if(heightPresent == 1.0)
	//{
	//	vec2 uv = texCoord;
	//	vec3 V = viewPosition;
	//	uv = ParallaxMapping(uv, V);
	//	//uv = ParallaxOcclusionMapping(uv, V);
	//	if(uv.x > 1.0 || uv.y > 1.0 || uv.x < 0.0 || uv.y < 0.0)
	//		discard;
	//
	//	vec3 normal = normalize(texture(NORMAL, uv).rgb * 2.0 - vec3(1.0));
	//	//gNormal = vec4(normal, gl_FragDepth);
	//	gNormal = normal;
	//}

	//else if(normalPresent == 1)
	//{
	//	vec3 normal = texture(NORMAL, texCoord).rgb;
	//	normal = normalize(normal * 2.0 - 1.0);
	//	normal = normalize(TBN * normal);
	//	//gNormal = vec4(normal.xyz, gl_FragDepth);
	//	gNormal = normal;
	//}

	// and the diffuse per-fragment color
	gAlbedoSpec.rgb = specular;
	   
	// store specular intensity in gAlbedoSpec's alpha component
	gAlbedoSpec.a = shininess;

	if(objectId==groundId)
	{
		vec2 New=texCoord*50;
		New=fract(New);
		Diffuse=texture(TEXTURE,New).xyz;
	}
	//else if(objectId==seaId)
	//{
	//	vec3 No = gNormal.xyz;
	//	vec3 eyePos = (WorldInverse * vec4(0.0,0.0,0.0,1.0)).xyz;
	//	vec3 V = normalize(eyePos - FragPos.xyz);
	//	vec2 New=vec2(texCoord.y*500,texCoord.x*500);
	//	New=fract(New);
	//	vec3 delta = texture(NORMAL, New).xyz;	//	vec2 uv;
	//	delta=delta*2.0 - vec3(1.0,1.0,1.0);	//	vec3 T = normalize(FragPos.xyz);	//	vec3 B= normalize(cross(T,No));
	//	vec3 N = delta.x*T + delta.y*B + delta.z*No;
	//	
	//	vec3 R= vec3(- ( 2*dot(V,No)*No - V) ) ;
	//	uv=vec2(1/2 - atan(R.y,R.x)/(2*3.14), acos(R.z)/3.14); 
	//	Diffuse =  texture(TEXTURE, uv).xyz;
	//}
	else if (objectId == roomId)
	{
		vec3 No = gNormal.xyz;
		vec3 eyePos = (WorldInverse * vec4(0.0,0.0,0.0,1.0)).xyz;
		vec3 V = normalize(eyePos - FragPos.xyz);
		vec2 New=vec2(texCoord.y*30,-texCoord.x*15);
		New=fract(New);								
		vec3 delta = texture(NORMAL, New).xyz;				vec2 uv;
		delta=delta*2.0 - vec3(1.0,1.0,1.0);		vec3 T = normalize(tanVec);		vec3 B= normalize(cross(T,No));
		gNormal = delta.x*T + delta.y*B + delta.z*No;
		Diffuse =texture(TEXTURE,New).xyz;
	}
	else
	Diffuse= texture(TEXTURE,texCoord).xyz + diffuse;
	//Diffuse = diffuse;
}  