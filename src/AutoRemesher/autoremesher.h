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
#ifndef AUTO_REMESHER_AUTO_REMESHER_H
#define AUTO_REMESHER_AUTO_REMESHER_H
#include <AutoRemesher/Progress>
#include <AutoRemesher/Vector3>
#include <atomic>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

namespace AutoRemesher {

class IsotropicRemesher;

enum class ModelType {
    Organic,
    HardSurface
};

typedef void (*AutoRemesherProgressHandler)(void* tag, float progress, const char* status);

class AutoRemesher {
public:
    AutoRemesher(const std::vector<Vector3>& vertices,
        const std::vector<std::vector<size_t>>& triangles)
        : m_vertices(vertices)
        , m_triangles(triangles)
    {
    }

    void setTargetTriangleCount(size_t targetTriangleCount)
    {
        m_targetTriangleCount = targetTriangleCount;
    }

    void setScaling(double scaling)
    {
        m_scaling = scaling;
    }

    void setProgressHandler(AutoRemesherProgressHandler progressHandler)
    {
        m_progressHandler = progressHandler;
    }

    void setTag(void* tag)
    {
        m_tag = tag;
    }

    void setModelType(ModelType modelType)
    {
        m_modelType = modelType;
    }

    void setGradientAdaptivity(double adaptivity)
    {
        m_adaptivity = adaptivity;
    }

    void setSharpEdgeDegrees(double degrees)
    {
        m_sharpEdgeDegrees = degrees;
    }

    void setAnisotropy(double anisotropy)
    {
        m_anisotropy = anisotropy;
    }

    void setSmoothNormalDegrees(double degrees)
    {
        m_smoothNormalDegrees = degrees;
    }

    const std::vector<Vector3>& remeshedVertices()
    {
        return m_remeshedVertices;
    }

    const std::vector<std::vector<size_t>>& remeshedQuads()
    {
        return m_remeshedQuads;
    }

    const std::vector<Vector3>& decimatedVertices()
    {
        return m_decimatedVertices;
    }

    const std::vector<std::vector<size_t>>& decimatedTriangles()
    {
        return m_decimatedTriangles;
    }

    bool decimated()
    {
        return m_decimated;
    }

    const std::vector<Vector3>& isotropicVertices()
    {
        return m_isotropicVertices;
    }

    const std::vector<std::vector<size_t>>& isotropicTriangles()
    {
        return m_isotropicTriangles;
    }

    const std::vector<uint8_t>& isotropicExtractedConnectionMoved()
    {
        return m_isotropicExtractedConnectionMoved;
    }

    const std::vector<std::vector<Vector2>>& isotropicOriginalTriangleUvs()
    {
        return m_isotropicOriginalTriangleUvs;
    }

    const std::vector<std::vector<Vector2>>& isotropicTriangleUvs()
    {
        return m_isotropicTriangleUvs;
    }

    const std::vector<Vector3>& isotropicSingularVertices()
    {
        return m_isotropicSingularVertices;
    }

    const std::vector<std::pair<Vector3, Vector3>>& isotropicExtractedConnections()
    {
        return m_isotropicExtractedConnections;
    }

    bool remesh();

    // `progress` is how far island `threadIndex` has got, 0..1.  `status` names
    // the step it is on, or nullptr to keep the island's current one.  Called
    // from the island worker threads.
    void updateProgress(size_t threadIndex, float progress, const char* status = nullptr);

    // Records how long a named pipeline step took, summed over the islands that
    // ran it.  `order` places the step in the phase report; it is the step's
    // position along the pipeline, so the report reads in execution order no
    // matter which island happened to reach the step first.
    void accumulateStageTime(const char* name, float order, long long microseconds);

    // A handler for one stage of one island: maps the stage's own 0..1 fraction
    // onto [begin, end] of that island's progress, and times each named step on
    // the way through for the phase report.  `stageOrder` is where the stage sits
    // along the pipeline, so the report reads in execution order.  Only the
    // island's own worker thread calls the result.
    ProgressHandler makeStageProgress(size_t islandIndex, float begin, float end, float stageOrder);

    const std::vector<std::string>& phaseReport()
    {
        return m_phaseReport;
    }

    static const double m_defaultSharpEdgeDegrees;

    // Per-island durations are accumulated in microseconds: a mesh split into
    // hundreds of islands spends well under a millisecond on most of them, and
    // truncating each one to whole milliseconds loses the bulk of the total.
    struct DecimationStats {
        std::atomic<long long> timeUs { 0 };
        std::atomic<size_t> islandsDecimated { 0 };
        std::atomic<size_t> islandsConsidered { 0 };
        std::atomic<size_t> trianglesBefore { 0 };
        std::atomic<size_t> trianglesAfter { 0 };
    };

private:
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_triangles;
    std::vector<Vector3> m_remeshedVertices;
    std::vector<std::vector<size_t>> m_remeshedQuads;
    std::vector<Vector3> m_decimatedVertices;
    std::vector<std::vector<size_t>> m_decimatedTriangles;
    bool m_decimated = false;
    std::vector<Vector3> m_isotropicVertices;
    std::vector<std::vector<size_t>> m_isotropicTriangles;
    std::vector<std::vector<Vector2>> m_isotropicTriangleUvs;
    std::vector<std::vector<Vector2>> m_isotropicOriginalTriangleUvs;
    std::vector<uint8_t> m_isotropicExtractedConnectionMoved;
    std::vector<Vector3> m_isotropicSingularVertices;
    std::vector<std::pair<Vector3, Vector3>> m_isotropicExtractedConnections;
    std::vector<float> m_threadProgress;
    std::vector<float> m_threadProgressWeights;
    std::vector<const char*> m_threadStatus;
    // The weighted sum of m_threadProgress, kept incrementally so that a
    // fine-grained update stays O(1) rather than a scan of every island.
    double m_progressSum = 0.0;
    int m_reportedPermille = -1;
    const char* m_reportedStatus = nullptr;
    std::vector<std::string> m_phaseReport;
    mutable std::mutex m_progressMutex;
    std::mutex m_stageTimingMutex;
    struct StageTime {
        std::string name;
        float order = 0.0f;
        long long microseconds = 0;
    };
    std::vector<StageTime> m_stageTimes;
    double m_scaling = 0.0;
    size_t m_targetTriangleCount = 0;
    double m_voxelSize = 0.0;
    double m_adaptivity = 1.0;
    double m_anisotropy = 1.0;
    double m_sharpEdgeDegrees = m_defaultSharpEdgeDegrees;
    double m_smoothNormalDegrees = 0.0;
    ModelType m_modelType = ModelType::Organic;
    AutoRemesherProgressHandler m_progressHandler = nullptr;
    void* m_tag = nullptr;

    static double calculateAverageEdgeLength(const std::vector<Vector3>& vertices,
        const std::vector<std::vector<size_t>>& faces);
    void initializeVoxelSize();
    static bool decimateIfTooDense(std::vector<Vector3>& vertices,
        std::vector<std::vector<size_t>>& triangles,
        double voxelSize,
        double sharpEdgeDegrees,
        size_t islandIndex,
        DecimationStats* stats);
    static void resample(std::vector<Vector3>& vertices,
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
        std::vector<std::vector<size_t>>* decimatedTrianglesOut);
    static double calculateMeshArea(const std::vector<Vector3>& vertices,
        const std::vector<std::vector<size_t>>& triangles);
};

}

#endif
