/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <igl/avg_edge_length.h>
#include <igl/barycenter.h>
#include <igl/comb_cross_field.h>
#include <igl/comb_frame_field.h>
#include <igl/compute_frame_field_bisectors.h>
#include <igl/cross_field_mismatch.h>
#include <igl/cut_mesh_from_singularities.h>
#include <igl/find_cross_field_singularities.h>
#include <igl/local_basis.h>
#include <igl/rotate_vectors.h>
#include <igl/copyleft/comiso/miq.h>
#include <igl/copyleft/comiso/nrosy.h>
#include <igl/copyleft/comiso/frame_field.h>
#include <igl/readOBJ.h>
#include <igl/writeOBJ.h>
#include <igl/readDMAT.h>
#include <igl/frame_field_deformer.h>
#include <igl/frame_to_cross_field.h>
#include <igl/PI.h>
#include <igl/principal_curvature.h>
#include <AutoRemesher/Parametrization>
#include <iostream>
#include <unordered_set>

namespace AutoRemesher
{
    
namespace Parametrization
{

bool miq(HalfEdge::Mesh &mesh, const Parameters &parameters)
{
    Eigen::MatrixXd V(mesh.vertexCount(), 3);
    Eigen::MatrixXi F(mesh.faceCount(), 3);

    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = vertexNum;
        V.row(vertexNum++) << 
            vertex->position.x(), 
            vertex->position.y(), 
            vertex->position.z();
    }
    
    size_t faceNum = 0;
    std::unordered_set<size_t> usedHeightIds;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        F.row(faceNum) << 
            h0->startVertex->outputIndex, 
            h1->startVertex->outputIndex, 
            h2->startVertex->outputIndex;
        ++faceNum;
    }
    
    Eigen::MatrixXd PD1, PD2;
    Eigen::MatrixXd PV1, PV2;
    igl::principal_curvature(V, F, PD1, PD2, PV1, PV2);
    
    std::unordered_set<HalfEdge::Vertex *> pickedVertices;
    size_t targetConstraintVertexCount = mesh.vertexCount() * parameters.constraintRatio;
    size_t constaintVertexCount = 0;
    float limitRelativeHeight = 0.2;
    mesh.orderVertexByFlatness();
    for (const auto &it: mesh.vertexOrderedByFlatness()) {
        limitRelativeHeight = it->relativeHeight;
        ++constaintVertexCount;
        if (constaintVertexCount >= targetConstraintVertexCount)
            break;
    }
    std::cerr << "limitRelativeHeight:" << limitRelativeHeight << std::endl;
    
    std::vector<int> constraintFaces;
    std::vector<Vector3> constaintDirections1;
    std::vector<Vector3> constaintDirections2;
    faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        
        auto addFeatured = [&](HalfEdge::HalfEdge *h) {
            if (h->startVertex->relativeHeight > limitRelativeHeight)
                return false;
  
            auto r1 = PD1.row(h->startVertex->outputIndex);
            auto r2 = PD2.row(h->startVertex->outputIndex);
            
            auto v1 = AutoRemesher::Vector3(r1.x(), r1.y(), r1.z());
            auto v2 = AutoRemesher::Vector3(r2.x(), r2.y(), r2.z());
            
            if (v1.isZero() || v2.isZero())
                return false;
            
            constraintFaces.push_back(faceNum);
            constaintDirections1.push_back(v1);
            constaintDirections2.push_back(v2);
            
            return true;
        };
        addFeatured(h0) || addFeatured(h1) || addFeatured(h2);
        
