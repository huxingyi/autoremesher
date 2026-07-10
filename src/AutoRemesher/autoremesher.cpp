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
#include <AutoRemesher/QuadExtractor>
#include <QDebug>
#include <atomic>
#include <chrono>
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
        const double isoAdaptivity = adaptivity;
        const double minRatio = 0.3;
        const double maxRatio = 3.0;

        std::vector<Vector3> normals(vertices.size());
        tbb::parallel_for(tbb::blocked_range<size_t>(0, triangles.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    const auto& tri = triangles[i];
                    Vector3 n = Vector3::normal(
                        vertices[tri[0]], vertices[tri[1]], vertices[tri[2]]);
                    normals[tri[0]] += n;
                    normals[tri[1]] += n;
                    normals[tri[2]] += n;
                }
            });
        tbb::parallel_for(tbb::blocked_range<size_t>(0, normals.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i)
                    normals[i].normalize();
            });

        std::vector<std::vector<size_t>> faceAroundVertex(vertices.size());
        for (size_t i = 0; i < triangles.size(); ++i) {
            for (size_t j = 0; j < 3; ++j)
                faceAroundVertex[triangles[i][j]].push_back(i);
        }

        std::vector<double> vertexCurvature(vertices.size(), 0.0);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, vertices.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t v = range.begin(); v != range.end(); ++v) {
                    const auto& faces = faceAroundVertex[v];
                    if (faces.empty())
                        continue;
                    const auto& normalV = normals[v];
                    double maxCurvature = 0.0;
                    for (const auto& faceIndex : faces) {
                        for (const auto& u : triangles[faceIndex]) {
                            if (u == v)
                                continue;
                            double dist = (vertices[u] - vertices[v]).length();
                            if (dist <= 0.0)
                                continue;
                            double cosA = Vector3::dotProduct(normalV, normals[u]);
                            if (cosA > 1.0)
                                cosA = 1.0;
                            else if (cosA < -1.0)
                                cosA = -1.0;
                            double curv = std::acos(cosA) / dist;
                            if (curv > maxCurvature)
                                maxCurvature = curv;
                        }
                    }
                    vertexCurvature[v] = maxCurvature;
                }
            });

        double sumCurvature = 0.0;
        for (const auto& c : vertexCurvature)
            sumCurvature += c;
        double avgCurvature = sumCurvature / vertexCurvature.size();

        if (avgCurvature > 0.0) {
            vertexTargetLengths.resize(vertices.size());
            tbb::parallel_for(tbb::blocked_range<size_t>(0, vertices.size()),
                [&](const tbb::blocked_range<size_t>& range) {
                    for (size_t v = range.begin(); v != range.end(); ++v) {
                        double normalized = vertexCurvature[v] / avgCurvature;
                        if (normalized < 1e-3)
                            normalized = 1e-3;
                        double multiplier = std::pow(normalized, -isoAdaptivity);
                        if (multiplier < minRatio)
                            multiplier = minRatio;
                        else if (multiplier > maxRatio)
                            multiplier = maxRatio;
                        vertexTargetLengths[v] = voxelSize * multiplier;
                    }
                });
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
                std::atomic<long long>* resampleTime)
                : m_contexts(contexts)
                , m_remesher(remesher)
                , m_resampleTime(resampleTime)
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

                    m_remesher->updateProgress(i, 0.3f);
                }
            }

        private:
            std::vector<IslandContext>* m_contexts = nullptr;
            AutoRemesher* m_remesher = nullptr;
            std::atomic<long long>* m_resampleTime = nullptr;
        };

        std::atomic<long long> resampleTime(0);

        tbb::parallel_for(tbb::blocked_range<size_t>(0, islandContexes.size()),
            IsotropicPhase(&islandContexes, this, &resampleTime));
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
                    thread.remesher = new QuadExtractor(&vertices,
                        &triangles,
                        uvs);
                    if (!thread.remesher->extract()) {
                        delete thread.remesher;
                        thread.remesher = nullptr;
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

}