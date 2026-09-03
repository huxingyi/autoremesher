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
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/IsotropicRemesher>
#include <AutoRemesher/MeshSeparator>
#include <AutoRemesher/Parameterizer>
#include <AutoRemesher/QuadExtractor>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
// Qt defines `emit` as a macro, which collides with TBB profiling.h's `void emit()`.
// macOS `<mach/mach.h>` also defines `emit`. Undefine before including TBB headers.
#if defined(__APPLE__) || defined(emit)
#undef emit
#endif

// oneAPI TBB (2021+) moved headers under <oneapi/tbb/>. Use __has_include where
// available (clang + GCC) to pick the right path, falling back to the legacy path.
#if defined(__has_include)
#if __has_include(<oneapi/tbb/blocked_range.h>)
#include <oneapi/tbb/blocked_range.h>
#else
#include <tbb/blocked_range.h>
#endif
#if __has_include(<oneapi/tbb/mutex.h>)
#include <oneapi/tbb/mutex.h>
#else
#include <tbb/mutex.h>
#endif
#if __has_include(<oneapi/tbb/parallel_for.h>)
#include <oneapi/tbb/parallel_for.h>
#include <oneapi/tbb/parallel_sort.h>
#else
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>
#endif
#else
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#include <tbb/parallel_sort.h>
#endif
#include <cfloat>
#include <meshoptimizer.h>
#include <unordered_map>
#include <unordered_set>

namespace AutoRemesher {

namespace {
    const float parallelPhaseBegin = 0.03f;
    const float parallelPhaseEnd = 0.95f;

    // How an island's own 0..1 progress splits across its three stages, from the
    // measured cost of each on a typical model.  The phase report prints the
    // real accumulated times, so these can be re-checked against a run.
    const float islandResampleEnd = 0.17f;
    const float islandParameterizeEnd = 0.50f;
    // Quad extraction runs from islandParameterizeEnd to 1.0.

    const double decimateTriggerRatio = 8.0;
    const double decimateTargetRatio = 4.0;

    void markSharpEdgeVertices(const std::vector<Vector3>& vertices,
        const std::vector<unsigned int>& indices,
        double sharpEdgeRadians,
        std::vector<unsigned char>& vertexLock)
    {
        const size_t faceCount = indices.size() / 3;

        std::vector<Vector3> faceNormals(faceCount);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, faceCount),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    faceNormals[i] = Vector3::normal(vertices[indices[i * 3 + 0]],
                        vertices[indices[i * 3 + 1]],
                        vertices[indices[i * 3 + 2]]);
                }
            });

        std::vector<std::pair<uint64_t, unsigned int>> edges(faceCount * 3);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, faceCount),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    for (size_t j = 0; j < 3; ++j) {
                        unsigned int first = indices[i * 3 + j];
                        unsigned int second = indices[i * 3 + (j + 1) % 3];
                        if (first > second)
                            std::swap(first, second);
                        edges[i * 3 + j] = { ((uint64_t)first << 32) | second, (unsigned int)i };
                    }
                }
            });
        // Every (edge, face) pair is distinct, so the parallel sort produces the
        // same order the serial one did.
        tbb::parallel_sort(edges.begin(), edges.end());

        for (size_t i = 0; i + 1 < edges.size(); ++i) {
            if (edges[i].first != edges[i + 1].first)
                continue;
            if (Vector3::angle(faceNormals[edges[i].second],
                    faceNormals[edges[i + 1].second])
                < sharpEdgeRadians)
                continue;
            vertexLock[(size_t)(edges[i].first >> 32)] |= meshopt_SimplifyVertex_Priority;
            vertexLock[(size_t)(edges[i].first & 0xffffffffu)] |= meshopt_SimplifyVertex_Priority;
        }
    }
}

const double AutoRemesher::m_defaultSharpEdgeDegrees = 90;

double AutoRemesher::calculateAverageEdgeLength(const std::vector<Vector3>& vertices,
    const std::vector<std::vector<size_t>>& faces)
{
    double sumOfLength = 0.0;
    size_t edgeCount = 0;
    for (const auto& face : faces) {
        for (size_t i = 0; i < face.size(); ++i) {
            size_t j = (i + 1) % face.size();
            sumOfLength += (vertices[face[i]] - vertices[face[j]]).length();
            ++edgeCount;
        }
    }
    if (0 == edgeCount)
        return 0.0;
    return sumOfLength / edgeCount;
}

void AutoRemesher::initializeVoxelSize()
{
    double area = calculateMeshArea(m_vertices, m_triangles);
    double triangleArea = area / m_targetTriangleCount;
    m_voxelSize = std::sqrt(triangleArea / (0.86602540378 * 0.5));
#if AUTO_REMESHER_DEBUG
    std::cerr << "Area: " << area << " voxelSize: " << m_voxelSize << std::endl;
#endif
}

double AutoRemesher::calculateMeshArea(const std::vector<Vector3>& vertices,
    const std::vector<std::vector<size_t>>& triangles)
{
    double area = 0.0;
    for (const auto& it : triangles) {
        area += Vector3::area(vertices[it[0]], vertices[it[1]], vertices[it[2]]);
    }
    return area;
}

