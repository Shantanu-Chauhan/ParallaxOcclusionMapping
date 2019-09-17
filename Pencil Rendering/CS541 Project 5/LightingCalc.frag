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

in vec3 normalVec, lightVec, eyeVec;
in vec2 texCoord;
in vec4 ShadowCoord;
in vec3 tanVec;

uniform sampler2D shadowMap;//SAMPLER
uniform sampler2D NORMAL;
uniform sampler2D TEXTURE;

uniform int objectId;
uniform vec3 diffuse;//Kd
uniform vec3 Light;//Ii 
uniform vec3 Ambient;//Ia

uniform vec3 specular;//Ks
uniform float shininess;//alpha exponent


vec3 LightingCalc()
{
    vec3 N = normalize(normalVec);
    vec3 L = normalize(lightVec);
	vec3 V= normalize(eyeVec);

    vec3 Kd = diffuse;   
	if(objectId==teapotId)
	{
		Kd=texture(TEXTURE,texCoord).xyz;
	}
	else
	if(objectId==boxId)
	{
		vec3 delta = texture(NORMAL, texCoord).xyz;		vec2 uv;
		delta=delta*2.0 - vec3(1.0,1.0,1.0);		vec3 T = normalize(tanVec);		vec3 B= normalize(cross(T,N));
		N = delta.x*T + delta.y*B + delta.z*N;
		Kd=texture(TEXTURE,texCoord).xyz;
	}
	else
	if(objectId==roomId)
	{
		vec3 V= normalize(eyeVec);
		vec2 New=vec2(texCoord.y*30,-texCoord.x*15);//to flip the texture
		New=fract(New);								//to flip the texture
		vec3 delta = texture(NORMAL, New).xyz;		//to flip the texture		vec2 uv;
		delta=delta*2.0 - vec3(1.0,1.0,1.0);		vec3 T = normalize(tanVec);		vec3 B= normalize(cross(T,N));
		N = delta.x*T + delta.y*B + delta.z*N;
		Kd=texture(TEXTURE,New).xyz;
	}
	else
	if(objectId==groundId)
	{
		vec2 New=texCoord*50;
		New=fract(New);
		Kd=texture(TEXTURE,New).xyz;
	}
	else 
	if(objectId==seaId)
	{
		vec3 V= normalize(eyeVec);
		vec2 New=vec2(texCoord.y*500,texCoord.x*500);
		New=fract(New);
		vec3 delta = texture(NORMAL, New).xyz;		vec2 uv;
		delta=delta*2.0 - vec3(1.0,1.0,1.0);		vec3 T = normalize(tanVec);		vec3 B= normalize(cross(T,N));
		N = delta.x*T + delta.y*B + delta.z*N;
		
		vec3 R= vec3(- ( 2*dot(V,N)*N - V) ) ;
		uv=vec2(1/2 - atan(R.y,R.x)/(2*3.14), acos(R.z)/3.14); 
		return texture(TEXTURE, uv).xyz;
	}
	else
	if(objectId==skyId)
	{
		vec3 V= normalize(eyeVec);
		vec2 uv;
		uv=vec2(1/2 - atan(V.y,V.x)/(2*3.14), acos(V.z)/3.14);
		return texture(TEXTURE, uv).xyz;
	}
	else 
	if(objectId== rPicId)
	{
		ivec2 uv = ivec2(floor(10.0*texCoord));
		if (uv[0]==9||uv[1]==9||uv[0]==0||uv[1]==0 )
			Kd=vec3(0.5,0.5,0.5);
		else
		{
			vec2 New=texCoord*1.2;
			New=New-vec2(0.1);
			Kd=texture(TEXTURE,New).xyz;
		}
	}
	if (objectId==lPicId) 
	{
        ivec2 uv = ivec2(floor(8.0*texCoord));
        if ((uv[0]+uv[1])%2==0)
            Kd = vec3(1.0);
			else
			Kd*=0.0;
	}

    vec3 Ks=specular;
	vec3 Ia=Ambient;
	vec3 Ii=Light;
	float alpha=shininess;
    
	vec3 H = normalize(L+V);
	float LN = max(dot(L,N),0.0);
	float HN = max(dot(H,N),0.0);
	float HNA=max(pow(HN,alpha),0.0);
	float LH=max(dot(L,H),0.0);

	vec3 temp=(Ia*Kd)+(Ii*Kd)*LN+Ii*Ks*HNA;
	 
	 //gl_FragColor.xyz=temp.xyz;//This is Phong

	 //BRDF
	 vec3 F=Ks+(vec3(1.0,1.0,1.0)-Ks)*pow((1.0-LH),5.0);
	 float G=1.0/pow(LH,2);
	 float D=((alpha+2.0)*pow(HN,alpha))/(2.0*3.14);

	 vec3 BRDF=(Kd/3.14)+((F*G*D)/4);

	 //gl_FragColor.xyz=Ia*Kd+Ii*LN*BRDF;
	 //BRDF

    //gl_FragColor.xyz = vec3(0.5,0.5,0.5)*Kd + Kd*max(dot(L,N),0.0);//Crippled
	if(ShadowCoord.w>0) 
	{
		vec2 shadowIndex = ShadowCoord.xy/ShadowCoord.w;
	if(shadowIndex.x>=0 && shadowIndex.x<=1 && shadowIndex.y>=0 && shadowIndex.y<=1) 
		{
			float lightdepth = texture(shadowMap, shadowIndex).w;
			float pixeldepth = ShadowCoord.w;

			if(pixeldepth > lightdepth+0.008)//(0.008 is the offset)The pixel is in shadow
				return Ia*Kd;//Only ambient
			else
				return Ia*Kd+Ii*LN*BRDF;//Else full light
		}
	else
		return Ia*Kd;//Outside only ambient
	}
	else
	return Ia*Kd;//Outside Only ambient
	//All outside only ambient will create the effect of rectangular lighting of the screen
}
