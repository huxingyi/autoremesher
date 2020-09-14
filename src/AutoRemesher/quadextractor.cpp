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
    bool collapsedShortEdges = collapseShortEdges(&crossPoints, &edgeConnectMap);
    //bool collapsedTriangles = collapseTriangles(&crossPoints, &edgeConnectMap);
    if (collapsedShortEdges/* || collapsedTriangles*/)
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
        for (const auto &v: it)
            addQuadVertex(v);
    }
    for (auto &it: m_remeshedQuads) {
        for (auto &v: it)
            v = oldToNewMap[v];
    }
    
    fixFlippedFaces();
    removeIsolatedFaces();
    while (removeNonManifoldFaces())
        removeIsolatedFaces();
    recordGoodQuads();
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-quads-withoutfix.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: m_remeshedQuads) {
            fprintf(fp, "f");
            for (const auto &v: it)
                fprintf(fp, " %zu", 1 + v);
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
#endif

    fixHoles();
    connectTvertices();
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-quads.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto &vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: m_remeshedQuads) {
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

bool QuadExtractor::removeIsolatedFaces()
{
    std::vector<std::vector<std::vector<size_t>>> quadsIslands;
    MeshSeparator::splitToIslands(m_remeshedQuads, quadsIslands);
    if (quadsIslands.empty())
        return false;
    m_remeshedQuads = *std::max_element(quadsIslands.begin(), quadsIslands.end(), [&](const std::vector<std::vector<size_t>> &first,
            const std::vector<std::vector<size_t>> &second) {
        return first.size() < second.size();
    });
    return true;
}

bool QuadExtractor::removeNonManifoldFaces()
{
    bool changed = false;
    std::map<std::pair<size_t, size_t>, size_t> edgeToFaceMap;
    MeshSeparator::buildEdgeToFaceMap(m_remeshedQuads, edgeToFaceMap);
    std::unordered_map<size_t, size_t> vertexOpenBoundaryCountMap;
    for (const auto &it: edgeToFaceMap) {
        if (edgeToFaceMap.end() != edgeToFaceMap.find({it.first.second, it.first.first}))
            continue;
        vertexOpenBoundaryCountMap[it.first.first]++;
        vertexOpenBoundaryCountMap[it.first.second]++;
    }
    std::vector<std::vector<size_t>> manifoldFaces;
    for (const auto &it: m_remeshedQuads) {
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
    m_remeshedQuads = manifoldFaces;
    return changed;
}

void QuadExtractor::recordGoodQuads()
{
    m_goodQuadHalfEdges.clear();
    for (const auto &it: m_remeshedQuads) {
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            m_goodQuadHalfEdges.insert({it[i], it[j]});
        }
    }
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
    auto calculateFaceNormal = [&](const std::vector<size_t> &corners) {
        Vector3 normals;
        for (size_t i = 0; i < corners.size(); ++i) {
            normals += Vector3::normal(points[corners[(i + 0) % corners.size()]], 
                points[corners[(i + 1) % corners.size()]], 
                points[corners[(i + 2) % corners.size()]]);
        }
        return normals.normalized();
    };
    
    std::set<std::tuple<size_t, size_t, size_t>> corners;
    std::set<std::pair<size_t, size_t>> halfEdges;
    auto isConerUsed = [&](size_t previous, size_t current, size_t next) {
        if (corners.end() != corners.find(std::make_tuple(previous, current, next)))
            return true;
        if (corners.end() != corners.find(std::make_tuple(next, current, previous)))
            return true;
        return false;
    };
    auto isQuadCornerConflits = [&](size_t level0, size_t level1, size_t level2, size_t level3) {
        if (isConerUsed(level0, level1, level2))
            return true;
        if (isConerUsed(level1, level2, level3))
            return true;
        if (isConerUsed(level3, level0, level1))
            return true;
        return false;
    };
    auto addQuadCorners = [&](size_t level0, size_t level1, size_t level2, size_t level3) {
        corners.insert(std::make_tuple(level0, level1, level2));
        corners.insert(std::make_tuple(level2, level1, level0));
        
        corners.insert(std::make_tuple(level1, level2, level3));
        corners.insert(std::make_tuple(level3, level2, level1));
        
        corners.insert(std::make_tuple(level3, level0, level1));
        corners.insert(std::make_tuple(level1, level0, level3));
    };
    auto isQuadHalfEdgeConflits = [&](size_t level0, size_t level1, size_t level2, size_t level3) {
        if (halfEdges.end() != halfEdges.find({level0, level1}))
            return true;
        if (halfEdges.end() != halfEdges.find({level1, level2}))
            return true;
        if (halfEdges.end() != halfEdges.find({level2, level3}))
            return true;
        if (halfEdges.end() != halfEdges.find({level3, level0}))
            return true;
        return false;
    };
    auto addQuadHalfEdges = [&](size_t level0, size_t level1, size_t level2, size_t level3) {
        halfEdges.insert({level0, level1});
        halfEdges.insert({level1, level2});
        halfEdges.insert({level2, level3});
        halfEdges.insert({level3, level0});
    };
    
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
                            continue;
                        }
                        if (!isQuadCornerConflits(level0, level1, level2, level3)) {
                            auto faceNormal = calculateFaceNormal({level0, level1, level2, level3});
                            if (Vector3::dotProduct(faceNormal, triangleNormal) > 0) {
                                if (!isQuadHalfEdgeConflits(level0, level1, level2, level3)) {
                                    quads->push_back({level0, level1, level2, level3});
                                    addQuadCorners(level0, level1, level2, level3);
                                    addQuadHalfEdges(level0, level1, level2, level3);
                                }
                            } else {
                                if (!isQuadHalfEdgeConflits(level3, level2, level1, level0)) {
                                    quads->push_back({level3, level2, level1, level0});
                                    addQuadCorners(level3, level2, level1, level0);
                                    addQuadHalfEdges(level3, level2, level1, level0);
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

void QuadExtractor::buildVertexNeighborMap(std::unordered_map<size_t, std::vector<size_t>> *vertexNeighborMap)
{
    for (const auto &it: m_remeshedQuads) {
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            (*vertexNeighborMap)[it[i]].push_back(it[j]);
            (*vertexNeighborMap)[it[j]].push_back(it[i]);
        }
    }
}

void QuadExtractor::connectTwoTvertices(size_t startVertex, const std::vector<size_t> &path)
{
    std::cerr << "Connecting t-vertices, start:" << startVertex << std::endl;
    for (size_t i = 0; i < path.size(); ++i)
        std::cerr << "path["<<i<<"]:" << path[i] << std::endl;
    
    std::unordered_set<size_t> needRemoveVertices;
    needRemoveVertices.insert(startVertex);
    for (const auto &it: path)
        needRemoveVertices.insert(it);
    
    bool changed = false;
    std::vector<std::vector<size_t>> remainFaces;
    for (const auto &it: m_remeshedQuads) {
        bool needRemoveFace = false;
        for (size_t i = 0; i < it.size(); ++i) {
            auto findNeedRemove = needRemoveVertices.find(it[i]);
            if (findNeedRemove == needRemoveVertices.end())
                continue;
            needRemoveFace = true;
            break;
        }
        if (needRemoveFace) {
            changed = true;
            continue;
        }
        remainFaces.push_back(it);
    }
    if (!changed)
        return;
    
    m_remeshedQuads = remainFaces;
    std::cerr << "Fixing holes for removed t-vertices from:" << startVertex << std::endl;
    recordGoodQuads();
    fixHoles();
}

void QuadExtractor::connectTvertices()
{
    while (!m_tVertices.empty()) {
        std::unordered_map<size_t, std::vector<size_t>> vertexNeighborMap;
        buildVertexNeighborMap(&vertexNeighborMap);
        
        std::unordered_map<size_t, size_t> parentMap;
        
        size_t startVertex = *m_tVertices.begin();
        std::cerr << "Searching nearest t-vertex, start:" << startVertex << std::endl;
        m_tVertices.erase(startVertex);
        std::queue<size_t> q;
        std::unordered_set<size_t> visited;
        visited.insert(startVertex);
        auto findNeighbor = vertexNeighborMap.find(startVertex);
        if (findNeighbor == vertexNeighborMap.end()) {
            m_tVertices.erase(startVertex);
            continue;
        }
        for (const auto &it: findNeighbor->second) {
            parentMap[it] = startVertex;
            q.push(it);
        }
        while (!q.empty()) {
            auto v = q.front();
            q.pop();
            if (visited.end() != visited.find(v))
                continue;
            if (m_tVertices.end() != m_tVertices.find(v)) {
                std::vector<size_t> path;
                size_t stopVertex = v;
                while (startVertex != v) {
                    path.push_back(v);
                    v = parentMap[v];
                }
                std::reverse(path.begin(), path.end());
                m_tVertices.erase(stopVertex);
                connectTwoTvertices(startVertex, path);
                break;
            }
            visited.insert(v);
            auto findNeighbor = vertexNeighborMap.find(v);
            if (findNeighbor == vertexNeighborMap.end())
                continue;
            for (const auto &it: findNeighbor->second) {
                if (visited.end() != visited.find(it))
                    continue;
                parentMap[it] = v;
                q.push(it);
            }
        }
    }
}

void QuadExtractor::makeTedge(const std::vector<size_t> &triangle)
{
    std::vector<std::pair<size_t, double>> cornerAngles;
    for (size_t i = 0; i < triangle.size(); ++i) {
        size_t h = (i + triangle.size() - 1) % triangle.size();
        size_t j = (i + 1) % triangle.size();
        cornerAngles.push_back({i, Vector3::angle((m_remeshedVertices[triangle[h]] - m_remeshedVertices[triangle[i]]).normalized(), 
            (m_remeshedVertices[triangle[j]] - m_remeshedVertices[triangle[i]]).normalized())});
    }
    size_t collapseIndex = std::max_element(cornerAngles.begin(), cornerAngles.end(), [](const std::pair<size_t, double> &first,
            const std::pair<size_t, double> &second) {
        return first.second < second.second;
    })->first;
    Vector3 collapseTo;
    for (size_t i = 0; i < triangle.size(); ++i) {
        if (i == collapseIndex)
            continue;
        collapseTo += m_remeshedVertices[triangle[i]];
    }
    collapseTo /= 2;
    m_remeshedVertices[triangle[collapseIndex]] = collapseTo;
    m_tVertices.insert(triangle[collapseIndex]);
}

void QuadExtractor::fixHoleWithQuads(std::vector<size_t> &hole, bool checkScore)
{
    for (;;) {
        if (hole.size() <= 2) {
            std::cerr << "fixHoleWithQuads cancel on edge length:" << hole.size() << std::endl;
            return;
        }
        
        if (3 == hole.size()) {
            std::cerr << "fixHoleWithQuads make t-edge" << std::endl;
            makeTedge(hole);
            return;
        }
        
        if (4 == hole.size()) {
            m_remeshedQuads.push_back({(size_t)hole[3], (size_t)hole[2], (size_t)hole[1], (size_t)hole[0]});
            std::cerr << "fixHoleWithQuads new quad:{" << m_remeshedQuads[m_remeshedQuads.size() - 1][0] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][1] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][2] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][3] << "}" << std::endl;
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
            if (m_goodQuadHalfEdges.end() != m_goodQuadHalfEdges.find({candidate[0], candidate[1]}) ||
                    m_goodQuadHalfEdges.end() != m_goodQuadHalfEdges.find({candidate[1], candidate[2]}) ||
                    m_goodQuadHalfEdges.end() != m_goodQuadHalfEdges.find({candidate[2], candidate[3]}) ||
                    m_goodQuadHalfEdges.end() != m_goodQuadHalfEdges.find({candidate[3], candidate[0]})) {
                std::cerr << "fixHoleWithQuads ignore score:" << score.second << " because conflicts with good quads" << std::endl;
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
            m_remeshedQuads.push_back(candidate);
            std::cerr << "fixHoleWithQuads new quad:{" << m_remeshedQuads[m_remeshedQuads.size() - 1][0] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][1] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][2] << "," <<
                m_remeshedQuads[m_remeshedQuads.size() - 1][3] << "}" << std::endl;
                
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

void QuadExtractor::fixHoles()
{
    Eigen::MatrixXi F(m_remeshedQuads.size() * 2, 3);
    for (size_t i = 0, j = 0; i < m_remeshedQuads.size(); ++i) {
        const auto &quad = m_remeshedQuads[i];
        F.row(j++) << quad[0], quad[1], quad[2];
        F.row(j++) << quad[2], quad[3], quad[0];
    }
    std::vector<std::vector<size_t>> loops;
    igl::boundary_loop(F, loops);
    
    std::cerr << "Detected holes:" << loops.size() << std::endl;
    
    for (auto &loop: loops) {
        if (loop.size() > 65) {
            std::cerr << "Ignore long hole at length:" << loop.size() << std::endl;
            continue;
        }
        if (loop.size() == 1)
            continue;
        std::cerr << "Fixing hole at length:" << loop.size() << "..." << std::endl;
        fixHoleWithQuads(loop, true);
        if (loop.size() >= 4)
            fixHoleWithQuads(loop, false);
    }
}

}