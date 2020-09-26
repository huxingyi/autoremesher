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
#include <AutoRemesher/MeshSeparator>
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
    if (collapseShortEdges(&crossPoints, &edgeConnectMap))
        simplifyGraph(edgeConnectMap);
    collapseTriangles(&crossPoints, &edgeConnectMap);
    if (removeSingleEndpoints(&crossPoints, &edgeConnectMap))
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
    extractMesh(crossPoints, crossPointSourceTriangles, edgeConnectMap, &m_remeshedPolygons);
    std::cerr << "Extract mesh done" << std::endl;
    
    fixHoles();

#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-quads-withoutfix.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: m_remeshedPolygons) {
            fprintf(fp, "f");
            std::unordered_set<size_t> indices;
            for (const auto &v: it) {
                indices.insert(v);
                fprintf(fp, " %zu", 1 + v);
            }
            if (indices.size() != it.size()) {
                std::cerr << "Found repeated vertices in face:";
                for (const auto &v: it) {
                    std::cerr << v << " ";
                }
                std::cerr << std::endl;
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
#endif
    
    bool changed = false;
    if (removeIsolatedFaces())
        changed = true;
    while (removeNonManifoldFaces()) {
        changed = true;
        removeIsolatedFaces();
    }
    
    if (changed) {
        rebuildHalfEdges();
        fixHoles();
    }
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-quads.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: m_remeshedPolygons) {
            fprintf(fp, "f");
            for (const auto &v: it)
                fprintf(fp, " %zu", 1 + v);
            fprintf(fp, "\n");
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

bool QuadExtractor::removeSingleEndpoints(std::vector<Vector3> *crossPoints,
        std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap)
{
    bool removed = false;
    std::unordered_map<size_t, std::unordered_set<size_t>> &graph = *edgeConnectMap;
    std::vector<size_t> endpoints;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        if (it->second.size() != 1)
            continue;
        endpoints.push_back(it->first);
    }
    for (const auto &endpoint: endpoints) {
        size_t loopIndex = endpoint;
        for (;;) {
            auto findEndpoint = graph.find(loopIndex);
            if (findEndpoint == graph.end())
                break;
            if (findEndpoint->second.size() != 1)
                break;
            size_t neighbor = *findEndpoint->second.begin();
            graph.erase(loopIndex);
            removed = true;
            auto findNeighbor = graph.find(neighbor);
            if (findNeighbor == graph.end())
                break;
            findNeighbor->second.erase(loopIndex);
            loopIndex = neighbor;
        }
    }
    return removed;
}

bool QuadExtractor::collapseTriangles(std::vector<Vector3> *crossPoints,
        std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap)
{
    std::unordered_map<size_t, std::unordered_set<size_t>> triangleEdges;
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
                    triangleEdges[level0].insert(level1);
                    triangleEdges[level0].insert(level2);
                    triangleEdges[level1].insert(level0);
                    triangleEdges[level1].insert(level2);
                    triangleEdges[level2].insert(level0);
                    triangleEdges[level2].insert(level1);
                    break;
                }
            }
        }
    }
    
    if (triangleEdges.empty())
        return false;
    
    std::vector<std::vector<size_t>> clusters;
    std::unordered_set<size_t> visited;
    for (const auto &edge: triangleEdges) {
        std::queue<size_t> q;
        q.push(edge.first);
        std::vector<size_t> group;
        while (!q.empty()) {
            size_t v = q.front();
            q.pop();
            if (visited.find(v) != visited.end())
                continue;
            visited.insert(v);
            group.push_back(v);
            auto findNeighbor = triangleEdges.find(v);
            if (findNeighbor == triangleEdges.end())
                continue;
            for (const auto &neighbor: findNeighbor->second) {
                if (visited.find(neighbor) != visited.end())
                    continue;
                q.push(neighbor);
            }
        }
        if (group.empty())
            continue;
        clusters.push_back(group);
    }
    
    std::cerr << "collapseTriangles clusters:" << clusters.size() << std::endl;
    
    for (const auto &group: clusters) {
        Vector3 center;
        for (const auto &v: group)
            center += (*crossPoints)[v];
        center /= group.size();
        (*crossPoints)[group[0]] = center;
        
        std::cerr << "group:" << group.size() << std::endl;
        for (size_t i = 0; i < group.size(); ++i)
            std::cerr << "group[" << i << "]:" << group[i] << std::endl;
    
        std::unordered_set<size_t> moveNeighbors;
        for (size_t i = 1; i < group.size(); ++i) {
            for (const auto &neighbor: (*edgeConnectMap)[group[i]])
                moveNeighbors.insert(neighbor);
        }
        moveNeighbors.erase(group[0]);

        for (const auto &neighbor: moveNeighbors) {
            (*edgeConnectMap)[group[0]].insert(neighbor);
            (*edgeConnectMap)[neighbor].insert(group[0]);
        }
        
        for (const auto &neighbor: moveNeighbors) {
            for (size_t i = 1; i < group.size(); ++i) {
                (*edgeConnectMap)[neighbor].erase(group[i]);
                if ((*edgeConnectMap)[neighbor].empty())
                    (*edgeConnectMap).erase(neighbor);
            }
        }
        
        for (size_t i = 1; i < group.size(); ++i) {
            (*edgeConnectMap).erase(group[i]);
            (*edgeConnectMap)[group[0]].erase(group[i]);
        }
        if ((*edgeConnectMap)[group[0]].empty())
            (*edgeConnectMap).erase(group[0]);
    }
    
    return true;
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
    double collapsedLength = averageEdgeLength * 0.01;
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

