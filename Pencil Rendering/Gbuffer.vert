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
out mat3 TBN;
out vec3 viewPosition;
out vec3 tanVec;
void main()
{   
	texCoord = vertexTexture;
    gl_Position=WorldProj*WorldView*ModelTr*vertex;
	FragPos = (ModelTr*vertex); 
	normalVec = vertexNormal*mat3(NormalTr); 

	// Calculate tangent space matrix for normal mapping
	mat4 MV = WorldView * ModelTr;
	vec4 P = MV * vec4(vertex.xyz, 1.0);

	vec3 N = normalize(mat3(MV) * vertexNormal);
	vec3 T = normalize(mat3(MV) * vertexTangent);
	vec3 B = normalize(cross(N, T));
	//T =mat3(ModelTr)*vertexTangent; // Proper way to transform a VECTOR

	//TBN = mat3(T, B, N);

	// Get eye direction in tangent space
	vec3 V = -P.xyz;
	viewPosition = normalize(vec3(dot(V,T), dot(V,B), dot(V,N)));
}
