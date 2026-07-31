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
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/IsotropicRemesher>
#include <AutoRemesher/MeshSeparator>
#include <AutoRemesher/Parameterizer>
#include <AutoRemesher/PositionKey>
#include <AutoRemesher/QuadExtractor>

#include <QDebug>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <geogram_report_progress.h>
#include <limits>
#include <queue>
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
#else
#include <tbb/parallel_for.h>
#endif
#else
#include <tbb/blocked_range.h>
#include <tbb/mutex.h>
#include <tbb/parallel_for.h>
#endif
#include <unordered_map>
#include <unordered_set>

thread_local void* geogram_report_progress_tag;
thread_local int geogram_report_progress_round;
thread_local int geogram_report_miq_iter = 0;
thread_local geogram_report_progress_handler geogram_report_progress_callback;

static std::atomic_flag s_geogramProgressLock = ATOMIC_FLAG_INIT;

struct GeogramProgressLockGuard {
    GeogramProgressLockGuard()
    {
        constexpr int kMaxAttempts = 6000;
        int attempts = 0;
        while (s_geogramProgressLock.test_and_set(std::memory_order_acquire)) {
            if (++attempts > kMaxAttempts) {
                std::cerr
                    << "Warning: Geogram progress lock appears abandoned "
                    << "(previous run may have crashed). Recovering."
                    << std::endl;
                s_geogramProgressLock.clear(std::memory_order_release);
                std::this_thread::sleep_for(std::chrono::milliseconds(1));
                attempts = 0;
            } else {
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    }
    ~GeogramProgressLockGuard()
    {
        s_geogramProgressLock.clear(std::memory_order_release);
    }
};

namespace AutoRemesher {

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
    qDebug() << "Area:" << area << " voxelSize:" << m_voxelSize;
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

struct ReportProgressContext {
    size_t islandIndex;
    AutoRemesher* autoRemesher;
};

static void ReportProgress(void* tag, float progress)
{
    ReportProgressContext* context = (ReportProgressContext*)tag;
#if AUTO_REMESHER_DEBUG
    //qDebug() << "Island[" << context->islandIndex << "]: round(" << geogram_report_progress_round << ") progress(" << (100 * progress) << "%)";
#endif
    static const char* qc_stages[] = {
        "brush + cross-field alignment",
        "singular vertex detection",
        "cut graph construction",
        "constraint building",
        "solver passes 0-1",
        "solver passes 2-3",
        "mixed-integer solve",
        "result extraction"
    };
    int r = geogram_report_progress_round;
    if (r >= 0 && r < 8) {
        context->autoRemesher->setCurrentStatus(
            "Island " + std::to_string(context->islandIndex + 1) + ": " + qc_stages[r]);
    }
    float base = 0.0f;
    float span = 0.0f;
    switch (r) {
    case 0:
        base = 0.0f;
        span = 0.015f;
        break;
    case 1:
        base = 0.015f;
        span = 0.01f;
        break;
    case 2:
        base = 0.025f;
        span = 0.015f;
        break;
    case 3:
        base = 0.04f;
        span = 0.02f;
        break;
    default:
        base = 0.06f;
        span = 0.94f;
        break;
    }
    float totalProgress = 0.3f + 0.6f * (base + span * progress);
    context->autoRemesher->updateProgress(context->islandIndex, totalProgress);
}

void AutoRemesher::resample(std::vector<Vector3>& vertices,
    std::vector<std::vector<size_t>>& triangles,
    double voxelSize,
    double adaptivity,
    double sharpEdgeDegrees,
    double smoothNormalDegrees,
    size_t islandIndex)
{
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

#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << islandIndex << "]: Uniformly remeshing on target edge length:" << voxelSize;
#endif
    IsotropicRemesher isotropicRemesher(vertices, triangles);
    isotropicRemesher.setTargetEdgeLength(voxelSize);
    if (!vertexTargetLengths.empty())
        isotropicRemesher.setVertexTargetEdgeLengths(&vertexTargetLengths);
    isotropicRemesher.setSharpEdgeDegrees(sharpEdgeDegrees);
    isotropicRemesher.setSmoothNormalDegrees(smoothNormalDegrees);
    isotropicRemesher.remesh();
    vertices = isotropicRemesher.remeshedVertices();
    triangles = isotropicRemesher.remeshedTriangles();
#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << islandIndex << "]: Uniformly remesh done, vertex count:" << vertices.size() << " triangle count:" << triangles.size();
#endif
}

void AutoRemesher::updateProgress(size_t threadIndex, float progress)
{
    if (nullptr == m_progressHandler)
        return;

    if (progress > m_threadProgress[threadIndex])
        m_threadProgress[threadIndex] = progress;
    float islandWeightedAvg = 0.0;
    for (size_t i = 0; i < m_threadProgress.size(); ++i)
        islandWeightedAvg += m_threadProgress[i] * m_threadProgressWeights[i];
    std::string statusCopy;
    {
        std::lock_guard<std::mutex> lock(m_currentStatusMutex);
        statusCopy = m_currentStatus;
    }
    m_progressHandler(m_tag, islandWeightedAvg, statusCopy.c_str());
}

bool AutoRemesher::remesh()
{
    geogram_report_progress_tag = nullptr;
    geogram_report_progress_round = 0;
    geogram_report_progress_callback = nullptr;

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.0, "Initializing...");

    if (m_symmetryEnabled) {
        preprocessSymmetricInputMesh();
    }

    auto t_start = std::chrono::high_resolution_clock::now();


    auto t_voxelStart = std::chrono::high_resolution_clock::now();
    setCurrentStatus("Computing voxel size...");
    initializeVoxelSize();
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.01f, "Computing voxel size...");
    auto t_voxelEnd = std::chrono::high_resolution_clock::now();

    std::vector<std::vector<std::vector<size_t>>> trianglesIslands;
    auto t_splitStart = std::chrono::high_resolution_clock::now();
    setCurrentStatus("Splitting mesh into islands...");
    MeshSeparator::splitToIslands(m_triangles, trianglesIslands);
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.02f, "Splitting mesh into islands...");
    auto t_afterSplit = std::chrono::high_resolution_clock::now();

