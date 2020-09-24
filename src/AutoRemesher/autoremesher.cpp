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
#include <limits>
#include <unordered_set>
#include <unordered_map>
#include <queue>
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/IsotropicRemesher>
#include <AutoRemesher/Parameterizer>
#include <AutoRemesher/QuadExtractor>
#include <AutoRemesher/VdbRemesher>
#include <AutoRemesher/MeshSeparator>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/tbb_thread.h>
#include <geogram_report_progress.h>

thread_local void *geogram_report_progress_tag;
thread_local int geogram_report_progress_round;
thread_local geogram_report_progress_handler geogram_report_progress_callback;

#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

namespace AutoRemesher
{
    
const double AutoRemesher::m_defaultSharpEdgeDegrees = 90;

double AutoRemesher::calculateAverageEdgeLength(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &faces)
{
    double sumOfLength = 0.0;
    size_t edgeCount = 0;
    for (const auto &face: faces) {
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

double AutoRemesher::calculateMeshArea(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles)
{
    double area = 0.0;
    for (const auto &it: triangles) {
        area += Vector3::area(vertices[it[0]], vertices[it[1]], vertices[it[2]]);
    }
    return area;
}

struct ReportProgressContext
{
    size_t islandIndex;
    AutoRemesher *autoRemesher;
};

static void ReportProgress(void *tag, float progress)
{
    ReportProgressContext *context = (ReportProgressContext *)tag;
#if AUTO_REMESHER_DEBUG
    //qDebug() << "Island[" << context->islandIndex << "]: round(" << geogram_report_progress_round << ") progress(" << (100 * progress) << "%)";
#endif
    context->autoRemesher->updateProgress(context->islandIndex, (float)geogram_report_progress_round / 8 + progress / 8);
}

void AutoRemesher::resample(std::vector<Vector3> &vertices, 
    std::vector<std::vector<size_t>> &triangles, 
    double voxelSize,
    size_t islandIndex)
{
    std::vector<Vector3> *vdbVertices = nullptr;
    std::vector<std::vector<size_t>> *vdbTriangles = nullptr;
    
#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << islandIndex << "]: Vdb remeshing on voxel size:" << voxelSize;
#endif
    double areaBeforeVdbRemesh = calculateMeshArea(vertices, triangles);
    VdbRemesher vdbRemesher(&vertices, &triangles);
    vdbRemesher.setVoxelSize(voxelSize);
    vdbRemesher.remesh();
    delete vdbVertices;
    delete vdbTriangles;
    vdbVertices = vdbRemesher.takeVdbVertices();
    vdbTriangles = vdbRemesher.takeVdbTriangles();
    double areaAfterVdbRemesh = calculateMeshArea(*vdbVertices, *vdbTriangles);
#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << islandIndex << "]: Vdb remeshed to vertex count:" << vdbVertices->size() << "triangle count:" << vdbTriangles->size() << "on voxel size:" << voxelSize;
    qDebug() << "Island[" << islandIndex << "]: Area before vdb remesh:" << areaBeforeVdbRemesh;
    qDebug() << "Island[" << islandIndex << "]: Area after vdb remesh:" << areaAfterVdbRemesh;
#endif
    bool isVdbGood = (vdbTriangles->size() >= 100 && std::abs(areaAfterVdbRemesh - areaBeforeVdbRemesh) < areaBeforeVdbRemesh * 0.5);
    
#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << islandIndex << "]: Uniformly remeshing... isVdbGood:" << isVdbGood;
#endif
    IsotropicRemesher *isotropicRemesher = nullptr;
    if (isVdbGood) {
        isotropicRemesher = new IsotropicRemesher(*vdbVertices, *vdbTriangles);
        isotropicRemesher->setTargetEdgeLength(calculateAverageEdgeLength(*vdbVertices, *vdbTriangles));
    } else {
        isotropicRemesher = new IsotropicRemesher(vertices, triangles);
        isotropicRemesher->setTargetEdgeLength(voxelSize);
    }
    isotropicRemesher->setSharpEdgeDegrees(m_defaultSharpEdgeDegrees);
    isotropicRemesher->remesh();
    vertices = isotropicRemesher->remeshedVertices();
    triangles = isotropicRemesher->remeshedTriangles();
    delete isotropicRemesher;
#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << islandIndex << "]: Uniformly remesh done, vertex count:" << vertices.size() << " triangle count:" << triangles.size();
#endif
    
    delete vdbVertices;
    delete vdbTriangles;
}

void AutoRemesher::updateProgress(size_t threadIndex, float progress)
{
    if (nullptr == m_progressHandler)
        return;
    
    m_threadProgress[threadIndex] = progress;
    float totalProgress = 0.0;
    for (size_t i = 0; i < m_threadProgress.size(); ++i)
        totalProgress += m_threadProgress[i] * m_threadProgressWeights[i];
    m_progressHandler(m_tag, totalProgress);
}

bool AutoRemesher::remesh()
{
    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 0.0);
    
    initializeVoxelSize();
    
    std::vector<std::vector<std::vector<size_t>>> m_trianglesIslands;
    MeshSeparator::splitToIslands(m_triangles, m_trianglesIslands);
    
    if (m_trianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        if (nullptr != m_progressHandler)
            m_progressHandler(m_tag, 1.0);
        return false;
    }
    
#if AUTO_REMESHER_DEBUG
    qDebug() << "Split to islands:" << m_trianglesIslands.size();
#endif
    
    struct IslandContext
    {
        std::vector<Vector3> vertices;
        std::vector<std::vector<size_t>> triangles;
        double voxelSize;
        double scaling;
    };

    std::vector<IslandContext> islandContexes;
    islandContexes.reserve(m_trianglesIslands.size());
    for (size_t islandIndex = 0; islandIndex < m_trianglesIslands.size(); ++islandIndex) {
        const auto &island = m_trianglesIslands[islandIndex];
        IslandContext context;
        std::unordered_set<size_t> addedIndices;
        std::unordered_map<size_t, size_t> oldToNewVertexMap;
        for (const auto &face: island) {
            std::vector<size_t> triangle;
            for (size_t i = 0; i < 3; ++i) {
                auto insertResult = addedIndices.insert(face[i]);
                if (insertResult.second) {
                    oldToNewVertexMap.insert({face[i], context.vertices.size()});
                    context.vertices.push_back(m_vertices[face[i]]);
                }
                triangle.push_back(oldToNewVertexMap[face[i]]);
            }
            context.triangles.push_back(triangle);
        }

        context.scaling = m_scaling;
        context.voxelSize = m_voxelSize;

        islandContexes.push_back(context);
    }
    
    class ParameterizationThread
    {
    public:
        ~ParameterizationThread()
        {
            delete parameterizer;
            delete remesher;
        }
        
        size_t islandIndex = 0;
        IslandContext *island = nullptr;
        Parameterizer *parameterizer = nullptr;
        QuadExtractor *remesher = nullptr;
        AutoRemesher *autoRemesher = nullptr;
        float progressWeight = 1.0;
    };
    
    m_threadProgressWeights.resize(islandContexes.size(), 1.0);
    std::vector<ParameterizationThread> parameterizationThreads(islandContexes.size());
    for (size_t i = 0; i < islandContexes.size(); ++i) {
        auto &thread = parameterizationThreads[i];
        auto &context = islandContexes[i];
        thread.islandIndex = i;
        thread.island = &context;
        thread.autoRemesher = this;
        if (!m_triangles.empty())
            m_threadProgressWeights[i] = (float)(((double)context.triangles.size() / m_triangles.size()));
    }
    m_threadProgress.resize(parameterizationThreads.size());

    class SurfaceParameterizer
    {
    public:
        SurfaceParameterizer(std::vector<ParameterizationThread> *parameterizationThreads) :
            m_parameterizationThreads(parameterizationThreads)
        {   
        }
        void operator()(const tbb::blocked_range<size_t> &range) const
        {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                auto &thread = (*m_parameterizationThreads)[i];
                
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: resampling...";
#endif
                
                resample(thread.island->vertices, thread.island->triangles, thread.island->voxelSize, thread.islandIndex);

                const auto &vertices = thread.island->vertices;
                const auto &triangles = thread.island->triangles;
                
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: parameterizing...";
#endif
                ReportProgressContext reportProgressContext;
                reportProgressContext.islandIndex = i;
                reportProgressContext.autoRemesher = thread.autoRemesher;
                geogram_report_progress_tag = &reportProgressContext;
                geogram_report_progress_round = 0;
                geogram_report_progress_callback = ReportProgress;
                
                thread.parameterizer = new Parameterizer(&vertices, 
                    &triangles,
                    nullptr);
                thread.parameterizer->setScaling(thread.island->scaling);
                thread.parameterizer->parameterize();
                
                std::vector<std::vector<Vector2>> *uvs = thread.parameterizer->takeTriangleUvs();
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: quad extracting...";
#endif
                thread.remesher = new QuadExtractor(&vertices, 
                    &triangles, 
                    uvs);
                if (!thread.remesher->extract()) {
                    delete thread.remesher;
                    thread.remesher = nullptr;
                }
#if AUTO_REMESHER_DEBUG
                if (nullptr != thread.remesher) {
                    qDebug() << "Island[" << thread.islandIndex << "]: remesh done, vertices:" << thread.remesher->remeshedVertices().size() << " quads:" << thread.remesher->remeshedQuads().size();
                } else {
                    qDebug() << "Island[" << thread.islandIndex << "]: remesh failed";
                }
#endif
                delete uvs;
            }
        }
    private:
        std::vector<ParameterizationThread> *m_parameterizationThreads = nullptr;
    };
    tbb::parallel_for(tbb::blocked_range<size_t>(0, parameterizationThreads.size()),
        SurfaceParameterizer(&parameterizationThreads));
        
    for (size_t i = 0; i < parameterizationThreads.size(); ++i) {
        auto &thread = parameterizationThreads[i];
        if (nullptr == thread.remesher)
            continue;
        const auto &quads = thread.remesher->remeshedQuads();
        if (quads.empty())
            continue;
        const auto &vertices = thread.remesher->remeshedVertices();
        size_t vertexStartIndex = m_remeshedVertices.size();
        m_remeshedVertices.reserve(m_remeshedVertices.size() + vertices.size());
        for (const auto &it: vertices) {
            m_remeshedVertices.push_back(it);
        }
        for (const auto &it: quads) {
            std::vector<size_t> quad;
            quad.reserve(it.size());
            for (const auto &v: it)
                quad.push_back(vertexStartIndex + v);
            m_remeshedQuads.push_back(quad);
        }
    }

#if AUTO_REMESHER_DEBUG
    qDebug() << "Remesh done";
#endif

    if (nullptr != m_progressHandler)
        m_progressHandler(m_tag, 1.0);

    return true;
}

}
