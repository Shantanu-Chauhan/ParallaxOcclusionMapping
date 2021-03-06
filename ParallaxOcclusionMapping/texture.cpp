///////////////////////////////////////////////////////////////////////
// A slight encapsulation of an OpenGL texture. This contains a method
// to read an image file into a texture, and methods to bind a texture
// to a shader for use, and unbind when done.
////////////////////////////////////////////////////////////////////////

#include<glew.h>
#include "math.h"
#include <fstream>
#include <stdlib.h>
#include<iostream>
#include "texture.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "stb_image.h"

#define CHECKERROR {GLenum err = glGetError(); if (err != GL_NO_ERROR) { fprintf(stderr, "OpenGL error (at line texture.cpp:%d): %s\n", __LINE__, gluErrorString(err)); exit(-1);} }

Texture::Texture(const std::string &path) : textureId(0)
{
    stbi_set_flip_vertically_on_load(true);
    int width, height, n;
   
	if (path.substr(path.find_last_of(".")) == ".hdr")
	{
		float* data = stbi_loadf(path.c_str(), &mWidth, &mHeight, &n, 0);

		if (!data)
		{
			std::cout << "[Texture] HDR Texture {0} could not be loaded" << path.c_str() << std::endl;
		}

		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F, mWidth, mHeight, 0, GL_RGB, GL_FLOAT, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);

		if (data)
		{
			stbi_image_free(data);
		}
		return;
	}
    unsigned char* image = stbi_load(path.c_str(), &width, &height, &n, 4);
	if (!image) {
		printf("\nRead error on file %s:\n  %s\n\n", path.c_str(), stbi_failure_reason());
		exit(-1);
	}
    // Here we create MIPMAP and set some useful modes for the texture
    glGenTextures(1, &textureId);   // Get an integer id for thi texture from OpenGL
    glBindTexture(GL_TEXTURE_2D, textureId);
    glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);  
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(image);

}

Texture::Texture(int width, int height)
{
	// Here we create MIPMAP and set some useful modes for the texture
	glGenTextures(1, &textureId);   // Get an integer id for thi texture from OpenGL
	glBindTexture(GL_TEXTURE_2D, textureId);
	glTexImage2D(GL_TEXTURE_2D, 0, (GLint)GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 10);
	//glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, (int)GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, (int)GL_LINEAR_MIPMAP_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

// Make a texture availabe to a shader program.  The unit parameter is
// a small integer specifying which texture unit should load the
// texture.  The name parameter is the sampler2d in the shader program
// which will provide access to the texture.
void Texture::Bind(const int unit, const int programId, const std::string& name)
{
    glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
    glBindTexture(GL_TEXTURE_2D, textureId);
    int loc = glGetUniformLocation(programId, name.c_str());
    glUniform1i(loc, unit);
}

//void Texture::Bind(const int unit, const int programId, const std::string& name)
//{
//	glActiveTexture((GLenum)((int)GL_TEXTURE0 + unit));
//	glBindTexture(GL_TEXTURE_2D, textureId);
//	int loc = glGetUniformLocation(programId, name.c_str());
//	glUniform1i(loc, unit);
//}

// Unbind a texture from a texture unit whne no longer needed.
void Texture::Unbind()
{  
    glBindTexture(GL_TEXTURE_2D, 0);
}
