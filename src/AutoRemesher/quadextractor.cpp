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
#include <AutoRemesher/QuadExtractor>
#include <AutoRemesher/PositionKey>
#include <AutoRemesher/Double>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <map>
#include <set>
#include <igl/boundary_loop.h>
#include <exploragram/hexdom/polygon.h>

namespace AutoRemesher
{

bool QuadExtractor::extract()
{
    std::cerr << "Extract connections..." << std::endl;
    std::vector<Vector3> crossPoints;
    std::vector<size_t> crossPointSourceTriangles;
    std::set<std::pair<size_t, size_t>> connections;
    extractConnections(&crossPoints, &crossPointSourceTriangles, &connections);
    std::cerr << "Extract connections done" << std::endl;
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-connections.obj", "wb");
        for (size_t i = 0; i < crossPoints.size(); ++i) {
            const auto &vertex = crossPoints[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: connections) {
            fprintf(fp, "l %zu %zu\n", 1 + it.first, 1 + it.second);
        }
        fclose(fp);
    }
#endif
    
    std::cerr << "Extract edges..." << std::endl;
    std::unordered_map<size_t, std::unordered_set<size_t>> edgeConnectMap;
    extractEdges(connections, &edgeConnectMap);
    bool collapsedShortEdges = collapseShortEdges(&crossPoints, &edgeConnectMap);
    bool collapsedTriangles = collapseTriangles(&crossPoints, &edgeConnectMap);
    if (collapsedShortEdges || collapsedTriangles)
        simplifyGraph(edgeConnectMap);
    std::cerr << "Extract edges done" << std::endl;
    
#if AUTO_REMESHER_DEV
    {
        std::vector<Vector3> edgeVertices;
        std::unordered_map<size_t, size_t> oldToNewMap;
        auto addEdgeVertex = [&](size_t vertexIndex) {
            auto insertResult = oldToNewMap.insert({vertexIndex, edgeVertices.size()});
            if (insertResult.second)
                edgeVertices.push_back(crossPoints[vertexIndex]);
            return insertResult.first->second;
        };
        for (const auto &it: edgeConnectMap) {
            addEdgeVertex(it.first);
            for (const auto &it2: it.second) {
                addEdgeVertex(it2);
            }
        }
        FILE *fp = fopen("debug-quadextractor-edges.obj", "wb");
        for (size_t i = 0; i < edgeVertices.size(); ++i) {
            const auto &vertex = edgeVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: edgeConnectMap) {
            for (const auto &it2: it.second)
                fprintf(fp, "l %zu %zu\n", 1 + oldToNewMap[it.first], 1 + oldToNewMap[it2]);
        }
        fclose(fp);
    }
#endif
    
    std::cerr << "Extract mesh..." << std::endl;
    extractMesh(crossPoints, crossPointSourceTriangles, edgeConnectMap, &m_remeshedQuads);
    std::cerr << "Extract mesh done" << std::endl;
    
    std::unordered_map<size_t, size_t> oldToNewMap;
    auto addQuadVertex = [&](size_t vertexIndex) {
        auto insertResult = oldToNewMap.insert({vertexIndex, m_remeshedVertices.size()});
        if (insertResult.second)
            m_remeshedVertices.push_back(crossPoints[vertexIndex]);
        return insertResult.first->second;
    };
    for (const auto &it: m_remeshedQuads) {
        addQuadVertex(it[0]);
        addQuadVertex(it[1]);
        addQuadVertex(it[2]);
        addQuadVertex(it[3]);
    }
    for (auto &it: m_remeshedQuads) {
        it[0] = oldToNewMap[it[0]];
        it[1] = oldToNewMap[it[1]];
        it[2] = oldToNewMap[it[2]];
        it[3] = oldToNewMap[it[3]];
    }
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-quads-withoutfix.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: m_remeshedQuads) {
            fprintf(fp, "f %zu %zu %zu %zu\n", 1 + it[0], 1 + it[1], 1 + it[2], 1 + it[3]);
        }
        fclose(fp);
    }
#endif
    
    fixFlippedFaces();
    fixHoles();
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-quads.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: m_remeshedQuads) {
            fprintf(fp, "f %zu %zu %zu %zu\n", 1 + it[0], 1 + it[1], 1 + it[2], 1 + it[3]);
        }
        fclose(fp);
    }
#endif

    return true;
}

