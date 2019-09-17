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

uniform int objectId;
uniform vec3 diffuse;//Kd

uniform vec3 specular;//Ks
uniform float shininess;//alpha exponent


layout (location = 0) out vec4 gPosition;
layout (location = 1) out vec3 gNormal;
layout (location = 2) out vec4 gAlbedoSpec;
layout (location = 3) out vec3 Diffuse;


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
	Diffuse=diffuse;

}  