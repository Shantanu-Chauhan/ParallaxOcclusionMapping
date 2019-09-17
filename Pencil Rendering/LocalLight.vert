/////////////////////////////////////////////////////////////////////////
// Vertex shader for lighting
//
// Copyright 2013 DigiPen Institute of Technology
////////////////////////////////////////////////////////////////////////
#version 330
uniform mat4 WorldView, WorldProj,WorldInverse, ModelTr;
in vec4 vertex;
//uniform vec3 lightPos;
//out vec3 lightVec;
//out vec3 eyeVec;
//out vec3 worldPos;
//out vec3 eyepos;

void main()
{   
    gl_Position = WorldProj*WorldView*ModelTr*vertex;
	//vec3 eyePos=(WorldInverse*vec4(0,0,0,1)).xyz;
	//worldPos = (ModelTr*vertex).xyz;
	//lightVec=lightPos-worldPos;
	//eyeVec =eyepos-worldPos;

}
