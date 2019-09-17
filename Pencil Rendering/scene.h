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
    nullId	= 0,
    skyId	= 1,
    seaId	= 2,
    groundId	= 3,
    roomId	= 4,
    boxId	= 5,
    frameId	= 6,
    lPicId	= 7,
    rPicId	= 8,
    teapotId	= 9,
    spheresId	= 10,
};

class Shader;

class Scene
{
public:
    // Viewing transformation parameters (suggested) FIXME: This is a
    // good place for the transformation values which are set by the
    // user mouse/keyboard actions and used in DrawScene to create the
    // transformation matrices.

    ProceduralGround* ground;

    // Light position parameters
    float lightSpin, lightTilt, lightDist;

    vec3 basePoint;  // Records where the scene building is centered.
    int mode; // Extra mode indicator hooked up to number keys and sent to shader
    
    // Viewport
    int width, height;

    // All objects in the scene are children of this single root object.
    Object* objectRoot;
	Object* localLights;
	Object* FSQ;
    std::vector<Object*> animated;
	std::vector<vec3>lights;
	std::vector<vec3>lightcolor;
	int numberoflights;
	//vec3 lights[1000];
	//vec3 lightcolor[1000];
    // Shader programs
    ShaderProgram* lightingProgram;//lighting
	ShaderProgram* shadowProgram;//shadow
	ShaderProgram* reflectionProgram1;
	ShaderProgram* GbufferProgram;
	ShaderProgram* AmbientProgram;
	ShaderProgram* LightingProgram;
	ShaderProgram* LocalLightProgram;
	FBO shadow;
	FBO reflection1;
	FBO reflection2;
	FBO Gbuffer;
    //void append(Object* m) { objects.push_back(m); }
    void InitializeScene();
    void DrawScene();

	//Variables
	 float spin;
	 float tilt;
	 float tx ;
	 float ty ;
	 float zoom;
	 float ry ;
	 float front;
	 float back;
	 float rx;
	 int time_since_last_refresh;
	 vec3 eye;
	 bool w;
	 bool a;
	 bool s;
	 bool d;
	 bool eyee;
	 int debug;
};
