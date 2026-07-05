#ifndef AUTO_REMESHER_MONOCHROME_OPENGL_VERTEX_H
#define AUTO_REMESHER_MONOCHROME_OPENGL_VERTEX_H
#include <QOpenGLFunctions>

#pragma pack(push)
#pragma pack(1)
struct MonochromeOpenGLVertex {
    GLfloat posX;
    GLfloat posY;
    GLfloat posZ;
    // Dark blue-grey
    GLfloat colorR = 0.15f;
    GLfloat colorG = 0.15f;
    GLfloat colorB = 0.18f;
    // Tiny bit of alpha lets the base color of the model show through the lines
    GLfloat alpha = 0.9f;
};
#pragma pack(pop)

#endif