bool AutoRemesher::decimateIfTooDense(std::vector<Vector3>& vertices,
    std::vector<std::vector<size_t>>& triangles,
    double voxelSize,
    double sharpEdgeDegrees,
    size_t islandIndex,
    DecimationStats* stats)
{
    if (nullptr != stats)
        ++stats->islandsConsidered;

    if (vertices.empty() || triangles.empty() || voxelSize <= 0.0)
        return false;

    if (vertices.size() > (size_t)std::numeric_limits<unsigned int>::max())
        return false;

    const double targetTriangleArea = voxelSize * voxelSize * 0.86602540378 * 0.5;
    if (targetTriangleArea <= 0.0)
        return false;
    const double islandTargetTriangleCount = calculateMeshArea(vertices, triangles) / targetTriangleArea;
    if (islandTargetTriangleCount < 1.0)
        return false;

    if ((double)triangles.size() < islandTargetTriangleCount * decimateTriggerRatio)
        return false;

    const size_t decimateTriangleCount = (size_t)(islandTargetTriangleCount * decimateTargetRatio);

    std::vector<unsigned int> indices;
    indices.reserve(triangles.size() * 3);
    for (const auto& triangle : triangles) {
        if (3 != triangle.size())
            return false;
        for (size_t i = 0; i < 3; ++i)
            indices.push_back((unsigned int)triangle[i]);
    }

    Vector3 lowerBound = vertices.front();
    Vector3 upperBound = vertices.front();
    for (const auto& position : vertices) {
        for (size_t i = 0; i < 3; ++i) {
            lowerBound[i] = std::min(lowerBound[i], position[i]);
            upperBound[i] = std::max(upperBound[i], position[i]);
        }
    }
    const Vector3 center = (lowerBound + upperBound) * 0.5;

    std::vector<float> positions;
    positions.reserve(vertices.size() * 3);
    for (const auto& position : vertices) {
        positions.push_back((float)(position.x() - center.x()));
        positions.push_back((float)(position.y() - center.y()));
        positions.push_back((float)(position.z() - center.z()));
    }

    std::vector<unsigned int> remap(vertices.size());
    const size_t weldedVertexCount = meshopt_generateVertexRemap(remap.data(),
        indices.data(), indices.size(),
        positions.data(), vertices.size(), sizeof(float) * 3);
    std::vector<Vector3> weldedVertices(weldedVertexCount);
    std::vector<float> weldedPositions(weldedVertexCount * 3);
    meshopt_remapIndexBuffer(indices.data(), indices.data(), indices.size(), remap.data());
    meshopt_remapVertexBuffer(weldedPositions.data(), positions.data(),
        vertices.size(), sizeof(float) * 3, remap.data());
    for (size_t i = 0; i < vertices.size(); ++i) {
        if (~0u != remap[i])
            weldedVertices[remap[i]] = vertices[i];
    }

    std::vector<unsigned char> vertexLock;
    if (sharpEdgeDegrees > 0.0) {
        vertexLock.assign(weldedVertexCount, 0);
        markSharpEdgeVertices(weldedVertices, indices,
            sharpEdgeDegrees * (M_PI / 180.0), vertexLock);
    }

    std::vector<unsigned int> decimated(indices.size());
    float resultError = 0.0f;
    decimated.resize(meshopt_simplifyWithAttributes(decimated.data(),
        indices.data(), indices.size(),
        weldedPositions.data(), weldedVertexCount, sizeof(float) * 3,
        nullptr, 0, nullptr, 0,
        vertexLock.empty() ? nullptr : vertexLock.data(),
        decimateTriangleCount * 3, FLT_MAX, meshopt_SimplifyRegularize, &resultError));

    if (decimated.size() < 3 || decimated.size() >= indices.size())
        return false;

    std::vector<size_t> outputIndexOfWelded(weldedVertexCount, std::numeric_limits<size_t>::max());
    std::vector<Vector3> decimatedVertices;
    std::vector<std::vector<size_t>> decimatedTriangles;
    decimatedTriangles.reserve(decimated.size() / 3);
    for (size_t i = 0; i + 2 < decimated.size(); i += 3) {
        std::vector<size_t> triangle(3);
        for (size_t j = 0; j < 3; ++j) {
            const unsigned int weldedIndex = decimated[i + j];
            if (std::numeric_limits<size_t>::max() == outputIndexOfWelded[weldedIndex]) {
                outputIndexOfWelded[weldedIndex] = decimatedVertices.size();
                decimatedVertices.push_back(weldedVertices[weldedIndex]);
            }
            triangle[j] = outputIndexOfWelded[weldedIndex];
        }
        decimatedTriangles.push_back(triangle);
    }

    if (nullptr != stats) {
        ++stats->islandsDecimated;
        stats->trianglesBefore += triangles.size();
        stats->trianglesAfter += decimatedTriangles.size();
    }

#if AUTO_REMESHER_DEBUG
    std::cerr << "Island[" << islandIndex << "]: Decimated " << triangles.size()
              << " triangles to " << decimatedTriangles.size()
              << " (target " << decimateTriangleCount
              << ", island target " << (size_t)islandTargetTriangleCount
              << "), normalized error: " << resultError << std::endl;
#else
    (void)islandIndex;
    (void)resultError;
#endif

    vertices = std::move(decimatedVertices);
    triangles = std::move(decimatedTriangles);
    return true;
}