void QuadExtractor::extractMesh(std::vector<Vector3> &points,
        const std::vector<size_t> &pointSourceTriangles,
        std::unordered_map<size_t, std::unordered_set<size_t>> &edgeConnectMap,
        std::vector<std::vector<size_t>> *quads)
{
    std::unordered_map<size_t, Vector3> triangleNormals;
    for (size_t pointIndex = 0; pointIndex < pointSourceTriangles.size(); ++pointIndex) {
        const auto &it = pointSourceTriangles[pointIndex];
        const auto &triangleVertices = (*m_triangles)[it];
        auto triangleNormal = Vector3::normal((*m_vertices)[triangleVertices[0]],
                (*m_vertices)[triangleVertices[1]], 
                (*m_vertices)[triangleVertices[2]]);
        triangleNormals.insert({pointIndex, triangleNormal});
    }
    
    auto calculateFaceNormal = [&](const std::vector<size_t> &corners) {
        Vector3 center;
        for (size_t i = 0; i < corners.size(); ++i) {
            center += points[corners[i]];
        }
        center /= corners.size();
        Vector3 normals;
        for (size_t i = 0; i < corners.size(); ++i) {
            normals += Vector3::normal(points[corners[(i + 0) % corners.size()]], 
                points[corners[(i + 1) % corners.size()]], 
                center);
        }
        return normals.normalized();
    };
    
    auto calculateSide = [&](const std::vector<size_t> &corners) {
        auto ringNormal = calculateFaceNormal(corners);
        Vector3 originalNormal;
        for (const auto &it: corners)
            originalNormal += triangleNormals[it];
        auto dot = Vector3::dotProduct(ringNormal, originalNormal.normalized());
        const double dotThreshold = 0.259; // > 75 or < 105 degrees
        if (dot > dotThreshold)
            return (int)1;
        else if (dot < -dotThreshold)
            return (int)-1;
        return (int)0;
    };
    
    std::set<std::tuple<size_t, size_t, size_t>> corners;
    auto &halfEdges = m_halfEdges;
    auto isConerUsed = [&](size_t previous, size_t current, size_t next) {
        if (corners.end() != corners.find(std::make_tuple(previous, current, next)))
            return true;
        if (corners.end() != corners.find(std::make_tuple(next, current, previous)))
            return true;
        return false;
    };
    auto isFaceCornerExist = [&](const std::vector<size_t> &vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            size_t k = (i + 2) % vertices.size();
            if (isConerUsed(vertices[i], vertices[j], vertices[k]))
                return true;
        }
        return false;
    };
    auto addFaceCorners = [&](const std::vector<size_t> &vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            size_t k = (i + 2) % vertices.size();
            corners.insert(std::make_tuple(vertices[i], vertices[j], vertices[k]));
            corners.insert(std::make_tuple(vertices[k], vertices[j], vertices[i]));
        }
    };
    auto isFaceHalfEdgeExist = [&](const std::vector<size_t> &vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            if (halfEdges.end() != halfEdges.find({vertices[i], vertices[j]}))
                return true;
        }
        return false;
    };
    auto addFaceHalfEdges = [&](const std::vector<size_t> &vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            halfEdges.insert({vertices[i], vertices[j]});
        }
    };
    
    size_t triangleRound = 4;
    for (size_t round = 0; round < 5; ++round) {
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
                if (halfEdges.find({level0, level1}) != halfEdges.end() &&
                        halfEdges.find({level1, level0}) != halfEdges.end())
                    continue;
                for (const auto &level2: findLevel2->second) {
                    if (level0 == level2)
                        continue;
                    auto findLevel3 = edgeConnectMap.find(level2);
                    if (findLevel3 == edgeConnectMap.end())
                        continue;
                    if (halfEdges.find({level1, level2}) != halfEdges.end() &&
                            halfEdges.find({level2, level1}) != halfEdges.end())
                        continue;
                    for (const auto &level3: findLevel3->second) {
                        if (level0 == level3) {
                            if (triangleRound == round) {
                                if (!isFaceCornerExist({level0, level1, level2})) {
                                    auto side = calculateSide({level0, level1, level2});
                                    if (side > 0) {
                                        if (!isFaceHalfEdgeExist({level0, level1, level2})) {
                                            quads->push_back({level0, level1, level2});
                                            addFaceCorners({level0, level1, level2});
                                            addFaceHalfEdges({level0, level1, level2});
                                        }
                                    } else if (side < 0) {
                                        if (!isFaceHalfEdgeExist({level2, level1, level0})) {
                                            quads->push_back({level2, level1, level0});
                                            addFaceCorners({level2, level1, level0});
                                            addFaceHalfEdges({level2, level1, level0});
                                        }
                                    }
                                }
                                break;
                            }
                        } else {
                            if (triangleRound == round)
                                break;
                        }
                        if (level1 == level3 || level0 == level3)
                            continue;
                        auto findLevel4 = edgeConnectMap.find(level3);
                        if (findLevel4 == edgeConnectMap.end())
                            continue;
                        if (halfEdges.find({level2, level3}) != halfEdges.end() &&
                                halfEdges.find({level3, level2}) != halfEdges.end())
                            continue;
                        for (const auto &level4: findLevel4->second) {
                            if (level0 != level4) {
                                if (level2 == level4 || level1 == level4)
                                    continue;
                                if (round < 1)
                                    continue;
                                auto findLevel5 = edgeConnectMap.find(level4);
                                if (findLevel5 == edgeConnectMap.end())
                                    continue;
                                if (halfEdges.find({level3, level4}) != halfEdges.end() &&
                                        halfEdges.find({level4, level3}) != halfEdges.end())
                                    continue;
                                for (const auto &level5: findLevel5->second) {
                                    if (level0 != level5) {
                                        if (level3 == level5 || level2 == level5 || level1 == level5)
                                            continue;
                                        if (round < 2)
                                            continue;
                                        auto findLevel6 = edgeConnectMap.find(level5);
                                        if (findLevel6 == edgeConnectMap.end())
                                            continue;
                                        if (halfEdges.find({level4, level5}) != halfEdges.end() &&
                                                halfEdges.find({level5, level4}) != halfEdges.end())
                                            continue;
                                        for (const auto &level6: findLevel6->second) {
                                            if (level0 != level6) {
                                                if (level4 == level6 || level3 == level6 || level2 == level6 || level1 == level6)
                                                    continue;
                                                if (round < 3)
                                                    continue;
                                                auto findLevel7 = edgeConnectMap.find(level6);
                                                if (findLevel7 == edgeConnectMap.end())
                                                    continue;
                                                if (halfEdges.find({level5, level6}) != halfEdges.end() &&
                                                        halfEdges.find({level6, level5}) != halfEdges.end())
                                                    continue;
                                                for (const auto &level7: findLevel7->second) {
                                                    if (level0 != level7)
                                                        continue;
                                                    if (3 != round)
                                                        break;
                                                    if (!isFaceCornerExist({level0, level1, level2, level3, level4, level5, level6})) {
                                                        auto side = calculateSide({level0, level1, level2, level3, level4, level5, level6});
                                                        if (side > 0) {
                                                            if (!isFaceHalfEdgeExist({level0, level1, level2, level3, level4, level5, level6})) {
                                                                quads->push_back({level0, level1, level2, level3, level4, level5, level6});
                                                                addFaceCorners({level0, level1, level2, level3, level4, level5, level6});
                                                                addFaceHalfEdges({level0, level1, level2, level3, level4, level5, level6});
                                                            }
                                                        } else if (side < 0) {
                                                            if (!isFaceHalfEdgeExist({level6, level5, level4, level3, level2, level1, level0})) {
                                                                quads->push_back({level6, level5, level4, level3, level2, level1, level0});
                                                                addFaceCorners({level6, level5, level4, level3, level2, level1, level0});
                                                                addFaceHalfEdges({level6, level5, level4, level3, level2, level1, level0});
                                                            }
                                                        }
                                                    }
                                                    break;
                                                }
                                                continue;
                                            }
                                            if (2 != round)
                                                break;
                                            if (!isFaceCornerExist({level0, level1, level2, level3, level4, level5})) {
                                                auto side = calculateSide({level0, level1, level2, level3, level4, level5});
                                                if (side > 0) {
                                                    if (!isFaceHalfEdgeExist({level0, level1, level2, level3, level4, level5})) {
                                                        quads->push_back({level0, level1, level2, level3, level4, level5});
                                                        addFaceCorners({level0, level1, level2, level3, level4, level5});
                                                        addFaceHalfEdges({level0, level1, level2, level3, level4, level5});
                                                    }
                                                } else if (side < 0) {
                                                    if (!isFaceHalfEdgeExist({level5, level4, level3, level2, level1, level0})) {
                                                        quads->push_back({level5, level4, level3, level2, level1, level0});
                                                        addFaceCorners({level5, level4, level3, level2, level1, level0});
                                                        addFaceHalfEdges({level5, level4, level3, level2, level1, level0});
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                        continue;
                                    }
                                    if (1 != round)
                                        break;
                                    if (!isFaceCornerExist({level0, level1, level2, level3, level4})) {
                                        auto side = calculateSide({level0, level1, level2, level3, level4});
                                        if (side > 0) {
                                            if (!isFaceHalfEdgeExist({level0, level1, level2, level3, level4})) {
                                                quads->push_back({level0, level1, level2, level3, level4});
                                                addFaceCorners({level0, level1, level2, level3, level4});
                                                addFaceHalfEdges({level0, level1, level2, level3, level4});
                                            }
                                        } else if (side < 0) {
                                            if (!isFaceHalfEdgeExist({level4, level3, level2, level1, level0})) {
                                                quads->push_back({level4, level3, level2, level1, level0});
                                                addFaceCorners({level4, level3, level2, level1, level0});
                                                addFaceHalfEdges({level4, level3, level2, level1, level0});
                                            }
                                        }
                                    }
                                    break;
                                }
                                continue;
                            }
                            if (0 != round)
                                break;
                            if (!isFaceCornerExist({level0, level1, level2, level3})) {
                                auto side = calculateSide({level0, level1, level2, level3});
                                if (side > 0) {
                                    if (!isFaceHalfEdgeExist({level0, level1, level2, level3})) {
                                        quads->push_back({level0, level1, level2, level3});
                                        addFaceCorners({level0, level1, level2, level3});
                                        addFaceHalfEdges({level0, level1, level2, level3});
                                    }
                                } else if (side < 0) {
                                    if (!isFaceHalfEdgeExist({level3, level2, level1, level0})) {
                                        quads->push_back({level3, level2, level1, level0});
                                        addFaceCorners({level3, level2, level1, level0});
                                        addFaceHalfEdges({level3, level2, level1, level0});
                                    }
                                }
                            }
                            break;
                        }
                    }
                }
            }
        }
    }
    
    m_remeshedVertices = points;
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

bool QuadExtractor::testPointInTriangle(const std::vector<Vector3> &points, 
        const std::vector<size_t> &triangle,
        const std::vector<size_t> &testPoints)
{
    Vector3 triangleNormal = Vector3::normal(points[triangle[0]],
        points[triangle[1]], points[triangle[2]]);
    std::vector<Vector3> pointsIn3d;
    for (const auto &it: triangle)
        pointsIn3d.push_back(points[it]);
    for (const auto &it: testPoints)
        pointsIn3d.push_back(points[it]);
    std::vector<Vector2> pointsIn2d;
    Vector3 origin = (points[triangle[0]] + points[triangle[1]] + points[triangle[2]]) / 3;
    Vector3 axis = (points[triangle[0]] - origin).normalized();
    Vector3::project(pointsIn3d, &pointsIn2d, triangleNormal, axis, origin);
    const Vector2 &a = pointsIn2d[0];
    const Vector2 &b = pointsIn2d[1];
    const Vector2 &c = pointsIn2d[2];
    for (size_t i = 3; i < pointsIn2d.size(); ++i) {
        if (Vector2::isInTriangle(a, b, c, pointsIn2d[i]))
            return true;
    }
    return false;
}

void QuadExtractor::rebuildHalfEdges()
{
    m_halfEdges.clear();
    for (const auto &it: m_remeshedPolygons) {
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            m_halfEdges.insert({it[i], it[j]});
        }
    }
}

void QuadExtractor::fixHoles()
{
    std::vector<std::vector<size_t>> loops;
    searchBoundaries(m_halfEdges, &loops);
    for (auto &loop: loops) {
        if (loop.size() > 65) {
            std::cerr << "Ignore long hole at length:" << loop.size() << std::endl;
            continue;
        }
        std::cerr << "Fixing hole at length:" << loop.size() << "..." << std::endl;
        fixHoleWithQuads(loop, true);
        if (loop.size() >= 4)
            fixHoleWithQuads(loop, false);
    }
}

void QuadExtractor::fixHoleWithQuads(std::vector<size_t> &hole, bool checkScore)
{
    auto recordHalfEdgesOfLastPolygon = [&]() {
        const auto &it = m_remeshedPolygons[m_remeshedPolygons.size() - 1];
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            m_halfEdges.insert({it[i], it[j]});
        }
    };
    
    for (;;) {
        if (hole.size() <= 2) {
            std::cerr << "fixHoleWithQuads cancel on edge length:" << hole.size() << std::endl;
            return;
        }
        
        if (3 == hole.size()) {
            m_remeshedPolygons.push_back({(size_t)hole[2], (size_t)hole[1], (size_t)hole[0]});
            recordHalfEdgesOfLastPolygon();
            return;
        }
        
        if (4 == hole.size()) {
            m_remeshedPolygons.push_back({(size_t)hole[3], (size_t)hole[2], (size_t)hole[1], (size_t)hole[0]});
            recordHalfEdgesOfLastPolygon();
            return;
        }
        
        std::vector<std::pair<int, double>> edgeScores;
        edgeScores.reserve(hole.size());
        for (int i = 0; i < hole.size(); ++i) {
            int h = (i + hole.size() - 1) % hole.size();
            int j = (i + 1) % hole.size();
            int k = (j + 1) % hole.size();
            auto left = (m_remeshedVertices[hole[h]] - m_remeshedVertices[hole[i]]).normalized();
            auto right = (m_remeshedVertices[hole[k]] - m_remeshedVertices[hole[j]]).normalized();
            edgeScores.push_back({i, Vector3::dotProduct(left, right)});
        }
        std::sort(edgeScores.begin(), edgeScores.end(), [](const std::pair<int, double> &first,
                const std::pair<int, double> &second) {
            return first.second < second.second;
        });
        bool holeChanged = false;
        for (int edgeIndex = edgeScores.size() - 1; edgeIndex >= 0; --edgeIndex) {
            const auto &score = edgeScores[edgeIndex];
            if (checkScore) {
                if (score.second <= 0) {
                    std::cerr << "fixHoleWithQuads failed, highest score(dot):" << score.second << std::endl;
                    return;
                }
            }
            int i = score.first;
            int h = (i + hole.size() - 1) % hole.size();
            int j = (i + 1) % hole.size();
            int k = (j + 1) % hole.size();
            std::vector<size_t> candidate = {(size_t)hole[k], (size_t)hole[j], (size_t)hole[i], (size_t)hole[h]};
            if (m_halfEdges.end() != m_halfEdges.find({candidate[0], candidate[1]}) ||
                    m_halfEdges.end() != m_halfEdges.find({candidate[1], candidate[2]}) ||
                    m_halfEdges.end() != m_halfEdges.find({candidate[2], candidate[3]}) ||
                    m_halfEdges.end() != m_halfEdges.find({candidate[3], candidate[0]})) {
                std::cerr << "fixHoleWithQuads ignore score:" << score.second << " because conflicts with existed quads" << std::endl;
                continue;
            }
            std::vector<size_t> remainPoints;
            for (int w = 0; w < hole.size(); ++w) {
                if (w == i || w == j || w == h || w == k)
                    continue;
                remainPoints.push_back(hole[w]);
            }
            if (testPointInTriangle(m_remeshedVertices, {candidate[0], candidate[1], candidate[2]}, remainPoints) ||
                    testPointInTriangle(m_remeshedVertices, {candidate[2], candidate[3], candidate[0]}, remainPoints)) {
                std::cerr << "fixHoleWithQuads ignore score:" << score.second << " because other point in the same loop fall into quad plane" << std::endl;
                continue;
            }
            m_remeshedPolygons.push_back(candidate);
            recordHalfEdgesOfLastPolygon();
                
            std::vector<size_t> newHole;
            for (int w = 0; w < hole.size(); ++w) {
                if (w == i || w == j)
                    continue;
                newHole.push_back(hole[w]);
            }
            hole = newHole;
            holeChanged = true;
            break;
        }
        if (!holeChanged)
            break;
    }
}

