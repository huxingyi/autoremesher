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
#include <queue>
#include <unordered_set>
#include <AutoRemesher/RelativeHeight>
#include <AutoRemesher/Vector3>

namespace AutoRemesher
{
    
void RelativeHeight::buildFaceAroundVertexMap(const std::vector<std::vector<size_t>> &triangles,
        std::map<size_t, std::vector<size_t>> *faceAroundVertexMap)
{
    faceAroundVertexMap->clear();
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto &it = triangles[i];
        (*faceAroundVertexMap)[it[0]].push_back(i);
        (*faceAroundVertexMap)[it[1]].push_back(i);
        (*faceAroundVertexMap)[it[2]].push_back(i);
    }
}

void RelativeHeight::calculateVertexNormals(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles,
        std::vector<Vector3> *vertexNormals)
{
    vertexNormals->clear();
    vertexNormals->resize(vertices.size());
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto &it = triangles[i];
        Vector3 normal = Vector3::normal((*m_vertices)[it[0]],
            (*m_vertices)[it[1]],
            (*m_vertices)[it[2]]);
        (*vertexNormals)[it[0]] += normal;
        (*vertexNormals)[it[1]] += normal;
        (*vertexNormals)[it[2]] += normal;
    }
    for (auto &it: *vertexNormals)
        it.normalize();
}

double RelativeHeight::calculateAverageEdgeLength(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles)
{
    double sumOfEdgeLengths = 0.0;
    size_t edgeCount = 0;
    for (const auto &it: triangles) {
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            sumOfEdgeLengths += (vertices[it[i]] - vertices[it[j]]).length();
            ++edgeCount;
        }
    }
    if (0 == edgeCount)
        return 0.0;
    return sumOfEdgeLengths / edgeCount;
}

void RelativeHeight::collectNearbyVertices(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles,
        const std::map<size_t, std::vector<size_t>> &faceAroundVertexMap,
        size_t vertexIndex, 
        std::vector<size_t> *nearbyVertexIndices, 
        double maxSquaredDistance)
{
    std::queue<size_t> q;
    q.push(vertexIndex);
    std::unordered_set<size_t> visited;
    while (!q.empty()) {
        size_t v = q.front();
        q.pop();
        if (visited.find(v) != visited.end())
            continue;
        visited.insert(v);
        if ((vertices[vertexIndex] - vertices[v]).lengthSquared() > maxSquaredDistance)
            continue;
        nearbyVertexIndices->push_back(v);
        auto findNeighbors = faceAroundVertexMap.find(v);
        if (findNeighbors == faceAroundVertexMap.end())
            continue;
        for (const auto &faceIndex: findNeighbors->second) {
            for (const auto &neighbor: triangles[faceIndex]) {
                if (visited.find(neighbor) != visited.end())
                    continue;
                q.push(neighbor);
            }
        }
    }
}

bool RelativeHeight::calculate()
{
    delete m_vertexRelativeHeights;
    m_vertexRelativeHeights = new std::vector<double>(m_vertices->size());
    
    delete m_vertexNormals;
    m_vertexNormals = new std::vector<Vector3>;
    calculateVertexNormals(*m_vertices, *m_triangles, m_vertexNormals);
    
    m_averageEdgeLength = calculateAverageEdgeLength(*m_vertices, *m_triangles);
    double maxSquaredDistance = std::pow(m_averageEdgeLength * 5, 2);
    
    std::cerr << "RelativeHeight averageEdgeLength:" << m_averageEdgeLength << " maxSquaredDistance:" << maxSquaredDistance << std::endl;
    
    delete m_faceAroundVertexMap;
    m_faceAroundVertexMap = new std::map<size_t, std::vector<size_t>>;
    buildFaceAroundVertexMap(*m_triangles, m_faceAroundVertexMap);
    
    double maxHeight = 0.0;
    for (size_t vertexIndex = 0; vertexIndex < m_vertices->size(); ++vertexIndex) {
        const auto &vertexPosition = (*m_vertices)[vertexIndex];
        const auto &vertexNormal = (*m_vertexNormals)[vertexIndex];
        double low = 0.0;
        double high = 0.0;
        auto project = [&](const Vector3 &position) {
            double projectedTo = Vector3::dotProduct(position - vertexPosition, vertexNormal);
            if (projectedTo < low)
                low = projectedTo;
            if (projectedTo > high)
                high = projectedTo;
        };
        std::vector<size_t> nearbyVertexIndices;
        collectNearbyVertices(*m_vertices, *m_triangles, *m_faceAroundVertexMap, vertexIndex,
            &nearbyVertexIndices, maxSquaredDistance);
        for (const auto &nearbyVertexIndex: nearbyVertexIndices)
            project((*m_vertices)[nearbyVertexIndex]);
        double height = high - low;
        bool isPositive = high <= std::abs(low);
        if (height > maxHeight)
            maxHeight = height;
        (*m_vertexRelativeHeights)[vertexIndex] = isPositive ? height : -height;
    }
    if (maxHeight > 0.0) {
        for (auto &it: *m_vertexRelativeHeights)
            it /= maxHeight;
    }
    
    return true;
}

}
