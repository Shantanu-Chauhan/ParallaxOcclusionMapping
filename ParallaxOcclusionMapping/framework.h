// All the includes neeed across several cpp files:
//   OpenGL, FreeGLUT, GLM, etc
#define GLEW_STATIC

#include <glew.h>
#include <glfw/glfw3.h>

#define GLM_FORCE_RADIANS
#define GLM_SWIZZLE
#include <glm/glm.hpp>
using namespace glm;

#include "shader.h"
#include "scene.h"
#include "interact.h"