void AutoRemesher::resample(std::vector<Vector3>& vertices,
    std::vector<std::vector<size_t>>& triangles,
    double voxelSize,
    double adaptivity,
    double sharpEdgeDegrees,
    double smoothNormalDegrees,
    size_t islandIndex,
    DecimationStats* decimationStats,
    std::atomic<long long>* adaptiveFieldTimeUs,
    const ProgressHandler* progressHandler,
    std::vector<Vector3>* decimatedVerticesOut,
    std::vector<std::vector<size_t>>* decimatedTrianglesOut)
{
    auto t_decimateStart = std::chrono::high_resolution_clock::now();
    decimateIfTooDense(vertices, triangles, voxelSize, sharpEdgeDegrees, islandIndex, decimationStats);
    if (nullptr != decimationStats) {
        decimationStats->timeUs += std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - t_decimateStart)
                                       .count();
    }

    if (nullptr != decimatedVerticesOut)
        *decimatedVerticesOut = vertices;
    if (nullptr != decimatedTrianglesOut)
        *decimatedTrianglesOut = triangles;

    auto t_fieldStart = std::chrono::high_resolution_clock::now();
    std::vector<double> vertexTargetLengths;
    if (adaptivity > 0.0 && !vertices.empty()) {
        // A target-length field redistributes the uniform triangle budget.  The
        // field is deliberately computed on the input mesh: IsotropicRemesher
        // propagates it to vertices created by edge splits.
        const double minRatio = 0.35;
        const double maxRatio = 3.0;
        const double epsilon = 1e-12;

        // Do not add face normals directly from parallel workers: adjacent
        // faces write to the same vertex.  Compute faces in parallel, then do
        // the small accumulation pass serially.
        std::vector<Vector3> faceNormals(triangles.size());
        std::vector<double> faceAreas(triangles.size(), 0.0);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, triangles.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const auto& tri = triangles[i];
                    faceAreas[i] = Vector3::area(
                        vertices[tri[0]], vertices[tri[1]], vertices[tri[2]]);
                    if (faceAreas[i] > epsilon)
                        faceNormals[i] = Vector3::normal(
                            vertices[tri[0]], vertices[tri[1]], vertices[tri[2]]);
                }
            });

        std::vector<Vector3> normals(vertices.size());
        std::vector<std::vector<size_t>> neighbors(vertices.size());
        for (size_t i = 0; i < triangles.size(); ++i) {
            const auto& tri = triangles[i];
            if (faceAreas[i] <= epsilon)
                continue;
            const Vector3 weightedNormal = faceNormals[i] * faceAreas[i];
            for (size_t j = 0; j < 3; ++j) {
                normals[tri[j]] += weightedNormal;
                neighbors[tri[j]].push_back(tri[(j + 1) % 3]);
                neighbors[tri[j]].push_back(tri[(j + 2) % 3]);
            }
        }
        tbb::parallel_for(tbb::blocked_range<size_t>(0, normals.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    normals[i].normalize();
                    auto& ring = neighbors[i];
                    std::sort(ring.begin(), ring.end());
                    ring.erase(std::unique(ring.begin(), ring.end()), ring.end());
                }
            });

        // Mean normal variation per unit length is less sensitive to a single
        // bad triangle than the previous maximum-one-ring estimate.
        std::vector<double> vertexCurvature(vertices.size(), 0.0);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, vertices.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t v = range.begin(); v != range.end(); ++v) {
                    const auto& ring = neighbors[v];
                    if (ring.empty() || normals[v].lengthSquared() <= epsilon)
                        continue;
                    double weightedCurvature = 0.0;
                    double totalWeight = 0.0;
                    for (const auto& u : ring) {
                        const double length = (vertices[u] - vertices[v]).length();
                        if (length <= epsilon || normals[u].lengthSquared() <= epsilon)
                            continue;
                        double cosine = Vector3::dotProduct(normals[v], normals[u]);
                        cosine = std::max(-1.0, std::min(1.0, cosine));
                        weightedCurvature += std::acos(cosine);
                        totalWeight += length;
                    }
                    if (totalWeight > epsilon)
                        vertexCurvature[v] = weightedCurvature / totalWeight;
                }
            });

        // A percentile reference prevents a few very sharp/noisy vertices from
        // making the rest of the surface appear flat.
        std::vector<double> nonZeroCurvatures;
        nonZeroCurvatures.reserve(vertexCurvature.size());
        for (double curvature : vertexCurvature) {
            if (curvature > epsilon)
                nonZeroCurvatures.push_back(curvature);
        }
        if (!nonZeroCurvatures.empty()) {
            const size_t referenceIndex = (nonZeroCurvatures.size() - 1) * 3 / 4;
            std::nth_element(nonZeroCurvatures.begin(),
                nonZeroCurvatures.begin() + referenceIndex, nonZeroCurvatures.end());
            const double curvatureReference = nonZeroCurvatures[referenceIndex];
            vertexTargetLengths.resize(vertices.size());
            std::vector<double> importance(vertices.size(), 1.0);
            const double strength = std::min(adaptivity, 2.0) * 7.0;
            tbb::parallel_for(tbb::blocked_range<size_t>(0, vertices.size()),
                [&](const tbb::blocked_range<size_t>& range) {
                    for (size_t v = range.begin(); v != range.end(); ++v) {
                        const double normalized = std::min(4.0,
                            vertexCurvature[v] / std::max(curvatureReference, epsilon));
                        importance[v] += strength * normalized * normalized;
                    }
                });

            // Keep integral(area / h^2) equal to the uniform field, which
            // preserves the budget implied by voxelSize while moving triangles
            // from flat regions to detailed ones.
            double totalArea = 0.0;
            double weightedImportance = 0.0;
            for (size_t i = 0; i < triangles.size(); ++i) {
                const auto& tri = triangles[i];
                totalArea += faceAreas[i];
                weightedImportance += faceAreas[i] * (importance[tri[0]] + importance[tri[1]] + importance[tri[2]]) / 3.0;
            }
            if (totalArea > epsilon) {
                const double averageImportance = weightedImportance / totalArea;
                for (size_t v = 0; v < vertices.size(); ++v) {
                    double multiplier = std::sqrt(averageImportance / importance[v]);
                    multiplier = std::max(minRatio, std::min(maxRatio, multiplier));
                    vertexTargetLengths[v] = voxelSize * multiplier;
                }
            } else {
                vertexTargetLengths.clear();
            }
        }
    }
    if (nullptr != adaptiveFieldTimeUs) {
        *adaptiveFieldTimeUs += std::chrono::duration_cast<std::chrono::microseconds>(
            std::chrono::high_resolution_clock::now() - t_fieldStart)
                                    .count();
    }

