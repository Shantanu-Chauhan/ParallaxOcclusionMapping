/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldProj, ModelTr,NormalTr;

in vec4 vertex;
in vec3 vertexNormal;

out vec4 FragPos;
out vec3 normalVec;
void LightingVert(vec3 eye);

void main()
{   
    gl_Position=WorldProj*WorldView*ModelTr*vertex;
	FragPos = (ModelTr*vertex); 
	normalVec = vertexNormal*mat3(NormalTr); 
}
