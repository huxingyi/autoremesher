// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_VIEWER_FONT_HAS_BEEN_INCLUDED
#define OPENVDB_VIEWER_FONT_HAS_BEEN_INCLUDED

#include <string>

#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif


namespace openvdb_viewer {

class BitmapFont13
{
public:
    BitmapFont13() {}

    static void initialize();

    static void enableFontRendering();
    static void disableFontRendering();

    static void print(GLint px, GLint py, const std::string&);

private:
    static GLuint sOffset;
    static GLubyte sCharacters[95][13];
};

} // namespace openvdb_viewer

#endif // OPENVDB_VIEWER_FONT_HAS_BEEN_INCLUDED
