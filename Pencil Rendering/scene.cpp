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

#include <glew.h>
#include <glfw/glfw3.h>
#include "math.h"
#include <fstream>
#include <stdlib.h>
#include <time.h>

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "shapes.h"
#include "scene.h"
#include "object.h"
#include "texture.h"
#include "transform.h"


const float PI = 3.14159f;
const float rad = PI / 180.0f;    // Convert degrees to radians

MAT4 Identity;

const float grndSize = 100.0;    // Island radius;  Minimum about 20;  Maximum 1000 or so
const int   grndTiles = int(grndSize);
const float grndOctaves = 4.0;  // Number of levels of detail to compute
const float grndFreq = 0.03;    // Number of hills per (approx) 50m
const float grndPersistence = 0.03; // Terrain roughness: Slight:0.01  rough:0.05
const float grndLow = -3.0;         // Lowest extent below sea level
const float grndHigh = 5.0;        // Highest extent above sea level



////////////////////////////////////////////////////////////////////////
// This macro makes it easy to sprinkle checks for OpenGL errors
// throughout your code.  Most OpenGL calls can record errors, and a
// careful programmer will check the error status *often*, perhaps as
// often as after every OpenGL call.  At the very least, once per
// refresh will tell you if something is going wrong.
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line scene.cpp:%d): %s\n", __LINE__); exit(-1);} }

// Create an RGB color from human friendly parameters: hue, saturation, value
vec3 HSV2RGB(const float h, const float s, const float v)
{
	if (s == 0.0)
		return vec3(v, v, v);

	int i = (int)(h * 6.0) % 6;
	float f = (h * 6.0f) - i;
	float p = v * (1.0f - s);
	float q = v * (1.0f - s * f);
	float t = v * (1.0f - s * (1.0f - f));
	if (i == 0)     return vec3(v, t, p);
	else if (i == 1)  return vec3(q, v, p);
	else if (i == 2)  return vec3(p, v, t);
	else if (i == 3)  return vec3(p, q, v);
	else if (i == 4)  return vec3(t, p, v);
	else   /*i == 5*/ return vec3(v, p, q);
}

////////////////////////////////////////////////////////////////////////
// Constructs a hemisphere of spheres of varying hues
Object* SphereOfSpheres(Shape* SpherePolygons)
{
	Object* ob = new Object(NULL, nullId);

	for (float angle = 0.0; angle < 360.0; angle += 18.0)
		for (float row = 0.075; row < PI / 2.0; row += PI / 2.0 / 6.0) {
			vec3 hue = HSV2RGB(angle / 360.0, 1.0f - 2.0f * row / PI, 1.0f);

			Object* sp = new Object(SpherePolygons, spheresId,
				hue, vec3(1.0, 1.0, 1.0), 120.0);
			float s = sin(row);
			float c = cos(row);
			ob->add(sp, Rotate(2, angle) * Translate(c, 0, s) * Scale(0.075 * c, 0.075 * c, 0.075 * c));
		}
	return ob;
}

////////////////////////////////////////////////////////////////////////
// Constructs a -1...+1  quad (canvas) framed by four (elongated) boxes
Object* FramedPicture(const MAT4& modelTr, const int objectId,
	Shape* BoxPolygons, Shape* QuadPolygons)
{
	// This draws the frame as four (elongated) boxes of size +-1.0
	float w = 0.05;             // Width of frame boards.

	Object* frame = new Object(NULL, nullId);
	Object* ob;

	vec3 woodColor(87.0 / 255.0, 51.0 / 255.0, 35.0 / 255.0);
	ob = new Object(BoxPolygons, frameId,
		woodColor, vec3(0.2, 0.2, 0.2), 10.0);
	frame->add(ob, Translate(0.0, 0.0, 1.0 + w) * Scale(1.0, w, w));
	frame->add(ob, Translate(0.0, 0.0, -1.0 - w) * Scale(1.0, w, w));
	frame->add(ob, Translate(1.0 + w, 0.0, 0.0) * Scale(w, w, 1.0 + 2 * w));
	frame->add(ob, Translate(-1.0 - w, 0.0, 0.0) * Scale(w, w, 1.0 + 2 * w));

	ob = new Object(QuadPolygons, objectId,
		woodColor, vec3(0.0, 0.0, 0.0), 10.0);
	frame->add(ob, Rotate(0, 90));

	return frame;
}

