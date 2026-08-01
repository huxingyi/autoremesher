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
#include <set>
#include <limits>
#include <iostream>
#include <algorithm>
#include <exploragram/hexdom/quad_cover.h>
#include <geogram/mesh/mesh_frame_field.h>
#include <geogram/mesh/mesh_geometry.h>
#include <geogram/mesh/mesh_io.h>
#include <map>
#include <tbb/blocked_range.h>
#include <tbb/combinable.h>
#include <tbb/parallel_for.h>

namespace AutoRemesher {

namespace {

size_t repairFoldedUv(GEO::Mesh& M, GEO::Attribute<GEO::vec2>& U, size_t* foldsBefore)
{
    const GEO::index_t facetCount = M.facets.nb();
    if (0 == facetCount)
        return 0;

    auto signedArea = [&](GEO::index_t f) {
        const GEO::vec2& a = U[3 * f];
        const GEO::vec2& b = U[3 * f + 1];
        const GEO::vec2& c = U[3 * f + 2];
        return 0.5 * ((b[0] - a[0]) * (c[1] - a[1]) - (c[0] - a[0]) * (b[1] - a[1]));
    };

    size_t positive = 0;
    size_t negative = 0;
    for (GEO::index_t f = 0; f < facetCount; ++f) {
        const double area = signedArea(f);
        if (area > 0.0)
            ++positive;
        else if (area < 0.0)
            ++negative;
    }
    const double sign = positive >= negative ? 1.0 : -1.0;

    std::vector<std::vector<GEO::index_t>> cornersOfVertex(M.vertices.nb());
    for (GEO::index_t c = 0; c < M.facet_corners.nb(); ++c)
        cornersOfVertex[M.facet_corners.vertex(c)].push_back(c);

    static const double Rot[4][2][2] = {
        { { 1, 0 }, { 0, 1 } },
        { { 0, 1 }, { -1, 0 } },
        { { -1, 0 }, { 0, -1 } },
        { { 0, -1 }, { 1, 0 } }
    };
    GEO::Attribute<bool> isSingular;
    isSingular.bind_if_is_defined(M.vertices.attributes(), "is_singular");
    GEO::Attribute<GEO::index_t> cornerConstraint;
    cornerConstraint.bind_if_is_defined(M.facet_corners.attributes(), "cnstr");
    GEO::Attribute<GEO::index_t> R;
    R.bind_if_is_defined(M.facet_corners.attributes(), "R");

    struct FanEntry {
        GEO::index_t corner;
        GEO::index_t rotation;
    };
    std::vector<std::vector<FanEntry>> fanOfVertex(M.vertices.nb());
    for (GEO::index_t v = 0; v < M.vertices.nb(); ++v) {
        const auto& corners = cornersOfVertex[v];
        if (corners.empty())
            continue;
        if (isSingular.is_bound() && isSingular[v])
            continue;
        bool constrained = false;
        for (const GEO::index_t c : corners) {
            if (cornerConstraint.is_bound() && 0 != cornerConstraint[c]) {
                constrained = true;
                break;
            }
        }
        if (constrained)
            continue;
        if (!R.is_bound()) {
            fanOfVertex[v].push_back({ corners.front(), 0 });
            for (size_t i = 1; i < corners.size(); ++i)
                fanOfVertex[v].push_back({ corners[i], 0 });
            continue;
        }
        std::vector<FanEntry> fan;
        GEO::index_t c = corners.front();
        GEO::index_t rotation = 0;
        bool closed = false;
        for (size_t step = 0; step < corners.size() + 1; ++step) {
            fan.push_back({ c, rotation });
            rotation = (rotation + R[c]) % 4;
            const GEO::index_t f2 = M.facet_corners.adjacent_facet(c);
            if (GEO::NO_FACET == f2)
                break;
            GEO::index_t next = GEO::NO_CORNER;
            for (GEO::index_t candidate = M.facets.corners_begin(f2);
                 candidate < M.facets.corners_end(f2); ++candidate) {
                if (M.facet_corners.vertex(candidate) == v) {
                    next = candidate;
                    break;
                }
            }
            if (GEO::NO_CORNER == next)
                break;
            c = next;
            if (c == corners.front()) {
                closed = true;
                break;
            }
        }
        if (!closed || 0 != rotation || fan.size() != corners.size())
            continue;
        fanOfVertex[v] = std::move(fan);
    }

    auto tangleAround = [&](GEO::index_t v) {
        double tangle = 0.0;
        for (const GEO::index_t c : cornersOfVertex[v])
            tangle += std::max(0.0, -signedArea(c / 3) * sign);
        return tangle;
    };
    auto worstAround = [&](GEO::index_t v) {
        double worst = std::numeric_limits<double>::max();
        for (const GEO::index_t c : cornersOfVertex[v])
            worst = std::min(worst, signedArea(c / 3) * sign);
        return worst;
    };

    std::vector<GEO::vec2> seamTransitionBefore(M.facet_corners.nb(), GEO::vec2(0.0, 0.0));
    auto seamTransition = [&](GEO::index_t c) {
        const GEO::index_t f2 = M.facet_corners.adjacent_facet(c);
        if (GEO::NO_FACET == f2 || !R.is_bound())
            return GEO::vec2(0.0, 0.0);
        const GEO::index_t e2 = M.facets.find_adjacent(f2, c / 3);
        if (GEO::NO_FACET == e2)
            return GEO::vec2(0.0, 0.0);
        const GEO::index_t c2 = M.facets.corners_begin(f2) + e2;
        const GEO::index_t c3 = M.facets.next_corner_around_facet(f2, c2);
        const auto& rotation = Rot[R[c] % 4];
        return GEO::vec2(
            U[c][0] - (rotation[0][0] * U[c3][0] + rotation[0][1] * U[c3][1]),
            U[c][1] - (rotation[1][0] * U[c3][0] + rotation[1][1] * U[c3][1]));
    };
    for (GEO::index_t c = 0; c < M.facet_corners.nb(); ++c)
        seamTransitionBefore[c] = seamTransition(c);

    constexpr size_t kMaximumSweeps = 256;
    const double alphas[] = { 1.0, 0.5, 0.25, 0.1 };
    size_t folded = 0;
    for (size_t sweep = 0; sweep < kMaximumSweeps; ++sweep) {
        std::vector<GEO::index_t> foldedFacets;
        for (GEO::index_t f = 0; f < facetCount; ++f) {
            if (signedArea(f) * sign <= 0.0)
                foldedFacets.push_back(f);
        }
        folded = foldedFacets.size();
        if (0 == sweep && nullptr != foldsBefore)
            *foldsBefore = folded;
        if (foldedFacets.empty())
            break;

        std::set<GEO::index_t> candidates;
        for (const GEO::index_t f : foldedFacets) {
            for (GEO::index_t k = 0; k < 3; ++k) {
                const GEO::index_t v = M.facet_corners.vertex(3 * f + k);
                candidates.insert(v);
                for (const GEO::index_t c : cornersOfVertex[v]) {
                    for (GEO::index_t j = 0; j < 3; ++j)
                        candidates.insert(M.facet_corners.vertex(3 * (c / 3) + j));
                }
            }
        }

        bool improved = false;
        for (const GEO::index_t v : candidates) {
            const auto& fan = fanOfVertex[v];
            if (fan.empty())
                continue;
            const auto& corners = cornersOfVertex[v];
            const double tangleBefore = tangleAround(v);
            const double worstBefore = worstAround(v);
            double sumX = 0.0;
            double sumY = 0.0;
            size_t neighborCount = 0;
            for (const GEO::index_t c : corners) {
                for (GEO::index_t j = 0; j < 3; ++j) {
                    const GEO::index_t other = 3 * (c / 3) + j;
                    if (other == c)
                        continue;
                    sumX += U[other][0];
                    sumY += U[other][1];
                    ++neighborCount;
                }
            }
            if (0 == neighborCount)
                continue;
            const GEO::vec2 reference = U[fan.front().corner];
            const GEO::vec2 target(sumX / neighborCount, sumY / neighborCount);
            std::vector<GEO::vec2> before(fan.size());
            for (size_t i = 0; i < fan.size(); ++i)
                before[i] = U[fan[i].corner];
            for (const double alpha : alphas) {
                const double stepX = alpha * (target[0] - reference[0]);
                const double stepY = alpha * (target[1] - reference[1]);
                for (size_t i = 0; i < fan.size(); ++i) {
                    const auto& rotation = Rot[(4 - fan[i].rotation) % 4];
                    U[fan[i].corner] = GEO::vec2(
                        before[i][0] + rotation[0][0] * stepX + rotation[0][1] * stepY,
                        before[i][1] + rotation[1][0] * stepX + rotation[1][1] * stepY);
                }
                const double tangleAfter = tangleAround(v);
                const bool better = tangleAfter < tangleBefore
                    || (tangleAfter <= tangleBefore && worstAround(v) > worstBefore);
                if (better) {
                    improved = true;
                    break;
                }
                for (size_t i = 0; i < fan.size(); ++i)
                    U[fan[i].corner] = before[i];
            }
        }
        if (!improved) {
            if (nullptr != getenv("AUTOREMESHER_DEBUG_FILL")) {
                size_t allMovable = 0;
                size_t touchesSeam = 0;
                size_t touchesSingular = 0;
                size_t touchesConstrained = 0;
                for (const GEO::index_t f : foldedFacets) {
                    bool everyoneMovable = true;
                    bool singular = false;
                    bool constrained = false;
                    for (GEO::index_t k = 0; k < 3; ++k) {
                        const GEO::index_t v = M.facet_corners.vertex(3 * f + k);
                        if (fanOfVertex[v].empty())
                            everyoneMovable = false;
                        if (isSingular.is_bound() && isSingular[v])
                            singular = true;
                        if (cornerConstraint.is_bound() && 0 != cornerConstraint[3 * f + k])
                            constrained = true;
                    }
                    if (everyoneMovable)
                        ++allMovable;
                    else if (singular)
                        ++touchesSingular;
                    else if (constrained)
                        ++touchesConstrained;
                    else
                        ++touchesSeam;
                }
                std::cerr << "  fold repair stalled after " << sweep << " sweep(s): "
                          << allMovable << " free, " << touchesSingular << " at a cone, "
                          << touchesConstrained << " on a sharp edge, " << touchesSeam
                          << " on a cut" << std::endl;
            }
            break;
        }
    }
    if (nullptr != getenv("AUTOREMESHER_DEBUG_FILL")) {
        double worstDrift = 0.0;
        for (GEO::index_t c = 0; c < M.facet_corners.nb(); ++c) {
            const GEO::vec2 now = seamTransition(c);
            worstDrift = std::max(worstDrift,
                std::max(std::abs(now[0] - seamTransitionBefore[c][0]),
                    std::abs(now[1] - seamTransitionBefore[c][1])));
        }
        std::cerr << "  seamlessness preserved to " << worstDrift
                  << " (anything but ~0 means a cut was torn)" << std::endl;
    }
    return folded;
}

inline double quadraticForm(const double* T, const GEO::vec3& t)
{
    return T[0] * t.x * t.x + T[2] * t.y * t.y + T[5] * t.z * t.z
        + 2.0 * (T[1] * t.x * t.y + T[3] * t.x * t.z + T[4] * t.y * t.z);
}

void computeFaceAnisotropyField(const GEO::Mesh& M,
    const GEO::Attribute<GEO::vec3>& B,
    double anisotropy,
    double maxAspectRatio,
    std::vector<double>* scalingU,
    std::vector<double>* scalingV)
{
    scalingU->assign(M.facets.nb(), 1.0);
    scalingV->assign(M.facets.nb(), 1.0);
    if (anisotropy <= 0.0 || maxAspectRatio <= 1.0)
        return;

    std::vector<double> tensors(M.vertices.nb() * 6, 0.0);
    for (GEO::index_t f1 = 0; f1 < M.facets.nb(); ++f1) {
        for (GEO::index_t c = M.facets.corners_begin(f1);
             c < M.facets.corners_end(f1); ++c) {
            const GEO::index_t f2 = M.facet_corners.adjacent_facet(c);
            if (GEO::NO_FACET == f2 || f2 < f1)
                continue;
            const GEO::vec3 e = GEO::Geom::mesh_corner_vector(M, c);
            const double edgeLength = GEO::length(e);
            if (edgeLength <= 0.0)
                continue;
            const GEO::vec3 d = e / edgeLength;
            const double weight = edgeLength * GEO::Geom::mesh_normal_angle(M, c);
            const double contribution[6] = {
                weight * d.x * d.x, weight * d.x * d.y, weight * d.y * d.y,
                weight * d.x * d.z, weight * d.y * d.z, weight * d.z * d.z
            };
            const GEO::index_t v1 = M.facet_corners.vertex(c);
            const GEO::index_t v2 = M.facet_corners.vertex(
                M.facets.next_corner_around_facet(f1, c));
            for (GEO::index_t i = 0; i < 6; ++i) {
                tensors[6 * v1 + i] += contribution[i];
                tensors[6 * v2 + i] += contribution[i];
            }
        }
    }

    {
        std::vector<std::vector<GEO::index_t>> vertexNeighbors(M.vertices.nb());
        for (GEO::index_t f = 0; f < M.facets.nb(); ++f) {
            for (GEO::index_t c = M.facets.corners_begin(f);
                 c < M.facets.corners_end(f); ++c) {
                const GEO::index_t v1 = M.facet_corners.vertex(c);
                const GEO::index_t v2 = M.facet_corners.vertex(
                    M.facets.next_corner_around_facet(f, c));
                vertexNeighbors[v1].push_back(v2);
                vertexNeighbors[v2].push_back(v1);
            }
        }
        constexpr GEO::index_t smoothingPasses = 12;
        constexpr double lambda = 0.5;
        std::vector<double> smoothed(tensors.size());
        for (GEO::index_t pass = 0; pass < smoothingPasses; ++pass) {
            tbb::parallel_for(tbb::blocked_range<GEO::index_t>(0, M.vertices.nb()),
                [&](const tbb::blocked_range<GEO::index_t>& range) {
                    for (GEO::index_t v = range.begin(); v != range.end(); ++v) {
                        const auto& neighbors = vertexNeighbors[v];
                        if (neighbors.empty()) {
                            for (GEO::index_t i = 0; i < 6; ++i)
                                smoothed[6 * v + i] = tensors[6 * v + i];
                            continue;
                        }
                        double average[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
                        for (const GEO::index_t w : neighbors) {
                            for (GEO::index_t i = 0; i < 6; ++i)
                                average[i] += tensors[6 * w + i];
                        }
                        for (GEO::index_t i = 0; i < 6; ++i) {
                            smoothed[6 * v + i] = (1.0 - lambda) * tensors[6 * v + i]
                                + lambda * average[i] / neighbors.size();
                        }
                    }
                });
            tensors.swap(smoothed);
        }
    }

    std::vector<double> curvatureAlongU(M.facets.nb(), 0.0);
    std::vector<double> curvatureAlongV(M.facets.nb(), 0.0);
    double totalCurvature = 0.0;
    for (GEO::index_t f = 0; f < M.facets.nb(); ++f) {
        double T[6] = { 0.0, 0.0, 0.0, 0.0, 0.0, 0.0 };
        for (GEO::index_t c = M.facets.corners_begin(f);
             c < M.facets.corners_end(f); ++c) {
            const GEO::index_t v = M.facet_corners.vertex(c);
            for (GEO::index_t i = 0; i < 6; ++i)
                T[i] += tensors[6 * v + i];
        }
        const GEO::vec3 N = GEO::normalize(GEO::Geom::mesh_facet_normal(M, f));
        const GEO::vec3 Bf = GEO::normalize(B[f]);
        const GEO::vec3 BTf = GEO::cross(N, Bf);
        curvatureAlongU[f] = std::fabs(quadraticForm(T, BTf));
        curvatureAlongV[f] = std::fabs(quadraticForm(T, Bf));
        totalCurvature += curvatureAlongU[f] + curvatureAlongV[f];
    }
    if (totalCurvature <= 0.0)
        return;

    const double floorCurvature = 1e-3 * totalCurvature / (2.0 * M.facets.nb());

    const double maxRho = std::sqrt(maxAspectRatio);
    for (GEO::index_t f = 0; f < M.facets.nb(); ++f) {
        double rho = std::pow(
            (curvatureAlongV[f] + floorCurvature)
                / (curvatureAlongU[f] + floorCurvature),
            0.25);
        rho = std::pow(rho, anisotropy);
        if (rho < 1.0 / maxRho)
            rho = 1.0 / maxRho;
        else if (rho > maxRho)
            rho = maxRho;
        (*scalingU)[f] = rho;
        (*scalingV)[f] = 1.0 / rho;
    }

    if (nullptr != getenv("AUTOREMESHER_DEBUG_ANISOTROPY")) {
        std::vector<double> sorted(*scalingU);
        std::sort(sorted.begin(), sorted.end());
        size_t clampedHigh = 0;
        size_t clampedLow = 0;
        for (const double rho : sorted) {
            if (rho >= maxRho - 1e-9)
                ++clampedHigh;
            else if (rho <= 1.0 / maxRho + 1e-9)
                ++clampedLow;
        }
        std::cerr << "  [Aniso] su/sv over " << sorted.size() << " facets:"
                  << " p10=" << sorted[sorted.size() / 10] * sorted[sorted.size() / 10]
                  << " median=" << sorted[sorted.size() / 2] * sorted[sorted.size() / 2]
                  << " p90=" << sorted[9 * sorted.size() / 10] * sorted[9 * sorted.size() / 10]
                  << "; clamped " << clampedHigh << " high, " << clampedLow << " low"
                  << std::endl;
    }
}

}

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
        std::vector<Vector3> faceNormals(m_triangles->size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, m_triangles->size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const auto& it = (*m_triangles)[i];
                    faceNormals[i] = Vector3::normal(
                        (*m_vertices)[it[0]], (*m_vertices)[it[1]], (*m_vertices)[it[2]]);
                }
            });
        for (size_t i = 0; i < m_triangles->size(); ++i) {
            const auto& it = (*m_triangles)[i];
            vertexNormals[it[0]] += faceNormals[i];
            vertexNormals[it[1]] += faceNormals[i];
            vertexNormals[it[2]] += faceNormals[i];
        }
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

    if (m_anisotropy > 0.0) {
        std::vector<double> faceScalingU;
        std::vector<double> faceScalingV;
        computeFaceAnisotropyField(M, B, m_anisotropy, m_maxAspectRatio,
            &faceScalingU, &faceScalingV);
        GEO::Attribute<double> facetScalingU(M.facets.attributes(), "adaptive_scaling_u");
        GEO::Attribute<double> facetScalingV(M.facets.attributes(), "adaptive_scaling_v");
        for (size_t i = 0; i < faceScalingU.size(); ++i) {
            facetScalingU[i] = faceScalingU[i];
            facetScalingV[i] = faceScalingV[i];
        }
    }

    GEO::Attribute<GEO::vec2> U(M.facet_corners.attributes(), "U");
    bool constrain_hard_edges = true;
    bool do_brush = true;
    bool integer_constraints = true;
    GEO::GlobalParam2d::quad_cover(&M, B, U, m_scaling, constrain_hard_edges, do_brush, integer_constraints, m_sharpEdgeDegrees);

    if (nullptr != getenv("AUTOREMESHER_DEBUG_ANISOTROPY")) {
        std::vector<double> realized;
        for (GEO::index_t f = 0; f < M.facets.nb(); ++f) {
            const GEO::index_t c0 = M.facets.corners_begin(f);
            const GEO::vec3 p0(M.vertices.point_ptr(M.facet_corners.vertex(c0)));
            const GEO::vec3 p1(M.vertices.point_ptr(M.facet_corners.vertex(c0 + 1)));
            const GEO::vec3 p2(M.vertices.point_ptr(M.facet_corners.vertex(c0 + 2)));
            const GEO::vec3 N = GEO::normalize(GEO::Geom::mesh_facet_normal(M, f));
            const GEO::vec3 Bf = GEO::normalize(B[f]);
            const GEO::vec3 BTf = GEO::cross(N, Bf);
            const GEO::vec3 e1 = p1 - p0;
            const GEO::vec3 e2 = p2 - p0;
            const double a11 = GEO::dot(e1, Bf), a21 = GEO::dot(e1, BTf);
            const double a12 = GEO::dot(e2, Bf), a22 = GEO::dot(e2, BTf);
            const double det = a11 * a22 - a12 * a21;
            if (std::fabs(det) < 1e-12)
                continue;
            const GEO::vec2 d1 = U[c0 + 1] - U[c0];
            const GEO::vec2 d2 = U[c0 + 2] - U[c0];
            const double j11 = (d1[0] * a22 - d2[0] * a21) / det;
            const double j12 = (-d1[0] * a12 + d2[0] * a11) / det;
            const double j21 = (d1[1] * a22 - d2[1] * a21) / det;
            const double j22 = (-d1[1] * a12 + d2[1] * a11) / det;
            const double e = (j11 * j11 + j12 * j12 + j21 * j21 + j22 * j22) / 2.0;
            const double g = (j11 * j22 - j12 * j21);
            const double disc = std::sqrt(std::max(0.0, e * e - g * g));
            const double s1 = std::sqrt(std::max(1e-18, e + disc));
            const double s2 = std::sqrt(std::max(1e-18, e - disc));
            if (s2 > 1e-9)
                realized.push_back(s1 / s2);
        }
        std::sort(realized.begin(), realized.end());
        if (!realized.empty()) {
            std::cerr << "  [Aniso] realized uv aspect ratio: median="
                      << realized[realized.size() / 2]
                      << " p90=" << realized[9 * realized.size() / 10]
                      << " over " << realized.size() << " facets" << std::endl;
        }
    }

    m_originalTriangleUvs.clear();
    m_originalTriangleUvs.reserve(m_triangles->size());
    for (size_t i = 0; i < m_triangles->size(); ++i) {
        const auto& a = U[3 * i + 0];
        const auto& b = U[3 * i + 1];
        const auto& c = U[3 * i + 2];
        m_originalTriangleUvs.push_back({ Vector2 { a[0], a[1] },
            Vector2 { b[0], b[1] },
            Vector2 { c[0], c[1] } });
    }

    {
        size_t foldsBefore = 0;
        const size_t foldsAfter = repairFoldedUv(M, U, &foldsBefore);
        if (foldsBefore > 0) {
            std::cerr << "Repaired parametrization fold-over: " << foldsBefore << " -> "
                      << foldsAfter << " folded triangle(s)" << std::endl;
        }
        if (nullptr != getenv("AUTOREMESHER_DEBUG_FILL")) {
            size_t movedTriangles = 0;
            for (size_t i = 0; i < m_originalTriangleUvs.size(); ++i) {
                bool moved = false;
                for (size_t k = 0; k < 3 && !moved; ++k) {
                    if (m_originalTriangleUvs[i][k].x() != U[3 * i + k][0]
                        || m_originalTriangleUvs[i][k].y() != U[3 * i + k][1])
                        moved = true;
                }
                if (moved)
                    ++movedTriangles;
            }
            std::cerr << "  [Param] preview: " << movedTriangles << " of "
                      << m_originalTriangleUvs.size()
                      << " triangle(s) differ between the original and repaired uv" << std::endl;
        }
    }

    // Capture singular vertex positions from the v_is_singular attribute
    m_singularVertexPositions.clear();
    {
        GEO::Attribute<bool> isSingular(M.vertices.attributes(), "is_singular");
        if (isSingular.is_bound()) {
            for (GEO::index_t v = 0; v < M.vertices.nb(); ++v) {
                if (isSingular[v]) {
                    m_singularVertexPositions.push_back(
                        Vector3 { (*m_vertices)[v].x(), (*m_vertices)[v].y(), (*m_vertices)[v].z() });
                }
            }
        }
    }

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