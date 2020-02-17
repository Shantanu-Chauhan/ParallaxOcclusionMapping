/////////////////////////////////////////////////////////////////////////
// Vertex shader for Shadow
////////////////////////////////////////////////////////////////////////
#version 330

uniform mat4 LightProj, LightView, ModelTr;
in vec4 vertex;

out vec4 position;

void main()
{      
    gl_Position = LightProj*LightView*ModelTr*vertex;
    position=gl_Position; 
}
