// All the includes neeed across several cpp files:
//   OpenGL, FreeGLUT, GLM, etc

#include <glbinding/gl/gl.h>
#include <glbinding/Binding.h>
using namespace gl;

#include <freeglut.h>

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "scene.h"
#include "interact.h"
