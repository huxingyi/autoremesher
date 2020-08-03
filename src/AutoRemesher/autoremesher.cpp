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
#include <AutoRemesher/AutoRemesher>
#include <AutoRemesher/IsotropicRemesher>
#include <AutoRemesher/HalfEdge>

namespace AutoRemesher
{
    
const double AutoRemesher::m_defaultTargetEdgeLength = 3.9;
    
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
        if (island.empty())
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

bool AutoRemesher::remesh()
{
    Vector3 origin;
    double recoverScale = 1.0;
    double scale = 100;
    double maxLength = 1.0;
    calculateNormalizedFactors(m_vertices, &origin, &maxLength);
    recoverScale = maxLength / scale;
    for (auto &v: m_vertices) {
        if (std::isinf(v.x()) || std::isinf(v.y()) || std::isinf(v.z()))
            std::cerr << "Found inf raw vertex:" << v << std::endl;
        v = scale * (v - origin) / maxLength;
        if (std::isinf(v.x()) || std::isinf(v.y()) || std::isinf(v.z()))
            std::cerr << "Found inf normalized vertex:" << v << std::endl;
    }
    
    std::vector<std::vector<std::vector<size_t>>> m_trianglesIslands;
    splitToIslands(m_triangles, m_trianglesIslands);
    //m_trianglesIslands = {m_triangles};
    
    if (m_trianglesIslands.empty()) {
        std::cerr << "Input mesh is empty" << std::endl;
        return false;
    }
    
    std::cerr << "Start remeshing, this may take a few minutes..." << std::endl;
    
    for (size_t islandIndex = 0; islandIndex < m_trianglesIslands.size(); ++islandIndex) {
        const auto &island = m_trianglesIslands[islandIndex];
        std::vector<Vector3> pickedVertices;
        std::vector<std::vector<size_t>> pickedTriangles;
        std::unordered_set<size_t> addedIndices;
        std::unordered_map<size_t, size_t> oldToNewVertexMap;
        for (const auto &face: island) {
            std::vector<size_t> triangle;
            for (size_t i = 0; i < 3; ++i) {
                auto insertResult = addedIndices.insert(face[i]);
                if (insertResult.second) {
                    oldToNewVertexMap.insert({face[i], pickedVertices.size()});
                    pickedVertices.push_back(m_vertices[face[i]]);
                }
                triangle.push_back(oldToNewVertexMap[face[i]]);
            }
            pickedTriangles.push_back(triangle);
        }
        std::cerr << "Remeshing surface #" << (islandIndex + 1) << "/" << m_trianglesIslands.size() << "(vertices:" << pickedVertices.size() << " triangles:" << pickedTriangles.size() << ")..." << std::endl;
        
        //std::vector<Vector3> &pickedVertices = m_vertices;
        //std::vector<std::vector<size_t>> &pickedTriangles = m_trianglesIslands[islandIndex];
        
        double localMaxLength = 1.0;
        Vector3 localOrigin;
        calculateNormalizedFactors(pickedVertices, &localOrigin, &localMaxLength);
        localMaxLength *= recoverScale;
        
        std::cerr << "localMaxLength:" << localMaxLength << " maxLength:" << maxLength << std::endl;

        IsotropicRemesher isotropicRemesher(pickedVertices, pickedTriangles);
        isotropicRemesher.setTargetEdgeLength(m_targetEdgeLength);
        isotropicRemesher.remesh();
        
        QuadRemesher quadRemesher(isotropicRemesher.remeshedVertices(), isotropicRemesher.remeshedTriangles());

        //AutoRemesher::QuadRemesher quadRemesher(pickedVertices, pickedTriangles);
        quadRemesher.setGradientSize(m_gradientSize * (localMaxLength / maxLength));
        //auto coutBuffer = std::cout.rdbuf();
        //auto cerrBuffer = std::cerr.rdbuf();
        //std::cout.rdbuf(nullptr);
        //std::cerr.rdbuf(nullptr);
        bool remeshSucceed = quadRemesher.remesh();
        //std::cout.rdbuf(coutBuffer);
        //std::cerr.rdbuf(cerrBuffer);
        if (!remeshSucceed) {
            std::cerr << "Surface #" << (islandIndex + 1) << "/" << m_trianglesIslands.size() << " failed to remesh" << std::endl;
            continue;
        }
        const auto &quads = quadRemesher.remeshedQuads();
        if (quads.empty())
            continue;
        const auto &vertices = quadRemesher.remeshedVertices();
        std::cerr << "Surface #" << (islandIndex + 1) << "/" << m_trianglesIslands.size() << " remesh succeed(vertices:" << vertices.size() << " quads:" << quads.size() << ")" << std::endl;
        size_t vertexStartIndex = m_remeshedVertices.size();
        //m_remeshedVertices.insert(m_remeshedVertices.end(), vertices.begin(), vertices.end());
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
    
    return true;
}

}
