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
thread_local geogram_report_progress_handler geogram_report_progress_callback;

#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

namespace AutoRemesher
{
    
const double AutoRemesher::m_defaultSharpEdgeDegrees = 90;
const double AutoRemesher::m_defaultScaling = 0.9;

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

struct ReportProgressContext
{
    size_t islandIndex;
};

static void ReportProgress(void *tag, float progress)
{
    ReportProgressContext *context = (ReportProgressContext *)tag;
#if AUTO_REMESHER_DEBUG
    qDebug() << "Island[" << context->islandIndex << "]: progress(" << (progress * 100) << "%)";
#endif
}

bool AutoRemesher::remesh()
{
#if AUTO_REMESHER_DEBUG
    qDebug() << "Vdb remeshing...";
#endif
    VdbRemesher vdbRemesher(&m_vertices, &m_triangles);
    vdbRemesher.remesh();    
    std::vector<Vector3> *vdbVertices = vdbRemesher.takeVdbVertices();
    std::vector<std::vector<size_t>> *vdbTriangles = vdbRemesher.takeVdbTriangles();
#if AUTO_REMESHER_DEBUG
    qDebug() << "Vdb remeshing done, vertices:" << vdbVertices->size() << " triangles:" << vdbTriangles->size();
#endif

    const std::vector<Vector3> *useVertices = nullptr;
    const std::vector<std::vector<size_t>> *useTriangles = nullptr;
    if (vdbTriangles->size() < 1000) {
        useVertices = &m_vertices;
        useTriangles = &m_triangles;
    } else {
        useVertices = vdbVertices;
        useTriangles = vdbTriangles;
    }
    
    double averageEdgeLength = vdbTriangles->size() < 1000 ? vdbRemesher.voxelSize() : calculateAverageEdgeLength(*useVertices, *useTriangles);
#if AUTO_REMESHER_DEBUG
    qDebug() << "Uniform remeshing... averageEdgeLength:" << averageEdgeLength;
#endif
    IsotropicRemesher *isotropicRemesher = new IsotropicRemesher(*useVertices, *useTriangles);
    isotropicRemesher->setSharpEdgeDegrees(m_defaultSharpEdgeDegrees);
    isotropicRemesher->setTargetEdgeLength(averageEdgeLength);
    isotropicRemesher->remesh();
    m_vertices = isotropicRemesher->remeshedVertices();
    m_triangles = isotropicRemesher->remeshedTriangles();
    delete isotropicRemesher;
    
    delete vdbVertices;
    delete vdbTriangles;

    std::vector<std::vector<std::vector<size_t>>> m_trianglesIslands;
    MeshSeparator::splitToIslands(m_triangles, m_trianglesIslands);
    
    if (m_trianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        return false;
    }
    
#if AUTO_REMESHER_DEBUG
    qDebug() << "Split to islands:" << m_trianglesIslands.size();
#endif
    
    struct IslandContext
    {
        std::vector<Vector3> vertices;
        std::vector<std::vector<size_t>> triangles;
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
        const IslandContext *island = nullptr;
        Parameterizer *parameterizer = nullptr;
        QuadExtractor *remesher = nullptr;
    };

    std::vector<ParameterizationThread> parameterizationThreads(islandContexes.size());
    for (size_t i = 0; i < islandContexes.size(); ++i) {
        auto &thread = parameterizationThreads[i];
        const auto &context = islandContexes[i];
        thread.islandIndex = i;
        thread.island = &context;
    }

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

                const auto &vertices = thread.island->vertices;
                const auto &triangles = thread.island->triangles;
                
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: parameterizing...";
#endif
                ReportProgressContext reportProgressContext;
                reportProgressContext.islandIndex = i;
                geogram_report_progress_tag = &reportProgressContext;
                geogram_report_progress_callback = ReportProgress;
                
                thread.parameterizer = new Parameterizer(&vertices, 
                    &triangles,
                    nullptr);
                thread.parameterizer->setScaling(thread.island->scaling);
                thread.parameterizer->parameterize();
                
                std::vector<std::vector<Vector2>> *uvs = thread.parameterizer->takeTriangleUvs();
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: quad extracing...";
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
            m_remeshedQuads.push_back({
                vertexStartIndex + it[0], 
                vertexStartIndex + it[1], 
                vertexStartIndex + it[2], 
                vertexStartIndex + it[3]
            });
        }
    }
    
    for (auto &it: m_remeshedVertices)
        it = it * vdbRemesher.recoverScale() + vdbRemesher.origin();

#if AUTO_REMESHER_DEBUG
    qDebug() << "Remesh done";
#endif
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("autoremesher-debug.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &v = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", v.x(), v.y(), v.z());
        }
        for (size_t i = 0; i < m_remeshedQuads.size(); ++i) {
            const auto &f = m_remeshedQuads[i];
            fprintf(fp, "f %zu %zu %zu %zu\n", 
                1 + f[0],
                1 + f[1],
                1 + f[2],
                1 + f[3]);
        }
        fclose(fp);
    }
#endif
#if AUTO_REMESHER_DEBUG
    qDebug() << "Remesh debug result saved";
#endif

    return true;
}

}