void QuadExtractor::searchBoundaries(const std::set<std::pair<size_t, size_t>> &halfEdges,
        std::vector<std::vector<size_t>> *loops)
{
    std::cerr << "Searching boundaries..." << std::endl;
    
    std::unordered_map<size_t, std::unordered_set<size_t>> nextMap;
    for (const auto &it: halfEdges) {
        if (halfEdges.end() != halfEdges.find({it.second, it.first}))
            continue;
        nextMap[it.first].insert(it.second);
    }
    
    while (!nextMap.empty()) {
        auto it = nextMap.begin();
        std::vector<size_t> loop;
        size_t startVertex = it->first;
        bool validate = false;
        std::cerr << "Searching loop from:" << startVertex << std::endl;
        while (it != nextMap.end()) {
            if (startVertex == it->first && loop.size() >= 3) {
                std::cerr << "Found valid loop, size:" << loop.size() << std::endl;
                validate = true;
                break;
            }
            std::cerr << "Loop add vertex:" << it->first << std::endl;
            loop.push_back(it->first);
            if (it->second.size() != 1) {
                std::cerr << "Break loop, because of next size:" << it->second.size() << std::endl;
                break;
            }
            it = nextMap.find(*it->second.begin());
        }
        for (const auto &v: loop)
            nextMap.erase(v);
        if (validate)
            loops->push_back(loop);
    }
    
    std::cerr << "Searching boundaries done" << std::endl;
}