////////////////////////////////////////////////////////////////////////
// A callback procedure, called regularly to update the atime global
// variable.
float atime = 0.0;
void animate(int speed)
{
	atime = 360.0 * glfwGetTime() / speed;
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as setting a
// number of other parameters.


//Used to initialize the local lights position, color and number in the scene.
void Scene::CreateLights()
{
	srand(time(nullptr));
	float x, y, z, r, g, b = 0.0f;
	if (!lights.empty())
	{
		lights.clear();
		lightcolor.clear();
	}
	for (int i = 0; i < numberOfRows; i++)
	{
		for (int j = 0; j < numberOfColumns; j++)
		{
			x = i - 20;
			y = j - 20;
			z = lightHeight;
			//r = g = b = 1.0f;
			r = (rand() % 1000) / 100;
			g = (rand() % 1000) / 100;
			b = (rand() % 1000) / 100;
			lights.push_back(vec3(x, y, z));
			lightcolor.push_back(vec3(r, g, b));
		}
	}
	lights.push_back(vec3(0.0f, 0.0f, 20.0f));
	lightcolor.push_back(vec3(1.0f, 0.0f, 0.0f));

}
void Scene::InitializeScene()
{
	KernalSize = 2;
	Filter = BlurFiler(KernalSize);
	shadowWidth = 2048;
	shadowHeight = 2048;
	glBlendFunc(GL_ONE, GL_ONE);
	localLightsToggle = true;
	globalLightToggle = true;
	lightHeight = 1.0f;
	GBufferNum = 0;
	glEnable(GL_DEPTH_TEST);
	CHECKERROR;
	maxdepth = 0.5f;
	alpha = 0.088f;
	// FIXME: This is a good place for initializing the transformation
	// values.
	debug = 0;
	spin = 0.0f;
	tilt = 0.0f;
	tx = 0.0f;
	ty = 0.0f;
	zoom = 150.0f;
	ry = 0.2f;
	front = 0.1f;
	back = 1000.0;
	eye = vec3(0.0f, -10.0f, 0.0f);
	time_since_last_refresh = 0.0f;
	eyee = true;
	w = false;
	a = false;
	s = false;
	d = false;
	teapotPos = glm::vec3(0.2, 0.0, 1.5);
	Light = vec3(3.0, 3.0, 3.0);
	AmbientLight = vec3(0.2, 0.2, 0.2);
	CHECKERROR;
	objectRoot = new Object(nullptr, nullId);
	skydome = new Object(nullptr, nullId);
	numberoflights = 2000;
	numberOfRows = 50;
	numberOfColumns = 40;
	CreateLights();
	// Set the initial light position parammeters
	lightSpin = 150.0;
	lightTilt = -45.0;
	lightDist = 200.0f;

	// Enable OpenGL depth-testing
	glEnable(GL_DEPTH_TEST);

	// FIXME:  Change false to true to randomize the central object position.
	ground = new ProceduralGround(grndSize, grndTiles, grndOctaves, grndFreq,
		grndPersistence, grndLow, grndHigh, false);

	basePoint = ground->highPoint;

	//Creating Computer shader program
	computeShaderProgramHorizontal = new ShaderProgram();
	computeShaderProgramHorizontal->AddShader("ComputeShaderHorizontal.comp", GL_COMPUTE_SHADER);
	computeShaderProgramHorizontal->LinkProgram();
	//Creating Computer shader program

	//Creating Computer shader program
	computeShaderProgramVertical = new ShaderProgram();
	computeShaderProgramVertical->AddShader("ComputeShaderVertical.comp", GL_COMPUTE_SHADER);
	computeShaderProgramVertical->LinkProgram();
	//Creating Computer shader program

	LightingProgram = new ShaderProgram();
	LightingProgram->AddShader("LightingPass.vert", GL_VERTEX_SHADER);
	LightingProgram->AddShader("LightingPass.frag", GL_FRAGMENT_SHADER);
	LightingProgram->LinkProgram();

	//Creating Shadow Shader program
	shadowProgram = new ShaderProgram();
	shadowProgram->AddShader("shadow.vert", GL_VERTEX_SHADER);
	shadowProgram->AddShader("shadow.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(shadowProgram->programId, 0, "vertex");
	shadowProgram->LinkProgram();
	//Creating Shadow Shader program

	//Creating Gbuffer Shader program	
	GbufferProgram = new ShaderProgram();
	GbufferProgram->AddShader("Gbuffer.vert", GL_VERTEX_SHADER);
	GbufferProgram->AddShader("Gbuffer.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(GbufferProgram->programId, 0, "vertex");
	glBindAttribLocation(GbufferProgram->programId, 1, "vertexNormal");
	GbufferProgram->LinkProgram();

	AmbientProgram = new ShaderProgram();
	AmbientProgram->AddShader("Ambient.vert", GL_VERTEX_SHADER);
	AmbientProgram->AddShader("Ambient.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(AmbientProgram->programId, 0, "vertex");
	AmbientProgram->LinkProgram();


	SkyDomeProgram = new ShaderProgram();
	SkyDomeProgram->AddShader("SkyDome.vert", GL_VERTEX_SHADER);
	SkyDomeProgram->AddShader("SkyDome.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(SkyDomeProgram->programId, 0, "vertex");
	SkyDomeProgram->LinkProgram();


	LocalLightProgram = new ShaderProgram();
	LocalLightProgram->AddShader("LocalLight.vert", GL_VERTEX_SHADER);
	LocalLightProgram->AddShader("LocalLight.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(LocalLightProgram->programId, 0, "vertex");
	LocalLightProgram->LinkProgram();

	// Create all the Polygon shapes
	Shape* TeapotPolygons = new Teapot(12);
	Shape* BoxPolygons = new Ply("box.ply");
	Shape* SpherePolygons = new Sphere(32);
	Shape* RoomPolygons = new Ply("room.ply");
	Shape* GroundPolygons = ground;
	Shape* QuadPolygons = new Quad();
	Shape* SeaPolygons = new Plane(2000.0, 50);

	// Various colors used in the subsequent models
	vec3 woodColor(87.0 / 255.0, 51.0 / 255.0, 35.0 / 255.0);
	vec3 brickColor(134.0 / 255.0, 60.0 / 255.0, 56.0 / 255.0);
	vec3 brassColor(0.5, 0.5, 0.1);
	vec3 grassColor(62.0 / 255.0, 102.0 / 255.0, 38.0 / 255.0);
	vec3 waterColor(0.3, 0.3, 1.0);

	// Creates all the models from which the scene is composed.  Each
	// is created with a polygon shape (possibly NULL), a
	// transformation, and the surface lighting parameters Kd, Ks, and
	// alpha.
	Object* central = new Object(NULL, nullId);
	Object* anim = new Object(NULL, nullId);
	Object* room = new Object(RoomPolygons, roomId, brickColor, vec3(0.0, 0.0, 0.0), 1);
	Object* teapot = new Object(TeapotPolygons, teapotId, brassColor, vec3(0.5, 0.5, 0.5), 120);
	Object* podium = new Object(BoxPolygons, boxId, vec3(woodColor), vec3(0.5, 0.5, 0.5), 10);
	Object* sky = new Object(SpherePolygons, skyId, vec3(), vec3(), 0);
	Object* ground = new Object(GroundPolygons, groundId, grassColor, vec3(0.0, 0.0, 0.0), 1);
	Object* sea = new Object(SeaPolygons, seaId, waterColor, vec3(0.1, 0.1, 0.1), 120);
	Object* spheres = SphereOfSpheres(SpherePolygons);
	Object* leftFrame = FramedPicture(Identity, lPicId, BoxPolygons, QuadPolygons);
	Object* rightFrame = FramedPicture(Identity, rPicId, BoxPolygons, QuadPolygons);
	//Object* sphere = new Object(SpherePolygons, nullId, vec3(), vec3(), 0);

	FSQ = new Object(QuadPolygons, nullId);
	localLights = new Object(SpherePolygons, nullId);


	// FIXME: This is where you could read in all the textures and
	// associate them with the various objects just created above.
	Texture* skyDome = new Texture("textures/Ocean.png");
	sky->TextureId = skyDome->textureId;

	Texture* seaNormal = new Texture("textures/ripples_normalmap.png");
	sea->TextureId = skyDome->textureId;
	sea->NormalId = seaNormal->textureId;

	Texture* teaPot = new Texture("textures/cracks.png");
	teapot->TextureId = teaPot->textureId;

	Texture* box = new Texture("textures/box.png");
	Texture* boxNormal = new Texture("textures/Brazilian_rosewood_pxr128_normal.png");
	podium->TextureId = box->textureId;
	podium->NormalId = boxNormal->textureId;

	Texture* walls = new Texture("textures/brick.png");
	Texture* wallNormal = new Texture("textures/Standard_red_pxr128_normal.png");
	room->TextureId = walls->textureId;
	room->NormalId = wallNormal->textureId;

	Texture* grass = new Texture("textures/grass.jpg");
	ground->TextureId = grass->textureId;

	Texture* rightPic = new Texture("textures/my-house-01.png");
	rightFrame->TextureId = rightPic->textureId;

	// Scene is composed of sky, ground, sea, room and some central models
	objectRoot->add(ground);
	objectRoot->add(room, Translate(basePoint.x, basePoint.y, basePoint.z));// REMOVE THIS FOR REFLECTIONS
	objectRoot->add(sea);
	objectRoot->add(central);
	skydome->add(sky, Scale(2000.0, 2000.0, 2000.0));
	//localLights->add(sphere, Scale(2.0, 2.0, 2.0));


	// Central model has a rudimentary animation (constant rotation on Z)
	animated.push_back(anim);

	// Central contains a teapot on a podium and an external sphere of spheres
	central->add(podium, Translate(0.0, 0, 0));
	central->add(anim, Translate(0.0, 0, 0));
	anim->add(teapot, Translate(0.1, 0.0, 1.5) * TeapotPolygons->modelTr);
	anim->add(spheres, Translate(0.0, 0.0, 0.0) * Scale(30.0, 30.0, 30.0));

	// Room contains two framed pictures
	room->add(leftFrame, Translate(-1.5, 9.85, 1.) * Scale(0.8, 0.8, 0.8));
	room->add(rightFrame, Translate(1.5, 9.85, 1.) * Scale(0.8, 0.8, 0.8));

	shadow.CreateFBO(shadowWidth, shadowHeight);
	Gbuffer.CreateG(width, height);
	InterBlur.CreateFBO(shadowWidth, shadowHeight);
	FinalBlur.CreateFBO(shadowWidth, shadowHeight);
	glGenBuffers(1, &blockID); // Generates block
	CHECKERROR;
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be
// drawn. (Which is often: 30 to 60 times per second are the common
// goals.)
void Scene::DrawScene()
{
	const float start = glfwGetTime() / 10000.0f;

	// Calculate the light's position.
	const float lPos[4] = {
	   basePoint.x + lightDist * cos(lightSpin * rad) * sin(lightTilt * rad),
	   basePoint.y + lightDist * sin(lightSpin * rad) * sin(lightTilt * rad),
	   basePoint.z + lightDist * cos(lightTilt * rad),
	   1.0 };
	// Set the viewport, and clear the screen

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Draw all objects (hierarchically down through each object's child list).

	for (std::vector<Object*>::iterator m = animated.begin(); m < animated.end(); m++)
		(*m)->animTr = Rotate(2, atime);

	// Compute Viewing and Perspective transformations.
	const float speed = 5.0f;
	const float step = speed * time_since_last_refresh;
	//printf("Start %f\n", time_since_last_refresh);
	rx = ry * ((float)width / (float)height);
	// Compute any continuously animating objects
	if (eyee)
	{
		if (w)
		{
			eye += step * vec3(sin(spin * PI / 180.0f), cos(spin * PI / 180.0f), 0.0);
		}
		if (s)
		{
			eye -= step * vec3(sin(spin * PI / 180.0f), cos(spin * PI / 180.0f), 0.0);
		}
		if (d)
		{
			eye += step * vec3(cos(spin * PI / 180.0f), -sin(spin * PI / 180.0f), 0.0);
		}
		if (a)
		{
			eye -= step * vec3(cos(spin * PI / 180.0f), -sin(spin * PI / 180.0f), 0.0);
		}
		eye.z = ground->HeightAt(eye.x, eye.y) + 2.0f;
		WorldView = Rotate(0, tilt - 90.0f) * Rotate(2, spin) * Translate(-eye.x, -eye.y, -eye.z);
	}
	else
		WorldView = Translate(tx, ty, -zoom) * Rotate(0, tilt - 90.0f) * Rotate(2, spin);

	WorldProj = Perspective(rx, ry, front, back);
	invert(&WorldView, &WorldInverse);

	//---------------------------------------------------      GBUFFER PROGRAM

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	GbufferProgram->Use();

	Gbuffer.Bind();

	glViewport(0, 0, width, height);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int programId = GbufferProgram->programId;

	int loc = glGetUniformLocation(programId, "WorldProj");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(programId, "GBufferNum");
	glUniform1i(loc, GBufferNum);
	objectRoot->Draw(GbufferProgram, Identity);

	Gbuffer.Unbind();

	GbufferProgram->Unuse();
	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);

	//---------------------------------------------------      GBUFFER PROGRAM

	//---------------------------------------------------      AMBIENT PROGRAM
	if(GBufferNum == 0){
		AmbientProgram->Use();
		programId = AmbientProgram->programId;

		loc = glGetUniformLocation(programId, "Ambient");
		glUniform3fv(loc, 1, &(AmbientLight[0]));

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.Diffuse);

		loc = glGetUniformLocation(programId, "Diffuse");
		glUniform1i(loc, 0);

		loc = glGetUniformLocation(programId, "width");
		glUniform1f(loc, width);

		loc = glGetUniformLocation(programId, "height");
		glUniform1f(loc, height);

		CHECKERROR;

		FSQ->Draw(AmbientProgram, Identity);

		CHECKERROR;
		AmbientProgram->Unuse();
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);
		//---------------------------------------------------     AMBIENT PROGRAM
	}

	//---------------------------------------------------     SHADOW PROGRAM

	const glm::vec3 Lightpos = glm::vec3(lPos[0], lPos[1], lPos[2]);
	MAT4 LightView = LookAt(Lightpos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	float nry;
	nry = 40.0 / lightDist;
	MAT4 LightProj = Perspective(nry, nry, front, back);

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);

	shadowProgram->Use();
	shadow.Bind();
	glViewport(0, 0, shadow.width, shadow.height);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	programId = shadowProgram->programId;


	loc = glGetUniformLocation(programId, "LightProj");
	glUniformMatrix4fv(loc, 1, GL_TRUE, LightProj.Pntr());
	loc = glGetUniformLocation(programId, "LightView");
	glUniformMatrix4fv(loc, 1, GL_TRUE, LightView.Pntr());

	//glEnable(GL_CULL_FACE);
	//glCullFace(GL_FRONT);
	objectRoot->Draw(shadowProgram, Identity);
	//glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);
	shadow.Unbind();
	CHECKERROR;
	shadowProgram->Unuse();
	CHECKERROR;


	////---------------------------------------------------     SHADOW PROGRAM

	////---------------------------------------------------     Horizontal SHADOW BLUR


	
	{
		//InterBlur.Bind();
		computeShaderProgramHorizontal->Use();
		programId = computeShaderProgramHorizontal->programId;
		int bindpoint = 0; // Start at zero, increment for other blocks
		loc = glGetUniformBlockIndex(programId, "blurKernel");
		glUniformBlockBinding(programId, loc, bindpoint);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
		glBufferData(GL_UNIFORM_BUFFER, Filter.size() * sizeof(float), &Filter[0], GL_STATIC_DRAW);//make this a single call till filter is changed		//a texture to the shader as an image2D	
		loc = glGetUniformLocation(programId, "src");
		glBindImageTexture(2, shadow.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glUniform1i(loc, 2);
		CHECKERROR;
		loc = glGetUniformLocation(programId, "dst");
		glBindImageTexture(3, InterBlur.textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);		glUniform1i(loc, 3);
		CHECKERROR;
		loc = glGetUniformLocation(programId, "weight");
		glUniform1ui(loc, KernalSize);
		CHECKERROR;
		glDispatchCompute(shadowWidth / 128, shadowHeight, 1);// Tiles WxH image with groups sized 128x1 
		glUseProgram(0);
		computeShaderProgramHorizontal->Unuse();
		//InterBlur.Unbind();
	}
	//	////---------------------------------------------------     Horizontal SHADOW BLUR

	//	////---------------------------------------------------     Vertical SHADOW BLUR
	{
		//FinalBlur.Bind();
		computeShaderProgramVertical->Use();
		programId = computeShaderProgramVertical->programId;
		int bindpoint = 0; // Start at zero, increment for other blocks
		loc = glGetUniformBlockIndex(programId, "blurKernel");
		glUniformBlockBinding(programId, loc, bindpoint);
		glBindBufferBase(GL_UNIFORM_BUFFER, bindpoint, blockID);
		glBufferData(GL_UNIFORM_BUFFER, Filter.size() * sizeof(float), &Filter[0], GL_STATIC_DRAW);		//a texture to the shader as an image2D	
		loc = glGetUniformLocation(programId, "src");
		glBindImageTexture(2, InterBlur.textureID, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
		glUniform1i(loc, 2);
		CHECKERROR;
		loc = glGetUniformLocation(programId, "dst");
		glBindImageTexture(3, FinalBlur.textureID, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);		glUniform1i(loc, 3);
		CHECKERROR;
		loc = glGetUniformLocation(programId, "weight");
		glUniform1ui(loc, KernalSize);
		CHECKERROR;
		glDispatchCompute(shadowWidth, shadowHeight / 128, 1);// Tiles WxH image with groups sized 128x1 
		glUseProgram(0);
		computeShaderProgramVertical->Unuse();
		//FinalBlur.Unbind();
	}
	//	////---------------------------------------------------     Vertical SHADOW BLUR

	//---------------------------------------------------     LIGHTING PROGRAM
	MAT4 ShadowMatrix;
	ShadowMatrix = Translate(0.5f, 0.5f, 0.5f) * Scale(0.5f, 0.5f, 0.5f) * LightProj * LightView;
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);

	glViewport(0, 0, width, height);
	if (globalLightToggle)
	{
		LightingProgram->Use();

		//glClear(GL_DEPTH_BUFFER_BIT);
		programId = LightingProgram->programId;
		loc = glGetUniformLocation(programId, "ShadowMatrix");
		glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());
		loc = glGetUniformLocation(programId, "WorldInverse");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
		loc = glGetUniformLocation(programId, "lightPos");
		glUniform3fv(loc, 1, &(lPos[0]));
		loc = glGetUniformLocation(programId, "width");
		glUniform1f(loc, width);
		loc = glGetUniformLocation(programId, "height");
		glUniform1f(loc, height);
		loc = glGetUniformLocation(programId, "Light");
		glUniform3fv(loc, 1, &(Light[0]));

		loc = glGetUniformLocation(programId, "alphaaa");
		glUniform1f(loc, alpha);
		loc = glGetUniformLocation(programId, "maxdepth");
		glUniform1f(loc, maxdepth);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.gPosition);

		glActiveTexture(GL_TEXTURE0); // Activate texture unit 2
		glBindTexture(GL_TEXTURE_2D, shadow.textureID); // Load texture into it

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.gNormal);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.gAlbedoSpec);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.Diffuse);

		glActiveTexture(GL_TEXTURE5); // Activate texture unit 2
		glBindTexture(GL_TEXTURE_2D, FinalBlur.textureID); // Load texture into it

		loc = glGetUniformLocation(programId, "shadowMap");
		glUniform1i(loc, 0);
		loc = glGetUniformLocation(programId, "gPosition");
		glUniform1i(loc, 1);
		loc = glGetUniformLocation(programId, "gNormal");
		glUniform1i(loc, 2);
		loc = glGetUniformLocation(programId, "gAlbedoSpec");
		glUniform1i(loc, 3);
		loc = glGetUniformLocation(programId, "Diffuse");
		glUniform1i(loc, 4);
		loc = glGetUniformLocation(programId, "blurShadowMap");
		glUniform1i(loc, 5);
		loc = glGetUniformLocation(programId, "GBufferNum");
		glUniform1i(loc, GBufferNum);

		CHECKERROR;
		FSQ->Draw(LightingProgram, Identity);
		CHECKERROR;

		LightingProgram->Unuse();

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE0); // Activate texture unit 2
		glBindTexture(GL_TEXTURE_2D, 0); // Load texture into it

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE4);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//---------------------------------------------------     LIGHTING PROGRAM

	//---------------------------------------------------     LOCAL LIGHTS PROGRAM
	if (localLightsToggle)
	{
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);


		LocalLightProgram->Use();
		programId = LocalLightProgram->programId;
		loc = glGetUniformLocation(programId, "WorldProj");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
		loc = glGetUniformLocation(programId, "WorldView");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
		loc = glGetUniformLocation(programId, "WorldInverse");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.gPosition);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.gNormal);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.gAlbedoSpec);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, Gbuffer.Diffuse);

		loc = glGetUniformLocation(programId, "gPosition");
		glUniform1i(loc, 0);
		loc = glGetUniformLocation(programId, "gNormal");
		glUniform1i(loc, 1);
		loc = glGetUniformLocation(programId, "gAlbedoSpec");
		glUniform1i(loc, 2);
		loc = glGetUniformLocation(programId, "Diffuse");
		glUniform1i(loc, 3);
		loc = glGetUniformLocation(programId, "width");
		glUniform1f(loc, width);
		loc = glGetUniformLocation(programId, "height");
		glUniform1f(loc, height);
		const float radius = 1.1f;
		loc = glGetUniformLocation(programId, "radius");
		glUniform1f(loc, radius);

		vec3 center;
		for (int i = 0; i < numberoflights; i++)
		{
			loc = glGetUniformLocation(programId, "Light");
			glUniform3fv(loc, 1, &(lightcolor[i][0]));
			center = vec3(lights[i].x, lights[i].y, lights[i].z);
			loc = glGetUniformLocation(programId, "center");
			glUniform3fv(loc, 1, &(center[0]));
			localLights->Draw(LocalLightProgram, Translate(lights[i].x, lights[i].y, 1) * Scale(radius, radius, radius));
		}
		glDisable(GL_CULL_FACE);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE2);
		glBindTexture(GL_TEXTURE_2D, 0);

		glActiveTexture(GL_TEXTURE3);
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	//---------------------------------------------------     LOCAL LIGHTS PROGRAM

	//--------------------------------------------------- SKY DOME PROGRAM
	{
		glDisable(GL_BLEND);
		glDisable(GL_CULL_FACE);

		// Copy depth contents from G-Buffer
		glBindFramebuffer(GL_READ_FRAMEBUFFER, Gbuffer.fboID);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);// write to default framebuffer
		glBlitFramebuffer(0, 0,Gbuffer.width, Gbuffer.height,
			0, 0, Gbuffer.width, Gbuffer.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		glEnable(GL_DEPTH_TEST);
		SkyDomeProgram->Use();
		glViewport(0, 0, width, height);
		glClearColor(0.5, 0.5, 0.5, 1.0);

		programId = SkyDomeProgram->programId;

		loc = glGetUniformLocation(programId, "WorldProj");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
		loc = glGetUniformLocation(programId, "WorldView");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
		loc = glGetUniformLocation(programId, "WorldInverse");
		glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
		skydome->Draw(SkyDomeProgram, Identity);
		SkyDomeProgram->Unuse();
	}

	//--------------------------------------------------- SKY DOME PROGRAM


	animate(36);
	const float end = glfwGetTime() / 1000.0f - start;
	time_since_last_refresh = end;
}

