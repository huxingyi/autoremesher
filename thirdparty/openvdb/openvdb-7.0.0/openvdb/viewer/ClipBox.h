// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0

#ifndef OPENVDB_VIEWER_CLIPBOX_HAS_BEEN_INCLUDED
#define OPENVDB_VIEWER_CLIPBOX_HAS_BEEN_INCLUDED

#include <openvdb/Types.h>

#if defined(__APPLE__) || defined(MACOSX)
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>
#else
#include <GL/gl.h>
#include <GL/glu.h>
#endif


namespace openvdb_viewer {

class ClipBox
{
public:
    ClipBox();

    void enableClipping() const;
    void disableClipping() const;

    void setBBox(const openvdb::BBoxd&);
    void setStepSize(const openvdb::Vec3d& s) { mStepSize = s; }

    void render();

    void update(double steps);
    void reset();

    bool isActive() const { return (mXIsActive || mYIsActive ||mZIsActive); }

    bool& activateXPlanes() { return mXIsActive;  }
    bool& activateYPlanes() { return mYIsActive;  }
    bool& activateZPlanes() { return mZIsActive;  }

    bool& shiftIsDown() { return mShiftIsDown; }
    bool& ctrlIsDown() { return mCtrlIsDown; }

    bool mouseButtonCallback(int button, int action);
    bool mousePosCallback(int x, int y);

private:
    void update() const;

    openvdb::Vec3d mStepSize;
    openvdb::BBoxd mBBox;
    bool mXIsActive, mYIsActive, mZIsActive, mShiftIsDown, mCtrlIsDown;
    GLdouble mFrontPlane[4], mBackPlane[4], mLeftPlane[4], mRightPlane[4],
        mTopPlane[4], mBottomPlane[4];
}; // class ClipBox

} // namespace openvdb_viewer

#endif // OPENVDB_VIEWER_CLIPBOX_HAS_BEEN_INCLUDED
