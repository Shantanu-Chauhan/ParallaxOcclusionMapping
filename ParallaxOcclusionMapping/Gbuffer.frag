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
in mat3 TBN1;
in vec3 viewPosition;

in vec3 normalVec;
in vec4 FragPos;
in vec3 tanVec;
out vec4 FragColor;
uniform int objectId;
uniform vec3 diffuse;//Kd
uniform mat4 WorldInverse;

uniform vec3 specular;//Ks
in vec2 texCoord;//texcoord
uniform float shininess;//alpha exponent
uniform sampler2D TEXTURE;
uniform sampler2D NORMAL;
uniform sampler2D HEIGHT;

uniform float heightScale;

uniform int GBufferNum;

layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 Diffuse;


vec2 NormalParallax(vec2 uv, vec3 ViewVector)
{
	float height = texture(HEIGHT, uv).x;
    return uv -  ViewVector.xy * (height * heightScale); //0.1 is the height scale
}

vec2 ParallaxOcclusion(vec2 uv, vec3 V)
{
	const float minLayers = 8;
	const float maxLayers = 32;
	float numLayers = mix(maxLayers, minLayers, abs(dot(vec3(0.0,0.0,1.0), V)));

	float layerDepth = 1.0 / numLayers;

	float currentLayerDepth = 0.0;

	vec2 P = vec2(V.x,V.y) * heightScale; //0.1 is height scale
	vec2 delUV = P / numLayers;

	vec2 currentUV = uv;
	float currentDepth = texture(HEIGHT, currentUV).r;

	while(currentLayerDepth < currentDepth)
	{
		currentUV -= delUV;
		currentDepth = texture(HEIGHT, currentUV).r;
		currentLayerDepth += layerDepth;
	}

	vec2 prevUV = currentUV + delUV;

	float afterDepth = currentDepth - currentLayerDepth;
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
	else if (objectId == roomId)
	{
		vec3 Normal = normalize(normalVec);
		vec3 T = normalize(tanVec);
		vec3 B = normalize(cross(T,Normal));
		mat3 TBN = transpose(mat3(T,B,Normal));

		vec3 eyePos = (WorldInverse * vec4(0.0,0.0,0.0,1.0)).xyz;
		vec3 V = normalize(eyePos - FragPos.xyz);

		vec2 newuv;
		vec3 delta;
		vec2 uv=vec2(texCoord.x,texCoord.y);
		//Used for normal map
		//delta = texture(NORMAL, uv).xyz;		
		//delta=delta*2.0 - vec3(1.0,1.0,1.0);

		vec3 tangentViewPos = vec3(TBN * V);

		//newuv = NormalParallax(uv, tangentViewPos);
		newuv = ParallaxOcclusion(uv, tangentViewPos);

		if(newuv.x>1 || newuv.x<0 ||newuv.y>1||newuv.y<0)
			discard;

		delta = texture(NORMAL, newuv).xyz;		
		delta = delta*2.0 - vec3(1.0,1.0,1.0);
		gNormal = normalize(delta);
		
		// Used for normal map
		//Diffuse =vec3(texture(TEXTURE, uv));
		Diffuse =texture(TEXTURE,newuv).xyz;
	}
	else
	Diffuse= texture(TEXTURE,texCoord).xyz + diffuse;
}  