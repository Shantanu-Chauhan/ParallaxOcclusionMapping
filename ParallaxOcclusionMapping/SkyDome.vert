/////////////////////////////////////////////////////////////////////////
// Vertex shader for SkyDome
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330
uniform mat4 ModelTr,WorldInverse,WorldProj,WorldView;

in vec4 vertex;
out vec3 eyeVec;

void main()
{   
	vec3 worldPos = (ModelTr*vertex).xyz;
	vec3 eyePos=(WorldInverse*vec4(0,0,0,1)).xyz;
	eyeVec =eyePos-worldPos;
	gl_Position=(WorldProj * WorldView * ModelTr*vertex);
}
