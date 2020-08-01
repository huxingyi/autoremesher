#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <qex.h>
#include <unordered_set>
#include <igl/boundary_loop.h>
#include <AutoRemesher/QuadRemesher>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/GuidelineGenerator>
#include <AutoRemesher/Radians>

namespace AutoRemesher
{

bool QuadRemesher::remesh()
{
    //std::cerr << "Generating guideline..." << std::endl;
    
    //AutoRemesher::GuidelineGenerator guidelineGenerator(&m_vertices, &m_triangles);
    //guidelineGenerator.generate();
    
    //std::cerr << "Generating guideline done" << std::endl;
   
    //const auto &guideline = guidelineGenerator.guidelineVertices();
    
    //guidelineGenerator.debugExportPly("C:\\Users\\Jeremy\\Desktop\\test-guideline.ply", guideline);
    
    AutoRemesher::HalfEdge::Mesh mesh(m_vertices, m_triangles);
    
    if (!mesh.isWatertight()) {
        std::cerr << "Mesh is not watertight" << std::endl;
        return false;
    }

    //mesh.markGuidelineEdgesAsFeatured();
    
    //if (!mesh.decimate()) {
    //    std::cerr << "Mesh decimate failed" << std::endl;
    //    return false;
    //}
    mesh.removeZeroAngleTriangles();
    
    mesh.debugExportVertexRelativeHeightPly("C:\\Users\\Jeremy\\Desktop\\test-debug.ply");
    //exit(0);
    
    //mesh.debugExportSegmentEdgesPly("C:\\Users\\Jeremy\\Desktop\\test-debug.ply");
    //mesh.debugExportPly("C:\\Users\\Jeremy\\Desktop\\test-decimated.ply");

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
        m_remeshedQuads.reserve(quadMesh.quad_count);
        for (unsigned int i = 0; i < quadMesh.quad_count; ++i) {
            const auto &src = quadMesh.quads[i];
            if (0 == src.indices[0] ||
                    0 == src.indices[1] ||
                    0 == src.indices[2] ||
                    0 == src.indices[3])
                continue;
            std::unordered_set<qex_Index> indices;
            indices.insert(src.indices[0]);
            indices.insert(src.indices[1]);
            indices.insert(src.indices[2]);
            indices.insert(src.indices[3]);
            if (4 != indices.size())
                continue;
            m_remeshedQuads.push_back(std::vector<size_t> {src.indices[0], src.indices[1], src.indices[2], src.indices[3]});
        }
        
        fixHoles();
        
        remeshSucceed = true;
    }
    
    free(triMesh.vertices);
    free(triMesh.tris);
    free(triMesh.uvTris);

    free(quadMesh.vertices);
    free(quadMesh.quads);
    
    return remeshSucceed;
}

void QuadRemesher::createCoonsPatchFrom(const std::vector<size_t> &c0,
        const std::vector<size_t> &c1,
        const std::vector<size_t> &d0,
        const std::vector<size_t> &d1)
{
    auto Lc_Position = [&](int s, int t) {
        float factor = (float)t / d0.size();
        return (1.0 - factor) * m_remeshedVertices[c0[s]] + factor * m_remeshedVertices[c1[s]];
    };
    auto Ld_Position = [&](int s, int t) {
        float factor = (float)s / c0.size();
        return (1.0 - factor) * m_remeshedVertices[d0[t]] + factor * m_remeshedVertices[d1[t]];
    };
    auto B_Position = [&](int s, int t) {
        float tFactor = (float)t / d0.size();
        float sFactor = (float)s / c0.size();
        return m_remeshedVertices[c0[0]] * (1.0 - sFactor) * (1.0 - tFactor) +
            m_remeshedVertices[c0[c0.size() - 1]] * sFactor * (1.0 - tFactor) +
            m_remeshedVertices[c1[0]] * (1.0 - sFactor) * tFactor +
            m_remeshedVertices[c1[c1.size() - 1]] * sFactor * tFactor;
    };
    auto C_Position = [&](int s, int t) {
        return Lc_Position(s, t) + Ld_Position(s, t) - B_Position(s, t);
    };
    
    std::vector<std::vector<size_t>> grid(c0.size());
    for (int s = 1; s < (int)c0.size() - 1; ++s) {
        grid[s].resize(d0.size());
        for (int t = 1; t < (int)d0.size() - 1; ++t) {
            grid[s][t] = m_remeshedVertices.size();
            m_remeshedVertices.push_back(C_Position(s, t));
        }
    }
    grid[0].resize(d0.size());
    grid[c0.size() - 1].resize(d0.size());
    for (size_t i = 0; i < c0.size(); ++i) {
        grid[i][0] = c0[i];
        grid[i][d0.size() - 1] = c1[i];
    }
    for (size_t i = 0; i < d0.size(); ++i) {
        grid[0][i] = d0[i];
        grid[c0.size() - 1][i] = d1[i];
    }
    for (int s = 1; s < (int)c0.size(); ++s) {
        for (int t = 1; t < (int)d0.size(); ++t) {
            std::vector<size_t> face = {
                grid[s - 1][t - 1],
                grid[s - 1][t],
                grid[s][t],
                grid[s][t - 1]
            };
            m_remeshedQuads.push_back(face);
        }
    }
}

