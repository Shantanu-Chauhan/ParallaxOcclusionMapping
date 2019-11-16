/////////////////////////////////////////////////////////////////////////
// Pixel shader for SkyDome
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

uniform int objectId;
uniform sampler2D TEXTURE;
in vec3 eyeVec;
out vec4 FragColor;


void main()
{
	vec3 V= normalize(eyeVec);
	vec2 uv;
	uv=vec2(1/2 - atan(V.y,V.x)/(2*3.14), acos(V.z)/3.14);
	FragColor.xyz =  texture(TEXTURE, uv).xyz;
	//FragColor.xyz =  vec3(1.0,0.0,0.0);
}  