void QuadExtractor::extractEdges(const std::set<std::pair<size_t, size_t>> &connections,
        std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap)
{
    auto &graph = *edgeConnectMap;
    for (const auto &it: connections) {
        graph[it.first].insert(it.second);
        graph[it.second].insert(it.first);
    }
    simplifyGraph(graph);
}

void QuadExtractor::simplifyGraph(std::unordered_map<size_t, std::unordered_set<size_t>> &graph)
{
    for (;;) {
        std::unordered_map<size_t, std::pair<size_t, size_t>> delayPairs;
        for (auto it = graph.begin(); it != graph.end(); ) {
            if (it->second.size() != 2) {
                ++it;
                continue;
            }
            size_t firstNeighbor, secondNeighbor;
            auto neighborIt = it->second.begin();
            firstNeighbor = *neighborIt++;
            secondNeighbor = *neighborIt++;
            if (delayPairs.end() != delayPairs.find(firstNeighbor) ||
                    delayPairs.end() != delayPairs.find(secondNeighbor)) {
                ++it;
                continue;
            }
            delayPairs.insert({it->first, {firstNeighbor, secondNeighbor}});
            it = graph.erase(it);
        }
        if (delayPairs.empty())
            break;
        for (const auto &it: delayPairs) {
            graph[it.second.first].erase(it.first);
            graph[it.second.first].insert(it.second.second);
            graph[it.second.second].erase(it.first);
            graph[it.second.second].insert(it.second.first);
        }
    }
}

bool QuadExtractor::collapseShortEdges(std::vector<Vector3> *crossPoints,
    std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap)
{
    double totalLength = 0.0;
    size_t edgeCount = 0;
    std::map<std::pair<size_t, size_t>, double> edgeLengths;
    for (const auto &it: *edgeConnectMap) {
        for (const auto &neighbor: it.second) {
            if (edgeLengths.end() != edgeLengths.find({neighbor, it.first}))
                continue;
            double edgeLength = ((*crossPoints)[it.first] - (*crossPoints)[neighbor]).length();
            totalLength += edgeLength;
            edgeLengths.insert({{it.first, neighbor}, edgeLength});
            ++edgeCount;
        }
    }
    if (0 == edgeCount)
        return false;
    double averageEdgeLength = totalLength / edgeCount;
    double collapsedLength = averageEdgeLength * 0.15;
    bool collapsed = false;
    for (const auto &it: edgeLengths) {
        if (it.second > collapsedLength)
            continue;
        collapseEdge(crossPoints, edgeConnectMap, it.first);
        collapsed = true;
    }
    return collapsed;
}

void QuadExtractor::collapseEdge(std::vector<Vector3> *crossPoints,
    std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap,
    const std::pair<size_t, size_t> &edge)
{
    auto findSecondNeighbors = edgeConnectMap->find(edge.second);
    if (findSecondNeighbors == edgeConnectMap->end())
        return;
    auto findFirstNeighbors = edgeConnectMap->find(edge.first);
    if (findFirstNeighbors == edgeConnectMap->end())
        return;
    if (findSecondNeighbors->second.end() == findSecondNeighbors->second.find(edge.first))
        return;
    if (findFirstNeighbors->second.end() == findFirstNeighbors->second.find(edge.second))
        return;
    auto firstNeighbors = findFirstNeighbors->second;
    (*crossPoints)[edge.second] = ((*crossPoints)[edge.first] + (*crossPoints)[edge.second]) * 0.5;
    for (const auto &neighbor: firstNeighbors) {
        if (neighbor == edge.second)
            continue;
        (*edgeConnectMap)[edge.second].insert(neighbor);
        (*edgeConnectMap)[neighbor].insert(edge.second);
        (*edgeConnectMap)[neighbor].erase(edge.first);
    }
    (*edgeConnectMap).erase(edge.first);
    (*edgeConnectMap)[edge.second].erase(edge.first);
    if ((*edgeConnectMap)[edge.second].empty())
        (*edgeConnectMap).erase(edge.second);
}

