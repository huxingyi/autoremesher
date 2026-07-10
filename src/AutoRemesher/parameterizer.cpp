/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <AutoRemesher/Parameterizer>
#include <cmath>
#include <exploragram/hexdom/quad_cover.h>
#include <geogram/mesh/mesh_frame_field.h>
#include <geogram/mesh/mesh_io.h>
#include <map>
#include <tbb/blocked_range.h>
#include <tbb/combinable.h>
#include <tbb/parallel_for.h>

namespace AutoRemesher {

std::vector<double> Parameterizer::computeFaceScalingField(const std::vector<Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles,
    const std::vector<Vector3>& vertexNormals,
    const std::map<size_t, std::vector<size_t>>& faceAroundVertexMap) const
{
    std::vector<double> faceScaling(triangles.size(), 1.0);
    if (m_adaptivity <= 0.0 || vertices.empty())
        return faceScaling;

    std::vector<double> vertexCurvature(vertices.size(), 0.0);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, vertices.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t v = range.begin(); v != range.end(); ++v) {
                auto findFaces = faceAroundVertexMap.find(v);
                if (findFaces == faceAroundVertexMap.end())
                    continue;
                const auto& normalV = vertexNormals[v];
                double maxCurvature = 0.0;
                for (const auto& faceIndex : findFaces->second) {
                    for (const auto& u : triangles[faceIndex]) {
                        if (u == v)
                            continue;
                        double distance = (vertices[u] - vertices[v]).length();
                        if (distance <= 0.0)
                            continue;
                        double cosAngle = Vector3::dotProduct(normalV, vertexNormals[u]);
                        if (cosAngle > 1.0)
                            cosAngle = 1.0;
                        else if (cosAngle < -1.0)
                            cosAngle = -1.0;
                        double curvature = std::acos(cosAngle) / distance;
                        if (curvature > maxCurvature)
                            maxCurvature = curvature;
                    }
                }
                vertexCurvature[v] = maxCurvature;
            }
        });

    double sumCurvature = 0.0;
    for (const auto& it : vertexCurvature)
        sumCurvature += it;
    double averageCurvature = sumCurvature / vertexCurvature.size();
    if (averageCurvature <= 0.0)
        return faceScaling;

    const double minRatio = 0.3;
    const double maxRatio = 3.0;
    tbb::parallel_for(tbb::blocked_range<size_t>(0, triangles.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                const auto& triangle = triangles[i];
                double faceCurvature = 0.0;
                for (const auto& v : triangle)
                    faceCurvature += vertexCurvature[v];
                faceCurvature /= triangle.size();
                double normalized = faceCurvature / averageCurvature;
                if (normalized < 1e-3)
                    normalized = 1e-3;
                double multiplier = std::pow(normalized, -m_adaptivity);
                if (multiplier < minRatio)
                    multiplier = minRatio;
                else if (multiplier > maxRatio)
                    multiplier = maxRatio;
                faceScaling[i] = multiplier;
            }
        });

    return faceScaling;
}

bool Parameterizer::parameterize()
{
#if AUTO_REMESHER_DEV
    {
        FILE* fp = fopen("debug-input-for-parameterization.obj", "wb");
        for (size_t i = 0; i < m_vertices->size(); ++i) {
            const auto& vertex = (*m_vertices)[i];
            fprintf(fp, "v %f %f %f\n", vertex[0], vertex[1], vertex[2]);
        }
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto& indices = (*m_triangles)[i];
            fprintf(fp, "f %zu %zu %zu\n", indices[0] + 1, indices[1] + 1, indices[2] + 1);
        }
        fclose(fp);
    }
