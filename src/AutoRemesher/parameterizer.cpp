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
#include <AutoRemesher/Parameterizer>
#include <AutoRemesher/Radians>
#include <iostream>
#include <unordered_set>
#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

namespace AutoRemesher
{

Parameterizer::Parameterizer(HalfEdge::Mesh *mesh, const Parameters &parameters) :
    m_parameters(parameters)
{
    m_mesh = mesh;
    
    m_V = new Eigen::MatrixXd(m_mesh->vertexCount(), 3);
    m_F = new Eigen::MatrixXi(m_mesh->faceCount(), 3);
    
    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = m_mesh->firstVertex(); nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = vertexNum;
        m_V->row(vertexNum++) << 
            vertex->position.x(), 
            vertex->position.y(), 
            vertex->position.z();
    }
    
    size_t faceNum = 0;
    for (HalfEdge::Face *face = m_mesh->firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        m_F->row(faceNum) << 
            h0->startVertex->outputIndex, 
            h1->startVertex->outputIndex, 
            h2->startVertex->outputIndex;
        ++faceNum;
    }
    
    m_PD1 = new Eigen::MatrixXd;
    m_PD2 = new Eigen::MatrixXd;
    Eigen::MatrixXd PV1, PV2;
    igl::principal_curvature(*m_V, *m_F, *m_PD1, *m_PD2, PV1, PV2);
    
    m_mesh->orderVertexByFlatness();
}

std::pair<double, double> Parameterizer::calculateLimitRelativeHeight(const std::pair<double, double> &limitRelativeHeight)
{
    size_t targetLowVertexCount = m_mesh->vertexCount() * limitRelativeHeight.first;
    size_t targetHighVertexCount = m_mesh->vertexCount() * limitRelativeHeight.second;
    size_t lowCount = 0;
    size_t highCount = 0;
    double limitLow = 0.0;
    double limitHigh = 1.0;
    const auto &vertices = m_mesh->vertexOrderedByFlatness();
    if (targetLowVertexCount < vertices.size())
        limitLow = vertices[targetLowVertexCount]->relativeHeight;
    if (targetHighVertexCount < vertices.size())
        limitHigh = vertices[targetHighVertexCount]->relativeHeight;
    return {limitLow, limitHigh};
}