#if AUTO_REMESHER_DEBUG
    std::cerr << "Island[" << islandIndex << "]: Uniformly remeshing on target edge length: " << voxelSize << std::endl;
#endif
    IsotropicRemesher isotropicRemesher(vertices, triangles);
    if (nullptr != progressHandler && *progressHandler)
        isotropicRemesher.setProgressHandler(*progressHandler);
    isotropicRemesher.setTargetEdgeLength(voxelSize);
    if (!vertexTargetLengths.empty())
        isotropicRemesher.setVertexTargetEdgeLengths(&vertexTargetLengths);
    isotropicRemesher.setSharpEdgeDegrees(sharpEdgeDegrees);
    isotropicRemesher.setSmoothNormalDegrees(smoothNormalDegrees);
    isotropicRemesher.remesh();
    vertices = isotropicRemesher.remeshedVertices();
    triangles = isotropicRemesher.remeshedTriangles();
#if AUTO_REMESHER_DEBUG
    std::cerr << "Island[" << islandIndex << "]: Uniformly remesh done, vertex count: " << vertices.size() << " triangle count: " << triangles.size() << std::endl;
#endif
}

void AutoRemesher::updateProgress(size_t threadIndex, float progress, const char* status)
{
    if (nullptr == m_progressHandler)
        return;

    std::lock_guard<std::mutex> lock(m_progressMutex);
    if (threadIndex >= m_threadProgress.size())
        return;
    if (nullptr != status && '\0' != status[0])
        m_threadStatus[threadIndex] = status;
    if (progress > m_threadProgress[threadIndex]) {
        m_progressSum += (double)(progress - m_threadProgress[threadIndex])
            * m_threadProgressWeights[threadIndex];
        m_threadProgress[threadIndex] = progress;
    }

    const double overall = parallelPhaseBegin
        + (parallelPhaseEnd - parallelPhaseBegin) * std::min(1.0, std::max(0.0, m_progressSum));

    // Steps now report many times per island, so only wake the UI when the bar
    // would actually move or the status line would change.
    const int permille = (int)(overall * 1000.0);
    const char* islandStatus = m_threadStatus[threadIndex];
    if (permille == m_reportedPermille && islandStatus == m_reportedStatus)
        return;
    m_reportedPermille = permille;
    m_reportedStatus = islandStatus;

    // With several islands in flight, the run as a whole is only as far along as
    // its slowest island, so that is the step worth naming.
    size_t slowest = threadIndex;
    for (size_t i = 0; i < m_threadProgress.size(); ++i) {
        if (m_threadProgress[i] < m_threadProgress[slowest])
            slowest = i;
    }
    const char* name = m_threadStatus[slowest];
    m_progressHandler(m_tag, (float)overall, nullptr != name ? name : "");
}

ProgressHandler AutoRemesher::makeStageProgress(size_t islandIndex, float begin, float end, float stageOrder)
{
    return [this, islandIndex, begin, end, stageOrder,
               lastTime = std::chrono::high_resolution_clock::now(),
               lastName = (const char*)nullptr,
               lastOrder = 0.0f](float fraction, const char* name) mutable {
        const auto now = std::chrono::high_resolution_clock::now();
        if (nullptr != lastName) {
            accumulateStageTime(lastName, lastOrder,
                std::chrono::duration_cast<std::chrono::microseconds>(now - lastTime).count());
        }
        lastTime = now;
        lastName = name;
        lastOrder = stageOrder + fraction;
        updateProgress(islandIndex, begin + (end - begin) * fraction, name);
    };
}

void AutoRemesher::accumulateStageTime(const char* name, float order, long long microseconds)
{
    if (nullptr == name || '\0' == name[0])
        return;
    std::lock_guard<std::mutex> lock(m_stageTimingMutex);
    for (auto& it : m_stageTimes) {
        if (it.name == name) {
            it.microseconds += microseconds;
            return;
        }
    }
    m_stageTimes.push_back({ name, order, microseconds });
}

bool AutoRemesher::remesh()
{
    // Validate inputs before any sizing math. In particular a zero target
    // triangle count would divide by zero in initializeVoxelSize().
    const char* invalidInputReason = nullptr;
    if (m_vertices.empty())
        invalidInputReason = "input mesh has no vertices";
    else if (m_triangles.empty())
        invalidInputReason = "input mesh has no triangles";
    else if (0 == m_targetTriangleCount)
        invalidInputReason = "target triangle count must be greater than zero";
    if (nullptr != invalidInputReason) {
        std::cerr << "Invalid remesh input: " << invalidInputReason << std::endl;
        if (nullptr != m_progressHandler)
            m_progressHandler(m_tag, 1.0, invalidInputReason);
        return false;
    }
    auto t_start = std::chrono::high_resolution_clock::now();

    // Each label names the step that is about to run, not the one that just
    // finished, so the status line matches what the process is actually doing.
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.0f, "Computing voxel size");
    auto t_voxelStart = std::chrono::high_resolution_clock::now();
    initializeVoxelSize();
    auto t_voxelEnd = std::chrono::high_resolution_clock::now();

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.01f, "Splitting mesh into islands");
    std::vector<std::vector<std::vector<size_t>>> trianglesIslands;
    auto t_splitStart = std::chrono::high_resolution_clock::now();
    MeshSeparator::splitToIslands(m_triangles, trianglesIslands);
    auto t_afterSplit = std::chrono::high_resolution_clock::now();

    if (trianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        if (nullptr != m_progressHandler)
            m_progressHandler(m_tag, 1.0, "Input mesh is empty");
        return false;
    }

