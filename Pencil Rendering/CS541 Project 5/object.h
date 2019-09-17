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

#ifndef _OBJECT
#define _OBJECT

#include "shapes.h"
#include "texture.h"
#include <utility>              // for pair<Object*,MAT4>

class Shader;
class Object;

// Object:: A shape, and its transformations, colors, and textures and sub-objects.
class Object
{
 public:
    Shape* shape;               // Polygons 
    MAT4 animTr;                // This model's animation transformation
    int objectId;               // Object id to be sent to the shader

    vec3 diffuseColor;          // Diffuse color of object
    vec3 specularColor;         // Specular color of object
    float shininess;            // Surface roughness value

    std::vector<std::pair<Object*,MAT4>> instances; // Pairs of sub-objects and transformations 

    Object(Shape* _shape, const int objectId,
           const vec3 _d=vec3(), const vec3 _s=vec3(), const float _n=1);

    // If this object is to be drawn with a texture, this is a good
    // place to store the texture id (a small positive integer).  The
    // texture id should be set in Scene::InitializeScene and used in
    // Object::Draw.
	unsigned int TextureId;
	unsigned int NormalId;
	void Object::Draw(ShaderProgram* program, MAT4& objectTr, bool dontdraw=false);

    void add(Object* m, MAT4 tr=MAT4()) { instances.push_back(std::make_pair(m,tr)); }
};

#endif
