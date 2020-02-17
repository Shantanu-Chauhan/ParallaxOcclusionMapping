////////////////////////////////////////////////////////////////////////
// The scene class contains all the parameters needed to define and
// draw a simple scene, including:
//   * Geometry
//   * Light parameters
//   * Material properties
//   * Viewport size parameters
//   * Viewing transformation values
//   * others ...
//
// Some of these parameters are set when the scene is built, and
// others are set by the framework in response to user mouse/keyboard
// interactions.  All of them can be used to draw the scene.

#include "shapes.h"
#include "object.h"
#include "texture.h"
#include"fbo.h"

enum ObjectIds {
	nullId = 0,
	skyId = 1,
	seaId = 2,
	groundId = 3,
	roomId = 4,
	boxId = 5,
	frameId = 6,
	lPicId = 7,
	rPicId = 8,
	teapotId = 9,
	spheresId = 10,
};

class Shader;

struct hammer {
	float Num = 20;
	float hammersley[100] = { 0.0f }; // [2*N]
};
class Scene
{
public:
	Texture* skyDome;
	hammer block;
	// Viewing transformation parameters (suggested) FIXME: This is a
	// good place for the transformation values which are set by the
	// user mouse/keyboard actions and used in DrawScene to create the
	// transformation matrices.
	MAT4 WorldProj, WorldView, WorldInverse;
	vec3 teapotPos, Light, Ambient, AmbientLight;
	ProceduralGround* ground;

	// Light position parameters
	float lightSpin, lightTilt, lightDist;
	Texture* irridianceMap;
	vec3 basePoint;  // Records where the scene building is centered.
	int mode; // Extra mode indicator hooked up to number keys and sent to shader

	// Viewport
	int width, height;

	// All objects in the scene are children of this single root object.
	Object* objectRoot;
	Object* skydome;
	Object* localLights;
	Object* FSQ;
	std::vector<float> BlurFiler(int weight);

	std::vector<Object*> animated;
	std::vector<vec3>lights;
	std::vector<vec3>lightcolor;
	int numberoflights;
	//vec3 lights[1000];
	//vec3 lightcolor[1000];
	// Shader programs
	ShaderProgram* shadowProgram;//shadow
	ShaderProgram* GbufferProgram;
	ShaderProgram* AmbientProgram;
	ShaderProgram* LightingProgram;
	ShaderProgram* LocalLightProgram;
	ShaderProgram* computeShaderProgramVertical;
	ShaderProgram* computeShaderProgramHorizontal;
	ShaderProgram* SkyDomeProgram;

	FBO shadow;
	FBO Gbuffer;
	FBO InterBlur;
	FBO FinalBlur;
	//void append(Object* m) { objects.push_back(m); }
	void InitializeScene();
	void DrawScene();

	unsigned int blockID;
	unsigned int IBLBlockID;
	int GBufferNum;
	void CreateLights();
	//Variables
	unsigned int KernalSize;
	std::vector<float> Filter;
	float maxdepth;
	float alpha;
	float spin;
	float tilt;
	float tx;
	float ty;
	float zoom;
	float ry;
	float front;
	float back;
	float rx;
	float time_since_last_refresh;
	vec3 eye;
	bool w;
	bool a;
	bool s;
	bool d;
	bool eyee;
	int debug;
	bool localLightsToggle;
	bool globalLightToggle;
	int numberOfRows;
	int numberOfColumns;
	float lightHeight;
	int shadowWidth, shadowHeight;
	float contrast;
	float exposure;
	float heightScale;
};