/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 WorldView, WorldProj, ModelTr,NormalTr;

in vec4 vertex;
in vec3 vertexNormal;
in vec3 vertexTangent;
in vec2 vertexTexture;

out vec4 FragPos;
out vec3 normalVec;
out vec2 texCoord;
out mat3 TBN1;
out vec3 viewPosition;
out vec3 tanVec;
void main()
{   
	texCoord = vertexTexture;
    gl_Position=WorldProj*WorldView*ModelTr*vertex;
	FragPos = (ModelTr*vertex); 
	normalVec = mat3(NormalTr) * vertexNormal; 
	tanVec = mat3(ModelTr) * vertexTangent; 
}