bool QuadExtractor::collapseTriangles(std::vector<Vector3> *crossPoints,
        std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap)
{
    std::set<std::tuple<size_t, size_t, size_t>> collapseList;
    for (const auto &level0It: *edgeConnectMap) {
        const auto &level0 = level0It.first;
        auto findLevel1 = (*edgeConnectMap).find(level0);
        if (findLevel1 == (*edgeConnectMap).end())
            continue;
        for (const auto &level1: findLevel1->second) {
            auto findLevel2 = (*edgeConnectMap).find(level1);
            if (findLevel2 == (*edgeConnectMap).end())
                continue;
            for (const auto &level2: findLevel2->second) {
                if (level0 == level2)
                    continue;
                auto findLevel3 = (*edgeConnectMap).find(level2);
                if (findLevel3 == (*edgeConnectMap).end())
                    continue;
                for (const auto &level3: findLevel3->second) {
                    if (level0 != level3)
                        continue;
                    std::vector<size_t> levels = {level0, level1, level2};
                    std::sort(levels.begin(), levels.end());
                    collapseList.insert({levels[0], levels[1], levels[2]});
                    break;
                }
            }
        }
    }
    if (collapseList.empty())
        return false;
    for (const auto &it: collapseList) {
        collapseEdge(crossPoints, edgeConnectMap, {std::get<0>(it), std::get<1>(it)});
        collapseEdge(crossPoints, edgeConnectMap, {std::get<1>(it), std::get<2>(it)});
    }
    return true;
}

void QuadExtractor::extractMesh(std::vector<Vector3> &points,
        const std::vector<size_t> &pointSourceTriangles,
        std::unordered_map<size_t, std::unordered_set<size_t>> &edgeConnectMap,
        std::vector<std::vector<size_t>> *quads)
{
    auto calculateFaceNormal = [&](const std::vector<size_t> &corners) {
        Vector3 normals;
        for (size_t i = 0; i < corners.size(); ++i) {
            normals += Vector3::normal(points[corners[(i + 0) % corners.size()]], 
                points[corners[(i + 1) % corners.size()]], 
                points[corners[(i + 2) % corners.size()]]);
        }
        return normals.normalized();
    };
    
    std::set<std::tuple<size_t, size_t, size_t, size_t>> candidates4;
    std::set<std::tuple<size_t, size_t, size_t, size_t, size_t>> candidates5;
    std::vector<std::vector<size_t>> face5s;
    for (const auto &level0It: edgeConnectMap) {
        const auto &level0 = level0It.first;
        auto findLevel1 = edgeConnectMap.find(level0);
        if (findLevel1 == edgeConnectMap.end())
            continue;
        const auto &triangleVertices = (*m_triangles)[pointSourceTriangles[level0]];
        auto triangleNormal = Vector3::normal((*m_vertices)[triangleVertices[0]],
            (*m_vertices)[triangleVertices[1]], 
            (*m_vertices)[triangleVertices[2]]);
        for (const auto &level1: findLevel1->second) {
            auto findLevel2 = edgeConnectMap.find(level1);
            if (findLevel2 == edgeConnectMap.end())
                continue;
            for (const auto &level2: findLevel2->second) {
                if (level0 == level2)
                    continue;
                auto findLevel3 = edgeConnectMap.find(level2);
                if (findLevel3 == edgeConnectMap.end())
                    continue;
                for (const auto &level3: findLevel3->second) {
                    if (level1 == level3)
                        continue;
                    auto findLevel4 = edgeConnectMap.find(level3);
                    if (findLevel4 == edgeConnectMap.end())
                        continue;
                    for (const auto &level4: findLevel4->second) {
                        if (level0 != level4) {
                            auto findLevel5 = edgeConnectMap.find(level4);
                            if (findLevel5 == edgeConnectMap.end())
                                continue;
                            for (const auto &level5: findLevel5->second) {
                                if (level0 != level5)
                                    continue;
                                std::vector<size_t> allLevels = {level0, level1, level2, level3, level4};
                                std::sort(allLevels.begin(), allLevels.end());
                                auto insertResult = candidates5.insert({allLevels[0], allLevels[1], allLevels[2], allLevels[3], allLevels[4]});
                                if (insertResult.second) {
                                    auto faceNormal = calculateFaceNormal({level0, level1, level2, level3, level4});
                                    if (Vector3::dotProduct(faceNormal, triangleNormal) > 0) {
                                        face5s.push_back({level0, level1, level2, level3, level4});
                                    } else {
                                        face5s.push_back({level4, level3, level2, level1, level0});
                                    }
                                }
                                break;
                            }
                            continue;
                        }
                        std::vector<size_t> allLevels = {level0, level1, level2, level3};
                        std::sort(allLevels.begin(), allLevels.end());
                        auto insertResult = candidates4.insert({allLevels[0], allLevels[1], allLevels[2], allLevels[3]});
                        if (insertResult.second) {
                            auto faceNormal = calculateFaceNormal({level0, level1, level2, level3});
                            if (Vector3::dotProduct(faceNormal, triangleNormal) > 0) {
                                quads->push_back({level0, level1, level2, level3});
                            } else {
                                quads->push_back({level3, level2, level1, level0});
                            }
                        }
                        break;
                    }
                }
            }
        }
    }
    
    if (!face5s.empty()) {
        std::unordered_map<size_t, size_t> replaceList;
        std::map<std::pair<size_t, size_t>, size_t> halfEdgeToFaceMap;
        for (size_t faceIndex = 0; faceIndex < face5s.size(); ++faceIndex) {
            const auto &it = face5s[faceIndex];
            for (size_t i = 0; i < it.size(); ++i) {
                size_t j = (i + 1) % it.size();
                halfEdgeToFaceMap[{it[i], it[j]}] = faceIndex;
            }
        }
        std::unordered_set<size_t> processedFaces;
        for (const auto &it: halfEdgeToFaceMap) {
            if (processedFaces.end() != processedFaces.find(it.second))
                continue;
            auto findOppositeFace = halfEdgeToFaceMap.find({it.first.second, it.first.first});
            if (findOppositeFace == halfEdgeToFaceMap.end())
                continue;
            if (processedFaces.end() != processedFaces.find(findOppositeFace->second))
                continue;
            processedFaces.insert(it.second);
            processedFaces.insert(findOppositeFace->second);
            auto removed = it.first.first;
            auto collapseTo = it.first.second;
            if (replaceList.end() != replaceList.find(collapseTo) || 
                    replaceList.end() != replaceList.find(removed))
                continue;
            replaceList[removed] = collapseTo;
            collapseEdge(&points, &edgeConnectMap, {removed, collapseTo});
            auto addQuad = [&](size_t face5Index) {
                std::vector<size_t> quad;
                quad.reserve(4);
                for (const auto &vertexIndex: face5s[face5Index]) {
                    if (vertexIndex == removed)
                        continue;
                    quad.push_back(vertexIndex);
                }
                if (4 == quad.size())
                    quads->push_back(quad);
            };
            addQuad(it.second);
            addQuad(findOppositeFace->second);
        }
        for (auto &it: *quads) {
            for (size_t i = 0; i < it.size(); ++i) {
                auto findReplacement = replaceList.find(it[i]);
                if (findReplacement == replaceList.end())
                    continue;
                it[i] = findReplacement->second;
            }
        }
    }
}

