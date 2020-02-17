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

uniform mat4 WorldInverse;

uniform vec3 Ambient;
uniform vec3 view;
uniform float width;
uniform float height;
out vec4 FragColor;
uniform sampler2D Diffuse;
uniform sampler2D gPosition;
uniform sampler2D irridianceMap;
uniform sampler2D gAlbedoSpec;
uniform sampler2D Normal;
uniform sampler2D skyDome;
uniform float exposure;
uniform float contrast;



uniform HammersleyBlock {
 float Num;
 float hammersley[100]; };


vec3 irridiance(vec3 normal)
{
	vec2 test;
	vec3 color;
	test = vec2(0.5 - (atan(normal.y,normal.x)/ (2 *3.14)) ,acos(normal.z)/3.14 );
	color = texture(irridianceMap , test).xyz;
	return color;
}

vec3 Specular(vec3 N,vec3 V , vec3 Ks , float alpha)
{	
	vec3 R = ( 2.0*(dot(N, V))*N) - V;
	vec3 A = normalize(vec3(-R.y, R.x, 0.0));
	vec3 B = normalize(cross(R, A));

	float WIDTH;
	float HEIGHT;

	ivec2 skydomesize = textureSize(skyDome,0);
	WIDTH = skydomesize.x;
	HEIGHT= skydomesize.y;

	vec3 specular = vec3(0.0, 0.0, 0.0);
	for(int i = 0; i < int(Num)*2; i += 2)
	{
		float u = hammersley[i];
		float v = acos(pow(hammersley[i+1], ( 1.0/ (alpha + 1.0) ) ) ) /3.14;

		vec3 L = vec3(cos(2.0*3.14*(0.5-u))*sin(3.14*v), sin(2.0*3.14*(0.5-u))*sin(3.14*v), cos(3.14*v));

		vec3 wk = normalize(L.x*A + L.y*B + L.z*R);

		vec3 H = normalize(wk+V);
		float NH = max(dot(N,H),0.0);
		//float NH =dot(N,H);

		float DH = (alpha + 2.0)*pow(NH,alpha) / (2 * 3.14);
		float level = 0.5*log2((WIDTH*HEIGHT) / Num) - 0.5*log2(DH);

		vec2 uv = vec2(0.5 - ((atan(wk.y, wk.x))/(2.0*3.14)), acos(-wk.z)/3.14);
		vec3 Liwk = textureLod(skyDome, uv, level).xyz;
	
		float G;
		vec3 F;
		float WH = max(dot(wk,H),0.0);
		//float WH = dot(wk,H);
		G = 1.0/pow(WH,2);
		//G = 1.0;
		F = Ks + (1-Ks) * pow(1-WH,5);
		specular += (1.0/Num) * G * (F /4.0) * Liwk * max(dot(N, wk), 0.0);
	}
	return specular;
}

void main()
{
	vec2 Position=gl_FragCoord.xy;
	Position.x=Position.x/width;
	Position.y=Position.y/height;
	
	vec3 gPos = texture(gPosition, Position).xyz;
	vec3 Kd = texture(Diffuse, Position).xyz; 
	vec3 normal = texture(Normal,Position).xyz;

	vec3 Ks = texture(gAlbedoSpec, Position).xyz; //Specular color
	float alpha = texture(gAlbedoSpec, Position).w; //shininess

	vec3 N = normalize(normal);
	vec3 eyePos = (WorldInverse * vec4(0.0,0.0,0.0,1.0)).xyz;
	vec3 V = normalize(eyePos - gPos);

	vec3 diffusePart = irridiance(normal)*Kd/3.14;
	
	vec3 specularPart = Specular(N,V,Ks,alpha);

	vec3 color = diffusePart + specularPart;
	//vec3 color = specularPart;
	//vec3 color = diffusePart;
	color = pow(  exposure*color / ( exposure * color + vec3(1.0,1.0,1.0) ) , vec3(contrast/2.2)  );
	FragColor = vec4(color,1.0);
}  