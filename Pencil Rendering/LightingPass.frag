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
uniform float alphaaa;
uniform float maxdepth;
uniform vec3 Light;

uniform int GBufferNum;

uniform sampler2D shadowMap;
uniform sampler2D blurShadowMap;
uniform sampler2D gPosition;
uniform sampler2D gNormal;
uniform sampler2D gAlbedoSpec;
uniform sampler2D Diffuse;

vec3 Cholesky(float m11, float m12, float m13, float m22, float m23, float m33, float z1, float z2, float z3)
{
    float a = sqrt(m11);
    float b = m12 / a;
    float c = m13 / a;
    float d = sqrt(m22 - (b*b));
    float e = (m23 - b*c) / d;
    float f = sqrt(m33 - c*c - e*e);

    float c1p = z1 / a;
    float c2p = (z2 - b*c1p) / d;
    float c3p = (z3 - c*c1p - e*c2p) / f;

    float c3 = c3p / f;
    float c2 = (c2p - e*c3) / d;
    float c1 = (c1p - b*c2 - c*c3) / a;

    return vec3(c1,c2,c3);
}

void main()
{   

vec2 Position=gl_FragCoord.xy;
	Position.x=Position.x/width;
	Position.y=Position.y/height;

	vec4 GPosition=texture(gPosition, Position);
	vec3 N = texture(gNormal, Position).xyz;
	vec3 Kd = texture(Diffuse, Position).xyz; 
	vec3 Ks=texture(gAlbedoSpec, Position).xyz;
float blur;
if(GBufferNum == 0)
{
	
	vec3 L = normalize(lightPos-GPosition.xyz);//lightpos-gposition?
	
	vec3 eye=(WorldInverse*vec4(0,0,0,1)).xyz;
	vec3 eyeVec=eye-GPosition.xyz;
	vec3 V= normalize(eyeVec);


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
	//FragColor.xyz=Ii*LN*BRDF;
	//FragColor.xyz=vec3(1.0,1.0,1.0);
	
	vec4 ShadowCoord;
	ShadowCoord=ShadowMatrix*vec4(GPosition.xyz,1.0);

	if(ShadowCoord.w>0) 
	{
		vec2 shadowIndex = ShadowCoord.xy/ShadowCoord.w;
	if(shadowIndex.x>=0 && shadowIndex.x<=1 && shadowIndex.y>=0 && shadowIndex.y<=1) 
		{
			vec4 lightdepth = texture(blurShadowMap, shadowIndex); //b
			float pixeldepth = ShadowCoord.w;   //zf
	
			vec4 bprime = (1 - alphaaa)* lightdepth + (alphaaa)*vec4(maxdepth);
	
			vec3 c = Cholesky(1.0,bprime.x,bprime.y,bprime.y,bprime.z,bprime.w,1.0,pixeldepth,pixeldepth*pixeldepth);
	
			float b4ac = sqrt((c.y*c.y) - (4 * c.x * c.z));
	
			float z2 = (-c.y + b4ac ) / ( 2 * c.z);
			float z3 = (-c.y - b4ac ) / ( 2 * c.z);
	
			if(z2>z3)
			{
				float tempz = z3;
				z3 = z2;
				z2 = tempz;
			}
	
			if(pixeldepth<=z2)
			{
				blur = 0.0;
			}
			else if(pixeldepth<=z3)
			{
				float A,B,C,D1;
				A = pixeldepth * z3;
				B = -bprime.x*(pixeldepth + z3) + bprime.y;
				C = z3 - z2;
				D1 = pixeldepth - z2;
				blur = (A + B)/ (C * D1);
			}
			else
			{
				float A,B,C,D1,E;
				A = z2 * z3;
				B = -bprime.x*(z2 + z3) + bprime.y;
				C = pixeldepth - z2;
				D1 = pixeldepth - z3;
				blur = 1.0 - ((A + B) / (C * D1));	
			}
		}
	else
		blur = 1.0;
	}
	else
	blur = 1.0;
	
	FragColor=vec4((1-blur)*Ii*LN*BRDF,1.0);//Else full light;
}
	else 
	if(GBufferNum == 1)
	{
		FragColor.xyz = GPosition.xyz;
	}
	else if(GBufferNum == 2)
	{
		FragColor.xyz = Ks;
	}
	else
	if(GBufferNum == 3)
	{
		FragColor.xyz = Kd;
	}
	else
	if(GBufferNum == 4)
	{
		FragColor.xyz = N;
	}
	else
	if(GBufferNum == 5)
	{
		FragColor.xyz = vec3(texture(shadowMap , Position).x/1000.0);
	}
	else
	if(GBufferNum == 6)
	{
		FragColor.xyz = vec3(texture(blurShadowMap , Position).x/1000.0);
	}

}  