#if AUTO_REMESHER_DEBUG
    std::cerr << "Split to islands: " << trianglesIslands.size() << std::endl;
#endif

    struct IslandContext {
        std::vector<Vector3> vertices;
        std::vector<std::vector<size_t>> triangles;
        double voxelSize;
        double scaling;
        double adaptivity;
        double anisotropy;
        double sharpEdgeDegrees;
        double smoothNormalDegrees;
    };

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.02f, "Building island contexts");
    // Islands are compacted independently of each other, and writing into a
    // pre-sized vector by index keeps them in the original order.
    std::vector<IslandContext> islandContexes(trianglesIslands.size());
    tbb::parallel_for(tbb::blocked_range<size_t>(0, trianglesIslands.size()),
        [&](const tbb::blocked_range<size_t>& range) {
            for (size_t islandIndex = range.begin(); islandIndex != range.end(); ++islandIndex) {
                const auto& island = trianglesIslands[islandIndex];
                IslandContext& context = islandContexes[islandIndex];
                context.triangles.reserve(island.size());
                std::unordered_map<size_t, size_t> oldToNewVertexMap;
                oldToNewVertexMap.reserve(island.size() * 2);
                for (const auto& face : island) {
                    std::vector<size_t> triangle;
                    triangle.reserve(3);
                    for (size_t i = 0; i < 3; ++i) {
                        auto insertResult = oldToNewVertexMap.insert({ face[i], context.vertices.size() });
                        if (insertResult.second)
                            context.vertices.push_back(m_vertices[face[i]]);
                        triangle.push_back(insertResult.first->second);
                    }
                    context.triangles.push_back(std::move(triangle));
                }

                context.scaling = m_scaling;
                context.voxelSize = m_voxelSize;
                context.adaptivity = m_adaptivity;
                context.anisotropy = m_anisotropy;
                context.sharpEdgeDegrees = m_sharpEdgeDegrees;
                context.smoothNormalDegrees = m_smoothNormalDegrees;
            }
        });
    auto t_buildEnd = std::chrono::high_resolution_clock::now();
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, parallelPhaseBegin, "Remeshing uniformly");

    std::atomic<long long> resampleTime(0);
    std::atomic<long long> adaptiveFieldTime(0);
    DecimationStats decimationStats;

    {
        m_threadProgressWeights.assign(islandContexes.size(), 1.0f);
        for (size_t i = 0; i < islandContexes.size(); ++i) {
            if (!m_triangles.empty())
                m_threadProgressWeights[i] = (float)(((double)islandContexes[i].triangles.size() / m_triangles.size()));
        }
        m_threadProgress.assign(islandContexes.size(), 0.0f);
        m_threadStatus.assign(islandContexes.size(), nullptr);
        m_progressSum = 0.0;

        struct IsotropicPhase {
            IsotropicPhase(std::vector<IslandContext>* contexts,
                AutoRemesher* remesher,
                std::atomic<long long>* resampleTime,
                std::atomic<long long>* adaptiveFieldTime,
                DecimationStats* decimationStats,
                std::vector<std::vector<Vector3>>* islandVertices,
                std::vector<std::vector<std::vector<size_t>>>* islandTriangles,
                std::vector<std::vector<Vector3>>* decimatedIslandVertices,
                std::vector<std::vector<std::vector<size_t>>>* decimatedIslandTriangles)
                : m_contexts(contexts)
                , m_remesher(remesher)
                , m_resampleTime(resampleTime)
                , m_adaptiveFieldTime(adaptiveFieldTime)
                , m_decimationStats(decimationStats)
                , m_islandVertices(islandVertices)
                , m_islandTriangles(islandTriangles)
                , m_decimatedIslandVertices(decimatedIslandVertices)
                , m_decimatedIslandTriangles(decimatedIslandTriangles)
            {
            }

            void operator()(const tbb::blocked_range<size_t>& range) const
            {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    auto& ctx = (*m_contexts)[i];

                    m_remesher->updateProgress(i, 0.0f, "Remeshing uniformly");
                    const ProgressHandler isotropicProgress = m_remesher->makeStageProgress(i,
                        0.0f, islandResampleEnd, -1.0f);

                    auto t0 = std::chrono::high_resolution_clock::now();
                    resample(ctx.vertices, ctx.triangles, ctx.voxelSize, ctx.adaptivity, ctx.sharpEdgeDegrees, ctx.smoothNormalDegrees, i, m_decimationStats,
                        m_adaptiveFieldTime, &isotropicProgress,
                        &(*m_decimatedIslandVertices)[i], &(*m_decimatedIslandTriangles)[i]);
                    auto t1 = std::chrono::high_resolution_clock::now();
                    *m_resampleTime += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

                    (*m_islandVertices)[i] = ctx.vertices;
                    (*m_islandTriangles)[i] = ctx.triangles;

                    m_remesher->updateProgress(i, islandResampleEnd);
                }
            }

        private:
            std::vector<IslandContext>* m_contexts = nullptr;
            AutoRemesher* m_remesher = nullptr;
            std::atomic<long long>* m_resampleTime = nullptr;
            std::atomic<long long>* m_adaptiveFieldTime = nullptr;
            DecimationStats* m_decimationStats = nullptr;
            std::vector<std::vector<Vector3>>* m_islandVertices = nullptr;
            std::vector<std::vector<std::vector<size_t>>>* m_islandTriangles = nullptr;
            std::vector<std::vector<Vector3>>* m_decimatedIslandVertices = nullptr;
            std::vector<std::vector<std::vector<size_t>>>* m_decimatedIslandTriangles = nullptr;
        };

        auto mergeIslands = [](const std::vector<std::vector<Vector3>>& islandVertices,
                                const std::vector<std::vector<std::vector<size_t>>>& islandTriangles,
                                std::vector<Vector3>& mergedVertices,
                                std::vector<std::vector<size_t>>& mergedTriangles) {
            for (size_t i = 0; i < islandVertices.size(); ++i) {
                const size_t vertexOffset = mergedVertices.size();
                mergedVertices.insert(mergedVertices.end(),
                    islandVertices[i].begin(), islandVertices[i].end());
                for (const auto& triangle : islandTriangles[i]) {
                    std::vector<size_t> offsetTriangle;
                    offsetTriangle.reserve(triangle.size());
                    for (const size_t index : triangle)
                        offsetTriangle.push_back(index + vertexOffset);
                    mergedTriangles.push_back(std::move(offsetTriangle));
                }
            }
        };

        m_isotropicVertices.clear();
        m_isotropicTriangles.clear();
        m_decimatedVertices.clear();
        m_decimatedTriangles.clear();
        std::vector<std::vector<Vector3>> isotropicIslandVertices(islandContexes.size());
        std::vector<std::vector<std::vector<size_t>>> isotropicIslandTriangles(islandContexes.size());
        std::vector<std::vector<Vector3>> decimatedIslandVertices(islandContexes.size());
        std::vector<std::vector<std::vector<size_t>>> decimatedIslandTriangles(islandContexes.size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, islandContexes.size()),
            IsotropicPhase(&islandContexes, this, &resampleTime, &adaptiveFieldTime,
                &decimationStats,
                &isotropicIslandVertices, &isotropicIslandTriangles,
                &decimatedIslandVertices, &decimatedIslandTriangles));
        mergeIslands(isotropicIslandVertices, isotropicIslandTriangles,
            m_isotropicVertices, m_isotropicTriangles);
        m_decimated = decimationStats.islandsDecimated.load() > 0;
        if (m_decimated) {
            mergeIslands(decimatedIslandVertices, decimatedIslandTriangles,
                m_decimatedVertices, m_decimatedTriangles);
        }
    }
    auto t_isotropicEnd = std::chrono::high_resolution_clock::now();

    class ParameterizationThread {
    public:
        size_t islandIndex = 0;
        IslandContext* island = nullptr;
        std::unique_ptr<Parameterizer> parameterizer;
        std::unique_ptr<QuadExtractor> remesher;
        AutoRemesher* autoRemesher = nullptr;
        std::vector<std::vector<Vector2>> capturedUvs;
        std::vector<std::vector<Vector2>> capturedOriginalUvs;
        std::vector<uint8_t> capturedExtractedConnectionMoved;
        std::vector<Vector3> capturedSingularVertices;
        std::vector<size_t> capturedSingularVertexIndices;
        std::vector<std::pair<Vector3, Vector3>> capturedExtractedConnections;
    };

    std::vector<ParameterizationThread> parameterizationThreads(islandContexes.size());
    for (size_t i = 0; i < islandContexes.size(); ++i) {
        auto& thread = parameterizationThreads[i];
        auto& context = islandContexes[i];
        thread.islandIndex = i;
        thread.island = &context;
        thread.autoRemesher = this;
    }

    class SurfaceParameterizer {
    public:
        SurfaceParameterizer(std::vector<ParameterizationThread>* parameterizationThreads,
            std::atomic<long long>* parameterizeTime,
            std::atomic<long long>* extractTime)
            : m_parameterizationThreads(parameterizationThreads)
            , m_parameterizeTime(parameterizeTime)
            , m_extractTime(extractTime)
        {
        }

        void operator()(const tbb::blocked_range<size_t>& range) const
        {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                auto& thread = (*m_parameterizationThreads)[i];

                auto t0 = std::chrono::high_resolution_clock::now();

                const auto& vertices = thread.island->vertices;
                const auto& triangles = thread.island->triangles;

                if (vertices.empty() || triangles.empty()) {
                    // Still retire the island, otherwise its share of the bar
                    // is never filled in and the total stalls short of the end.
                    thread.autoRemesher->updateProgress(thread.islandIndex, 1.0f);
                    continue;
                }

                thread.autoRemesher->updateProgress(thread.islandIndex, islandResampleEnd);
                thread.parameterizer = std::make_unique<Parameterizer>(&vertices,
                    &triangles,
                    nullptr);
                thread.parameterizer->setProgressHandler(
                    thread.autoRemesher->makeStageProgress(thread.islandIndex,
                        islandResampleEnd, islandParameterizeEnd, 0.0f));
                if (thread.island->scaling > 0.0)
                    thread.parameterizer->setScaling(thread.island->scaling);
                thread.parameterizer->setGradientAdaptivity(thread.island->adaptivity);
                thread.parameterizer->setAnisotropy(thread.island->anisotropy);
                thread.parameterizer->setSharpEdgeDegrees(thread.island->sharpEdgeDegrees);
                bool parameterizeSucceeded = true;
                try {
                    parameterizeSucceeded = thread.parameterizer->parameterize();
                } catch (const std::exception& e) {
                    // A pathological island must not abort the whole remesh,
                    // so log the parameterizer failure and skip its quads.
                    parameterizeSucceeded = false;
                    std::cerr << "Island " << (thread.islandIndex + 1)
                              << ": parameterization failed (" << e.what()
                              << "), skipping this island." << std::endl;
                } catch (...) {
                    parameterizeSucceeded = false;
                    std::cerr << "Island " << (thread.islandIndex + 1)
                              << ": parameterization failed (unknown error), skipping this island." << std::endl;
                }

                auto t1 = std::chrono::high_resolution_clock::now();
                *m_parameterizeTime += std::chrono::duration_cast<std::chrono::microseconds>(t1 - t0).count();

                if (parameterizeSucceeded) {
                    thread.autoRemesher->updateProgress(thread.islandIndex, islandParameterizeEnd);
                    std::unique_ptr<std::vector<std::vector<Vector2>>> uvs = thread.parameterizer->takeTriangleUvs();
                    if (uvs) {
                        // Save a copy of UVs for the [param] preview overlay
                        thread.capturedUvs = *uvs;
                        thread.capturedOriginalUvs = thread.parameterizer->originalTriangleUvs();
                    }
                    // Capture singular vertex positions for the [param] preview
                    thread.capturedSingularVertices = thread.parameterizer->singularVertexPositions();
                    thread.capturedSingularVertexIndices = thread.parameterizer->singularVertexIndices();
                    thread.remesher = std::make_unique<QuadExtractor>(&vertices,
                        &triangles,
                        uvs.get());
                    thread.remesher->setOriginalTriangleUvs(&thread.capturedOriginalUvs);
                    thread.remesher->setSingularVertices(&thread.capturedSingularVertexIndices);
                    thread.remesher->setProgressHandler(
                        thread.autoRemesher->makeStageProgress(thread.islandIndex,
                            islandParameterizeEnd, 1.0f, 1.0f));
                    if (!thread.remesher->extract()) {
                        thread.remesher.reset();
                    } else {
                        thread.capturedExtractedConnections = thread.remesher->extractedConnections();
                        thread.capturedExtractedConnectionMoved = thread.remesher->extractedConnectionMoved();
                    }
                }
                thread.autoRemesher->updateProgress(thread.islandIndex, 1.0f);
                auto t2 = std::chrono::high_resolution_clock::now();
                *m_extractTime += std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count();
            }
        }

    private:
        std::vector<ParameterizationThread>* m_parameterizationThreads = nullptr;
        std::atomic<long long>* m_parameterizeTime = nullptr;
        std::atomic<long long>* m_extractTime = nullptr;
    };
    std::atomic<long long> parameterizeTimeAccumulated(0);
    std::atomic<long long> extractTimeAccumulated(0);

    tbb::parallel_for(tbb::blocked_range<size_t>(0, parameterizationThreads.size()),
        SurfaceParameterizer(&parameterizationThreads,
            &parameterizeTimeAccumulated,
            &extractTimeAccumulated));
    auto t_parallelEnd = std::chrono::high_resolution_clock::now();

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, parallelPhaseEnd, "Merging mesh islands");

    // Merge isotropic UVs from all islands (for [param] preview)
    m_isotropicTriangleUvs.clear();
    m_isotropicOriginalTriangleUvs.clear();
    for (size_t i = 0; i < parameterizationThreads.size(); ++i) {
        auto& thread = parameterizationThreads[i];
        if (thread.capturedUvs.empty())
            continue;
        m_isotropicTriangleUvs.insert(m_isotropicTriangleUvs.end(),
            thread.capturedUvs.begin(), thread.capturedUvs.end());
        m_isotropicOriginalTriangleUvs.insert(m_isotropicOriginalTriangleUvs.end(),
            thread.capturedOriginalUvs.begin(), thread.capturedOriginalUvs.end());
    }

    // Merge singular vertex positions from all islands (for [param] preview)
    m_isotropicSingularVertices.clear();
    for (size_t i = 0; i < parameterizationThreads.size(); ++i) {
        auto& thread = parameterizationThreads[i];
        if (thread.capturedSingularVertices.empty())
            continue;
        m_isotropicSingularVertices.insert(m_isotropicSingularVertices.end(),
            thread.capturedSingularVertices.begin(), thread.capturedSingularVertices.end());
    }

    // Merge the raw quad-extraction connections for the [param] preview.
    m_isotropicExtractedConnections.clear();
    m_isotropicExtractedConnectionMoved.clear();
    for (const auto& thread : parameterizationThreads) {
        m_isotropicExtractedConnections.insert(m_isotropicExtractedConnections.end(),
            thread.capturedExtractedConnections.begin(), thread.capturedExtractedConnections.end());
        m_isotropicExtractedConnectionMoved.resize(m_isotropicExtractedConnections.size()
                - thread.capturedExtractedConnections.size(),
            0);
        m_isotropicExtractedConnectionMoved.insert(m_isotropicExtractedConnectionMoved.end(),
            thread.capturedExtractedConnectionMoved.begin(),
            thread.capturedExtractedConnectionMoved.end());
        m_isotropicExtractedConnectionMoved.resize(m_isotropicExtractedConnections.size(), 0);
    }
    for (size_t i = 0; i < parameterizationThreads.size(); ++i) {
        auto& thread = parameterizationThreads[i];
        if (nullptr == thread.remesher)
            continue;
        const auto& quads = thread.remesher->remeshedQuads();
        if (quads.empty())
            continue;
        const auto& vertices = thread.remesher->remeshedVertices();
        size_t vertexStartIndex = m_remeshedVertices.size();
        m_remeshedVertices.reserve(m_remeshedVertices.size() + vertices.size());
        for (const auto& it : vertices) {
            m_remeshedVertices.push_back(it);
        }
        for (const auto& it : quads) {
            std::vector<size_t> quad;
            quad.reserve(it.size());
            for (const auto& v : it)
                quad.push_back(vertexStartIndex + v);
            m_remeshedQuads.push_back(quad);
        }
    }

    auto t_mergeEnd = std::chrono::high_resolution_clock::now();

    const auto elapsedUs = [](const std::chrono::high_resolution_clock::time_point& from,
                               const std::chrono::high_resolution_clock::time_point& to) {
        return std::chrono::duration_cast<std::chrono::microseconds>(to - from).count();
    };
    const long long t_voxelUs = elapsedUs(t_voxelStart, t_voxelEnd);
    const long long t_splitUs = elapsedUs(t_splitStart, t_afterSplit);
    const long long t_buildUs = elapsedUs(t_afterSplit, t_buildEnd);
    const long long t_isotropicWallUs = elapsedUs(t_buildEnd, t_isotropicEnd);
    const long long t_parameterizeWallUs = elapsedUs(t_isotropicEnd, t_parallelEnd);
    const long long t_parallelWallUs = elapsedUs(t_buildEnd, t_parallelEnd);
    const long long t_mergeUs = elapsedUs(t_parallelEnd, t_mergeEnd);
    const long long t_totalUs = elapsedUs(t_start, t_mergeEnd);

    const long long t_decimateUs = decimationStats.timeUs.load();
    const long long t_adaptiveFieldUs = adaptiveFieldTime.load();
    const size_t decimatedIslands = decimationStats.islandsDecimated.load();

    m_phaseReport.clear();
    {
        std::ostringstream line;
        // Whole milliseconds hide the per-island steps on a mesh split into many
        // small islands, so keep one decimal place.
        const auto milliseconds = [](long long microseconds) {
            std::ostringstream value;
            value.setf(std::ios::fixed);
            value.precision(1);
            value << (double)microseconds / 1000.0 << " ms";
            return value.str();
        };
        auto phase = [&](const char* name, long long microseconds) {
            line.str(std::string());
            line << name << ": " << milliseconds(microseconds);
            m_phaseReport.push_back(line.str());
        };

        line.str(std::string());
        line << "Islands: " << islandContexes.size()
             << ", input triangles: " << m_triangles.size();
        m_phaseReport.push_back(line.str());

        phase("Compute voxel size", t_voxelUs);
        phase("Split into islands", t_splitUs);
        phase("Build island contexts", t_buildUs);

        line.str(std::string());
        if (decimatedIslands > 0) {
            line << "Mesh simplifier: RAN on " << decimatedIslands << " of "
                 << decimationStats.islandsConsidered.load() << " islands, "
                 << decimationStats.trianglesBefore.load() << " -> "
                 << decimationStats.trianglesAfter.load() << " triangles, "
                 << milliseconds(t_decimateUs);
        } else {
            line << "Mesh simplifier: SKIPPED (no island above "
                 << (long long)decimateTriggerRatio << "x target triangle count), "
                 << milliseconds(t_decimateUs);
        }
        m_phaseReport.push_back(line.str());

        // The accumulated figures sum the islands, so on a multi-island mesh they
        // add up to more than the wall clock next to them.  That gap is the point:
        // accumulated / wall is how many cores the phase actually kept busy.
        phase("Adaptive target length field (accumulated)", t_adaptiveFieldUs);
        phase("Isotropic remesh (accumulated)",
            resampleTime.load() - t_decimateUs - t_adaptiveFieldUs);
        phase("Parameterize (accumulated)", parameterizeTimeAccumulated.load());
        phase("Quad extract (accumulated)", extractTimeAccumulated.load());

        {
            std::lock_guard<std::mutex> lock(m_stageTimingMutex);
            std::sort(m_stageTimes.begin(), m_stageTimes.end(),
                [](const StageTime& first, const StageTime& second) {
                    return first.order < second.order;
                });
            for (const auto& it : m_stageTimes) {
                line.str(std::string());
                line << "    " << it.name << ": " << milliseconds(it.microseconds);
                m_phaseReport.push_back(line.str());
            }
        }

        phase("Isotropic phase wall clock", t_isotropicWallUs);
        phase("Parameterize phase wall clock", t_parameterizeWallUs);
        phase("Parallel phase wall clock", t_parallelWallUs);

        {
            const long long accumulated = resampleTime.load()
                + parameterizeTimeAccumulated.load() + extractTimeAccumulated.load();
            line.str(std::string());
            line.setf(std::ios::fixed);
            line.precision(2);
            line << "Cores kept busy across the parallel phase: "
                 << (t_parallelWallUs > 0 ? (double)accumulated / t_parallelWallUs : 0.0)
                 << " (islands are the unit of parallelism)";
            m_phaseReport.push_back(line.str());
        }

        phase("Merge islands", t_mergeUs);
        phase("Total", t_totalUs);
    }

    for (const auto& line : m_phaseReport)
        std::cerr << line << std::endl;

#if AUTO_REMESHER_DEBUG
    std::cerr << "Remesh done" << std::endl;
#endif

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 1.0, "Done");

    return true;
}

}
