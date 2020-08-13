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
#include <unordered_set>
#include <unordered_map>
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/IsotropicRemesher>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/Parameterizer>
#include <AutoRemesher/MeshCutter>
#include <tbb/parallel_for.h>
#include <tbb/blocked_range.h>
#include <tbb/tbb_thread.h>
#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

namespace AutoRemesher
{
    
const double AutoRemesher::m_defaultTargetEdgeLength = 3.9;
const std::pair<double, double> AutoRemesher::m_defaultConstraintRatio = {0.55, 1.0};
const size_t AutoRemesher::m_defaultMaxSingularityCount = 320;
const size_t AutoRemesher::m_defaultMaxVertexCount = 7000;
const double AutoRemesher::m_defaultSharpEdgeDegrees = 60;
const double AutoRemesher::m_defaultGradientSize = 170;
    
void AutoRemesher::buildEdgeToFaceMap(const std::vector<std::vector<size_t>> &triangles, std::map<std::pair<size_t, size_t>, size_t> &edgeToFaceMap)
{
    edgeToFaceMap.clear();
    for (size_t index = 0; index < triangles.size(); ++index) {
        const auto &face = triangles[index];
        for (size_t i = 0; i < 3; i++) {
            size_t j = (i + 1) % 3;
            edgeToFaceMap[{face[i], face[j]}] = index;
        }
    }
}

void AutoRemesher::splitToIslands(const std::vector<std::vector<size_t>> &triangles, std::vector<std::vector<std::vector<size_t>>> &islands)
{
    std::map<std::pair<size_t, size_t>, size_t> edgeToFaceMap;
    buildEdgeToFaceMap(triangles, edgeToFaceMap);
    
    std::unordered_set<size_t> processedFaces;
    std::queue<size_t> waitFaces;
    for (size_t indexInGroup = 0; indexInGroup < triangles.size(); ++indexInGroup) {
        if (processedFaces.find(indexInGroup) != processedFaces.end())
            continue;
        waitFaces.push(indexInGroup);
        std::vector<std::vector<size_t>> island;
        while (!waitFaces.empty()) {
            size_t index = waitFaces.front();
            waitFaces.pop();
            if (processedFaces.find(index) != processedFaces.end())
                continue;
            const auto &face = triangles[index];
            for (size_t i = 0; i < 3; i++) {
                size_t j = (i + 1) % 3;
                auto findOppositeFaceResult = edgeToFaceMap.find({face[j], face[i]});
                if (findOppositeFaceResult == edgeToFaceMap.end())
                    continue;
                waitFaces.push(findOppositeFaceResult->second);
            }
            island.push_back(triangles[index]);
            processedFaces.insert(index);
        }
        if (island.size() < 4)
            continue;
        islands.push_back(island);
    }
}

void AutoRemesher::calculateNormalizedFactors(const std::vector<Vector3> &vertices, Vector3 *origin, double *maxLength)
{
    double minX = std::numeric_limits<double>::max();
    double maxX = std::numeric_limits<double>::lowest();
    double minY = std::numeric_limits<double>::max();
    double maxY = std::numeric_limits<double>::lowest();
    double minZ = std::numeric_limits<double>::max();
    double maxZ = std::numeric_limits<double>::lowest();
    for (const auto &v: vertices) {
        if (v.x() < minX)
            minX = v.x();
        if (v.x() > maxX)
            maxX = v.x();
        if (v.y() < minY)
            minY = v.y();
        if (v.y() > maxY)
            maxY = v.y();
        if (v.z() < minZ)
            minZ = v.z();
        if (v.z() > maxZ)
            maxZ = v.z();
    }
    Vector3 length = {
        (maxX - minX) * 0.5,
        (maxY - minY) * 0.5,
        (maxZ - minZ) * 0.5,
    };
    *maxLength = length[0];
    if (length[1] > *maxLength)
        *maxLength = length[1];
    if (length[2] > *maxLength)
        *maxLength = length[2];
    *origin = {
        (maxX + minX) * 0.5,
        (maxY + minY) * 0.5,
        (maxZ + minZ) * 0.5,
    };
}

IsotropicRemesher *AutoRemesher::createIsotropicRemesh(const std::vector<Vector3> sourceVertices,
    const std::vector<std::vector<size_t>> sourceTriangles,
    double sharpEdgeDegrees, 
    size_t targetVertexCount,
    double *targetEdgeLength)
{
    IsotropicRemesher *isotropicRemesher = nullptr;
    double minTargetVertexCount = targetVertexCount * 0.9;
    
    if (Double::isZero(*targetEdgeLength))
        *targetEdgeLength = m_defaultTargetEdgeLength;
    
    while (nullptr == isotropicRemesher || isotropicRemesher->remeshedVertices().size() < minTargetVertexCount) {
        delete isotropicRemesher;
        isotropicRemesher = new IsotropicRemesher(sourceVertices, sourceTriangles);
        isotropicRemesher->setSharpEdgeDegrees(sharpEdgeDegrees);
        isotropicRemesher->setTargetEdgeLength(*targetEdgeLength);
        isotropicRemesher->remesh();
#if AUTO_REMESHER_DEBUG
        qDebug() << "isotropicRemesher from vertices " << sourceVertices.size() << " to " << isotropicRemesher->remeshedVertices().size() << " targetEdgeLength:" << *targetEdgeLength;
#endif
        *targetEdgeLength *= 0.9;
    }
    
    while (nullptr == isotropicRemesher || isotropicRemesher->remeshedVertices().size() > targetVertexCount) {
        delete isotropicRemesher;
#if AUTO_REMESHER_DEBUG
        qDebug() << "isotropicRemesher remeshing targetEdgeLength:" << *targetEdgeLength;
#endif
        isotropicRemesher = new IsotropicRemesher(sourceVertices, sourceTriangles);
        isotropicRemesher->setSharpEdgeDegrees(sharpEdgeDegrees);
        isotropicRemesher->setTargetEdgeLength(*targetEdgeLength);
        isotropicRemesher->remesh();
#if AUTO_REMESHER_DEBUG
        qDebug() << "isotropicRemesher from vertices " << sourceVertices.size() << " to " << isotropicRemesher->remeshedVertices().size() << " targetEdgeLength:" << *targetEdgeLength;
#endif
        *targetEdgeLength *= 1.1;
    }

    return isotropicRemesher;
}

bool AutoRemesher::remesh()
{
    Vector3 origin;
    double recoverScale = 1.0;
    double scale = 100;
    double maxLength = 1.0;
    calculateNormalizedFactors(m_vertices, &origin, &maxLength);
    recoverScale = maxLength / scale;
    for (auto &v: m_vertices) {
        v = scale * (v - origin) / maxLength;
    }
    
    std::vector<std::vector<std::vector<size_t>>> m_trianglesIslands;
    splitToIslands(m_triangles, m_trianglesIslands);
    
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
        double gradientSize;
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
        
        double localMaxLength = 1.0;
        Vector3 localOrigin;
        calculateNormalizedFactors(context.vertices, &localOrigin, &localMaxLength);
        localMaxLength *= recoverScale;
        
        context.gradientSize = m_gradientSize * (localMaxLength / maxLength);
        
#if AUTO_REMESHER_DEBUG
        qDebug() << "Gradient size[" << islandIndex << "/" << m_trianglesIslands.size() << "]:" << context.gradientSize;
#endif
        
        islandContexes.push_back(context);
    }
    