#endif

    std::vector<Vector3> vertexNormals(m_vertices->size());
    {
        tbb::combinable<std::vector<Vector3>> perThreadNormals(
            [&]() { return std::vector<Vector3>(m_vertices->size()); });
        tbb::parallel_for(tbb::blocked_range<size_t>(0, m_triangles->size()),
            [&](const tbb::blocked_range<size_t>& range) {
                auto& local = perThreadNormals.local();
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const auto& it = (*m_triangles)[i];
                    Vector3 n = Vector3::normal(
                        (*m_vertices)[it[0]], (*m_vertices)[it[1]], (*m_vertices)[it[2]]);
                    local[it[0]] += n;
                    local[it[1]] += n;
                    local[it[2]] += n;
                }
            });
        perThreadNormals.combine_each([&](const std::vector<Vector3>& local) {
            for (size_t i = 0; i < vertexNormals.size(); ++i)
                vertexNormals[i] += local[i];
        });
        tbb::parallel_for(tbb::blocked_range<size_t>(0, vertexNormals.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i)
                    vertexNormals[i].normalize();
            });
    }

    std::map<size_t, std::vector<size_t>> faceAroundVertexMap;
    for (size_t i = 0; i < m_triangles->size(); ++i) {
        const auto& it = (*m_triangles)[i];
        faceAroundVertexMap[it[0]].push_back(i);
        faceAroundVertexMap[it[1]].push_back(i);
        faceAroundVertexMap[it[2]].push_back(i);
    }

    std::vector<double> faceScalingField = computeFaceScalingField(*m_vertices,
        *m_triangles, vertexNormals, faceAroundVertexMap);

    GEO::Mesh M;
    auto makeMesh = [](GEO::Mesh& M, const std::vector<Vector3>& vertices, const std::vector<std::vector<size_t>>& triangles) {
        M.vertices.set_dimension(3);
        std::vector<GEO::index_t> meshVertices(vertices.size());
        for (size_t i = 0; i < vertices.size(); ++i) {
            const auto& row = vertices[i];
            auto v = M.vertices.create_vertex();
            meshVertices[i] = v;
            double coords[] = { row[0], row[1], row[2] };
            if (M.vertices.single_precision()) {
                float* p = M.vertices.single_precision_point_ptr(v);
                for (GEO::index_t c = 0; c < 3; ++c)
                    p[c] = float(coords[c]);
            } else {
                double* p = M.vertices.point_ptr(v);
                for (GEO::index_t c = 0; c < 3; ++c)
                    p[c] = coords[c];
            }
        }
        for (size_t i = 0; i < triangles.size(); ++i) {
            const auto& row = triangles[i];
            GEO::index_t f = M.facets.create_polygon(3);
            for (GEO::index_t lv = 0; lv < 3; ++lv)
                M.facets.set_vertex(f, lv, meshVertices[row[lv]]);
        }
        M.facets.connect();

        for (GEO::index_t c = 0; c < M.facet_corners.nb(); ++c) {
            GEO::index_t f2 = M.facet_corners.adjacent_facet(c);
            if (f2 == GEO::NO_FACET)
                continue;
            GEO::index_t f1 = c / 3;
            GEO::index_t e2 = M.facets.find_adjacent(f2, f1);
            if (e2 == GEO::NO_FACET) {
                M.facet_corners.set_adjacent_facet(c, GEO::NO_FACET);
                continue;
            }
            GEO::index_t c2 = M.facets.corners_begin(f2) + e2;
            GEO::index_t c3 = M.facets.next_corner_around_facet(f2, c2);
            if (M.facet_corners.vertex(c) != M.facet_corners.vertex(c3)) {
                // Inconsistently-oriented (non-manifold) edge: break it on BOTH
                // sides. c2 is the reciprocal corner -- find_adjacent guarantees
                // its adjacent_facet is f1 -- so c2 (not a neighbour of it) is the
                // corner that must also be cleared. Clearing the wrong corner
                // leaves c2 -> f1 dangling and trips geogram's quad_cover
                // adjacency assertion (quad_cover.cpp:204) on non-manifold meshes.
                M.facet_corners.set_adjacent_facet(c, GEO::NO_FACET);
                M.facet_corners.set_adjacent_facet(c2, GEO::NO_FACET);
            }
        }
    };
    makeMesh(M, *m_vertices, *m_triangles);

    GEO::Attribute<GEO::vec3> B(M.facets.attributes(), "B");
    if (nullptr == m_triangleFieldVectors) {
        GEO::Mesh originalM;
        makeMesh(originalM, *m_vertices, *m_triangles);

        GEO::FrameField FF;
        FF.set_use_spatial_search(false);
        FF.create_from_surface_mesh(originalM, false, m_sharpEdgeDegrees);
        const auto& frames = FF.frames();
        for (GEO::index_t f : originalM.facets) {
            B[f] = GEO::vec3(
                frames[9 * f + 0],
                frames[9 * f + 1],
                frames[9 * f + 2]);
        }
    } else {
        for (size_t i = 0; i < m_triangleFieldVectors->size(); ++i) {
            const auto& row = (*m_triangleFieldVectors)[i];
            B[i] = GEO::vec3(row[0], row[1], row[2]);
        }
    }

    GEO::Attribute<double> facetScaling(M.facets.attributes(), "adaptive_scaling");
    for (size_t i = 0; i < faceScalingField.size(); ++i)
        facetScaling[i] = faceScalingField[i];

    GEO::Attribute<GEO::vec2> U(M.facet_corners.attributes(), "U");
    bool constrain_hard_edges = true;
    bool do_brush = true;
    bool integer_constraints = true;
    GEO::GlobalParam2d::quad_cover(&M, B, U, m_scaling, constrain_hard_edges, do_brush, integer_constraints, m_sharpEdgeDegrees);

    delete m_triangleUvs;
    m_triangleUvs = new std::vector<std::vector<Vector2>>;
    m_triangleUvs->reserve(m_triangles->size());
    size_t faceCornerIndex = 0;
    for (size_t i = 0; i < m_triangles->size(); ++i) {
        const auto& v0 = U[faceCornerIndex++];
        const auto& v1 = U[faceCornerIndex++];
        const auto& v2 = U[faceCornerIndex++];
        m_triangleUvs->push_back({ Vector2 { v0[0], v0[1] },
            Vector2 { v1[0], v1[1] },
            Vector2 { v2[0], v2[1] } });
    }