void QuadRemesher::fixHoles()
{
    Eigen::MatrixXi F(m_remeshedQuads.size() * 2, 3);
    for (size_t i = 0, j = 0; i < m_remeshedQuads.size(); ++i) {
        const auto &quad = m_remeshedQuads[i];
        F.row(j++) << quad[0], quad[1], quad[2];
        F.row(j++) << quad[2], quad[3], quad[0];
    }
    
    auto angle2d = [](const Vector2 &a, const Vector2 &b) {
        Vector3 first(a.x(), a.y(), 0.0);
        Vector3 second(b.x(), b.y(), 0.0);
        return Vector3::angle(first, second);
    };
    
    auto findCorner = [&](const std::vector<Vector2> &loop) {
        std::vector<std::pair<size_t, double>> corners(loop.size());
        for (size_t i = 0; i < loop.size(); ++i) {
            size_t j = (i + 1) % loop.size();
            size_t k = (i + 2) % loop.size();
            auto &corner = corners[j];
            corner.first = j;
            corner.second = std::abs(Radians::toDegrees(angle2d(loop[i] - loop[j],
                loop[k] - loop[j])) - 90.0);
        }
        return std::min_element(corners.begin(), corners.end(), [&](const std::pair<size_t, double> &firstCorner,
                const std::pair<size_t, double> &secondCorner) {
            return firstCorner.second < secondCorner.second;
        })->first;
    };
    
    auto isCorner = [&](const std::vector<Vector2> &loop, size_t index) {
        auto degrees = Radians::toDegrees(angle2d(loop[(index + 1) % loop.size()] - loop[index],
                loop[(index + loop.size() - 1) % loop.size()] - loop[index]));
        bool isTrue = std::abs(degrees - 90.0) <= 30;
        std::cerr << "isCorner degrees:" << degrees << " index:" << index << "/" << loop.size() << " is?" << isTrue << std::endl;
        return isTrue;
    };
    
    std::vector<std::vector<int>> loops;
    igl::boundary_loop(F, loops);
    for (const auto &loop: loops) {
        Vector3 origin;
        for (const auto &it: loop) {
            origin += m_remeshedVertices[it];
        }
        origin /= loop.size();
        
        Vector3 projectNormal;
        for (size_t i = 0; i < loop.size(); ++i) {
            size_t j = (i + 1) % loop.size();
            projectNormal += Vector3::normal(m_remeshedVertices[loop[i]],
                m_remeshedVertices[loop[j]],
                origin);
        }
        projectNormal.normalize();
        
        Vector3 projectAxis = m_remeshedVertices[loop[0]] - origin;
        
        std::vector<Vector3> ringPoints;
        ringPoints.reserve(loop.size());
        for (const auto &it: loop) {
            ringPoints.push_back(m_remeshedVertices[it]);
        }
        std::vector<Vector2> ringPointsIn2d;
        Vector3::project(ringPoints, &ringPointsIn2d, projectNormal, projectAxis, origin);
        
        if (4 == loop.size()) {
            m_remeshedQuads.push_back(std::vector<size_t> {
                (size_t)loop[0], (size_t)loop[1], (size_t)loop[2], (size_t)loop[3]
            });
        } else if (loop.size() > 4 && loop.size() % 2 == 0) {
            size_t cornerIndex = findCorner(ringPointsIn2d);
            size_t nextCornerIndex = cornerIndex;
            size_t previousCornerIndex = cornerIndex;
            std::cerr << "Looking corners" << std::endl;
            if (isCorner(ringPointsIn2d, cornerIndex)) {
                for (int i = 1; i < loop.size(); ++i) {
                    if (isCorner(ringPointsIn2d, (cornerIndex + i) % loop.size())) {
                        nextCornerIndex = (cornerIndex + i) % loop.size();
                        break;
                    }
                }
                for (int i = 1; i < loop.size(); ++i) {
                    if (isCorner(ringPointsIn2d, (cornerIndex + loop.size() - i) % loop.size())) {
                        previousCornerIndex = (cornerIndex + loop.size() - i) % loop.size();
                        break;
                    }
                }
            } else {
                std::cerr << "Found corner failed" << std::endl;
            }
            int rows = (nextCornerIndex + loop.size() - cornerIndex) % loop.size();
            int columns = (cornerIndex + loop.size() - previousCornerIndex) % loop.size();
            std::cerr << "Initial rows:" << rows << " columns:" << columns << std::endl;
            std::cerr << "cornerIndex:" << cornerIndex << " nextCornerIndex:" << nextCornerIndex << " previousCornerIndex:" << previousCornerIndex << std::endl;
            if (columns < rows) {
                rows = loop.size() / 2 - columns;
            }
            columns = loop.size() / 2 - rows;
            if (rows > 0 && columns > 0) {
                std::vector<size_t> edges[4];
                size_t offset = cornerIndex;
                for (int i = 0; i <= rows; ++i)
                    edges[0].push_back(loop[(offset++) % loop.size()]);
                --offset;
                for (int i = 0; i <= columns; ++i)
                    edges[1].push_back(loop[(offset++) % loop.size()]);
                --offset;
                for (int i = 0; i <= rows; ++i)
                    edges[2].push_back(loop[(offset++) % loop.size()]);
                --offset;
                for (int i = 0; i <= columns; ++i)
                    edges[3].push_back(loop[(offset++) % loop.size()]);
                std::reverse(edges[2].begin(), edges[2].end());
                std::reverse(edges[3].begin(), edges[3].end());
                std::cerr << "createCoonsPatchFrom rows:" << rows << " columns:" << columns << " loops:" << loop.size() << std::endl;
                createCoonsPatchFrom(edges[0], edges[2], edges[3], edges[1]);
                std::cerr << "createCoonsPatchFrom done" << std::endl;
            } else {
                std::cerr << "Found other corner failed" << std::endl;
            }
        } else {
            std::cerr << "Found unfixable boundary loop, length:" << loop.size() << std::endl;
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

}

#endif