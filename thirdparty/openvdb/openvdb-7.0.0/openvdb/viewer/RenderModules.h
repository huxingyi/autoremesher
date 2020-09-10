// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_VIEWER_RENDERMODULES_HAS_BEEN_INCLUDED
#define OPENVDB_VIEWER_RENDERMODULES_HAS_BEEN_INCLUDED

#include <openvdb/openvdb.h>
#include <openvdb/tools/VolumeToMesh.h>
#include <openvdb/tools/MeshToVolume.h>
#include <openvdb/tools/PointScatter.h>
#include <openvdb/tree/LeafManager.h>
#include <openvdb/math/Operators.h>
#include <string>
#include <vector>

#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif


namespace openvdb_viewer {

// OpenGL helper objects

class BufferObject
{
public:
    BufferObject();
    ~BufferObject();

    void render() const;

    /// @note accepted @c primType: GL_POINTS, GL_LINE_STRIP, GL_LINE_LOOP,
    /// GL_LINES, GL_TRIANGLE_STRIP, GL_TRIANGLE_FAN, GL_TRIANGLES,
    /// GL_QUAD_STRIP, GL_QUADS and GL_POLYGON
    void genIndexBuffer(const std::vector<GLuint>&, GLenum primType);

    void genVertexBuffer(const std::vector<GLfloat>&);
    void genNormalBuffer(const std::vector<GLfloat>&);
    void genColorBuffer(const std::vector<GLfloat>&);

    void clear();

private:
    GLuint mVertexBuffer, mNormalBuffer, mIndexBuffer, mColorBuffer;
    GLenum mPrimType;
    GLsizei mPrimNum;
};


class ShaderProgram
{
public:
    ShaderProgram();
    ~ShaderProgram();

    void setVertShader(const std::string&);
    void setFragShader(const std::string&);

    void build();
    void build(const std::vector<GLchar*>& attributes);

    void startShading() const;
    void stopShading() const;

    void clear();

private:
    GLuint mProgram, mVertShader, mFragShader;
};


////////////////////////////////////////


/// @brief interface class
class RenderModule
{
public:
    virtual ~RenderModule() {}

    virtual void render() = 0;

    bool visible() { return mIsVisible; }
    void setVisible(bool b) { mIsVisible = b; }

protected:
    RenderModule(): mIsVisible(true) {}

    bool mIsVisible;
};


////////////////////////////////////////


/// @brief Basic render module, axis gnomon and ground plane.
class ViewportModule: public RenderModule
{
public:
    ViewportModule();
    ~ViewportModule() override = default;

    void render() override;

private:
    float mAxisGnomonScale, mGroundPlaneScale;
};


////////////////////////////////////////


/// @brief Tree topology render module
class TreeTopologyModule: public RenderModule
{
public:
    TreeTopologyModule(const openvdb::GridBase::ConstPtr&);
    ~TreeTopologyModule() override = default;

    void render() override;

private:
    void init();

    const openvdb::GridBase::ConstPtr& mGrid;
    BufferObject mBufferObject;
    bool mIsInitialized;
    ShaderProgram mShader;
};


////////////////////////////////////////


/// @brief Module to render active voxels as points
class VoxelModule: public RenderModule
{
public:
    VoxelModule(const openvdb::GridBase::ConstPtr&);
    ~VoxelModule() override = default;

    void render() override;

private:
    void init();

    const openvdb::GridBase::ConstPtr& mGrid;
    BufferObject mInteriorBuffer, mSurfaceBuffer, mVectorBuffer;
    bool mIsInitialized;
    ShaderProgram mFlatShader, mSurfaceShader;
};


////////////////////////////////////////


/// @brief Surfacing render module
class MeshModule: public RenderModule
{
public:
    MeshModule(const openvdb::GridBase::ConstPtr&);
    ~MeshModule() override = default;

    void render() override;

private:
    void init();

    const openvdb::GridBase::ConstPtr& mGrid;
    BufferObject mBufferObject;
    bool mIsInitialized;
    ShaderProgram mShader;
};

} // namespace openvdb_viewer

#endif // OPENVDB_VIEWER_RENDERMODULES_HAS_BEEN_INCLUDED