void QuadExtractor::extractConnections(std::vector<Vector3> *crossPoints, 
    std::vector<size_t> *sourceTriangles,
    std::set<std::pair<size_t, size_t>> *connections)
{
    std::map<PositionKey, size_t> crossPointMap;
    
    auto addCrossPoint = [&](const Vector3 &position3, size_t triangleIndex) {
        auto insertResult = crossPointMap.insert({position3, crossPoints->size()});
        if (insertResult.second) {
            crossPoints->push_back(position3);
            sourceTriangles->push_back(triangleIndex);
        }
        return insertResult.first->second;
    };
    auto addConnection = [&](size_t fromPointIndex, size_t toPointIndex) {
        if (fromPointIndex != toPointIndex)
            connections->insert({fromPointIndex, toPointIndex});
    };
    
    struct CrossPoint
    {
        Vector3 position3;
        Vector2 position2;
        int integer;
    };
    
    for (size_t triangleIndex = 0; triangleIndex < m_triangles->size(); ++triangleIndex) {
        //std::cerr << "Extract connections for triangle:" << triangleIndex << "..." << std::endl;
        
        const auto &cornerUvs = (*m_triangleUvs)[triangleIndex];
        const auto &cornerIndices = (*m_triangles)[triangleIndex];

        // Extract intersections of isolines with edges
        std::map<int, std::vector<std::vector<CrossPoint>>> lines[2];
        bool edgeCollapsed[2][3] = {{false, false, false},
            {false, false, false}};
        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                size_t k = (j + 1) % 3;
                const auto &current = cornerUvs[j];
                const auto &next = cornerUvs[k];
                if ((Double::isZero((double)(int)current[i] - current[i]) && 
                        Double::isZero(current[i] - next[i]))) {
                    int integer = (int)current[i];
                    edgeCollapsed[i][j] = true;
                    CrossPoint fromPoint;
                    fromPoint.position3 = (*m_vertices)[cornerIndices[j]];
                    fromPoint.position2 = cornerUvs[j];
                    fromPoint.integer = integer;
                    CrossPoint toPoint;
                    toPoint.position3 = (*m_vertices)[cornerIndices[k]];
                    toPoint.position2 = cornerUvs[k];
                    toPoint.integer = integer;
                    lines[i][integer].push_back({fromPoint, toPoint});
                }
            }
            std::map<int, std::vector<CrossPoint>> points;
            for (size_t j = 0; j < 3; ++j) {
                size_t k = (j + 1) % 3;
                const auto &current = cornerUvs[j];
                const auto &next = cornerUvs[k];
                double distance = std::abs(current[i] - next[i]);
                if ((int)current[i] != (int)next[i] ||
                        (current[i] > 0) != (next[i] > 0)) {
                    int lowInteger, highInteger;
                    double fromPosition;
                    double toPosition;
                    size_t fromIndex, toIndex;
                    if (current[i] < next[i]) {
                        lowInteger = (int)current[i];
                        highInteger = (int)next[i];
                        fromPosition = current[i];
                        toPosition = next[i];
                        fromIndex = j;
                        toIndex = k;
                    } else {
                        lowInteger = (int)next[i];
                        highInteger = (int)current[i];
                        fromPosition = next[i];
                        toPosition = current[i];
                        fromIndex = k;
                        toIndex = j;
                    }
                    for (int integer = lowInteger; integer <= highInteger; ++integer) {
                        double ratio = (integer - fromPosition) / distance;
                        if (ratio < 0 || ratio > 1)
                            continue;
                        if (Double::isZero(ratio) || Double::isZero(ratio - 1.0)) {
                            if (edgeCollapsed[i][j])
                                continue;
                        }
                        CrossPoint point;
                        point.position3 = (*m_vertices)[cornerIndices[fromIndex]] * (1 - ratio) + (*m_vertices)[cornerIndices[toIndex]] * ratio;
                        point.position2 = cornerUvs[fromIndex] * (1 - ratio) + cornerUvs[toIndex] * ratio;
                        point.integer = integer;
                        points[integer].push_back(point);
                    }
                }
            }
            for (const auto &it: points) {
                for (size_t pointIndex = 0; pointIndex < it.second.size(); ++pointIndex) {
                    size_t nextPointIndex = (pointIndex + 1) % it.second.size();
                    const auto &point = it.second[pointIndex];
                    const auto &nextPoint = it.second[nextPointIndex];
                    lines[i][it.first].push_back({point, nextPoint});
                }
            }
        }
        
        //std::cerr << "Segment lines by isolines for triangle:" << triangleIndex << "..." << std::endl;
        
        // Segment lines by isolines
        for (size_t i = 0; i < 2; ++i) {
            size_t j = (i + 1) % 2;
            for (const auto &targetIt: lines[i]) {
                for (const auto &target: targetIt.second) {
                    std::vector<std::vector<CrossPoint>> segments = {target};
                    for (const auto &splitIt: lines[j]) {
                        const auto &split = splitIt.second.begin();
                        const auto &coordIndex = j;
                        double segmentPosition = split[0][0].position2[coordIndex];
                        for (int segmentIndex = (int)segments.size() - 1; segmentIndex >= 0; --segmentIndex) {
                            auto &segment = segments[segmentIndex];
                            double fromPosition;
                            double toPosition;
                            size_t fromIndex;
                            size_t toIndex;
                            const auto &uv0 = segment[0].position2;
                            const auto &uv1 = segment[1].position2;
                            double distance = std::abs(uv0[coordIndex] - uv1[coordIndex]);
                            if (Double::isZero(distance))
                                continue;
                            if (uv0[coordIndex] < uv1[coordIndex]) {
                                fromPosition = uv0[coordIndex];
                                toPosition = uv1[coordIndex];
                                fromIndex = 0;
                                toIndex = 1;
                            } else {
                                fromPosition = uv1[coordIndex];
                                toPosition = uv0[coordIndex];
                                fromIndex = 1;
                                toIndex = 0;
                            }
                            if (segmentPosition < fromPosition || 
                                    segmentPosition > toPosition)
                                continue;
                            double ratio = (segmentPosition - fromPosition) / distance;
                            //std::cerr << "Split at ratio:" << ratio << std::endl;
                            Vector3 position3 = segment[fromIndex].position3 * (1 - ratio) + segment[toIndex].position3 * ratio;
                            Vector2 position2 = segment[fromIndex].position2 * (1 - ratio) + segment[toIndex].position2 * ratio;
                            int integer = segment[toIndex].integer;
                            CrossPoint newFromPoint;
                            newFromPoint.position3 = position3;
                            newFromPoint.position2 = position2;
                            newFromPoint.integer = integer;
                            CrossPoint newToPoint = segment[toIndex];
                            segment[toIndex] = newFromPoint;
                            segments.push_back({newFromPoint, newToPoint});
                        }
                    }
                    for (const auto &segment: segments) {
                        addConnection(addCrossPoint(segment[0].position3, triangleIndex), 
                            addCrossPoint(segment[1].position3, triangleIndex));
                    }
                }
            }
        }
    }
}