    if (trianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        if (nullptr != m_progressHandler)
            m_progressHandler(m_tag, 1.0, "Input mesh is empty");
        return false;
    }

#if AUTO_REMESHER_DEBUG
    qDebug() << "Split to islands:" << trianglesIslands.size();
#endif

    struct IslandContext {
        std::vector<Vector3> vertices;
        std::vector<std::vector<size_t>> triangles;
        double voxelSize;
        double scaling;
        double adaptivity;
        double sharpEdgeDegrees;
        double smoothNormalDegrees;
    };

    std::vector<IslandContext> islandContexes;
    islandContexes.reserve(trianglesIslands.size());
    for (size_t islandIndex = 0; islandIndex < trianglesIslands.size(); ++islandIndex) {
        const auto& island = trianglesIslands[islandIndex];
        IslandContext context;
        std::unordered_set<size_t> addedIndices;
        std::unordered_map<size_t, size_t> oldToNewVertexMap;
        for (const auto& face : island) {
            std::vector<size_t> triangle;
            for (size_t i = 0; i < 3; ++i) {
                auto insertResult = addedIndices.insert(face[i]);
                if (insertResult.second) {
                    oldToNewVertexMap.insert({ face[i], context.vertices.size() });
                    context.vertices.push_back(m_vertices[face[i]]);
                }
                triangle.push_back(oldToNewVertexMap[face[i]]);
            }
            context.triangles.push_back(triangle);
        }

        context.scaling = m_scaling;
        context.voxelSize = m_voxelSize;
        context.adaptivity = m_adaptivity;
        context.sharpEdgeDegrees = m_sharpEdgeDegrees;
        context.smoothNormalDegrees = m_smoothNormalDegrees;

        islandContexes.push_back(context);
    }
    setCurrentStatus("Building island contexts...");
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.03f, "Building island contexts...");
    auto t_buildEnd = std::chrono::high_resolution_clock::now();

    {
        m_threadProgressWeights.resize(islandContexes.size(), 1.0);
        for (size_t i = 0; i < islandContexes.size(); ++i) {
            if (!m_triangles.empty())
                m_threadProgressWeights[i] = (float)(((double)islandContexes[i].triangles.size() / m_triangles.size()));
        }
        m_threadProgress.resize(islandContexes.size());

        struct IsotropicPhase {
            IsotropicPhase(std::vector<IslandContext>* contexts,
                AutoRemesher* remesher,
                std::atomic<long long>* resampleTime,
                std::vector<Vector3>* outVertices,
                std::vector<std::vector<size_t>>* outTriangles)
                : m_contexts(contexts)
                , m_remesher(remesher)
                , m_resampleTime(resampleTime)
                , m_outVertices(outVertices)
                , m_outTriangles(outTriangles)
            {
            }

            void operator()(const tbb::blocked_range<size_t>& range) const
            {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    auto& ctx = (*m_contexts)[i];

                    m_remesher->setCurrentStatus(
                        "Island " + std::to_string(i + 1) + ": isotropic remeshing...");
                    m_remesher->updateProgress(i, 0.0f);

                    auto t0 = std::chrono::high_resolution_clock::now();
                    resample(ctx.vertices, ctx.triangles, ctx.voxelSize, ctx.adaptivity, ctx.sharpEdgeDegrees, ctx.smoothNormalDegrees, i);
                    auto t1 = std::chrono::high_resolution_clock::now();
                    *m_resampleTime += std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

                    // Collect isotropic mesh vertices/triangles for later preview
                    // Must accumulate in the correct order across islands
                    size_t vertexOffset = m_outVertices->size();
                    for (const auto& v : ctx.vertices)
                        m_outVertices->push_back(v);
                    for (const auto& tri : ctx.triangles) {
                        std::vector<size_t> offsetTri;
                        for (auto idx : tri)
                            offsetTri.push_back(idx + vertexOffset);
                        m_outTriangles->push_back(offsetTri);
                    }

                    m_remesher->updateProgress(i, 0.3f);
                }
            }

        private:
            std::vector<IslandContext>* m_contexts = nullptr;
            AutoRemesher* m_remesher = nullptr;
            std::atomic<long long>* m_resampleTime = nullptr;
            std::vector<Vector3>* m_outVertices = nullptr;
            std::vector<std::vector<size_t>>* m_outTriangles = nullptr;
        };

        std::atomic<long long> resampleTime(0);

        m_isotropicVertices.clear();
        m_isotropicTriangles.clear();
        tbb::parallel_for(tbb::blocked_range<size_t>(0, islandContexes.size()),
            IsotropicPhase(&islandContexes, this, &resampleTime,
                &m_isotropicVertices, &m_isotropicTriangles));
    }

    class ParameterizationThread {
    public:
        ~ParameterizationThread()
        {
            delete parameterizer;
            delete remesher;
        }

        size_t islandIndex = 0;
        IslandContext* island = nullptr;
        Parameterizer* parameterizer = nullptr;
        QuadExtractor* remesher = nullptr;
        AutoRemesher* autoRemesher = nullptr;
        std::vector<std::vector<Vector2>> capturedUvs;
        std::vector<Vector3> capturedSingularVertices;
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

                if (vertices.empty() || triangles.empty())
                    continue;

                ReportProgressContext reportProgressContext;
                reportProgressContext.islandIndex = i;
                reportProgressContext.autoRemesher = thread.autoRemesher;
                geogram_report_progress_tag = &reportProgressContext;
                geogram_report_progress_round = 0;
                geogram_report_progress_callback = ReportProgress;

                thread.autoRemesher->setCurrentStatus(
                    "Island " + std::to_string(thread.islandIndex + 1) + ": computing normals & frame field...");
                thread.autoRemesher->updateProgress(thread.islandIndex, 0.3f);
                thread.parameterizer = new Parameterizer(&vertices,
                    &triangles,
                    nullptr);
                thread.parameterizer->setScaling(thread.island->scaling);
                thread.parameterizer->setGradientAdaptivity(thread.island->adaptivity);
                thread.parameterizer->setSharpEdgeDegrees(thread.island->sharpEdgeDegrees);
                bool parameterizeSucceeded = true;
                try {
                    GeogramProgressLockGuard lock;
                    thread.parameterizer->parameterize();
                } catch (const std::exception& e) {
                    // Geogram reports failed assertions by throwing (e.g. the
                    // manifold checks in quad_cover). One pathological island must
                    // not abort the whole remesh, so log it and skip its quads.
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
                *m_parameterizeTime += std::chrono::duration_cast<std::chrono::milliseconds>(t1 - t0).count();

                if (parameterizeSucceeded) {
                    thread.autoRemesher->setCurrentStatus(
                        "Island " + std::to_string(thread.islandIndex + 1) + ": extracting quads...");
                    thread.autoRemesher->updateProgress(thread.islandIndex, 0.9f);
                    std::vector<std::vector<Vector2>>* uvs = thread.parameterizer->takeTriangleUvs();
                    if (uvs) {
                        // Save a copy of UVs for the [param] preview overlay
                        thread.capturedUvs = *uvs;
                    }
                    // Capture singular vertex positions for the [param] preview
                    thread.capturedSingularVertices = thread.parameterizer->singularVertexPositions();
                    thread.remesher = new QuadExtractor(&vertices,
                        &triangles,
                        uvs);
                    if (!thread.remesher->extract()) {
                        delete thread.remesher;
                        thread.remesher = nullptr;
                    } else {
                        thread.capturedExtractedConnections = thread.remesher->extractedConnections();
                    }
                    delete uvs;
                }
                thread.autoRemesher->updateProgress(thread.islandIndex, 1.0f);
                thread.autoRemesher->setCurrentStatus(
                    "Island " + std::to_string(thread.islandIndex + 1) + ": done");
                auto t2 = std::chrono::high_resolution_clock::now();
                *m_extractTime += std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
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

    setCurrentStatus("Merging mesh islands...");
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.95f, "Merging mesh islands...");

    // Merge isotropic UVs from all islands (for [param] preview)
    m_isotropicTriangleUvs.clear();
    for (size_t i = 0; i < parameterizationThreads.size(); ++i) {
        auto& thread = parameterizationThreads[i];
        if (thread.capturedUvs.empty())
            continue;
        m_isotropicTriangleUvs.insert(m_isotropicTriangleUvs.end(),
            thread.capturedUvs.begin(), thread.capturedUvs.end());
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
    for (const auto& thread : parameterizationThreads) {
        m_isotropicExtractedConnections.insert(m_isotropicExtractedConnections.end(),
            thread.capturedExtractedConnections.begin(), thread.capturedExtractedConnections.end());
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

    if (m_symmetryEnabled) {
        setCurrentStatus("Applying symmetry & mirror pass...");
        applySymmetryPass();
    }

    auto t_mergeEnd = std::chrono::high_resolution_clock::now();

    auto t_voxelMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_voxelEnd - t_voxelStart).count();
    auto t_splitMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_afterSplit - t_splitStart).count();
    auto t_buildMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_buildEnd - t_afterSplit).count();
    auto t_parallelWallMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_parallelEnd - t_buildEnd).count();
    auto t_mergeMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_mergeEnd - t_parallelEnd).count();
    auto t_totalMs = std::chrono::duration_cast<std::chrono::milliseconds>(t_mergeEnd - t_start).count();

    qDebug() << "Quad mesh breakdown: total" << t_totalMs << "ms"
             << "| voxel" << t_voxelMs << "ms"
             << "| split" << t_splitMs << "ms"
             << "| build" << t_buildMs << "ms"
             << "| parallel" << t_parallelWallMs << "ms"
             << "  (param" << parameterizeTimeAccumulated.load() << "ms"
             << "| extract" << extractTimeAccumulated.load() << "ms)"
             << "| merge" << t_mergeMs << "ms";

