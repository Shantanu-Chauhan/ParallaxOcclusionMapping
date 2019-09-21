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

#include "math.h"
#include <fstream>
#include <stdlib.h>

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;
#include <AntTweakBar.h>

#include <freeglut.h>
#include <glu.h>                // For gluErrorString

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
const float rad = PI/180.0f;    // Convert degrees to radians

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
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line scene.cpp:%d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }

// Create an RGB color from human friendly parameters: hue, saturation, value
vec3 HSV2RGB(const float h, const float s, const float v)
{
    if (s == 0.0)
        return vec3(v,v,v);

    int i = (int)(h*6.0) % 6;
    float f = (h*6.0f) - i;
    float p = v*(1.0f - s);
    float q = v*(1.0f - s*f);
    float t = v*(1.0f - s*(1.0f-f));
    if      (i == 0)     return vec3(v,t,p);
    else if (i == 1)  return vec3(q,v,p);
    else if (i == 2)  return vec3(p,v,t);
    else if (i == 3)  return vec3(p,q,v);
    else if (i == 4)  return vec3(t,p,v);
    else   /*i == 5*/ return vec3(v,p,q);
}

////////////////////////////////////////////////////////////////////////
// Constructs a hemisphere of spheres of varying hues
Object* SphereOfSpheres(Shape* SpherePolygons)
{
    Object* ob = new Object(NULL, nullId);
    
    for (float angle=0.0;  angle<360.0;  angle+= 18.0)
        for (float row=0.075;  row<PI/2.0;  row += PI/2.0/6.0) {   
            vec3 hue = HSV2RGB(angle/360.0, 1.0f-2.0f*row/PI, 1.0f);

            Object* sp = new Object(SpherePolygons, spheresId,
                                    hue, vec3(1.0, 1.0, 1.0), 120.0);
            float s = sin(row);
            float c = cos(row);
            ob->add(sp, Rotate(2,angle)*Translate(c,0,s)*Scale(0.075*c,0.075*c,0.075*c));
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
    
    vec3 woodColor(87.0/255.0,51.0/255.0,35.0/255.0);
    ob = new Object(BoxPolygons, frameId,
                    woodColor, vec3(0.2, 0.2, 0.2), 10.0);
    frame->add(ob, Translate(0.0, 0.0, 1.0+w)*Scale(1.0, w, w));
    frame->add(ob, Translate(0.0, 0.0, -1.0-w)*Scale(1.0, w, w));
    frame->add(ob, Translate(1.0+w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));
    frame->add(ob, Translate(-1.0-w, 0.0, 0.0)*Scale(w, w, 1.0+2*w));

    ob = new Object(QuadPolygons, objectId,
                    woodColor, vec3(0.0, 0.0, 0.0), 10.0);
    frame->add(ob, Rotate(0,90));

    return frame;
}

////////////////////////////////////////////////////////////////////////
// A callback procedure, called regularly to update the atime global
// variable.
float atime = 0.0;
void animate(int value)
{
    atime = 360.0*glutGet((GLenum)GLUT_ELAPSED_TIME)/36000;
    glutPostRedisplay();

    // Schedule next call to this function
    glutTimerFunc(30, animate, 1);
}

////////////////////////////////////////////////////////////////////////
// InitializeScene is called once during setup to create all the
// textures, shape VAOs, and shader programs as well as setting a
// number of other parameters.
void Scene::InitializeScene()
{
	bar = TwNewBar("TweakBar");
	
    glEnable(GL_DEPTH_TEST);
    CHECKERROR;
	TwAddVarRW(bar, "NameOfMyVariable", TW_TYPE_FLOAT, &spin,"");
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
	time_since_last_refresh = 0;
	eyee = true;
	w = false;
	a = false;
	s = false;
	d = false;
    CHECKERROR;
    objectRoot = new Object(NULL, nullId);
    
    // Set the initial light position parammeters
    lightSpin = 150.0;
    lightTilt = -45.0;
    lightDist = 1000.0;
	numberoflights = 500;
	srand(time(NULL));
	float x, y, z, r, g, b;
	int numberOfRows = 50;
	int numberOfColumns = 30;
	for (int i = 0; i < numberOfRows; i++)
	{
		for (int j = 0; j < numberOfColumns; j++)
		{
			x = i-20;
			y = j-10;
			z = 10.0f;
			r = g = b = 1.0f;
			lights.push_back(vec3(x, y, z));
			lightcolor.push_back(vec3(r, g, b));
		}
	}

	//for (float i = 0.0f; i < numberoflights; i++)
	//{

	//	x = i;
	//	y = (rand() % 100)-50;
	//	z = 10.0f;
	//	r = (rand() % 1000)/100;
	//	g = (rand() % 1000)/100;
	//	b = (rand() % 1000)/100;
	//	//r = 3.5;
	//	//g = 3.5;
	//	//b = 3.5;
	//	lights.push_back(vec3(x, y, z));
	//	lightcolor.push_back(vec3(r,g,b));
	//}

    // Enable OpenGL depth-testing
    glEnable(GL_DEPTH_TEST);

    // FIXME:  Change false to true to randomize the central object position.
    ground =  new ProceduralGround(grndSize, grndTiles, grndOctaves, grndFreq,
                                   grndPersistence, grndLow, grndHigh, false);

    basePoint = ground->highPoint;

    // Create the lighting shader program from source code files.
    lightingProgram = new ShaderProgram();
    lightingProgram->AddShader("lighting.vert", GL_VERTEX_SHADER);
    lightingProgram->AddShader("lighting.frag", GL_FRAGMENT_SHADER);
	lightingProgram->AddShader("LightingCalc.vert", GL_VERTEX_SHADER); //NEW
	lightingProgram->AddShader("LightingCalc.frag", GL_FRAGMENT_SHADER); //NEW
    glBindAttribLocation(lightingProgram->programId, 0, "vertex");
    glBindAttribLocation(lightingProgram->programId, 1, "vertexNormal");
    glBindAttribLocation(lightingProgram->programId, 2, "vertexTexture");
    glBindAttribLocation(lightingProgram->programId, 3, "vertexTangent");
    lightingProgram->LinkProgram();

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
	
	//Creating Reflection1 Shader program
	reflectionProgram1 = new ShaderProgram();
	reflectionProgram1->AddShader("ReflectionPass1.vert", GL_VERTEX_SHADER);
	reflectionProgram1->AddShader("ReflectionPass1.frag", GL_FRAGMENT_SHADER);
	reflectionProgram1->AddShader("LightingCalc.vert", GL_VERTEX_SHADER); //NEW
	reflectionProgram1->AddShader("LightingCalc.frag", GL_FRAGMENT_SHADER); //NEW
	glBindAttribLocation(reflectionProgram1->programId, 0, "vertex");
	glBindAttribLocation(reflectionProgram1->programId, 1, "vertexNormal");
	glBindAttribLocation(reflectionProgram1->programId, 2, "vertexTexture");
	glBindAttribLocation(reflectionProgram1->programId, 3, "vertexTangent");
	reflectionProgram1->LinkProgram();
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


	LocalLightProgram = new ShaderProgram();
	LocalLightProgram->AddShader("LocalLight.vert", GL_VERTEX_SHADER);
	LocalLightProgram->AddShader("LocalLight.frag", GL_FRAGMENT_SHADER);
	glBindAttribLocation(LocalLightProgram->programId, 0, "vertex");
	LocalLightProgram->LinkProgram();

    // Create all the Polygon shapes
    Shape* TeapotPolygons =  new Teapot(12);
    Shape* BoxPolygons = new Ply("box.ply");
    Shape* SpherePolygons = new Sphere(32);
    Shape* RoomPolygons = new Ply("room.ply");
    Shape* GroundPolygons = ground;
    Shape* QuadPolygons = new Quad();
    Shape* SeaPolygons = new Plane(2000.0, 50);

    // Various colors used in the subsequent models
    vec3 woodColor(87.0/255.0, 51.0/255.0, 35.0/255.0);
    vec3 brickColor(134.0/255.0, 60.0/255.0, 56.0/255.0);
    vec3 brassColor(0.5, 0.5, 0.1);
    vec3 grassColor(62.0/255.0, 102.0/255.0, 38.0/255.0);
    vec3 waterColor(0.3, 0.3, 1.0);
 
    // Creates all the models from which the scene is composed.  Each
    // is created with a polygon shape (possibly NULL), a
    // transformation, and the surface lighting parameters Kd, Ks, and
    // alpha.
    Object* central    = new Object(NULL, nullId);
    Object* anim       = new Object(NULL, nullId);
    Object* room       = new Object(RoomPolygons, roomId, brickColor, vec3(0.0, 0.0, 0.0), 1);
    Object* teapot     = new Object(TeapotPolygons, teapotId, brassColor, vec3(0.5, 0.5, 0.5), 120);
    Object* podium     = new Object(BoxPolygons, boxId, vec3(woodColor), vec3(0.3, 0.3, 0.3), 10); 
    Object* sky        = new Object(SpherePolygons, skyId, vec3(), vec3(), 0);
    Object* ground     = new Object(GroundPolygons, groundId, grassColor, vec3(0.0, 0.0, 0.0), 1);
    Object* sea        = new Object(SeaPolygons, seaId, waterColor, vec3(1.0, 1.0, 1.0), 120);
    Object* spheres    = SphereOfSpheres(SpherePolygons);
    Object* leftFrame  = FramedPicture(Identity, lPicId, BoxPolygons, QuadPolygons);
    Object* rightFrame = FramedPicture(Identity, rPicId, BoxPolygons, QuadPolygons); 
	//Object* sphere = new Object(SpherePolygons, nullId, vec3(), vec3(), 0);
	
	FSQ = new Object(QuadPolygons, nullId);
	localLights = new Object(SpherePolygons, nullId);


    // FIXME: This is where you could read in all the textures and
    // associate them with the various objects just created above.
	Texture*skyDome = new Texture("textures/Ocean.png");
	sky->TextureId = skyDome->textureId;
	
	Texture*seaNormal = new Texture("textures/ripples_normalmap.png");
	sea->TextureId = skyDome->textureId;
	sea->NormalId = seaNormal->textureId;

	Texture* teaPot = new Texture("textures/cracks.png");
	teapot->TextureId = teaPot->textureId;

	Texture* box= new Texture("textures/box.png");
	Texture* boxNormal= new Texture("textures/Brazilian_rosewood_pxr128_normal.png");
	podium->TextureId = box->textureId;
	podium->NormalId = boxNormal->textureId;

	Texture* walls = new Texture("textures/brick.png");
	Texture* wallNormal = new Texture("textures/Standard_red_pxr128_normal.png");
	room->TextureId = walls->textureId;
	room->NormalId = wallNormal->textureId;

	Texture*grass = new Texture("textures/grass.jpg");
	ground->TextureId = grass->textureId;

	Texture*rightPic = new Texture("textures/my-house-01.png");
	rightFrame->TextureId = rightPic->textureId;

    // Scene is composed of sky, ground, sea, room and some central models
    objectRoot->add(ground);
    objectRoot->add(room, Translate(basePoint.x, basePoint.y, basePoint.z));// REMOVE THIS FOR REFLECTIONS
    objectRoot->add(sea);
    objectRoot->add(central);
	objectRoot->add(sky, Scale(2000.0, 2000.0, 2000.0));
	//localLights->add(sphere, Scale(2.0, 2.0, 2.0));


    // Central model has a rudimentary animation (constant rotation on Z)
    animated.push_back(anim);

    // Central contains a teapot on a podium and an external sphere of spheres
    central->add(podium, Translate(0.0, 0,0));
    central->add(anim, Translate(0.0, 0,0));
    anim->add(teapot, Translate(0.1, 0.0, 1.5)*TeapotPolygons->modelTr);
    anim->add(spheres, Translate(0.0, 0.0, 0.0)*Scale(30.0, 30.0, 30.0));
    
    // Room contains two framed pictures
    room->add(leftFrame, Translate(-1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));
    room->add(rightFrame, Translate( 1.5, 9.85, 1.)*Scale(0.8, 0.8, 0.8));

    // Schedule first timed animation call
    glutTimerFunc(30, animate, 1);
	
	shadow.CreateFBO(2048, 2048);
	reflection1.CreateFBO(1024,1024);
	reflection2.CreateFBO(1024,1024);

	Gbuffer.CreateG(width, height);
    CHECKERROR;
}

////////////////////////////////////////////////////////////////////////
// Procedure DrawScene is called whenever the scene needs to be
// drawn. (Which is often: 30 to 60 times per second are the common
// goals.)
void Scene::DrawScene()
{	
	const float start = glutGet((GLenum)GLUT_ELAPSED_TIME);
	// Calculate the light's position.
	const float lPos[4] = {
	   basePoint.x + lightDist * cos(lightSpin*rad)*sin(lightTilt*rad),
	   basePoint.y + lightDist * sin(lightSpin*rad)*sin(lightTilt*rad),
	   basePoint.z + lightDist * cos(lightTilt*rad),
	   1.0 };
	// Set the viewport, and clear the screen

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	// Draw all objects (hierarchically down through each object's child list).

	for (std::vector<Object*>::iterator m = animated.begin(); m < animated.end(); m++)
		(*m)->animTr = Rotate(2, atime);

	// Compute Viewing and Perspective transformations.
	MAT4 WorldProj, WorldView, WorldInverse;
	const int delta_time = glutGet((GLenum)GLUT_ELAPSED_TIME) - time_since_last_refresh;
	const float speed = 0.01f;
	const float step = speed * delta_time;
	rx = ry * ((float)width / (float)height);
	// Compute any continuously animating objects
	if (eyee)
	{
		if (w)
		{
			eye += step * vec3(sin(spin*PI / 180.0f), cos(spin*PI / 180.0f), 0.0);
		}
		if (s)
		{
			eye -= step * vec3(sin(spin*PI / 180.0f), cos(spin*PI / 180.0f), 0.0);
		}
		if (d)
		{
			eye += step * vec3(cos(spin*PI / 180.0f), -sin(spin*PI / 180.0f), 0.0);
		}
		if (a)
		{
			eye -= step * vec3(cos(spin*PI / 180.0f), -sin(spin*PI / 180.0f), 0.0);
		}
		eye.z = ground->HeightAt(eye.x, eye.y) + 2.0f;
		WorldView = Rotate(0, tilt - 90.0f)*Rotate(2, spin)*Translate(-eye.x, -eye.y, -eye.z);
	}
	else
		WorldView = Translate(tx, ty, -zoom) * Rotate(0, tilt - 90.0f)*Rotate(2, spin);

	WorldProj = Perspective(rx, ry, front, back);
	invert(&WorldView, &WorldInverse);
	const vec3 Teapot(0.2, 0.0, 1.5);
	const vec3 Light(3.5, 3.5, 3.5), Ambient(0.2, 0.2, 0.2);//:light and ambient set
	vec3 AmbientLight(0.3, 0.3, 0.3);

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
	objectRoot->Draw(GbufferProgram, Identity);
	Gbuffer.Unbind();
	GbufferProgram->Unuse();

	glActiveTexture(GL_TEXTURE7);
	glBindTexture(GL_TEXTURE_2D, Gbuffer.gPosition);

	glActiveTexture(GL_TEXTURE8);
	glBindTexture(GL_TEXTURE_2D, Gbuffer.gNormal);

	glActiveTexture(GL_TEXTURE9);
	glBindTexture(GL_TEXTURE_2D, Gbuffer.gAlbedoSpec);

	glActiveTexture(GL_TEXTURE10);
	glBindTexture(GL_TEXTURE_2D, Gbuffer.Diffuse);

	glDisable(GL_BLEND);
	glDisable(GL_DEPTH_TEST);
	AmbientProgram->Use();
	programId = AmbientProgram->programId;
	loc = glGetUniformLocation(programId, "Ambient");
	glUniform3fv(loc, 1, &(AmbientLight[0]));
	loc = glGetUniformLocation(programId, "Diffuse");
	glUniform1i(loc, 10);
	loc = glGetUniformLocation(programId, "width");
	glUniform1f(loc, width);
	loc = glGetUniformLocation(programId, "height");
	glUniform1f(loc, height);
	CHECKERROR;
	FSQ->Draw(AmbientProgram, Identity);
	CHECKERROR;
	AmbientProgram->Unuse();
	//SHADOW SHADER START

	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	shadowProgram->Use();
	shadow.Bind();
	glViewport(0, 0, shadow.width, shadow.height);
	glClearColor(0.5, 0.5, 0.5, 1.0);
	glClear(GL_DEPTH_BUFFER_BIT);
	programId = shadowProgram->programId;

	const glm::vec3 Lightpos = glm::vec3(lPos[0], lPos[1], lPos[2]);

	MAT4 LightView = LookAt(Lightpos, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	float nry;
	nry = 40.0 / lightDist;
	MAT4 LightProj = Perspective(nry, nry, front, back);
	//shadow
	loc = glGetUniformLocation(programId, "LightProj");
	glUniformMatrix4fv(loc, 1, GL_TRUE, LightProj.Pntr());
	loc = glGetUniformLocation(programId, "LightView");
	glUniformMatrix4fv(loc, 1, GL_TRUE, LightView.Pntr());

	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	objectRoot->Draw(shadowProgram, Identity);
	glDisable(GL_CULL_FACE);
	glClear(GL_DEPTH_BUFFER_BIT);
	shadow.Unbind();
	CHECKERROR;
	shadowProgram->Unuse();
	CHECKERROR;

	glActiveTexture(GL_TEXTURE2); // Activate texture unit 2
	glBindTexture(GL_TEXTURE_2D, shadow.textureID); // Load texture into it

	//SHADOW SHADER END!!!

	MAT4 ShadowMatrix;
	ShadowMatrix = Translate(0.5f, 0.5f, 0.5f)*Scale(0.5f, 0.5f, 0.5f)*LightProj*LightView;
	//2 REFLECTIONS PASSES BETWEEN THIS

	//first relection pass
	/*reflectionProgram1->Use();
	reflection1.Bind();
	glViewport(0, 0, reflection1.width,reflection1.height);
	glClear(GL_DEPTH_BUFFER_BIT|GL_COLOR_BUFFER_BIT);
	programId = reflectionProgram1->programId;

	loc = glGetUniformLocation(programId, "eyepos");//center of relflection is the center of teapot
	glUniform3fv(loc, 1, &(Teapot[0]));

	loc = glGetUniformLocation(programId, "c");//value of C
	glUniform1f(loc, 1);//setting the value of C

	loc = glGetUniformLocation(programId, "shadowMap");
	glUniform1i(loc, 2); // Tell shader texture is in unit 2

	loc = glGetUniformLocation(programId, "ShadowMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());
	// Setup the perspective and viewing matrices for normal viewing.
	loc = glGetUniformLocation(programId, "lightPos");
	glUniform3fv(loc, 1, &(lPos[0]));
	loc = glGetUniformLocation(programId, "mode");
	glUniform1i(loc, mode);
	loc = glGetUniformLocation(programId, "Light");
	glUniform3fv(loc, 1, &(Light[0]));
	loc = glGetUniformLocation(programId, "Ambient");
	glUniform3fv(loc, 1, &(Ambient[0]));
	objectRoot->Draw(reflectionProgram1, Identity,true);
	reflection1.Unbind();
	CHECKERROR;
	reflectionProgram1->Unuse();
	CHECKERROR;
	glActiveTexture(GL_TEXTURE3); // Activate texture unit 2
	glBindTexture(GL_TEXTURE_2D, reflection1.textureID); // Load texture into it

	//first relection pass//bot

	//second relection pass
	reflectionProgram1->Use();
	reflection2.Bind();
	glViewport(0, 0, reflection2.width, reflection2.height);
	glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
	programId = reflectionProgram1->programId;

	loc = glGetUniformLocation(programId, "eyepos");//center of relflection is the center of teapot
	glUniform3fv(loc, 1, &(Teapot[0]));

	loc = glGetUniformLocation(programId, "c");//value of C
	glUniform1f(loc, -1);//setting the value of C

	loc = glGetUniformLocation(programId, "shadowMap");
	glUniform1i(loc, 2); // Tell shader texture is in unit 2


	loc = glGetUniformLocation(programId, "ShadowMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());
	loc = glGetUniformLocation(programId, "lightPos");
	glUniform3fv(loc, 1, &(lPos[0]));
	loc = glGetUniformLocation(programId, "mode");
	glUniform1i(loc, mode);


	loc = glGetUniformLocation(programId, "Light");
	glUniform3fv(loc, 1, &(Light[0]));
	loc = glGetUniformLocation(programId, "Ambient");
	glUniform3fv(loc, 1, &(Ambient[0]));
	objectRoot->Draw(reflectionProgram1, Identity,true);

	reflection2.Unbind();
	CHECKERROR;
	reflectionProgram1->Unuse();
	CHECKERROR;
	glActiveTexture(GL_TEXTURE4); // Activate texture unit 2
	glBindTexture(GL_TEXTURE_2D, reflection2.textureID); // Load texture into it

	// Use the lighting shader
	//glEnable(GL_BLEND);
	//glDisable(GL_DEPTH_TEST);
	lightingProgram->Use();

	glClear(GL_DEPTH_BUFFER_BIT);
	programId = lightingProgram->programId;

	glActiveTexture(GL_TEXTURE2); // Activate texture unit 2
	glBindTexture(GL_TEXTURE_2D, shadow.textureID); // Load texture into it

	loc = glGetUniformLocation(programId, "shadowMap");
	glUniform1i(loc, 2); // Tell shader texture is in unit 2

	//skyDome->Bind(5, programId, "skyDome");
	//teaPot->Bind(6, programId, "teaPot");

	loc = glGetUniformLocation(programId, "TOPReflection");
	glUniform1i(loc, 3); // Tell shader texture is in unit 3

	loc = glGetUniformLocation(programId, "BOTReflection");
	glUniform1i(loc, 4); // Tell shader texture is in unit 4

	loc = glGetUniformLocation(programId, "ShadowMatrix");
	glUniformMatrix4fv(loc, 1, GL_TRUE, ShadowMatrix.Pntr());
	// Setup the perspective and viewing matrices for normal viewing.
	loc = glGetUniformLocation(programId, "WorldProj");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(programId, "WorldInverse");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
	loc = glGetUniformLocation(programId, "lightPos");
	glUniform3fv(loc, 1, &(lPos[0]));
	loc = glGetUniformLocation(programId, "mode");
	glUniform1i(loc, mode);

	loc = glGetUniformLocation(programId, "Light");
	glUniform3fv(loc, 1, &(Light[0]));
	loc = glGetUniformLocation(programId, "Ambient");
	glUniform3fv(loc, 1, &(Ambient[0]));
	objectRoot->Draw(lightingProgram, Identity);

	CHECKERROR;
	lightingProgram->Unuse();
	CHECKERROR;
	*/
	//Use the lighting shader

	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glViewport(0, 0, width, height);
	LightingProgram->Use();

	glClear(GL_DEPTH_BUFFER_BIT);
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
	loc = glGetUniformLocation(programId, "shadowMap");
	glUniform1i(loc, 2);
	loc = glGetUniformLocation(programId, "gPosition");
	glUniform1i(loc, 7);
	loc = glGetUniformLocation(programId, "gNormal");
	glUniform1i(loc, 8);
	loc = glGetUniformLocation(programId, "gAlbedoSpec");
	glUniform1i(loc, 9);
	loc = glGetUniformLocation(programId, "Diffuse");
	glUniform1i(loc, 10);
	CHECKERROR;
	FSQ->Draw(LightingProgram, Identity);
	CHECKERROR;
	LightingProgram->Unuse();


	//INFINITE LIGHTS
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_FRONT);
	glDisable(GL_CULL_FACE);

	LocalLightProgram->Use();
	programId = LocalLightProgram->programId;
	loc = glGetUniformLocation(programId, "WorldProj");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldProj.Pntr());
	loc = glGetUniformLocation(programId, "WorldView");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldView.Pntr());
	loc = glGetUniformLocation(programId, "WorldInverse");
	glUniformMatrix4fv(loc, 1, GL_TRUE, WorldInverse.Pntr());
	loc = glGetUniformLocation(programId, "lightPos");
	glUniform3fv(loc, 1, &(lPos[0]));
	loc = glGetUniformLocation(programId, "gPosition");
	glUniform1i(loc, 7);
	loc = glGetUniformLocation(programId, "gNormal");
	glUniform1i(loc, 8);
	loc = glGetUniformLocation(programId, "gAlbedoSpec");
	glUniform1i(loc, 9);
	loc = glGetUniformLocation(programId, "Diffuse");
	glUniform1i(loc, 10);
	loc = glGetUniformLocation(programId, "width");
	glUniform1f(loc, width);
	loc = glGetUniformLocation(programId, "height");
	glUniform1f(loc, height);
	const float radius = 1.0f;
	loc = glGetUniformLocation(programId, "radius");
	glUniform1f(loc, radius);

	//int number = 1000;
	
	for (int i = 0;i < numberoflights; i++)
	{
		//vec3 LightColor(1.0, 0, 0);
		loc = glGetUniformLocation(programId, "Light");
		glUniform3fv(loc, 1, &(lightcolor[i][0]));
		//draw geometry to invoke the pixel shader
		vec3 center(lights[i].x, lights[i].y, 1);
		loc = glGetUniformLocation(programId, "center");
		glUniform3fv(loc, 1, &(center[0]));
		localLights->Draw(LocalLightProgram, Translate(lights[i].x, lights[i].y, 1)*Scale(radius, radius, radius));
	}

	//for (float angle = 0.0; angle < 360.0; angle += 18.0)
	//	for (float row = 0.075; row < PI / 2.0; row += PI / 2.0 / 6.0)
	//	{
	//		float s = sin(row);
	//		float c = cos(row);
	//		//vec3 LightColor(1.0, 0.0, 0.0);
	//		vec3 LightColor = HSV2RGB(angle / 360.0, 1.0f - 2.0f*row / PI, 1.0f);
	//		loc = glGetUniformLocation(programId, "Light");
	//		glUniform3fv(loc, 1, &(LightColor[0]));
	//		//draw geometry to invoke the pixel shader
	//		vec3 center(c, 0, s);
	//		loc = glGetUniformLocation(programId, "center");
	//		glUniform3fv(loc, 1, &(center[0]));
	//		localLights->Draw(LocalLightProgram, Rotate(2, angle)*Translate(c, 0, s)*Scale(radius, radius, radius));
	//	}

	LocalLightProgram->Unuse();
	//INFINITE LIGHTS END
//glutKeyboardFunc((GLUTkeyboardfun)TwEventKeyboardGLUT);
	// - Directly redirect GLUT special key events to AntTweakBar
	TwDraw();
	time_since_last_refresh = glutGet((GLenum)GLUT_ELAPSED_TIME);
	const float end = glutGet((GLenum)GLUT_ELAPSED_TIME)-start;
}

