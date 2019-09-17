////////////////////////////////////////////////////////////////////////
// A small library of 4x4 matrix operations needed for graphics
// transformations.  MAT4 is a 4x4 float matrix class with indexing
// and printing methods.  A small list or procedures are supplied to
// create Rotate, Scale, Translate, and Perspective matrices and to
// return the product of any two such.

#ifndef _TRANSFORM_
#define _TRANSFORM_

#include <fstream>
#include"glm/glm.hpp"
typedef float ROW4[4];

// The 4x4 matrix class with methods for indexing.
class MAT4
{
public:
    float M[4][4];

    // Initilaize all matrices to the identity
    MAT4()
    {
        for( int i=0; i<4; i++ )
            for( int j=0; j<4; j++)
                M[i][j] = i==j ? 1.0f : 0.0f;
    }

    // Some indexing operations for the matrix
    ROW4& operator[](const int i)  { return M[i]; }
    const ROW4& operator[](const int i) const { return M[i]; }

    // Used to communicate to OpenGL
    float* Pntr();
    
    // Simple procedure to print a 4x4 matrix -- useful for debugging
    void print();
};

// Factory functions to create specific transformations, multiply two, and invert one.
MAT4 Rotate(const int i, const float theta);
MAT4 Scale(const float x, const float y, const float z);
MAT4 Translate(const float x, const float y, const float z);
MAT4 Perspective(const float rx, const float ry,
                 const float front, const float back);
MAT4 operator* (const MAT4 A, const MAT4 B);

MAT4 LookAt(glm::vec3 LightPos,glm::vec3 LightDir,glm::vec3 UpDir);

int invert(const MAT4* mat, MAT4* inv);

#endif
