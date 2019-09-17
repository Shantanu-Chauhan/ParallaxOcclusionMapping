/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldInverse, WorldProj, ModelTr, NormalTr,ShadowMatrix;
uniform vec3 lightPos;

in vec4 vertex;
in vec3 vertexNormal;
in vec2 vertexTexture;
in vec3 vertexTangent;
in vec2 uv;
out vec3 tanVec;

void LightingVert(vec3 eye);

void main()
{      
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
	vec3 eyePos=(WorldInverse*vec4(0,0,0,1)).xyz;
	LightingVert(eyePos);
	tanVec =mat3(ModelTr)*vertexTangent; // Proper way to transform a VECTOR
}
