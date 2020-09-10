// Copyright Contributors to the OpenVDB Project
// SPDX-License-Identifier: MPL-2.0
//
/// @file geometry.cc
/// @author FX R&D OpenVDB team

#include "geometry.h"

#include <GU/GU_PrimPoly.h>
#include <UT/UT_Version.h>

namespace houdini_utils {


void
createBox(GU_Detail& gdp, UT_Vector3 corners[8],
    const UT_Vector3* color, bool shaded, float alpha)
{
    // Create points
    GA_Offset ptoff[8];
    for (size_t i = 0; i < 8; ++i) {
        ptoff[i] = gdp.appendPointOffset();
        gdp.setPos3(ptoff[i], corners[i].x(), corners[i].y(), corners[i].z());
    }

    if (color != NULL) {
        GA_RWHandleV3 cd(gdp.addDiffuseAttribute(GA_ATTRIB_POINT));
        for (size_t i = 0; i < 8; ++i) {
            cd.set(ptoff[i], *color);
        }
    }

    if (alpha < 0.99) {
        GA_RWHandleF A(gdp.addAlphaAttribute(GA_ATTRIB_POINT));
        for (size_t i = 0; i < 8; ++i) {
            A.set(ptoff[i], alpha);
        }
    }

    GEO_PrimPoly *poly;
    if (shaded) {
        // Bottom
        poly = GU_PrimPoly::build(&gdp, 0);
        poly->appendVertex(ptoff[0]);
        poly->appendVertex(ptoff[1]);
        poly->appendVertex(ptoff[2]);
        poly->appendVertex(ptoff[3]);
        poly->close();

        // Top
        poly = GU_PrimPoly::build(&gdp, 0);
        poly->appendVertex(ptoff[7]);
        poly->appendVertex(ptoff[6]);
        poly->appendVertex(ptoff[5]);
        poly->appendVertex(ptoff[4]);
        poly->close();

        // Front
        poly = GU_PrimPoly::build(&gdp, 0);
        poly->appendVertex(ptoff[4]);
        poly->appendVertex(ptoff[5]);
        poly->appendVertex(ptoff[1]);
        poly->appendVertex(ptoff[0]);
        poly->close();

        // Back
        poly = GU_PrimPoly::build(&gdp, 0);
        poly->appendVertex(ptoff[6]);
        poly->appendVertex(ptoff[7]);
        poly->appendVertex(ptoff[3]);
        poly->appendVertex(ptoff[2]);
        poly->close();

        // Left
        poly = GU_PrimPoly::build(&gdp, 0);
        poly->appendVertex(ptoff[0]);
        poly->appendVertex(ptoff[3]);
        poly->appendVertex(ptoff[7]);
        poly->appendVertex(ptoff[4]);
        poly->close();

        // Right
        poly = GU_PrimPoly::build(&gdp, 0);
        poly->appendVertex(ptoff[1]);
        poly->appendVertex(ptoff[5]);
        poly->appendVertex(ptoff[6]);
        poly->appendVertex(ptoff[2]);
        poly->close();

    } else {

        // 12 Edges as one line
        poly = GU_PrimPoly::build(&gdp, 0, GU_POLY_OPEN);
        poly->appendVertex(ptoff[0]);
        poly->appendVertex(ptoff[1]);
        poly->appendVertex(ptoff[2]);
        poly->appendVertex(ptoff[3]);
        poly->appendVertex(ptoff[0]);
        poly->appendVertex(ptoff[4]);
        poly->appendVertex(ptoff[5]);
        poly->appendVertex(ptoff[6]);
        poly->appendVertex(ptoff[7]);
        poly->appendVertex(ptoff[4]);
        poly->appendVertex(ptoff[5]);
        poly->appendVertex(ptoff[1]);
        poly->appendVertex(ptoff[2]);
        poly->appendVertex(ptoff[6]);
        poly->appendVertex(ptoff[7]);
        poly->appendVertex(ptoff[3]);
    }
} // createBox

} // namespace houdini_utils