#if AUTO_REMESHER_DEV
    {
        FILE* fp = fopen("debug-uv.obj", "wb");
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto& uv = (*m_triangleUvs)[i];
            fprintf(fp, "v %f %f %f\n", uv[0][0], 0.0, uv[0][1]);
            fprintf(fp, "v %f %f %f\n", uv[1][0], 0.0, uv[1][1]);
            fprintf(fp, "v %f %f %f\n", uv[2][0], 0.0, uv[2][1]);
        }
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            fprintf(fp, "f %zu %zu %zu\n", i * 3 + 1, i * 3 + 2, i * 3 + 3);
        }
        fclose(fp);
    }

    auto normalizeUv = [](double x) {
        return 0.5 + x * 0.5;
    };
    {
        FILE* fp = fopen("quadcover.obj", "wb");
        fprintf(fp, "mtllib quadcover.mtl\n");
        fprintf(fp, "usemtl quadcover\n");
        for (size_t i = 0; i < m_vertices->size(); ++i) {
            const auto& row = (*m_vertices)[i];
            fprintf(fp, "v %f %f %f\n", row[0], row[1], row[2]);
        }
        size_t faceCornerIndex = 0;
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto& v0 = U[faceCornerIndex++];
            const auto& v1 = U[faceCornerIndex++];
            const auto& v2 = U[faceCornerIndex++];
            fprintf(fp, "vt %f %f\n", normalizeUv(v0[0]), normalizeUv(v0[1]));
            fprintf(fp, "vt %f %f\n", normalizeUv(v1[0]), normalizeUv(v1[1]));
            fprintf(fp, "vt %f %f\n", normalizeUv(v2[0]), normalizeUv(v2[1]));
        }

        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto& row = (*m_triangles)[i];
            fprintf(fp, "f %zu/%zu %zu/%zu %zu/%zu\n",
                1 + row[0], i * 3 + 1,
                1 + row[1], i * 3 + 2,
                1 + row[2], i * 3 + 3);
        }
        fclose(fp);
    }
    {
        FILE* fp = fopen("quadcover.mtl", "wb");
        fprintf(fp, "newmtl quadcover\n");
        fprintf(fp, "Ka 1.000 1.000 1.000\n");
        fprintf(fp, "Kd 1.000 1.000 1.000\n");
        fprintf(fp, "Ks 0.000 0.000 0.000\n");
        fprintf(fp, "d 1.0\n");
        fprintf(fp, "illum 2\n");
        fprintf(fp, "map_Kd crossuv.png\n");
        fclose(fp);
    }
#endif

    return false;
}

}