void QuadExtractor::fixFlippedFaces()
{
    std::map<std::pair<size_t, size_t>, std::vector<size_t>> halfEdgeToFaceMap;
    for (size_t i = 0; i < m_remeshedQuads.size(); ++i) {
        const auto &face = m_remeshedQuads[i];
        for (size_t j = 0; j < face.size(); ++j) {
            size_t k = (j + 1) % face.size();
            halfEdgeToFaceMap[{face[j], face[k]}].push_back(i);
        }
    }
    std::unordered_map<size_t, size_t> faceConflicts;
    for (const auto &it: halfEdgeToFaceMap) {
        if (it.second.size() == 1)
            continue;
        for (const auto &conflictFace: it.second)
            faceConflicts[conflictFace]++;
    }
    for (const auto &it: faceConflicts) {
        if (it.second >= 3) {
            std::reverse(m_remeshedQuads[it.first].begin(), m_remeshedQuads[it.first].end());
        }
    }
}

void QuadExtractor::fixHoles()
{
    Eigen::MatrixXi F(m_remeshedQuads.size() * 2, 3);
    for (size_t i = 0, j = 0; i < m_remeshedQuads.size(); ++i) {
        const auto &quad = m_remeshedQuads[i];
        F.row(j++) << quad[0], quad[1], quad[2];
        F.row(j++) << quad[2], quad[3], quad[0];
    }
    std::vector<std::vector<int>> loops;
    igl::boundary_loop(F, loops);
    
    std::cerr << "Detected holes:" << loops.size() << std::endl;
    
    for (const auto &loop: loops) {
        if (loop.size() > 14) {
            std::cerr << "Ignore long hole at length:" << loop.size() << std::endl;
            continue;
        }
        std::cerr << "Fixing hole at length:" << loop.size() << "..." << std::endl;
        GEO::vector<GEO::vec3> loopPoints;
        std::vector<int> loopVertices = loop;
        for (const auto &it: loop) {
            const auto &source = m_remeshedVertices[it];
            loopPoints.push_back(GEO::vec3(source[0], source[1], source[2]));
        }
        GEO::Poly3d polygon(loopPoints);
        GEO::vector<GEO::index_t> quadVertices;
        if (!polygon.try_quadrangulate(quadVertices)) {
            std::cerr << "Fix hole failed at length:" << loop.size() << std::endl;
            continue;
        }
        if (loopPoints.size() > loop.size()) {
            for (GEO::index_t i = loop.size(); i < loopPoints.size(); ++i) {
                const auto &source = loopPoints[i];
                loopVertices.push_back(m_remeshedVertices.size());
                std::cerr << "Add new vertex:" << m_remeshedVertices.size() << std::endl;
                m_remeshedVertices.push_back(Vector3(source[0], source[1], source[2]));
            }
        }
        for (GEO::index_t i = 0; i + 4 <= quadVertices.size(); i += 4) {
            m_remeshedQuads.push_back({
                (size_t)loopVertices[quadVertices[i + 3]],
                (size_t)loopVertices[quadVertices[i + 2]],
                (size_t)loopVertices[quadVertices[i + 1]],
                (size_t)loopVertices[quadVertices[i + 0]]
            });
            std::cerr << "Add new quad:{" << m_remeshedQuads[m_remeshedQuads.size() - 1][0] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][1] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][2] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][3] << "}" << std::endl;
        }
    }
}

}