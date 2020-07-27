#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <qex.h>
#include <AutoRemesher/QuadRemesher>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/GuidelineGenerator>

namespace AutoRemesher
{

bool QuadRemesher::remesh()
{
    //AutoRemesher::GuidelineGenerator guidelineGenerator(&m_vertices, &m_triangles);
    //guidelineGenerator.generate();
    
    //const auto &guideline = guidelineGenerator.guidelineVertices();
    
    //guidelineGenerator.debugExportPly("C:\\Users\\Jeremy\\Desktop\\test-guideline.ply", guideline);
    
    AutoRemesher::HalfEdge::Mesh mesh(m_vertices, m_triangles);

    //mesh.markGuidelineEdgesAsFeatured();
    
    //if (!mesh.decimate()) {
    //    std::cerr << "Mesh decimate failed" << std::endl;
    //    return false;
    //}
    
    //mesh.debugExportVertexRelativeHeightPly("C:\\Users\\Jeremy\\Desktop\\test-debug.ply");
    //exit(0);
    
    mesh.debugExportSegmentEdgesPly("C:\\Users\\Jeremy\\Desktop\\test-debug.ply");
    mesh.debugExportPly("C:\\Users\\Jeremy\\Desktop\\test-decimated.ply");

    qex_TriMesh triMesh = {0};
    qex_QuadMesh quadMesh = {0};
    
    triMesh.vertex_count = mesh.vertexCount();
    triMesh.tri_count = mesh.faceCount();
    
    triMesh.vertices = (qex_Point3*)malloc(sizeof(qex_Point3) * triMesh.vertex_count);
    triMesh.tris = (qex_Tri*)malloc(sizeof(qex_Tri) * triMesh.tri_count);
    triMesh.uvTris = (qex_UVTri*)malloc(sizeof(qex_UVTri) * triMesh.tri_count);
    
    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = mesh.firstVertex(); nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = vertexNum;
        triMesh.vertices[vertexNum++] = qex_Point3 {{
            (double)vertex->position.x(), 
            (double)vertex->position.y(), 
            (double)vertex->position.z()
        }};
    }
    
    std::vector<HalfEdge::HalfEdge *> triangleHalfEdges;
    triangleHalfEdges.reserve(triMesh.tri_count * 3);
    size_t faceNum = 0;
    for (HalfEdge::Face *face = mesh.firstFace(); nullptr != face; face = face->_next) {
        HalfEdge::HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge::HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge::HalfEdge *h2 = h1->nextHalfEdge;
        triMesh.tris[faceNum] = qex_Tri {{
            (qex_Index)h0->startVertex->outputIndex, 
            (qex_Index)h1->startVertex->outputIndex, 
            (qex_Index)h2->startVertex->outputIndex
        }};
        triangleHalfEdges.push_back(h0);
        triangleHalfEdges.push_back(h1);
        triangleHalfEdges.push_back(h2);
        ++faceNum;
    }
    
    bool remeshSucceed = false;
    if (mesh.parametrize(m_gradientSize, m_constraintStength)) {
        
        mesh.debugExportUvObj("C:\\Users\\Jeremy\\Desktop\\test-uv.obj");
        
        faceNum = 0;
        for (size_t i = 0; i < triangleHalfEdges.size(); ) {
            auto &h0 = triangleHalfEdges[i++];
            auto &h1 = triangleHalfEdges[i++];
            auto &h2 = triangleHalfEdges[i++];
            triMesh.uvTris[faceNum++] = qex_UVTri {{
                qex_Point2 {{h0->startVertexUv[0], h0->startVertexUv[1]}}, 
                qex_Point2 {{h1->startVertexUv[0], h1->startVertexUv[1]}}, 
                qex_Point2 {{h2->startVertexUv[0], h2->startVertexUv[1]}}
            }};
        }

        qex_extractQuadMesh(&triMesh, nullptr, &quadMesh);
        
        m_remeshedVertices.resize(quadMesh.vertex_count);
        for (unsigned int i = 0; i < quadMesh.vertex_count; ++i) {
            const auto &src = quadMesh.vertices[i];
            m_remeshedVertices[i] = Vector3 {(double)src.x[0], (double)src.x[1], (double)src.x[2]};
        }
        m_remeshedQuads.resize(quadMesh.quad_count);
        for (unsigned int i = 0; i < quadMesh.quad_count; ++i) {
            const auto &src = quadMesh.quads[i];
            m_remeshedQuads[i] = std::vector<size_t> {src.indices[0], src.indices[1], src.indices[2], src.indices[3]};
        }
        
        remeshSucceed = true;
    }
    
    free(triMesh.vertices);
    free(triMesh.tris);
    free(triMesh.uvTris);

    free(quadMesh.vertices);
    free(quadMesh.quads);
    
    return remeshSucceed;
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

}

#endif