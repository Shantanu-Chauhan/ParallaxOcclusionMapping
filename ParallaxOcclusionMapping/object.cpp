////////////////////////////////////////////////////////////////////////
// A lightweight class representing an instance of an object that can
// be drawn onscreen.  An Object consists of a shape (batch of
// triangles), and various transformation, color and texture
// parameters.  It also contains a list of child objects to be drawn
// in a hierarchical fashion under the control of parent's
// transformations.
//
// Methods consist of a constructor, and a Draw procedure, and an
// append for building hierarchies of objects.
#include<glew.h>
#include "math.h"
#include <fstream>
#include <stdlib.h>


#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "shapes.h"
#include "scene.h"
#include "transform.h"
#include <glfw/glfw3.h>
#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line object.cpp:%d): %s\n", __LINE__); exit(-1);} }


Object::Object(Shape* _shape, const int _objectId,
               const vec3 _diffuseColor, const vec3 _specularColor, const float _shininess)
    : diffuseColor(_diffuseColor), specularColor(_specularColor), shininess(_shininess),
      shape(_shape), objectId(_objectId),TextureId(0),NormalId(0),HeightId(0)
     
{}


void Object::Draw(ShaderProgram* program, MAT4& objectTr,bool dontdraw)
{
    // Inform the shader of the surface values Kd, Ks, and alpha.
    int loc = glGetUniformLocation(program->programId, "diffuse");
    glUniform3fv(loc, 1, &diffuseColor[0]);

    loc = glGetUniformLocation(program->programId, "specular");
    glUniform3fv(loc, 1, &specularColor[0]);

    loc = glGetUniformLocation(program->programId, "shininess");
    glUniform1f(loc, shininess);

    // Inform the shader of which object is being drawn so it can make
    // object specific decisions.
    loc = glGetUniformLocation(program->programId, "objectId");
    glUniform1i(loc, objectId);

    // Inform the shader of this objects model transformation.  Oddly,
    // the inverse of the model transformation is needed transforming
    // normals.
    loc = glGetUniformLocation(program->programId, "ModelTr");
    glUniformMatrix4fv(loc, 1, GL_TRUE, objectTr.Pntr());
    

    MAT4 inv;
    invert(&objectTr, &inv);
    loc = glGetUniformLocation(program->programId, "NormalTr");
    glUniformMatrix4fv(loc, 1, GL_TRUE, inv.Pntr());

    // If this oject has an associated texture, this is the place to
    // load the texture into a texture-unit of your choice and inform
    // the shader program of the texture-unit number.  See
    // Texture::Bind for the 4 lines of code to do exactly that.
	if (TextureId > 0)
	{
		glActiveTexture((GLenum)((int)GL_TEXTURE0 + 5));
		glBindTexture(GL_TEXTURE_2D, TextureId);
		loc = glGetUniformLocation(program->programId, "TEXTURE");
		glUniform1i(loc, 5);
		//loc = glGetUniformLocation(program->programId, "diffuse");
		/*glm::vec3 zero(0.0f);
		glUniform3fv(loc, 1, &zero[0]);*/
	}

	if (NormalId > 0)
	{
		glActiveTexture((GLenum)((int)GL_TEXTURE0 + 6));
		glBindTexture(GL_TEXTURE_2D, NormalId);
		loc = glGetUniformLocation(program->programId, "NORMAL");
		glUniform1i(loc, 6);
	}
	if (HeightId > 0)
	{
		glActiveTexture((GLenum)((int)GL_TEXTURE0 + 7));
		glBindTexture(GL_TEXTURE_2D, HeightId);
		loc = glGetUniformLocation(program->programId, "HEIGHT");
		glUniform1i(loc, 7);
	}
    // Draw this object
    CHECKERROR;
	if (shape)
	{
		if (dontdraw &&this->objectId == teapotId)//IS THIS AIGHT?
			;
		else
		shape->DrawVAO();
	}
    CHECKERROR;

	if (TextureId > 0)
	{
		glActiveTexture((GLenum)((int)GL_TEXTURE0 + 5));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (NormalId > 0)
	{
		glActiveTexture((GLenum)((int)GL_TEXTURE0 + 6));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
	if (HeightId > 0)
	{
		glActiveTexture((GLenum)((int)GL_TEXTURE0 + 7));
		glBindTexture(GL_TEXTURE_2D, 0);
	}
    // Recursively draw each sub-objects, each with its own transformation.
    for (int i=0;  i<instances.size();  i++) {
        MAT4 itr = objectTr*instances[i].second*animTr;
        instances[i].first->Draw(program, itr,dontdraw); }
    
    CHECKERROR;
}
