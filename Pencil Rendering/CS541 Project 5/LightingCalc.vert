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

out vec3 normalVec, lightVec;
out vec2 texCoord;
out vec3 worldPos;
out vec3 eyeVec;
out vec4 ShadowCoord;
out vec3 tanVec;


void LightingVert(vec3 eye)
{      
    worldPos = (ModelTr*vertex).xyz;
    normalVec = vertexNormal*mat3(NormalTr); 
	ShadowCoord = ShadowMatrix * ModelTr * vertex;
	lightVec=lightPos-worldPos;
	eyeVec =eye-worldPos;
    texCoord = vertexTexture; 
	tanVec =mat3(ModelTr)*vertexTangent; // Proper way to transform a VECTOR
}