void Parameterizer::prepareConstraints(const std::pair<double, double> &limitRelativeHeight,
    Eigen::VectorXi **b,
    Eigen::MatrixXd **bc1,
    Eigen::MatrixXd **bc2)
{
    std::vector<int> constraintFaces;
    std::vector<Vector3> constaintDirections1;
    std::vector<Vector3> constaintDirections2;
    
    size_t faceNum = 0;
    for (HalfEdge::Face *face = m_mesh->firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        
        auto addFeatured = [&](HalfEdge::HalfEdge *h) {
            if (m_parameters.constrainOnFlatArea) {
                if (h->startVertex->relativeHeight >= limitRelativeHeight.first && 
                        h->startVertex->relativeHeight <= limitRelativeHeight.second)
                    return false;
            } else {
                if (h->startVertex->relativeHeight < limitRelativeHeight.first || 
                        h->startVertex->relativeHeight > limitRelativeHeight.second)
                    return false;
            }
            
            auto r1 = m_PD1->row(h->startVertex->outputIndex);
            auto r2 = m_PD2->row(h->startVertex->outputIndex);
            
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
    
    delete *b;
    delete *bc1;
    delete *bc2;
    
    *b = new Eigen::VectorXi(constraintFaces.size());
    *bc1 = new Eigen::MatrixXd(constaintDirections1.size(), 3);
    *bc2 = new Eigen::MatrixXd(constaintDirections2.size(), 3);
    
    for (size_t i = 0; i < constraintFaces.size(); ++i) {
        const auto &v1 = constaintDirections1[i];
        const auto &v2 = constaintDirections2[i];
        (*b)->row(i) << constraintFaces[i];
        (*bc1)->row(i) << v1.x(), v1.y(), v1.z();
        (*bc2)->row(i) << v2.x(), v2.y(), v2.z();
    }
}

bool Parameterizer::miq(size_t *singularityCount, 
    const Eigen::VectorXi &b,
    const Eigen::MatrixXd &bc1,
    const Eigen::MatrixXd &bc2,
    bool calculateSingularityOnly)
{
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
    igl::copyleft::comiso::frame_field(*m_V, *m_F, b, bc1, bc2, FF1, FF2);

    // Deform the mesh to transform the frame field in a cross field
    igl::frame_field_deformer(
        *m_V, *m_F, FF1, FF2, V_deformed, FF1_deformed, FF2_deformed);
        
#if AUTO_REMESHER_DEV
    igl::writeOBJ("debug-origin.obj", *m_V, *m_F);
    igl::writeOBJ("debug-deformed.obj", V_deformed, *m_F);
#endif

    // Find the closest crossfield to the deformed frame field
    igl::frame_to_cross_field(V_deformed, *m_F, FF1_deformed, FF2_deformed, X1_deformed);

    // Find a smooth crossfield that interpolates the deformed constraints
    Eigen::MatrixXd bc_x(b.size(), 3);
    for (unsigned i = 0; i < b.size(); ++i)
        bc_x.row(i) = X1_deformed.row(b(i));

    Eigen::VectorXd S;
    igl::copyleft::comiso::nrosy(
        *m_V,
        *m_F,
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
    Eigen::MatrixXd B1, B2, B3;
    igl::local_basis(V_deformed, *m_F, B1, B2, B3);
    X2_deformed =
    igl::rotate_vectors(X1_deformed, Eigen::VectorXd::Constant(1, igl::PI / 2), B1, B2);

    // Global seamless parametrization
    {
        Eigen::MatrixXd BIS1, BIS2;
        igl::compute_frame_field_bisectors(V_deformed, *m_F, X1_deformed, X2_deformed, BIS1, BIS2);

        Eigen::MatrixXd BIS1_combed, BIS2_combed;
        igl::comb_cross_field(V_deformed, *m_F, BIS1, BIS2, BIS1_combed, BIS2_combed);

        Eigen::MatrixXi Handle_MMatch;
        igl::cross_field_mismatch(V_deformed, *m_F, BIS1_combed, BIS2_combed, true, Handle_MMatch);

        Eigen::Matrix<int, Eigen::Dynamic, 1> isSingularity, singularityIndex;
        igl::find_cross_field_singularities(V_deformed, *m_F, Handle_MMatch, isSingularity, singularityIndex);
        
        *singularityCount = 0;
        for (int i = 0; i < isSingularity.rows(); ++i) {
            if (isSingularity(i))
                ++(*singularityCount);
        }
        if (calculateSingularityOnly)
            return true;
        
        m_vertexValences.clear();
        m_vertexValences.reserve(singularityIndex.rows());
        for (int i = 0; i < singularityIndex.rows(); ++i) {
            m_vertexValences.push_back(4 + singularityIndex(i));
        }

        Eigen::Matrix<int, Eigen::Dynamic, 3> Handle_Seams;
        igl::cut_mesh_from_singularities(V_deformed, *m_F, Handle_MMatch, Handle_Seams);

        Eigen::MatrixXd PD1_combed, PD2_combed;
        igl::comb_frame_field(V_deformed, *m_F, X1_deformed, X2_deformed, BIS1_combed, BIS2_combed, PD1_combed, PD2_combed);
        
        double stiffness = 5.0;
        bool directRound = false;
        unsigned int iter = 2;
        unsigned int localIter = 5;
        bool doRound = true;
        bool singularityRound = true;
        const std::vector<int> roundVertices = std::vector<int>();
        const std::vector<std::vector<int>> hardFeatures = std::vector<std::vector<int>>();
        igl::copyleft::comiso::miq(V_deformed,
            *m_F,
            PD1_combed,
            PD2_combed,
            Handle_MMatch,
            isSingularity,
            Handle_Seams,
            UV,
            FUV,
            m_parameters.gradientSize,
            stiffness,
            directRound,
            iter,
            localIter,
            doRound,
            singularityRound,
            roundVertices,
            hardFeatures);
    }
    
    if (FUV.rows() != m_mesh->faceCount()) {
        std::cerr << "miq failed" << std::endl;
        return false;
    }
    
    size_t faceNum = 0;
    for (HalfEdge::Face *face = m_mesh->firstFace(); nullptr != face; face = face->_next) {
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