#if AUTO_REMESHER_DEBUG
    qDebug() << "Remesh done";
#endif

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 1.0, "Done");

    return true;
}

void AutoRemesher::preprocessSymmetricInputMesh()
{
    m_symmetryPlaneValid = false;
    if (!m_symmetryEnabled || m_vertices.empty() || m_triangles.empty())
        return;

    int axis = m_symmetryAxis; // 0=X, 1=Y, 2=Z
    if (axis < 0 || axis > 2) axis = 0;

    double bboxMin = std::numeric_limits<double>::max();
    double bboxMax = std::numeric_limits<double>::lowest();
    for (const auto& v : m_vertices) {
        double c = v[axis];
        if (c < bboxMin) bboxMin = c;
        if (c > bboxMax) bboxMax = c;
    }
    double meshCenter = (bboxMin + bboxMax) * 0.5;
    double bboxSpan = std::max(bboxMax - bboxMin, 1e-4);
    double cutPlanePos = meshCenter + m_centerOffset * (0.5 * bboxSpan);
    double effectiveTolerance = (m_seamTolerance <= 0.05) ? std::max(m_seamTolerance * bboxSpan, 1e-4) : m_seamTolerance;

    std::vector<std::vector<size_t>> halfTriangles;
    halfTriangles.reserve(m_triangles.size());
    std::set<size_t> discardedVertIndices;

    for (const auto& tri : m_triangles) {
        if (tri.size() < 3) continue;
        double centroidCoord = 0.0;
        for (auto vIdx : tri) {
            if (vIdx < m_vertices.size()) {
                centroidCoord += m_vertices[vIdx][axis];
            }
        }
        centroidCoord /= (double)tri.size();

        if (centroidCoord >= cutPlanePos) {
            halfTriangles.push_back(tri);
        } else {
            for (auto vIdx : tri)
                discardedVertIndices.insert(vIdx);
        }
    }

    if (halfTriangles.empty() || halfTriangles.size() >= m_triangles.size()) {
        return; // Cut plane is outside mesh bounds, skip
    }

    std::set<size_t> activeVertIndices;
    for (const auto& tri : halfTriangles) {
        for (auto vIdx : tri) {
            activeVertIndices.insert(vIdx);
        }
    }

    std::vector<Vector3> newVertices;
    std::map<size_t, size_t> oldToNewIndexMap;

    for (auto oldIdx : activeVertIndices) {
        Vector3 v = m_vertices[oldIdx];
        // Project onto the cut plane: seam verts (shared with a discarded face) MUST
        // land exactly on the plane or the mirrored half cannot weld to them (cracks);
        // below-plane verts and the tolerance band are projected for a straight seam.
        bool onSeam = discardedVertIndices.count(oldIdx) > 0;
        if (onSeam || v[axis] < cutPlanePos || std::abs(v[axis] - cutPlanePos) <= effectiveTolerance) {
            v[axis] = cutPlanePos;
        }
        size_t newIdx = newVertices.size();
        newVertices.push_back(v);
        oldToNewIndexMap[oldIdx] = newIdx;
    }

    std::vector<std::vector<size_t>> newTriangles;
    newTriangles.reserve(halfTriangles.size() * 2);

    for (const auto& tri : halfTriangles) {
        std::vector<size_t> t;
        t.reserve(tri.size());
        size_t onPlaneCount = 0;
        for (auto vIdx : tri) {
            size_t ni = oldToNewIndexMap[vIdx];
            if (std::abs(newVertices[ni][axis] - cutPlanePos) <= 1e-9)
                ++onPlaneCount;
            t.push_back(ni);
        }
        if (onPlaneCount == tri.size())
            continue; // fully in the plane -> zero area, would mirror onto itself
        newTriangles.push_back(t);
    }

    if (newTriangles.empty())
        return;

    size_t halfVertCount = newVertices.size();
    size_t halfTriCount = newTriangles.size();

    std::vector<size_t> mirroredVertexIndexMap(halfVertCount);
    for (size_t i = 0; i < halfVertCount; ++i) {
        Vector3 v = newVertices[i];
        if (std::abs(v[axis] - cutPlanePos) <= 1e-9) {
            mirroredVertexIndexMap[i] = i;
        } else {
            Vector3 mv = v;
            mv[axis] = 2.0 * cutPlanePos - v[axis];

            size_t newIdx = newVertices.size();
            newVertices.push_back(mv);
            mirroredVertexIndexMap[i] = newIdx;
        }
    }

    for (size_t i = 0; i < halfTriCount; ++i) {
        const auto& tri = newTriangles[i];
        if (tri.size() == 3) {
            std::vector<size_t> mirroredTri = {
                mirroredVertexIndexMap[tri[2]],
                mirroredVertexIndexMap[tri[1]],
                mirroredVertexIndexMap[tri[0]]
            };
            if (mirroredTri[0] == tri[2] && mirroredTri[1] == tri[1] && mirroredTri[2] == tri[0])
                continue; // maps onto itself, would duplicate
            newTriangles.push_back(mirroredTri);
        }
    }

    m_vertices = std::move(newVertices);
    m_triangles = std::move(newTriangles);
    m_symmetryPlanePos = cutPlanePos;
    m_symmetryEffectiveTolerance = effectiveTolerance;
    m_symmetryPlaneValid = true;
}