        ++faceNum;
    }
    
    Eigen::VectorXi b(constraintFaces.size());
    Eigen::MatrixXd bc1(constaintDirections1.size(), 3);
    Eigen::MatrixXd bc2(constaintDirections2.size(), 3);
    
    for (size_t i = 0; i < constraintFaces.size(); ++i) {
        const auto &v1 = constaintDirections1[i];
        const auto &v2 = constaintDirections2[i];
        b.row(i) << constraintFaces[i];
        bc1.row(i) << v1.x(), v1.y(), v1.z();
        bc2.row(i) << v2.x(), v2.y(), v2.z();
    }

    // Global parametrization
    Eigen::MatrixXd UV;
    Eigen::MatrixXi FUV;

    // Interpolated frame field
    Eigen::MatrixXd FF1, FF2;

    // Deformed mesh
    Eigen::MatrixXd V_deformed;

    // Frame field on deformed
    Eigen::MatrixXd FF1_deformed;
    Eigen::MatrixXd FF2_deformed;

    // Cross field on deformed
    Eigen::MatrixXd X1_deformed;
    Eigen::MatrixXd X2_deformed;
    
    // Interpolate the frame field
    igl::copyleft::comiso::frame_field(V, F, b, bc1, bc2, FF1, FF2);

    // Deform the mesh to transform the frame field in a cross field
    igl::frame_field_deformer(
        V, F, FF1, FF2, V_deformed, FF1_deformed, FF2_deformed);

    // Find the closest crossfield to the deformed frame field
    igl::frame_to_cross_field(V_deformed, F, FF1_deformed, FF2_deformed, X1_deformed);

    // Find a smooth crossfield that interpolates the deformed constraints
    Eigen::MatrixXd bc_x(b.size(), 3);
    for (unsigned i = 0; i < b.size(); ++i)
        bc_x.row(i) = X1_deformed.row(b(i));

    Eigen::VectorXd S;
    igl::copyleft::comiso::nrosy(
             V,
             F,
             b,
             bc_x,
             Eigen::VectorXi(),
             Eigen::VectorXd(),
             Eigen::MatrixXd(),
             4,
             0.5,
             X1_deformed,
             S);

    // The other representative of the cross field is simply rotated by 90 degrees
    Eigen::MatrixXd B1,B2,B3;
    igl::local_basis(V_deformed,F,B1,B2,B3);
    X2_deformed =
    igl::rotate_vectors(X1_deformed, Eigen::VectorXd::Constant(1,igl::PI/2), B1, B2);

    // Global seamless parametrization
    igl::copyleft::comiso::miq(V_deformed,
           F,
           X1_deformed,
           X2_deformed,
           UV,
           FUV,
           parameters.gradientSize,
           5.0,
           false,
           2);
    
    if (FUV.rows() != mesh.faceCount()) {
        std::cerr << "miq failed" << std::endl;
        return false;
    }
    
    faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        const auto &triangleVertexIndices = FUV.row(faceNum++);
        const auto &v0 = UV.row(triangleVertexIndices[0]);
        const auto &v1 = UV.row(triangleVertexIndices[1]);
        const auto &v2 = UV.row(triangleVertexIndices[2]);
        h0->startVertexUv[0] = v0[0];
        h0->startVertexUv[1] = v0[1];
        h1->startVertexUv[0] = v1[0];
        h1->startVertexUv[1] = v1[1];
        h2->startVertexUv[0] = v2[0];
        h2->startVertexUv[1] = v2[1];
    }
    
    return true;
}

}

}


#if 1