bool QuadExtractor::removeIsolatedFaces()
{
    std::vector<std::vector<std::vector<size_t>>> quadsIslands;
    MeshSeparator::splitToIslands(m_remeshedPolygons, quadsIslands);
    if (quadsIslands.empty())
        return false;
    m_remeshedPolygons = *std::max_element(quadsIslands.begin(), quadsIslands.end(), [&](const std::vector<std::vector<size_t>> &first,
            const std::vector<std::vector<size_t>> &second) {
        return first.size() < second.size();
    });
    return true;
}

bool QuadExtractor::removeNonManifoldFaces()
{
    bool changed = false;
    std::map<std::pair<size_t, size_t>, size_t> edgeToFaceMap;
    MeshSeparator::buildEdgeToFaceMap(m_remeshedPolygons, edgeToFaceMap);
    std::unordered_map<size_t, size_t> vertexOpenBoundaryCountMap;
    for (const auto &it: edgeToFaceMap) {
        if (edgeToFaceMap.end() != edgeToFaceMap.find({it.first.second, it.first.first}))
            continue;
        vertexOpenBoundaryCountMap[it.first.first]++;
        vertexOpenBoundaryCountMap[it.first.second]++;
    }
    std::vector<std::vector<size_t>> manifoldFaces;
    for (const auto &it: m_remeshedPolygons) {
        bool isNonManifold = false;
        for (size_t i = 0; i < it.size(); ++i) {
            auto findCount = vertexOpenBoundaryCountMap.find(it[i]);
            if (findCount == vertexOpenBoundaryCountMap.end())
                continue;
            if (findCount->second > 2) {
                isNonManifold = true;
                break;
            }
        }
        if (isNonManifold) {
            changed = true;
            continue;
        }
        manifoldFaces.push_back(it);
    }
    m_remeshedPolygons = manifoldFaces;
    return changed;
}

}