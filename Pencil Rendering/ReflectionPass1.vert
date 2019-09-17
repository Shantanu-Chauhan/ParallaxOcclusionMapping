/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr,ShadowMatrix;
uniform vec3 lightPos;
uniform vec3 eyepos;
uniform float c;


in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;
in vec2 uv;


out vec3 worldPos;
void LightingVert(vec3 eyePos);
void main()
{  
	worldPos = (ModelTr*vertex).xyz;
	vec3 eyePos=eyepos;
	vec3 R=worldPos-eyePos;
	vec3 Ra=R/length(R);
    gl_Position.x =Ra.x/(1+(c*Ra.z));
	gl_Position.y=Ra.y/(1+(c*Ra.z));
	gl_Position.z=(c*Ra.z*length(R)/1000)-1.0;
	gl_Position.w=1.0;
	LightingVert(eyePos);
}