    class ParameterizationThread
    {
    public:
        ~ParameterizationThread()
        {
            delete isotropicRemesher;
            delete parameterizer;
            delete mesh;
            delete remesher;
        }
        
        HalfEdge::Mesh *takeMesh()
        {
            HalfEdge::Mesh *m = mesh;
            mesh = nullptr;
            return m;
        }
        
        size_t islandIndex = 0;
        const IslandContext *island = nullptr;
        IsotropicRemesher *isotropicRemesher = nullptr;
        Parameterizer *parameterizer = nullptr;
        HalfEdge::Mesh *mesh = nullptr;
        double sharpEdgeDegrees = m_defaultSharpEdgeDegrees;
        double targetEdgeLength = m_defaultTargetEdgeLength;
        std::pair<double, double> limitRelativeHeight = {0.0, 1.0};
        size_t singularityCount = 0;
        bool singularityCountCalculated = false;
        QuadRemesher *remesher = nullptr;
        bool miqSucceed = false;
    };

    std::vector<ParameterizationThread> parameterizationThreads(islandContexes.size());
    for (size_t i = 0; i < islandContexes.size(); ++i) {
        auto &thread = parameterizationThreads[i];
        const auto &context = islandContexes[i];
        thread.islandIndex = i;
        thread.island = &context;
    }