void AutoRemesher::applySymmetryPass()
{
    if (!m_symmetryEnabled || !m_symmetryPlaneValid || m_remeshedVertices.empty() || m_remeshedQuads.empty())
        return;

    int axis = m_symmetryAxis; // 0=X, 1=Y, 2=Z
    if (axis < 0 || axis > 2) axis = 0;

    // Mirror around the exact plane the input was symmetrized on. Recomputing it from
    // the remeshed bounding box picks a slightly different plane (quad_cover is not
    // symmetric), which shifts the seam and breaks the mirror.
    double cutPlanePos = m_symmetryPlanePos;
    double effectiveTolerance = m_symmetryEffectiveTolerance;

    // 1. Filter quads: keep quads whose centroid lies on positive side of symmetry plane
    std::vector<std::vector<size_t>> halfQuads;
    halfQuads.reserve(m_remeshedQuads.size());
    std::set<size_t> discardedVertIndices;

    for (const auto& q : m_remeshedQuads) {
        if (q.size() < 3) continue;
        double centroidCoord = 0.0;
        for (auto vIdx : q) {
            if (vIdx < m_remeshedVertices.size()) {
                centroidCoord += m_remeshedVertices[vIdx][axis];
            }
        }
        centroidCoord /= (double)q.size();

        if (centroidCoord >= cutPlanePos) {
            halfQuads.push_back(q);
        } else {
            for (auto vIdx : q)
                discardedVertIndices.insert(vIdx);
        }
    }

    // Safety Guard: If cut plane is outside mesh boundary (all or no quads selected), do not duplicate model!
    if (halfQuads.empty() || halfQuads.size() >= m_remeshedQuads.size()) {
        return;
    }

    // Re-index active vertices from halfQuads
    std::set<size_t> activeVertIndices;
    for (const auto& q : halfQuads) {
        for (auto vIdx : q) {
            activeVertIndices.insert(vIdx);
        }
    }

    std::vector<Vector3> newVertices;
    std::map<size_t, size_t> oldToNewIndexMap;

    for (auto oldIdx : activeVertIndices) {
        Vector3 v = m_remeshedVertices[oldIdx];
        // Project onto the cut plane: seam verts (shared with a discarded quad) MUST
        // land exactly on the plane or the mirrored half cannot weld to them (cracks);
        // below-plane verts and the tolerance band are projected for a straight seam.
        bool onSeam = discardedVertIndices.count(oldIdx) > 0;
        if (onSeam || v[axis] < cutPlanePos || std::abs(v[axis] - cutPlanePos) <= effectiveTolerance) {
            v[axis] = cutPlanePos;
        }
        size_t newIdx = newVertices.size();
        newVertices.push_back(v);
        oldToNewIndexMap[oldIdx] = newIdx;
    }

    std::vector<std::vector<size_t>> newQuads;
    newQuads.reserve(halfQuads.size() * 2);

    for (const auto& q : halfQuads) {
        std::vector<size_t> quad;
        quad.reserve(q.size());
        size_t onPlaneCount = 0;
        for (auto vIdx : q) {
            size_t ni = oldToNewIndexMap[vIdx];
            if (std::abs(newVertices[ni][axis] - cutPlanePos) <= 1e-9)
                ++onPlaneCount;
            quad.push_back(ni);
        }
        if (onPlaneCount == q.size())
            continue; // fully in the plane -> degenerate wall, would mirror onto itself
        newQuads.push_back(quad);
    }

    if (newQuads.empty())
        return;

    // 2. Mirror newVertices across cutPlanePos to create opposite half
    size_t halfVertCount = newVertices.size();
    size_t halfQuadCount = newQuads.size();

    std::vector<size_t> mirroredVertexIndexMap(halfVertCount);
    for (size_t i = 0; i < halfVertCount; ++i) {
        Vector3 v = newVertices[i];
        if (std::abs(v[axis] - cutPlanePos) <= 1e-9) {
            mirroredVertexIndexMap[i] = i; // Seam vertex connects directly to itself!
        } else {
            Vector3 mv = v;
            mv[axis] = 2.0 * cutPlanePos - v[axis];

            size_t newIdx = newVertices.size();
            newVertices.push_back(mv);
            mirroredVertexIndexMap[i] = newIdx;
        }
    }

    // 3. Mirror halfQuads with reversed vertex order to maintain outward face normals
    for (size_t i = 0; i < halfQuadCount; ++i) {
        const auto& q = newQuads[i];
        std::vector<size_t> mirroredQuad(q.size());
        bool selfMapped = true;
        for (size_t j = 0; j < q.size(); ++j) {
            size_t src = q[q.size() - 1 - j];
            mirroredQuad[j] = mirroredVertexIndexMap[src];
            if (mirroredQuad[j] != src)
                selfMapped = false;
        }
        if (selfMapped)
            continue; // maps onto itself, would duplicate
        newQuads.push_back(mirroredQuad);
    }

    m_remeshedVertices = std::move(newVertices);
    m_remeshedQuads = std::move(newQuads);
}


}
