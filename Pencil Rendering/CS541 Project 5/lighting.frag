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
uniform sampler2D TOPReflection;//sampler
uniform sampler2D BOTReflection;//sampler
uniform sampler2D NORMAL;
uniform sampler2D TEXTURE;


uniform int objectId;
uniform vec3 diffuse;//Kd
uniform vec3 Light;//Ii 
uniform vec3 Ambient;//Ia

uniform vec3 specular;//Ks
uniform float shininess;//alpha exponent

vec3 LightingCalc();
void main()
{
	vec3 light=LightingCalc();
	gl_FragColor.xyz=light;
	if(objectId==teapotId)
	{
		vec3 N = normalize(normalVec);
		vec3 V= normalize(eyeVec);
		vec3 R= 2*(dot(V,N)*N)-V;
		R=R/length(R);
		vec2 uv;
		if(R.z>0)
		{
			uv=vec2((R.x/(1+R.z)),(R.y/(1+R.z)));
			uv=uv*vec2(0.5,0.5);
			uv=uv+vec2(0.5,0.5);
			gl_FragColor.xyz+=0.1*texture(TOPReflection, uv).xyz;//THIS CAN BE ENABLED TO GET THE REFLECTION PLUS COLOR
			//gl_FragColor.xyz=texture(TOPReflection, uv).xyz;
		}
		else
		{
			uv=vec2((R.x/(1-R.z)),(R.y/(1-R.z)));
			uv=uv*vec2(0.5,0.5);
			uv=uv+vec2(0.5,0.5);
			gl_FragColor.xyz+=0.1*texture(BOTReflection, uv).xyz;//THIS CAN BE ENABLED TO GET THE REFLECTION PLUS COLOR
			//gl_FragColor.xyz=texture(BOTReflection, uv).xyz;
		}
	}
}