extern "C" {
    // http://www.netlib.org/clapack/
    
    typedef int integer;
    typedef unsigned int uinteger;
    typedef char* address;
    typedef double doublereal;

    void daxpy_(const integer* n,
               const doublereal* da,
               const doublereal* dx,
               const integer* incx,
               doublereal* dy,
               const integer* incy) {
      /* System generated locals */
      integer i__1;

      /* Local variables */
      static thread_local integer i, m, ix, iy, mp1;

    /*     constant times a vector plus a vector.
           uses unrolled loops for increments equal to one.
           jack dongarra, linpack, 3/11/78.
           modified 12/3/93, array(1) declarations changed to array(*)



       Parameter adjustments
           Function Body */
    #define DY(I) dy[(I)-1]
    #define DX(I) dx[(I)-1]

      if (*n <= 0) {
        return;
      }
      if (*da == 0.) {
        return;
      }
      if (*incx == 1 && *incy == 1) {
        goto L20;
      }

      /*        code for unequal increments or equal increments
                  not equal to 1 */

      ix = 1;
      iy = 1;
      if (*incx < 0) {
        ix = (-(*n) + 1) * *incx + 1;
      }
      if (*incy < 0) {
        iy = (-(*n) + 1) * *incy + 1;
      }
      i__1 = *n;
      for (i = 1; i <= *n; ++i) {
        DY(iy) += *da * DX(ix);
        ix += *incx;
        iy += *incy;
        /* L10: */
      }
      return;

    /*        code for both increments equal to 1


              clean-up loop */

    L20:
      m = *n % 4;
      if (m == 0) {
        goto L40;
      }
      i__1 = m;
      for (i = 1; i <= m; ++i) {
        DY(i) += *da * DX(i);
        /* L30: */
      }
      if (*n < 4) {
        return;
      }
    L40:
      mp1 = m + 1;
      i__1 = *n;
      for (i = mp1; i <= *n; i += 4) {
        DY(i) += *da * DX(i);
        DY(i + 1) += *da * DX(i + 1);
        DY(i + 2) += *da * DX(i + 2);
        DY(i + 3) += *da * DX(i + 3);
        /* L50: */
      }
      return;
    } /* daxpy_ */

    doublereal ddot_(const integer *n,
                     const doublereal *dx,
                     const integer *incx,
                     const doublereal *dy,
                     const integer *incy) {
      /* System generated locals */
      integer i__1;
      doublereal ret_val;

      /* Local variables */
      static thread_local integer i, m;
      static thread_local doublereal dtemp;
      static thread_local integer ix, iy, mp1;

    /*     forms the dot product of two vectors.
           uses unrolled loops for increments equal to one.
           jack dongarra, linpack, 3/11/78.
           modified 12/3/93, array(1) declarations changed to array(*)



       Parameter adjustments
           Function Body */
    #define DY(I) dy[(I)-1]
    #define DX(I) dx[(I)-1]

      ret_val = 0.;
      dtemp = 0.;
      if (*n <= 0) {
        return ret_val;
      }
      if (*incx == 1 && *incy == 1) {
        goto L20;
      }

      /*        code for unequal increments or equal increments
                  not equal to 1 */

      ix = 1;
      iy = 1;
      if (*incx < 0) {
        ix = (-(*n) + 1) * *incx + 1;
      }
      if (*incy < 0) {
        iy = (-(*n) + 1) * *incy + 1;
      }
      i__1 = *n;
      for (i = 1; i <= *n; ++i) {
        dtemp += DX(ix) * DY(iy);
        ix += *incx;
        iy += *incy;
        /* L10: */
      }
      ret_val = dtemp;
      return ret_val;

    /*        code for both increments equal to 1


              clean-up loop */

    L20:
      m = *n % 5;
      if (m == 0) {
        goto L40;
      }
      i__1 = m;
      for (i = 1; i <= m; ++i) {
        dtemp += DX(i) * DY(i);
        /* L30: */
      }
      if (*n < 5) {
        goto L60;
      }
    L40:
      mp1 = m + 1;
      i__1 = *n;
      for (i = mp1; i <= *n; i += 5) {
        dtemp = dtemp + DX(i) * DY(i) + DX(i + 1) * DY(i + 1) +
                DX(i + 2) * DY(i + 2) + DX(i + 3) * DY(i + 3) +
                DX(i + 4) * DY(i + 4);
        /* L50: */
      }
    L60:
      ret_val = dtemp;
      return ret_val;
    } /* ddot_ */

    doublereal dnrm2_(const integer *n, const doublereal *x, const integer *incx) {
      /* System generated locals */
      integer i__1, i__2;
      doublereal ret_val, d__1;

      /* Builtin functions */
      double sqrt(doublereal);

      /* Local variables */
      static thread_local doublereal norm, scale, absxi;
      static thread_local integer ix;
      static thread_local doublereal ssq;

    /*  DNRM2 returns the euclidean norm of a vector via the function
        name, so that

           DNRM2 := sqrt( x'*x )



        -- This version written on 25-October-1982.
           Modified on 14-October-1993 to inline the call to DLASSQ.
           Sven Hammarling, Nag Ltd.



       Parameter adjustments
           Function Body */
    #define X(I) x[(I)-1]

      if (*n < 1 || *incx < 1) {
        norm = 0.;
      } else if (*n == 1) {
        norm = abs(X(1));
      } else {
        scale = 0.;
        ssq = 1.;
        /*        The following loop is equivalent to this call to the LAPACK

                  auxiliary routine:
                  CALL DLASSQ( N, X, INCX, SCALE, SSQ ) */

        i__1 = (*n - 1) * *incx + 1;
        i__2 = *incx;
        for (ix = 1;
             *incx < 0 ? ix >= (*n - 1) * *incx + 1 : ix <= (*n - 1) * *incx + 1;
             ix += *incx) {
          if (X(ix) != 0.) {
            absxi = (d__1 = X(ix), abs(d__1));
            if (scale < absxi) {
              /* Computing 2nd power */
              d__1 = scale / absxi;
              ssq = ssq * (d__1 * d__1) + 1.;
              scale = absxi;
            } else {
              /* Computing 2nd power */
              d__1 = absxi / scale;
              ssq += d__1 * d__1;
            }
          }
          /* L10: */
        }
        norm = scale * sqrt(ssq);
      }

      ret_val = norm;
      return ret_val;

      /*     End of DNRM2. */

    } /* dnrm2_ */
}

#endif