std::vector<float>Scene::BlurFiler(int weight)
{

	//Highly optimized code! (*!STAR!*)
	//blurSize = 5;
	//int s = blurSize / 2;
	//float normB = 1.0f / (s * sqrt(3.141592f * 2));
	//for (int i = -blurSize; i <= blurSize; ++i)
	//	//for (int i = 0; i <= blurSize; ++i)
	//{
	//	float w = normB * exp(-(i * i) / (2.0f * s * s));
	//	blurWeights.emplace_back(w);
	//}


	//Declare thread - group - shared - memory v[128 + 2 * w + 1] floats
		//actually must be constant size : v[128 + <largest filter size>]
	//limit weight to something???
	int Weight = weight;
	std::vector<float> weights;
	float s = weight / 2.0f;
	// s=w/2 controls the
	//width of the bell curve. (The usual recommendation of s = w / 3 puts too many near - zeros at
	//the ends of the weight array which is inefficient for real - time uses.)
	float totalWeight = 0.0f;
	for (int i = -Weight; i <= Weight; ++i)
	{
		float exp1 = (-(i * i)) / (2.0f * s * s);
		float actualWeight = exp(exp1);
		weights.push_back(actualWeight);
		totalWeight += actualWeight;
	}
	for (auto& weight : weights)
	{
		weight /= totalWeight;
	}
	return weights;
}
