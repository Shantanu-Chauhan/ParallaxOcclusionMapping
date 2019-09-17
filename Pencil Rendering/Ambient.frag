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

uniform vec3 Ambient;
uniform float width;
uniform float height;
out vec4 FragColor;
uniform sampler2D Diffuse;

void main()
{
	vec2 Position=gl_FragCoord.xy;
	Position.x=Position.x/width;
	Position.y=Position.y/height;
	vec3 Kd = texture(Diffuse, Position).xyz; 
	FragColor.xyz=Ambient*Kd;
}  