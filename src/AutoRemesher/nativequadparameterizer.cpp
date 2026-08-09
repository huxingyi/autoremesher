/*
 *  Copyright (c) 2026 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved.
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
#include <AutoRemesher/ConstrainedLeastSquares>
#include <AutoRemesher/MixedIntegerLeastSquares>
#include <AutoRemesher/NativeQuadParameterizer>
#include <AutoRemesher/SurfaceMesh>

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <queue>
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>

namespace AutoRemesher {
namespace {
    Vector3 unit(const Vector3& v, const Vector3& fallback)
    {
        return v.length() < 1e-12 ? fallback.normalized() : v.normalized();
    }

    int edgeQuarterTurn(const SurfaceMesh& mesh, size_t corner,
        const std::vector<Vector3>& field, const std::vector<Vector3>& normals)
    {
        const size_t opposite = mesh.oppositeCorner(corner);
        if (opposite == SurfaceMesh::npos)
            return 0;
        const size_t f = mesh.cornerFace(corner), g = mesh.cornerFace(opposite);
        if (f > g) {
            const int r = edgeQuarterTurn(mesh, opposite, field, normals);
            return (4 - r) % 4;
        }
        size_t v0 = mesh.cornerVertex(corner), v1 = mesh.cornerVertex(mesh.nextCorner(corner));
        if (v1 < v0)
            std::swap(v0, v1);
        const Vector3 e = unit(mesh.position(v1) - mesh.position(v0), Vector3(1, 0, 0));
        const Vector3 y0 = unit(Vector3::crossProduct(normals[f], e), Vector3(0, 1, 0));
        const Vector3 yg = unit(Vector3::crossProduct(normals[g], e), Vector3(0, 1, 0));
        const double a0 = std::atan2(Vector3::dotProduct(field[f], y0), Vector3::dotProduct(field[f], e));
        int best = 0;
        double bestError = 1e100;
        Vector3 candidate = field[g];
        for (int r = 0; r < 4; ++r) {
            const double ag = std::atan2(Vector3::dotProduct(candidate, yg), Vector3::dotProduct(candidate, e));
            double d = std::fabs(a0 - ag);
            while (d > M_PI)
                d = std::fabs(d - 2.0 * M_PI);
            if (d < bestError) {
                bestError = d;
                best = r;
            }
            candidate = Vector3::crossProduct(normals[g], candidate);
        }
        return best;
    }

    void rotateCoordinate(int r, double x, double y, double* rx, double* ry)
    {
        switch ((r % 4 + 4) % 4) {
        case 0:
            *rx = x;
            *ry = y;
            break;
        case 1:
            *rx = y;
            *ry = -x;
            break;
        case 2:
            *rx = -x;
            *ry = -y;
            break;
        default:
            *rx = -y;
            *ry = x;
            break;
        }
    }

    enum EdgeConstraint { ConstraintNone = 0,
        ConstraintU = 1,
        ConstraintV = 2 };
    int edgeConstraint(const SurfaceMesh& mesh, size_t c, const std::vector<Vector3>& field,
        const std::vector<Vector3>& normals, double hardEdgeDegrees)
    {
        if (mesh.oppositeCorner(c) != SurfaceMesh::npos && std::fabs(mesh.normalAngle(c)) * 180.0 / M_PI < hardEdgeDegrees)
            return ConstraintNone;
        const size_t f = mesh.cornerFace(c);
        const Vector3 edge = unit(mesh.edgeVector(c), Vector3(1, 0, 0));
        const Vector3 b = unit(field[f], edge);
        const Vector3 br = unit(Vector3::crossProduct(normals[f], b), Vector3(0, 1, 0));
        const bool alongB = std::acos(std::max(-1.0, std::min(1.0, std::fabs(Vector3::dotProduct(edge, b))))) < 10.0 * M_PI / 180.0;
        const bool alongBr = std::acos(std::max(-1.0, std::min(1.0, std::fabs(Vector3::dotProduct(edge, br))))) < 10.0 * M_PI / 180.0;
        if (alongB == alongBr)
            return ConstraintNone;
        return alongB ? ConstraintV : ConstraintU;
    }

    size_t repairFoldedUv(const SurfaceMesh& mesh, const std::vector<int>& rotation,
        const std::vector<char>& singular, const std::vector<char>& constrained,
        std::vector<double>* uv, size_t* before)
    {
        const size_t faces = mesh.faceCount();
        auto area = [&](size_t f) { const size_t c=3*f;
        return .5*(((*uv)[2*(c+1)]-(*uv)[2*c])*((*uv)[2*(c+2)+1]-(*uv)[2*c+1])-((*uv)[2*(c+1)+1]-(*uv)[2*c+1])*((*uv)[2*(c+2)]-(*uv)[2*c])); };
        size_t positive = 0, negative = 0;
        for (size_t f = 0; f < faces; ++f) {
            const double a = area(f);
            if (a > 0)
                ++positive;
            else if (a < 0)
                ++negative;
        }
        const double sign = positive >= negative ? 1.0 : -1.0;
        struct Fan {
            size_t corner;
            int turn;
        };
        std::vector<std::vector<Fan>> fans(mesh.vertexCount());
        for (size_t v = 0; v < mesh.vertexCount(); ++v) {
            const auto& incident = mesh.cornersAroundVertex(v);
            if (incident.empty() || singular[v])
                continue;
            bool hard = false;
            for (size_t c : incident)
                if (constrained[c]) {
                    hard = true;
                    break;
                }
            if (hard)
                continue;
            std::vector<Fan> fan;
            size_t start = incident.front(), c = start;
            int turn = 0;
            for (size_t step = 0; step <= incident.size(); ++step) {
                fan.push_back({ c, turn });
                const size_t opposite = mesh.oppositeCorner(c);
                if (opposite == SurfaceMesh::npos) {
                    fan.clear();
                    break;
                }
                turn = (turn + rotation[c]) % 4;
                c = mesh.nextCorner(opposite);
                if (c == start)
                    break;
            }
            if (!fan.empty() && c == start && turn == 0 && fan.size() == incident.size())
                fans[v] = std::move(fan);
        }
        auto tangle = [&](size_t v) { double value=0; for(size_t c:mesh.cornersAroundVertex(v))value+=std::max(0.0,-area(c/3)*sign);return value; };
        auto worst = [&](size_t v) { double value=std::numeric_limits<double>::max();for(size_t c:mesh.cornersAroundVertex(v))value=std::min(value,area(c/3)*sign);return value; };
        size_t folded = 0;
        for (size_t sweep = 0; sweep < 256; ++sweep) {
            std::vector<size_t> foldedFaces;
            for (size_t f = 0; f < faces; ++f)
                if (area(f) * sign <= 0)
                    foldedFaces.push_back(f);
            folded = foldedFaces.size();
            if (sweep == 0 && before)
                *before = folded;
            if (foldedFaces.empty())
                break;
            std::vector<char> candidate(mesh.vertexCount(), 0);
            for (size_t f : foldedFaces)
                for (size_t l = 0; l < 3; ++l) {
                    const size_t v = mesh.cornerVertex(3 * f + l);
                    candidate[v] = 1;
                    for (size_t c : mesh.cornersAroundVertex(v))
                        for (size_t j = 0; j < 3; ++j)
                            candidate[mesh.cornerVertex(3 * (c / 3) + j)] = 1;
                }
            bool improved = false;
            for (size_t v = 0; v < mesh.vertexCount(); ++v)
                if (candidate[v] && !fans[v].empty()) {
                    const double oldTangle = tangle(v), oldWorst = worst(v);
                    double sx = 0, sy = 0;
                    size_t count = 0;
                    for (size_t c : mesh.cornersAroundVertex(v))
                        for (size_t j = 0; j < 3; ++j)
                            if (3 * (c / 3) + j != c) {
                                const size_t q = 3 * (c / 3) + j;
                                sx += (*uv)[2 * q];
                                sy += (*uv)[2 * q + 1];
                                ++count;
                            }
                    if (!count)
                        continue;
                    const size_t reference = fans[v].front().corner;
                    const double dx = sx / count - (*uv)[2 * reference], dy = sy / count - (*uv)[2 * reference];
                    std::vector<std::pair<double, double>> saved;
                    for (const Fan& e : fans[v])
                        saved.push_back({ (*uv)[2 * e.corner], (*uv)[2 * e.corner + 1] });
                    for (double alpha : { 1.0, .5, .25, .1 }) {
                        for (size_t i = 0; i < fans[v].size(); ++i) {
                            double rx, ry;
                            rotateCoordinate(4 - fans[v][i].turn, alpha * dx, alpha * dy, &rx, &ry);
                            const size_t q = fans[v][i].corner;
                            (*uv)[2 * q] = saved[i].first + rx;
                            (*uv)[2 * q + 1] = saved[i].second + ry;
                        }
                        const double newTangle = tangle(v);
                        if ((newTangle < oldTangle || (newTangle <= oldTangle && worst(v) > oldWorst))) {
                            improved = true;
                            break;
                        }
                        for (size_t i = 0; i < fans[v].size(); ++i) {
                            const size_t q = fans[v][i].corner;
                            (*uv)[2 * q] = saved[i].first;
                            (*uv)[2 * q + 1] = saved[i].second;
                        }
                    }
                }
            if (!improved) {
                if (nullptr) {
                    size_t freeFaces = 0, cones = 0, hard = 0, cuts = 0;
                    for (size_t f : foldedFaces) {
                        bool allFree = true, atCone = false, atHard = false;
                        for (size_t l = 0; l < 3; ++l) {
                            const size_t c = 3 * f + l, v = mesh.cornerVertex(c);
                            if (fans[v].empty())
                                allFree = false;
                            atCone = atCone || singular[v];
                            atHard = atHard || constrained[c];
                        }
                        if (allFree)
                            ++freeFaces;
                        else if (atCone)
                            ++cones;
                        else if (atHard)
                            ++hard;
                        else
                            ++cuts;
                    }
                    std::cerr << "  native fold repair stalled after " << sweep
                              << " sweep(s): " << freeFaces << " free, " << cones
                              << " at a cone, " << hard << " on a sharp edge, "
                              << cuts << " on a cut" << std::endl;
                }
                break;
            }
        }
        return folded;
    }
}

bool NativeQuadParameterizer::parameterize(const std::vector<Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles,
    const std::vector<Vector3>* guidance, double scaling,
    double hardEdgeDegrees, Result* result,
    const std::vector<double>* faceScaling,
    const std::vector<double>* faceScalingU,
    const std::vector<double>* faceScalingV)
{
    if (vertices.empty() || triangles.empty() || scaling <= 0.0)
        return false;
    SurfaceMesh mesh(vertices, triangles);
    if (mesh.faceCount() != triangles.size())
        return false;
    const size_t corners = mesh.cornerCount();
    const double scale = std::max(1e-12, scaling * mesh.averageEdgeLength());
    std::vector<Vector3> normals(mesh.faceCount());
    result->field.resize(mesh.faceCount());
    const bool hasGuidance = guidance && guidance->size() == mesh.faceCount();
    tbb::parallel_for(tbb::blocked_range<size_t>(0, mesh.faceCount()), [&](const tbb::blocked_range<size_t>& range) {
        for (size_t faceIndex = range.begin(); faceIndex != range.end(); ++faceIndex) {
            normals[faceIndex] = unit(mesh.faceNormal(faceIndex), Vector3(0, 0, 1));
            Vector3 tangentAxis(1, 0, 0);
            if (std::fabs(Vector3::dotProduct(tangentAxis, normals[faceIndex])) > .8)
                tangentAxis = Vector3(0, 1, 0);
            Vector3 fieldDirection = hasGuidance ? (*guidance)[faceIndex] : tangentAxis;
            if (!hasGuidance)
                fieldDirection = fieldDirection - normals[faceIndex] * Vector3::dotProduct(fieldDirection, normals[faceIndex]);
            result->field[faceIndex] = unit(fieldDirection, mesh.edgeVector(3 * faceIndex));
        }
    });
    const std::vector<Vector3> fieldBeforeBrush = result->field;
    std::vector<double> activeScalingU(mesh.faceCount(), 1.0), activeScalingV(mesh.faceCount(), 1.0);
    const bool trackDirectionalScale = faceScalingU && faceScalingV
        && faceScalingU->size() == mesh.faceCount() && faceScalingV->size() == mesh.faceCount();
    if (trackDirectionalScale) {
        activeScalingU = *faceScalingU;
        activeScalingV = *faceScalingV;
    }

    if (!(guidance && guidance->size() == mesh.faceCount())) {
        std::vector<double> alpha(2 * mesh.faceCount(), 0.0);
        std::vector<char> locked(mesh.faceCount(), 0);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, mesh.faceCount()), [&](const tbb::blocked_range<size_t>& range) {
            for (size_t faceIndex = range.begin(); faceIndex != range.end(); ++faceIndex) {
                alpha[2 * faceIndex] = 1.0;
                for (size_t localCorner = 0; localCorner < 3; ++localCorner) {
                    const size_t cornerIndex = 3 * faceIndex + localCorner, oppositeCornerIndex = mesh.oppositeCorner(cornerIndex);
                    if (oppositeCornerIndex != SurfaceMesh::npos && std::fabs(mesh.normalAngle(cornerIndex)) * 180.0 / M_PI < hardEdgeDegrees)
                        continue;
                    const Vector3 edge = unit(mesh.edgeVector(cornerIndex), Vector3(1, 0, 0));
                    const Vector3 fieldDirection = result->field[faceIndex];
                    const Vector3 perpendicular = unit(Vector3::crossProduct(normals[faceIndex], fieldDirection), Vector3(0, 1, 0));
                    const double fieldAngle = std::atan2(Vector3::dotProduct(edge, perpendicular), Vector3::dotProduct(edge, fieldDirection));
                    alpha[2 * faceIndex] = std::cos(4.0 * fieldAngle);
                    alpha[2 * faceIndex + 1] = std::sin(4.0 * fieldAngle);
                    locked[faceIndex] = 1;
                }
            }
        });
        for (int iteration = 0; iteration < 40; ++iteration) {
            std::vector<double> next = alpha;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, mesh.faceCount()), [&](const tbb::blocked_range<size_t>& range) {
                for (size_t f = range.begin(); f != range.end(); ++f)
                    if (!locked[f]) {
                        double x = alpha[2 * f], y = alpha[2 * f + 1];
                        const Vector3 bf = result->field[f];
                        const Vector3 btf = unit(Vector3::crossProduct(normals[f], bf), Vector3(0, 1, 0));
                        for (size_t l = 0; l < 3; ++l) {
                            const size_t oc = mesh.oppositeCorner(3 * f + l);
                            if (oc == SurfaceMesh::npos)
                                continue;
                            const size_t g = mesh.cornerFace(oc);
                            Vector3 bg = result->field[g];
                            bg = unit(bg - normals[f] * Vector3::dotProduct(bg, normals[f]), bf);
                            const double d = std::atan2(Vector3::dotProduct(bg, btf), Vector3::dotProduct(bg, bf));
                            const double cs = std::cos(4.0 * d), sn = std::sin(4.0 * d);
                            x += cs * alpha[2 * g] - sn * alpha[2 * g + 1];
                            y += sn * alpha[2 * g] + cs * alpha[2 * g + 1];
                        }
                        const double length = std::hypot(x, y);
                        if (length > 1e-12) {
                            next[2 * f] = x / length;
                            next[2 * f + 1] = y / length;
                        }
                    }
            });
            alpha.swap(next);
        }
        tbb::parallel_for(tbb::blocked_range<size_t>(0, mesh.faceCount()), [&](const tbb::blocked_range<size_t>& range) {
            for (size_t faceIndex = range.begin(); faceIndex != range.end(); ++faceIndex) {
                const double fieldAngle = .25 * std::atan2(alpha[2 * faceIndex + 1], alpha[2 * faceIndex]);
                const Vector3 fieldDirection = result->field[faceIndex];
                const Vector3 perpendicular = unit(Vector3::crossProduct(normals[faceIndex], fieldDirection), Vector3(0, 1, 0));
                result->field[faceIndex] = unit(fieldDirection * std::cos(fieldAngle) + perpendicular * std::sin(fieldAngle), fieldDirection);
            }
        });
    }

    std::vector<char> seen(mesh.faceCount(), 0);
    if (mesh.faceCount() != 0) {
        std::queue<size_t> q;
        q.push(0);
        seen[0] = 1;
        while (!q.empty()) {
            size_t f = q.front();
            q.pop();
            for (size_t l = 0; l < 3; ++l) {
                size_t c = 3 * f + l, oc = mesh.oppositeCorner(c);
                if (oc == SurfaceMesh::npos)
                    continue;
                size_t g = mesh.cornerFace(oc);
                if (!seen[g]) {
                    int turns = edgeQuarterTurn(mesh, c, result->field, normals);
                    Vector3 brushed = result->field[g];
                    for (int k = 0; k < turns; ++k)
                        brushed = Vector3::crossProduct(normals[g], brushed);
                    result->field[g] = unit(brushed, mesh.edgeVector(3 * g));
                    seen[g] = 1;
                    q.push(g);
                }
            }
        }
    }
    std::vector<int> rotation(corners, 0);
    for (size_t c = 0; c < corners; ++c) {
        const size_t oc = mesh.oppositeCorner(c);
        if (oc == SurfaceMesh::npos)
            continue;
        rotation[c] = edgeQuarterTurn(mesh, c, result->field, normals);
    }
    result->cornerRotations = rotation;
    std::vector<signed char> cornerConstraints(corners, ConstraintNone);
    tbb::parallel_for(tbb::blocked_range<size_t>(0, corners), [&](const tbb::blocked_range<size_t>& range) {
        for (size_t c = range.begin(); c != range.end(); ++c)
            cornerConstraints[c] = static_cast<signed char>(edgeConstraint(mesh, c, result->field, normals, hardEdgeDegrees));
    });
    size_t directionalSwaps = 0;
    if (trackDirectionalScale)
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            const Vector3 before = unit(fieldBeforeBrush[f], result->field[f]);
            const Vector3 after = unit(result->field[f], before);
            const Vector3 perpendicular = unit(Vector3::crossProduct(normals[f], after), before);
            if (std::fabs(Vector3::dotProduct(before, after)) < std::fabs(Vector3::dotProduct(before, perpendicular)))
                std::swap(activeScalingU[f], activeScalingV[f]), ++directionalSwaps;
        }
    if (nullptr && trackDirectionalScale)
        std::cerr << "Native brush directional swaps=" << directionalSwaps << std::endl;
    std::vector<char> insideBall(corners, 0), ballSeen(mesh.faceCount(), 0);
    if (mesh.faceCount() != 0) {
        std::queue<size_t> q;
        q.push(0);
        ballSeen[0] = 1;
        while (!q.empty()) {
            const size_t f = q.front();
            q.pop();
            for (size_t l = 0; l < 3; ++l) {
                const size_t c = 3 * f + l, oc = mesh.oppositeCorner(c);
                if (oc == SurfaceMesh::npos || rotation[c] != 0)
                    continue;
                const size_t g = mesh.cornerFace(oc);
                if (!ballSeen[g]) {
                    ballSeen[g] = 1;
                    q.push(g);
                    insideBall[c] = insideBall[oc] = 1;
                }
            }
        }
    }
    std::vector<char> seam(corners, 0);
    for (size_t c = 0; c < corners; ++c)
        seam[c] = (mesh.oppositeCorner(c) == SurfaceMesh::npos || !insideBall[c]);
    std::vector<size_t> borderDegree(mesh.vertexCount(), 0);
    for (size_t c = 0; c < corners; ++c)
        if (seam[c])
            ++borderDegree[mesh.cornerVertex(c)];
    bool changed = true;
    while (changed) {
        changed = false;
        for (size_t c = 0; c < corners; ++c) {
            const size_t oc = mesh.oppositeCorner(c);
            if (oc == SurfaceMesh::npos || !seam[c] || rotation[c] != 0)
                continue;
            const size_t v0 = mesh.cornerVertex(c);
            if (borderDegree[v0] != 1)
                continue;
            const size_t v1 = mesh.cornerVertex(mesh.nextCorner(c));
            seam[c] = seam[oc] = 0;
            insideBall[c] = insideBall[oc] = 1;
            if (borderDegree[v0] > 0)
                --borderDegree[v0];
            if (borderDegree[v1] > 0)
                --borderDegree[v1];
            changed = true;
        }
    }

    const size_t uvVariables = 2 * corners;
    const size_t variables = 2 * uvVariables;
    auto addRotation = [](MixedIntegerLeastSquares& s, size_t ax, size_t bx,
                           int r, double sign) {
        r = (r % 4 + 4) % 4;
        if (r == 0) {
            s.addConstraint(ax, 1, bx, sign);
            s.addConstraint(ax + 1, 1, bx + 1, sign);
        } else if (r == 1) {
            s.addConstraint(ax, 1, bx + 1, sign);
            s.addConstraint(ax + 1, 1, bx, -sign);
        } else if (r == 2) {
            s.addConstraint(ax, 1, bx, -sign);
            s.addConstraint(ax + 1, 1, bx + 1, -sign);
        } else {
            s.addConstraint(ax, 1, bx + 1, -sign);
            s.addConstraint(ax + 1, 1, bx, sign);
        }
    };
    struct OrientationBarrier {
        std::vector<std::pair<size_t, double>> row;
        double rhs;
        double weight;
    };
    auto solve = [&](std::vector<double>* values,
                     const std::vector<OrientationBarrier>& barriers,
                     bool integerTransitions = true) {
        MixedIntegerLeastSquares s(variables);
        if (integerTransitions) {
            for (size_t t = 0; t < 2 * corners; ++t)
                s.setVariablePeriod(uvVariables + t, 2);
        }
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            const Vector3 u = result->field[f], v = unit(Vector3::crossProduct(normals[f], u), mesh.edgeVector(3 * f));
            const double faceScale = faceScaling && faceScaling->size() == mesh.faceCount()
                ? std::max(1e-12, (*faceScaling)[f])
                : 1.0;
            const double directionalU = std::max(1e-12, activeScalingU[f]);
            const double directionalV = std::max(1e-12, activeScalingV[f]);
            const double su = scale * faceScale * directionalU;
            const double sv = su * directionalV;
            for (size_t l = 0; l < 3; ++l) {
                size_t c = 3 * f + l, n = mesh.nextCorner(c);
                Vector3 e = mesh.edgeVector(c);
                const double weight = su * sv;
                s.addEnergy(2 * n, 1, 2 * c, -1, Vector3::dotProduct(u, e) / su, weight);
                s.addEnergy(2 * n + 1, 1, 2 * c + 1, -1, Vector3::dotProduct(v, e) / sv, weight);
            }
        }
        for (const OrientationBarrier& barrier : barriers)
            s.addEnergy(barrier.row, barrier.rhs, barrier.weight);
        s.addConstraint(0, 1);
        s.addConstraint(1, 1);
        size_t hardCoordinateCount = 0;
        for (size_t c = 0; c < corners; ++c) {
            size_t oc = mesh.oppositeCorner(c);
            if (oc == SurfaceMesh::npos)
                continue;
            const size_t tc = uvVariables + 2 * c;
            const size_t toc = uvVariables + 2 * oc;
            int r = rotation[c];
            if (seam[c]) {
                addRotation(s, tc, toc, r, 1.0);
            } else {
                s.addConstraint(tc, 1);
                s.addConstraint(tc + 1, 1);
            }
        }
        for (size_t c = 0; c < corners; ++c) {
            const size_t oc = mesh.oppositeCorner(c);
            if (oc == SurfaceMesh::npos)
                continue;
            const size_t other = mesh.nextCorner(oc);
            const size_t tc = uvVariables + 2 * c;
            const int r = (rotation[c] % 4 + 4) % 4;
            if (r == 0) {
                s.addConstraint(2 * c, 1, 2 * other, -1, tc, -1);
                s.addConstraint(2 * c + 1, 1, 2 * other + 1, -1, tc + 1, -1);
            } else if (r == 1) {
                s.addConstraint(2 * c, 1, 2 * other + 1, -1, tc, -1);
                s.addConstraint(2 * c + 1, 1, 2 * other, 1, tc + 1, -1);
            } else if (r == 2) {
                s.addConstraint(2 * c, 1, 2 * other, 1, tc, -1);
                s.addConstraint(2 * c + 1, 1, 2 * other + 1, 1, tc + 1, -1);
            } else {
                s.addConstraint(2 * c, 1, 2 * other + 1, 1, tc, -1);
                s.addConstraint(2 * c + 1, 1, 2 * other, -1, tc + 1, -1);
            }
        }
        for (size_t vertex = 0; vertex < mesh.vertexCount(); ++vertex) {
            const auto& incident = mesh.cornersAroundVertex(vertex);
            if (incident.empty())
                continue;
            size_t start = incident.front(), c = start;
            int accumulated = 0;
            bool closed = true;
            std::vector<std::pair<size_t, int>> wheel;
            do {
                if (mesh.oppositeCorner(c) == SurfaceMesh::npos) {
                    closed = false;
                    break;
                }
                wheel.push_back({ c, accumulated });
                accumulated = (accumulated + rotation[c]) % 4;
                c = mesh.nextCorner(mesh.oppositeCorner(c));
            } while (c != start && wheel.size() <= incident.size() + 1);
            if (!closed || c != start || accumulated != 0)
                continue;
            for (int coord = 0; coord < 2; ++coord) {
                std::vector<std::pair<size_t, double>> row;
                for (const auto& item : wheel) {
                    const size_t t = uvVariables + 2 * item.first;
                    const int r = item.second;
                    if (coord == 0) {
                        if (r == 0)
                            row.push_back({ t, 1 });
                        else if (r == 1)
                            row.push_back({ t + 1, 1 });
                        else if (r == 2)
                            row.push_back({ t, -1 });
                        else
                            row.push_back({ t + 1, -1 });
                    } else {
                        if (r == 0)
                            row.push_back({ t + 1, 1 });
                        else if (r == 1)
                            row.push_back({ t, -1 });
                        else if (r == 2)
                            row.push_back({ t + 1, -1 });
                        else
                            row.push_back({ t, 1 });
                    }
                }
                s.addConstraint(row);
            }
        }
        for (size_t c = 0; c < corners; ++c) {
            const size_t n = mesh.nextCorner(c);
            const int constraint = cornerConstraints[c];
            if (constraint == ConstraintV) {
                s.setVariablePeriod(2 * c + 1, 1);
                s.setVariablePeriod(2 * n + 1, 1);
                hardCoordinateCount += 2;
                s.addConstraint(2 * c + 1, 1, 2 * n + 1, -1);
            } else if (constraint == ConstraintU) {
                s.setVariablePeriod(2 * c, 1);
                s.setVariablePeriod(2 * n, 1);
                hardCoordinateCount += 2;
                s.addConstraint(2 * c, 1, 2 * n, -1);
            }
        }
        s.finalizeConstraints();
        for (size_t iteration = 0; iteration < 100; ++iteration) {
            if (!s.solveIteration())
                return false;
            if (s.converged())
                break;
        }
        values->resize(variables);
        for (size_t i = 0; i < variables; ++i)
            (*values)[i] = s.value(i);
        return s.converged();
    };
    std::vector<double> allValues;
    if (!solve(&allValues, {}))
        return false;
    auto solveFrozenTransitions = [&](const std::vector<double>& fixedT,
                                      const std::vector<OrientationBarrier>& barriers,
                                      std::vector<double>* values, const std::vector<double>* previous = nullptr,
                                      double damping = 0.0) {
        ConstrainedLeastSquares system(uvVariables);
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            const Vector3 u = result->field[f];
            const Vector3 v = unit(Vector3::crossProduct(normals[f], u), mesh.edgeVector(3 * f));
            const double faceScale = faceScaling && faceScaling->size() == mesh.faceCount()
                ? std::max(1e-12, (*faceScaling)[f])
                : 1.0;
            const double su = scale * faceScale * std::max(1e-12, activeScalingU[f]);
            const double sv = su * std::max(1e-12, activeScalingV[f]);
            const double weight = su * sv;
            for (size_t l = 0; l < 3; ++l) {
                const size_t c = 3 * f + l, n = mesh.nextCorner(c);
                const Vector3 e = mesh.edgeVector(c);
                system.addEnergy({ { 2 * n, 1 }, { 2 * c, -1 } }, Vector3::dotProduct(u, e) / su, weight);
                system.addEnergy({ { 2 * n + 1, 1 }, { 2 * c + 1, -1 } }, Vector3::dotProduct(v, e) / sv, weight);
            }
        }
        system.addConstraint({ { 0, 1 } }, fixedT[0]);
        system.addConstraint({ { 1, 1 } }, fixedT[1]);
        for (size_t c = 0; c < corners; ++c) {
            const size_t oc = mesh.oppositeCorner(c);
            if (oc == SurfaceMesh::npos)
                continue;
            const size_t other = mesh.nextCorner(oc), t = uvVariables + 2 * c;
            const int r = (rotation[c] % 4 + 4) % 4;
            if (r == 0) {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other, -1 } }, fixedT[t]);
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other + 1, -1 } }, fixedT[t + 1]);
            } else if (r == 1) {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other + 1, -1 } }, fixedT[t]);
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other, 1 } }, fixedT[t + 1]);
            } else if (r == 2) {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other, 1 } }, fixedT[t]);
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other + 1, 1 } }, fixedT[t + 1]);
            } else {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other + 1, 1 } }, fixedT[t]);
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other, -1 } }, fixedT[t + 1]);
            }
        }
        for (size_t c = 0; c < corners; ++c) {
            const size_t n = mesh.nextCorner(c);
            const int constraint = cornerConstraints[c];
            if (constraint == ConstraintU)
                system.addConstraint({ { 2 * c, 1 }, { 2 * n, -1 } });
            else if (constraint == ConstraintV)
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * n + 1, -1 } });
        }
        for (const OrientationBarrier& barrier : barriers)
            system.addEnergy(barrier.row, barrier.rhs, barrier.weight);
        if (previous != nullptr && damping > 0.0) {
            for (size_t u = 0; u < uvVariables; ++u)
                system.addEnergy({ { u, 1.0 } }, (*previous)[u], damping);
        }
        return system.solve(values);
    };
    auto projectFrozenGradient = [&](const std::vector<double>& gradient,
                                     std::vector<double>* direction) {
        ConstrainedLeastSquares system(uvVariables);
        for (size_t u = 0; u < uvVariables; ++u)
            system.addEnergy({ { u, 1 } }, -gradient[u]);
        system.addConstraint({ { 0, 1 } });
        system.addConstraint({ { 1, 1 } });
        for (size_t c = 0; c < corners; ++c) {
            const size_t oc = mesh.oppositeCorner(c);
            if (oc == SurfaceMesh::npos)
                continue;
            const size_t other = mesh.nextCorner(oc);
            const int r = (rotation[c] % 4 + 4) % 4;
            if (r == 0) {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other, -1 } });
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other + 1, -1 } });
            } else if (r == 1) {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other + 1, -1 } });
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other, 1 } });
            } else if (r == 2) {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other, 1 } });
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other + 1, 1 } });
            } else {
                system.addConstraint({ { 2 * c, 1 }, { 2 * other + 1, 1 } });
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * other, -1 } });
            }
        }
        for (size_t c = 0; c < corners; ++c) {
            const size_t n = mesh.nextCorner(c);
            const int constraint = cornerConstraints[c];
            if (constraint == ConstraintU)
                system.addConstraint({ { 2 * c, 1 }, { 2 * n, -1 } });
            else if (constraint == ConstraintV)
                system.addConstraint({ { 2 * c + 1, 1 }, { 2 * n + 1, -1 } });
        }
        return system.solve(direction);
    };
    const bool frozenUntangle = nullptr != nullptr;
    auto runFrozenUntangler = [&]() {
        const auto signedArea = [&](const std::vector<double>& x, size_t f) {
            const size_t c = 3 * f;
            return .5 * ((x[2 * (c + 1)] - x[2 * c]) * (x[2 * (c + 2) + 1] - x[2 * c + 1]) - (x[2 * (c + 1) + 1] - x[2 * c + 1]) * (x[2 * (c + 2)] - x[2 * c]));
        };
        size_t positive = 0, negative = 0;
        double meanPositiveArea = 0.0;
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            const double a = signedArea(allValues, f);
            if (a > 0) {
                ++positive;
                meanPositiveArea += a;
            } else if (a < 0)
                ++negative;
        }
        const double sign = positive >= negative ? 1.0 : -1.0;
        double targetFraction = .02;
        if (const char* value = nullptr)
            targetFraction = std::max(0.0, std::atof(value));
        const double target = std::max(1e-8,
            (meanPositiveArea / std::max<size_t>(1, positive)) * targetFraction);
        double barrierWeight = 100.0;
        if (const char* value = nullptr)
            barrierWeight = std::max(1e-8, std::atof(value));
        double damping = 1e-3;
        if (const char* value = nullptr)
            damping = std::max(1e-12, std::atof(value));
        size_t maximumOuterIterations = 40;
        if (const char* value = nullptr)
            maximumOuterIterations = std::max<size_t>(1, std::strtoul(value, nullptr, 10));
        for (size_t outer = 0; outer < maximumOuterIterations; ++outer) {
            std::vector<OrientationBarrier> barriers;
            for (size_t f = 0; f < mesh.faceCount(); ++f) {
                const size_t c = 3 * f;
                const double x0 = allValues[2 * c], y0 = allValues[2 * c + 1];
                const double x1 = allValues[2 * (c + 1)], y1 = allValues[2 * (c + 1) + 1];
                const double x2 = allValues[2 * (c + 2)], y2 = allValues[2 * (c + 2) + 1];
                const double a = .5 * ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0));
                if (a * sign >= target)
                    continue;
                const double gx0 = .5 * (y1 - y2) * sign, gy0 = .5 * (x2 - x1) * sign;
                const double gx1 = .5 * (y2 - y0) * sign, gy1 = .5 * (x0 - x2) * sign;
                const double gx2 = .5 * (y0 - y1) * sign, gy2 = .5 * (x1 - x0) * sign;
                const double linear = gx0 * x0 + gy0 * y0 + gx1 * x1 + gy1 * y1 + gx2 * x2 + gy2 * y2;
                barriers.push_back({ { { 2 * c, gx0 }, { 2 * c + 1, gy0 }, { 2 * (c + 1), gx1 }, { 2 * (c + 1) + 1, gy1 }, { 2 * (c + 2), gx2 }, { 2 * (c + 2) + 1, gy2 } }, target - a * sign + linear, barrierWeight });
            }
            if (barriers.empty())
                break;
            std::vector<double> candidate;
            if (!solveFrozenTransitions(allValues, barriers, &candidate, &allValues, damping))
                break;
            size_t before = 0, after = 0;
            double beforeDeficit = 0.0, afterDeficit = 0.0;
            for (size_t f = 0; f < mesh.faceCount(); ++f) {
                const double oldDeficit = std::max(0.0, target - signedArea(allValues, f) * sign);
                const double newDeficit = std::max(0.0, target - signedArea(candidate, f) * sign);
                beforeDeficit += oldDeficit * oldDeficit;
                afterDeficit += newDeficit * newDeficit;
                if (signedArea(allValues, f) * sign <= 0)
                    ++before;
                if (signedArea(candidate, f) * sign <= 0)
                    ++after;
            }
            if (nullptr)
                std::cerr << "Native frozen orientation barrier: " << before << " -> " << after << std::endl;
            if (after > before || (after == before && afterDeficit >= beforeDeficit)) {
                damping *= 10.0;
                if (damping > 1e8)
                    break;
                continue;
            }
            for (size_t u = 0; u < uvVariables; ++u)
                allValues[u] = candidate[u];
            damping = std::max(1e-12, damping * .5);
            if (after == 0)
                break;
        }
        if (nullptr) {
            size_t maximumIterations = 16;
            if (const char* value = nullptr)
                maximumIterations = std::max<size_t>(1, std::strtoul(value, nullptr, 10));
            for (size_t iteration = 0; iteration < maximumIterations; ++iteration) {
                size_t positiveNow = 0;
                double mean = 0.0;
                for (size_t f = 0; f < mesh.faceCount(); ++f) {
                    const double a = signedArea(allValues, f) * sign;
                    if (a > 0) {
                        ++positiveNow;
                        mean += a;
                    }
                }
                mean /= std::max<size_t>(1, positiveNow);
                const double width = std::max(1e-6, mean * .05);
                const double targetArea = width * .1;
                std::vector<double> gradient(uvVariables, 0.0);
                auto objective = [&](const std::vector<double>& x) {
                    double value = 0.0;
                    for (size_t f = 0; f < mesh.faceCount(); ++f) {
                        const double deficit = std::max(0.0, targetArea - signedArea(x, f) * sign);
                        value += deficit * deficit;
                    }
                    return value;
                };
                for (size_t f = 0; f < mesh.faceCount(); ++f) {
                    const size_t c = 3 * f;
                    const double x0 = allValues[2 * c], y0 = allValues[2 * c + 1];
                    const double x1 = allValues[2 * (c + 1)], y1 = allValues[2 * (c + 1) + 1];
                    const double x2 = allValues[2 * (c + 2)], y2 = allValues[2 * (c + 2) + 1];
                    const double a = .5 * ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0)) * sign;
                    const double d = a < targetArea ? -2.0 * (targetArea - a) : 0.0;
                    const double gx[3] = { .5 * (y1 - y2) * sign, .5 * (y2 - y0) * sign, .5 * (y0 - y1) * sign };
                    const double gy[3] = { .5 * (x2 - x1) * sign, .5 * (x0 - x2) * sign, .5 * (x1 - x0) * sign };
                    for (size_t l = 0; l < 3; ++l) {
                        gradient[2 * (c + l)] += d * gx[l];
                        gradient[2 * (c + l) + 1] += d * gy[l];
                    }
                }
                std::vector<double> direction;
                if (!projectFrozenGradient(gradient, &direction))
                    break;
                double directionInfinity = 0.0;
                for (double d : direction)
                    directionInfinity = std::max(directionInfinity, std::fabs(d));
                if (directionInfinity < 1e-16)
                    break;
                const double directionScale = .25 * std::sqrt(mean) / directionInfinity;
                const double before = objective(allValues);
                bool accepted = false;
                for (const double alpha : { 1.0, .5, .25, .125, .0625, .03125, .015625, .0078125 }) {
                    std::vector<double> trial = allValues;
                    for (size_t u = 0; u < uvVariables; ++u)
                        trial[u] += alpha * directionScale * direction[u];
                    if (objective(trial) >= before)
                        continue;
                    allValues.swap(trial);
                    accepted = true;
                    break;
                }
                if (!accepted)
                    break;
                if (nullptr && iteration % 10 == 0)
                    std::cerr << "Native projected flip descent objective=" << objective(allValues) << std::endl;
            }
        }
    };
    if (frozenUntangle)
        runFrozenUntangler();
    if (nullptr || nullptr) {
        const auto signedArea = [&](const std::vector<double>& x, size_t f) {
            const size_t c = 3 * f;
            return .5 * ((x[2 * (c + 1)] - x[2 * c]) * (x[2 * (c + 2) + 1] - x[2 * c + 1]) - (x[2 * (c + 1) + 1] - x[2 * c + 1]) * (x[2 * (c + 2)] - x[2 * c]));
        };
        size_t positive = 0, negative = 0;
        double meanPositiveArea = 0.0;
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            const double a = signedArea(allValues, f);
            if (a > 0) {
                ++positive;
                meanPositiveArea += a;
            } else if (a < 0)
                ++negative;
        }
        const double sign = positive >= negative ? 1.0 : -1.0;
        meanPositiveArea /= std::max<size_t>(1, positive);
        const double target = std::max(1e-5, meanPositiveArea * .02);
        for (size_t outer = 0; outer < 12; ++outer) {
            std::vector<OrientationBarrier> barriers;
            for (size_t f = 0; f < mesh.faceCount(); ++f) {
                const size_t c = 3 * f;
                const double x0 = allValues[2 * c], y0 = allValues[2 * c + 1];
                const double x1 = allValues[2 * (c + 1)], y1 = allValues[2 * (c + 1) + 1];
                const double x2 = allValues[2 * (c + 2)], y2 = allValues[2 * (c + 2) + 1];
                const double a = .5 * ((x1 - x0) * (y2 - y0) - (y1 - y0) * (x2 - x0));
                if (a * sign >= target)
                    continue;
                const double gx0 = .5 * (y1 - y2) * sign, gy0 = .5 * (x2 - x1) * sign;
                const double gx1 = .5 * (y2 - y0) * sign, gy1 = .5 * (x0 - x2) * sign;
                const double gx2 = .5 * (y0 - y1) * sign, gy2 = .5 * (x1 - x0) * sign;
                const double linear = gx0 * x0 + gy0 * y0 + gx1 * x1 + gy1 * y1 + gx2 * x2 + gy2 * y2;
                barriers.push_back({ { { 2 * c, gx0 }, { 2 * c + 1, gy0 }, { 2 * (c + 1), gx1 }, { 2 * (c + 1) + 1, gy1 }, { 2 * (c + 2), gx2 }, { 2 * (c + 2) + 1, gy2 } },
                    target - a * sign + linear, 100.0 });
            }
            if (barriers.empty())
                break;
            std::vector<double> next;
            const bool integerTransitions = nullptr == nullptr;
            if (!solve(&next, barriers, integerTransitions))
                break;
            if (!integerTransitions) {
                for (size_t c = 0; c < corners; ++c) {
                    const size_t oc = mesh.oppositeCorner(c);
                    if (oc == SurfaceMesh::npos)
                        continue;
                    const size_t tc = uvVariables + 2 * c;
                    if (!seam[c]) {
                        next[tc] = 0.0;
                        next[tc + 1] = 0.0;
                        continue;
                    }
                    if (c > oc)
                        continue;
                    const double x = 2.0 * std::round(next[tc] * .5);
                    const double y = 2.0 * std::round(next[tc + 1] * .5);
                    next[tc] = x;
                    next[tc + 1] = y;
                    double ox, oy;
                    rotateCoordinate(4 - rotation[c], -x, -y, &ox, &oy);
                    const size_t toc = uvVariables + 2 * oc;
                    next[toc] = ox;
                    next[toc + 1] = oy;
                }
                std::vector<double> reprojected;
                if (!solveFrozenTransitions(next, barriers, &reprojected))
                    break;
                for (size_t u = 0; u < uvVariables; ++u)
                    next[u] = reprojected[u];
            }
            size_t before = 0, after = 0;
            for (size_t f = 0; f < mesh.faceCount(); ++f) {
                if (signedArea(allValues, f) * sign <= 0)
                    ++before;
                if (signedArea(next, f) * sign <= 0)
                    ++after;
            }
            if (nullptr)
                std::cerr << "Native orientation barrier: " << before << " -> " << after << std::endl;
            if (after >= before)
                break;
            allValues.swap(next);
            if (after == 0)
                break;
        }
    }
    if (nullptr) {
        size_t folded = 0;
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            size_t c = 3 * f;
            const double a = (allValues[2 * (c + 1)] - allValues[2 * c]) * (allValues[2 * (c + 2) + 1] - allValues[2 * c + 1]) - (allValues[2 * (c + 1) + 1] - allValues[2 * c + 1]) * (allValues[2 * (c + 2)] - allValues[2 * c]);
            if (a <= 0)
                ++folded;
        }
        std::cerr << "Native cover relaxed folds=" << folded << std::endl;
    }
    std::vector<double> uv(allValues.begin(), allValues.begin() + uvVariables);
    for (double& coordinate : uv) {
        const double integer = std::round(coordinate);
        if (std::fabs(coordinate - integer) < 0.05)
            coordinate = integer;
    }
    result->singularVertices.clear();
    std::vector<char> singular(mesh.vertexCount(), 0), constrained(corners, 0);
    for (size_t vertex = 0; vertex < mesh.vertexCount(); ++vertex) {
        const auto& fan = mesh.cornersAroundVertex(vertex);
        int sum = 0;
        bool boundary = false;
        for (size_t c : fan) {
            sum = (sum + rotation[c]) % 4;
            if (mesh.oppositeCorner(c) == SurfaceMesh::npos)
                boundary = true;
        }
        if (!boundary && sum != 0) {
            singular[vertex] = 1;
            result->singularVertices.push_back(vertex);
        }
    }
    for (size_t c = 0; c < corners; ++c) {
        const size_t n = mesh.nextCorner(c);
        if (cornerConstraints[c] != ConstraintNone)
            constrained[c] = constrained[n] = 1;
    }
    size_t repairedBefore = 0;
    const size_t repairedAfter = repairFoldedUv(mesh, rotation, singular, constrained, &uv, &repairedBefore);
    if (nullptr)
        std::cerr << "Native fold repair: " << repairedBefore << " -> " << repairedAfter << std::endl;
    if (nullptr) {
        double lo[2] = { uv[0], uv[1] }, hi[2] = { uv[0], uv[1] };
        size_t folded = 0;
        for (size_t c = 0; c < corners; ++c)
            for (size_t k = 0; k < 2; ++k) {
                lo[k] = std::min(lo[k], uv[2 * c + k]);
                hi[k] = std::max(hi[k], uv[2 * c + k]);
            }
        for (size_t f = 0; f < mesh.faceCount(); ++f) {
            size_t c = 3 * f;
            double a = (uv[2 * (c + 1)] - uv[2 * c]) * (uv[2 * (c + 2) + 1] - uv[2 * c + 1]) - (uv[2 * (c + 1) + 1] - uv[2 * c + 1]) * (uv[2 * (c + 2)] - uv[2 * c]);
            if (a <= 0)
                ++folded;
        }
        double maximumModuloResidual = 0.0;
        for (size_t c = 0; c < corners; ++c) {
            const size_t oc = mesh.oppositeCorner(c);
            if (oc == SurfaceMesh::npos)
                continue;
            const size_t other = mesh.nextCorner(oc);
            const int r = (rotation[c] % 4 + 4) % 4;
            const double ox = uv[2 * other], oy = uv[2 * other + 1];
            const double rx = r == 0 ? ox : r == 1 ? oy
                : r == 2                           ? -ox
                                                   : -oy;
            const double ry = r == 0 ? oy : r == 1 ? -ox
                : r == 2                           ? -oy
                                                   : ox;
            maximumModuloResidual = std::max(maximumModuloResidual, std::fabs(uv[2 * c] - rx - 2.0 * std::round((uv[2 * c] - rx) / 2.0)));
            maximumModuloResidual = std::max(maximumModuloResidual, std::fabs(uv[2 * c + 1] - ry - 2.0 * std::round((uv[2 * c + 1] - ry) / 2.0)));
        }
        std::cerr << "Native cover uv: u=[" << lo[0] << "," << hi[0] << "] v=[" << lo[1] << "," << hi[1] << "] folds=" << folded << " max seam modulo-2 residual=" << maximumModuloResidual << std::endl;
    }
    result->triangleUvs.assign(mesh.faceCount(), std::vector<Vector2>(3));
    for (size_t f = 0; f < mesh.faceCount(); ++f)
        for (size_t l = 0; l < 3; ++l) {
            size_t c = 3 * f + l;
            result->triangleUvs[f][l] = Vector2(uv[2 * c], uv[2 * c + 1]);
        }
    return true;
}
}
