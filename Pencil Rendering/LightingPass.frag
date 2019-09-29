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

out vec4 FragColor;
//in vec4 gl_FragCoord;

uniform float width;
uniform float height;

uniform mat4 WorldInverse,ShadowMatrix;
uniform vec3 lightPos;
uniform vec3 Light;


uniform sampler2D shadowMap;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D Diffuse;

void main()
{    
	vec2 Position=gl_FragCoord.xy;
	Position.x=Position.x/width;
	Position.y=Position.y/height;

	vec4 GPosition=texture(gPosition, Position);
	vec3 N = texture(gNormal, Position).xyz;
    
	vec3 L = normalize(lightPos-GPosition.xyz);//lightpos-gposition?
	
	vec3 eye=(WorldInverse*vec4(0,0,0,1)).xyz;
	vec3 eyeVec=eye-GPosition.xyz;
	vec3 V= normalize(eyeVec);

	vec3 Kd = texture(Diffuse, Position).xyz; 
	vec3 Ks=texture(gAlbedoSpec, Position).xyz;

	vec3 Ii=Light;
	float alpha=texture(gAlbedoSpec, Position).w;
    
	vec3 H = normalize(L+V);
	float LN = max(dot(L,N),0.0);
	float HN = max(dot(H,N),0.0);
	float HNA=max(pow(HN,alpha),0.0);
	float LH=max(dot(L,H),0.0);

	//BRDF
	vec3 F=Ks+(vec3(1.0,1.0,1.0)-Ks)*pow((1.0-LH),5.0);
	float G=1.0/pow(LH,2);
	float D=((alpha+2.0)*pow(HN,alpha))/(2.0*3.14);

	vec3 BRDF=(Kd/3.14)+((F*G*D)/4);
	FragColor.xyz=Ii*LN*BRDF;
	//FragColor.xyz=vec3(1.0,1.0,1.0);
	
	vec4 ShadowCoord;
	ShadowCoord=ShadowMatrix*GPosition;

	if(ShadowCoord.w>0) 
	{
		vec2 shadowIndex = ShadowCoord.xy/ShadowCoord.w;
	if(shadowIndex.x>=0 && shadowIndex.x<=1 && shadowIndex.y>=0 && shadowIndex.y<=1) 
		{
			float lightdepth = texture(shadowMap, shadowIndex).w;
			float pixeldepth = ShadowCoord.w;

			if(pixeldepth > lightdepth+0.008)//(0.008 is the offset)The pixel is in shadow
				FragColor.xyz= vec3(0,0,0);//Only ambient
			else
				FragColor.xyz=Ii*LN*BRDF;//Else full light
		}
	else
		FragColor.xyz=vec3(0,0,0);//Outside only ambient
	}
	else
	FragColor.xyz=vec3(0,0,0);//Outside Only ambient

}  