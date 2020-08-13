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
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <qex.h>
#include <unordered_set>
#include <igl/boundary_loop.h>
#include <AutoRemesher/QuadRemesher>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/Radians>

namespace AutoRemesher
{

bool QuadRemesher::remesh()
{
    qex_TriMesh triMesh = {0};
    qex_QuadMesh quadMesh = {0};
    qex_Valence *vertexValences = nullptr;
    
    if (nullptr != m_vertexValences) {
        vertexValences = (qex_Valence *)malloc(sizeof(qex_Valence) * m_vertexValences->size());
        for (size_t i = 0; i < m_vertexValences->size(); ++i) {
            vertexValences[i] = (qex_Valence)(*m_vertexValences)[i];
        }
    }
    
    triMesh.vertex_count = m_mesh->vertexCount();
    triMesh.tri_count = m_mesh->faceCount();
    
    triMesh.vertices = (qex_Point3*)malloc(sizeof(qex_Point3) * triMesh.vertex_count);
    triMesh.tris = (qex_Tri*)malloc(sizeof(qex_Tri) * triMesh.tri_count);
    triMesh.uvTris = (qex_UVTri*)malloc(sizeof(qex_UVTri) * triMesh.tri_count);
    
    size_t vertexNum = 0;
    for (HalfEdge::Vertex *vertex = m_mesh->firstVertex(); nullptr != vertex; vertex = vertex->_next) {
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
    for (HalfEdge::Face *face = m_mesh->firstFace(); nullptr != face; face = face->_next) {
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
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("qex-tris.obj", "wb");
        for (size_t i = 0; i < triMesh.vertex_count; ++i) {
            const auto &position = triMesh.vertices[i];
            fprintf(fp, "v %f %f %f\n", position.x[0], position.x[1], position.x[2]);
        }
        for (size_t i = 0; i < triMesh.tri_count; ++i) {
            const auto &tris = triMesh.tris[i];
            fprintf(fp, "f %d %d %d\n", 1 + tris.indices[0], 1 + tris.indices[1], 1 + tris.indices[2]);
        }
        fclose(fp);
    }
    {
        FILE *fp = fopen("qex-uv.obj", "wb");
        for (size_t i = 0; i < triangleHalfEdges.size(); ) {
            auto &h0 = triangleHalfEdges[i++];
            auto &h1 = triangleHalfEdges[i++];
            auto &h2 = triangleHalfEdges[i++];
            fprintf(fp, "v %f %f %f\n", h0->startVertexUv[0], 0.0f, h0->startVertexUv[1]);
            fprintf(fp, "v %f %f %f\n", h1->startVertexUv[0], 0.0f, h1->startVertexUv[1]);
            fprintf(fp, "v %f %f %f\n", h2->startVertexUv[0], 0.0f, h2->startVertexUv[1]);
        }
        faceNum = 0;
        for (size_t i = 0; i < triangleHalfEdges.size(); i += 3) {
            fprintf(fp, "f %d %d %d\n", faceNum + 1, faceNum + 2, faceNum + 3);
            faceNum += 3;
        }
        fclose(fp);
    }
    {
        FILE *fp = fopen("qex-valencens.txt", "wb");
        for (size_t i = 0; i < m_vertexValences->size(); ++i) {
            fprintf(fp, "%zu\n", (*m_vertexValences)[i]);
        }
        fclose(fp);
    }
#endif
    
    qex_extractQuadMesh(&triMesh, vertexValences, &quadMesh);
    
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

    free(triMesh.vertices);
    free(triMesh.tris);
    free(triMesh.uvTris);

    free(quadMesh.vertices);
    free(quadMesh.quads);
    
    free(vertexValences);
    
    return true;
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
        bool isTrue = std::abs(degrees - 90.0) <= 45;
        std::cerr << "isCorner degrees:" << degrees << " index:" << index << "/" << loop.size() << " is?" << isTrue << std::endl;
        return isTrue;
    };
    
    std::vector<std::vector<int>> loops;
    igl::boundary_loop(F, loops);
    for (const auto &loop: loops) {
        if (loop.size() > 14)
            continue;
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

}