    class UniformRemesher
    {
    public:
        UniformRemesher(std::vector<ParameterizationThread> *parameterizationThreads, ConstrainedArea constrainedArea) :
            m_parameterizationThreads(parameterizationThreads),
            m_constrainedArea(constrainedArea)
        {   
        }
        void operator()(const tbb::blocked_range<size_t> &range) const
        {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                auto &thread = (*m_parameterizationThreads)[i];
                thread.targetEdgeLength = 0.0;
                thread.isotropicRemesher = AutoRemesher::createIsotropicRemesh(thread.island->vertices,
                    thread.island->triangles,
                    m_defaultSharpEdgeDegrees, 
                    m_defaultMaxVertexCount,
                    &thread.targetEdgeLength);
                    
                thread.mesh = new HalfEdge::Mesh(thread.isotropicRemesher->remeshedVertices(), 
                    thread.isotropicRemesher->remeshedTriangles());
                
                Parameterizer::Parameters parameters;
                parameters.gradientSize = thread.island->gradientSize;
                parameters.constrainOnFlatArea = ConstrainedArea::ConstrainedAreaFlat == m_constrainedArea;

                thread.parameterizer = new Parameterizer(thread.mesh, parameters);
                
                auto constraintRatio = m_defaultConstraintRatio;
                const double positiveStep = 0.01;
                const double step = parameters.constrainOnFlatArea ? -positiveStep : positiveStep;
                for (; constraintRatio.first < m_defaultConstraintRatio.second && constraintRatio.first >= positiveStep; constraintRatio.first += step) {
                    thread.limitRelativeHeight = thread.parameterizer->calculateLimitRelativeHeight(constraintRatio);
#if AUTO_REMESHER_DEBUG
                    qDebug() << "Island[" << thread.islandIndex << "]: test limitRelativeHeight:" << thread.limitRelativeHeight << " ratio:" << constraintRatio;
#endif
                    Eigen::VectorXi *b = nullptr;
                    Eigen::MatrixXd *bc1 = nullptr;
                    Eigen::MatrixXd *bc2 = nullptr;
                    thread.parameterizer->prepareConstraints(thread.limitRelativeHeight,
                        &b, &bc1, &bc2);
                    thread.parameterizer->miq(&thread.singularityCount, *b, *bc1, *bc2, true);
                    delete b;
                    delete bc1;
                    delete bc2;
#if AUTO_REMESHER_DEBUG
                    qDebug() << "Island[" << thread.islandIndex << "]: test singularityCount:" << thread.singularityCount << " on ratio:" << constraintRatio.first;
#endif
                    if (thread.singularityCount <= m_defaultMaxSingularityCount) {
                        thread.singularityCountCalculated = true;
                        break;
                    }
                }
            }
        }
    private:
        std::vector<ParameterizationThread> *m_parameterizationThreads = nullptr;
        ConstrainedArea m_constrainedArea;
    };
    tbb::parallel_for(tbb::blocked_range<size_t>(0, parameterizationThreads.size()),
        UniformRemesher(&parameterizationThreads, m_constrainedArea));
    
    std::vector<ParameterizationThread *> validParameterizationThreads;
    for (size_t i = 0; i < parameterizationThreads.size(); ++i) {
        auto &thread = parameterizationThreads[i];
        if (thread.singularityCountCalculated) {
#if AUTO_REMESHER_DEBUG
            qDebug() << "Island[" << thread.islandIndex << "/" << islandContexes.size() << "]: has valid singularity count:" << thread.singularityCount;
#endif
            validParameterizationThreads.push_back(&thread);
            continue;
        }
#if AUTO_REMESHER_DEBUG
        qDebug() << "Island[" << thread.islandIndex << "/" << islandContexes.size() << "]: has invalid singularity count:" << thread.singularityCount << "IGNORED";
#endif
    }
    
    class Miqer
    {
    public:
        Miqer(std::vector<ParameterizationThread *> *validParameterizationThreads) :
            m_validParameterizationThreads(validParameterizationThreads)
        {
        }
        
        void operator()(const tbb::blocked_range<size_t> &range) const
        {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                auto &thread = *(*m_validParameterizationThreads)[i];
                Eigen::VectorXi *b = nullptr;
                Eigen::MatrixXd *bc1 = nullptr;
                Eigen::MatrixXd *bc2 = nullptr;
                thread.parameterizer->prepareConstraints(thread.limitRelativeHeight, &b, &bc1, &bc2);
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: parameterizing... on singularity count:" << thread.singularityCount;
#endif
#if AUTO_REMESHER_DEV
                thread.mesh->debugExportRelativeHeightPly("debug-height.ply");
                thread.mesh->debugExportLimitRelativeHeightPly("debug-limit.ply", thread.limitRelativeHeight);
#endif
                thread.miqSucceed = thread.parameterizer->miq(&thread.singularityCount, *b, *bc1, *bc2, false);
                delete b;
                delete bc1;
                delete bc2;
                if (!thread.miqSucceed) {
#if AUTO_REMESHER_DEBUG
                    qDebug() << "Island[" << thread.islandIndex << "]: parameterize failed on singularity count:" << thread.singularityCount;
#endif
                    continue;
                }
#if AUTO_REMESHER_DEBUG
                qDebug() << "Island[" << thread.islandIndex << "]: parameterize succeed on singularity count:" << thread.singularityCount;
#endif
            }
        }
    private:
        std::vector<ParameterizationThread *> *m_validParameterizationThreads = nullptr;
    };
    tbb::parallel_for(tbb::blocked_range<size_t>(0, validParameterizationThreads.size()),
        Miqer(&validParameterizationThreads));
        
    for (size_t i = 0; i < validParameterizationThreads.size(); ++i) {
        auto &thread = *validParameterizationThreads[i];
        if (!thread.miqSucceed)
            continue;
#if AUTO_REMESHER_DEBUG
        qDebug() << "Island[" << thread.islandIndex << "]: remeshing...";
#endif
        thread.remesher = new QuadRemesher(thread.mesh, &thread.parameterizer->getVertexValences());
        if (!thread.remesher->remesh()) {
            delete thread.remesher;
            thread.remesher = nullptr;
        }
#if AUTO_REMESHER_DEBUG
        if (nullptr != thread.remesher) {
            qDebug() << "Island[" << thread.islandIndex << "]: remesh done, quads:" << thread.remesher->remeshedQuads().size();
        } else {
            qDebug() << "Island[" << thread.islandIndex << "]: remesh failed";
        }
#endif
        if (nullptr == thread.remesher)
            continue;
        const auto &quads = thread.remesher->remeshedQuads();
        if (quads.empty())
            continue;
        const auto &vertices = thread.remesher->remeshedVertices();
        size_t vertexStartIndex = m_remeshedVertices.size();
        m_remeshedVertices.reserve(m_remeshedVertices.size() + vertices.size());
        for (const auto &it: vertices) {
            m_remeshedVertices.push_back(it * recoverScale + origin);
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
