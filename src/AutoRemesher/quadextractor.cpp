/*
 *  Copyright (c) 2026 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
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
#include <AutoRemesher/Double>
#include <AutoRemesher/MeshSeparator>
#include <AutoRemesher/PositionKey>
#include <AutoRemesher/QuadExtractor>
#include <algorithm>
#include <axisalignedboundingbox.h>
#include <axisalignedboundingboxtree.h>
#include <iostream>
#include <limits>
#include <map>
#include <set>
#if defined(__has_include)
#if __has_include(<oneapi/tbb/blocked_range.h>)
#include <oneapi/tbb/blocked_range.h>
#include <oneapi/tbb/parallel_for.h>
#else
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#endif
#else
#include <tbb/blocked_range.h>
#include <tbb/parallel_for.h>
#endif
#include <unordered_map>
#include <unordered_set>

namespace AutoRemesher {

bool QuadExtractor::extract()
{
    // The fractions are the measured share of extraction each step costs.  The
    // topology cleanup passes at the end are over half of it, so they report
    // individually instead of as one long silent block.
    const auto report = [this](float fraction, const char* name) {
        if (m_progressHandler)
            m_progressHandler(fraction, name);
    };

    report(0.0f, "Extracting connections");
    std::cerr << "Extract connections..." << std::endl;
    std::vector<Vector3> crossPoints;
    std::vector<size_t> crossPointSourceTriangles;
    std::set<std::pair<size_t, size_t>> connections;
    extractConnections(&crossPoints, &crossPointSourceTriangles, &connections);
    report(0.07f, "Holding singular lines");
    holdSingularLines(&crossPoints, &crossPointSourceTriangles, &connections);
    m_extractedConnections.clear();
    m_extractedConnectionMoved.clear();
    m_extractedConnections.reserve(connections.size());
    std::vector<uint8_t> triangleMoved;
    if (nullptr != m_originalTriangleUvs
        && m_originalTriangleUvs->size() == m_triangleUvs->size()) {
        triangleMoved.assign(m_triangleUvs->size(), 0);
        for (size_t i = 0; i < triangleMoved.size(); ++i) {
            const auto& before = (*m_originalTriangleUvs)[i];
            const auto& after = (*m_triangleUvs)[i];
            for (size_t k = 0; k < 3 && k < before.size() && k < after.size(); ++k) {
                if (before[k].x() != after[k].x() || before[k].y() != after[k].y()) {
                    triangleMoved[i] = 1;
                    break;
                }
            }
        }
    }
    m_extractedConnectionMoved.reserve(connections.size());
    for (const auto& connection : connections) {
        m_extractedConnections.emplace_back(crossPoints[connection.first],
            crossPoints[connection.second]);
        const auto edge = std::make_pair(std::min(connection.first, connection.second),
            std::max(connection.first, connection.second));
        if (m_addedConnections.end() != m_addedConnections.find(edge)) {
            m_extractedConnectionMoved.push_back(2);
        } else if (!triangleMoved.empty()) {
            const size_t firstTriangle = crossPointSourceTriangles[connection.first];
            const size_t secondTriangle = crossPointSourceTriangles[connection.second];
            m_extractedConnectionMoved.push_back(
                (triangleMoved[firstTriangle] || triangleMoved[secondTriangle]) ? 1 : 0);
        } else {
            m_extractedConnectionMoved.push_back(0);
        }
    }
    std::cerr << "Extract connections done" << std::endl;

#if AUTO_REMESHER_DEV
    {
        FILE* fp = fopen("debug-quadextractor-connections.obj", "wb");
        for (size_t i = 0; i < crossPoints.size(); ++i) {
            const auto& vertex = crossPoints[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto& it : connections) {
            fprintf(fp, "l %zu %zu\n", 1 + it.first, 1 + it.second);
        }
        fclose(fp);
    }
#endif

    report(0.21f, "Extracting edges");
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
            auto insertResult = oldToNewMap.insert({ vertexIndex, edgeVertices.size() });
            if (insertResult.second)
                edgeVertices.push_back(crossPoints[vertexIndex]);
            return insertResult.first->second;
        };
        for (const auto& it : edgeConnectMap) {
            addEdgeVertex(it.first);
            for (const auto& it2 : it.second) {
                addEdgeVertex(it2);
            }
        }
        FILE* fp = fopen("debug-quadextractor-edges.obj", "wb");
        for (size_t i = 0; i < edgeVertices.size(); ++i) {
            const auto& vertex = edgeVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto& it : edgeConnectMap) {
            for (const auto& it2 : it.second)
                fprintf(fp, "l %zu %zu\n", 1 + oldToNewMap[it.first], 1 + oldToNewMap[it2]);
        }
        fclose(fp);
    }
#endif

    report(0.25f, "Extracting mesh");
    std::cerr << "Extract mesh..." << std::endl;
    extractMesh(crossPoints, crossPointSourceTriangles, edgeConnectMap, &m_remeshedPolygons);
    std::cerr << "Extract mesh done" << std::endl;

    report(0.29f, "Fixing holes");
    fixHoles();

#if AUTO_REMESHER_DEV
    {
        FILE* fp = fopen("debug-quadextractor-quads-withoutfix.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto& vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto& it : m_remeshedPolygons) {
            fprintf(fp, "f");
            std::unordered_set<size_t> indices;
            for (const auto& v : it) {
                indices.insert(v);
                fprintf(fp, " %zu", 1 + v);
            }
            if (indices.size() != it.size()) {
                std::cerr << "Found repeated vertices in face:";
                for (const auto& v : it) {
                    std::cerr << v << " ";
                }
                std::cerr << std::endl;
            }
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
#endif

    report(0.30f, "Removing non-manifold faces");
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

    {
        std::set<size_t> usedVertices;
        for (const auto& face : m_remeshedPolygons) {
            for (const auto& v : face)
                usedVertices.insert(v);
        }
        if (usedVertices.size() < m_remeshedVertices.size()) {
            std::vector<Vector3> compactedVertices;
            std::unordered_map<size_t, size_t> oldToNew;
            compactedVertices.reserve(usedVertices.size());
            for (const auto& oldIndex : usedVertices) {
                oldToNew[oldIndex] = compactedVertices.size();
                compactedVertices.push_back(m_remeshedVertices[oldIndex]);
            }
            for (auto& face : m_remeshedPolygons) {
                for (auto& v : face)
                    v = oldToNew[v];
            }
            m_remeshedVertices = std::move(compactedVertices);
        }
    }

    report(0.31f, "Smoothing and projecting");
    std::cerr << "Smooth and project..." << std::endl;
    smoothAndProject(5);
    std::cerr << "Smooth and project done" << std::endl;

    report(0.44f, "Splitting seven edge faces");
    splitSevenEdgeFaces();
    report(0.45f, "Splitting six edge faces");
    splitSixEdgeFaces();
    // A pentagon is the best place for a triangle to end up, it comes out of the
    // collapse as a quad, so the triangles run first and the merge takes care of
    // whatever pentagons are left over
    report(0.46f, "Cleaning up triangles");
    cleanupTriangles();
    report(0.53f, "Merging shared five edge faces");
    ProgressHandler mergeProgress;
    if (m_progressHandler) {
        mergeProgress = [this](float fraction, const char* name) {
            m_progressHandler(0.53f + (0.85f - 0.53f) * fraction, name);
        };
    }
    mergeSharedFiveEdgeFaces(mergeProgress ? &mergeProgress : nullptr);
    // Runs last, it only reconnects quad pairs, so it wants the triangles and
    // pentagons to have become quads already
    report(0.85f, "Switching high valence edges");
    switchHighValenceEdges();
    report(0.89f, "Converting triangle and five edge fans");
    convertTriangleAndFiveEdgeFans();
    report(0.93f, "Collapsing three valence diagonals");
    collapseThreeValenceDiagonals();
    report(0.97f, "Merging double shared edge quads");
    mergeDoubleSharedEdgeQuads();
    report(1.0f, "");

#if AUTO_REMESHER_DEV
    {
        FILE* fp = fopen("debug-quadextractor-quads.obj", "wb");
        for (size_t i = 0; i < m_remeshedVertices.size(); ++i) {
            const auto& vertex = m_remeshedVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto& it : m_remeshedPolygons) {
            fprintf(fp, "f");
            for (const auto& v : it)
                fprintf(fp, " %zu", 1 + v);
            fprintf(fp, "\n");
        }
        fclose(fp);
    }
#endif

    return true;
}

void QuadExtractor::extractEdges(const std::set<std::pair<size_t, size_t>>& connections,
    std::unordered_map<size_t, std::unordered_set<size_t>>* edgeConnectMap)
{
    auto& graph = *edgeConnectMap;
    for (const auto& it : connections) {
        graph[it.first].insert(it.second);
        graph[it.second].insert(it.first);
    }
    simplifyGraph(graph);
}

void QuadExtractor::simplifyGraph(std::unordered_map<size_t, std::unordered_set<size_t>>& graph)
{
    for (;;) {
        std::unordered_map<size_t, std::pair<size_t, size_t>> delayPairs;
        for (auto it = graph.begin(); it != graph.end();) {
            if (it->second.size() != 2) {
                ++it;
                continue;
            }
            size_t firstNeighbor, secondNeighbor;
            auto neighborIt = it->second.begin();
            firstNeighbor = *neighborIt++;
            secondNeighbor = *neighborIt++;
            if (delayPairs.end() != delayPairs.find(firstNeighbor) || delayPairs.end() != delayPairs.find(secondNeighbor)) {
                ++it;
                continue;
            }
            delayPairs.insert({ it->first, { firstNeighbor, secondNeighbor } });
            it = graph.erase(it);
        }
        if (delayPairs.empty())
            break;
        for (const auto& it : delayPairs) {
            graph[it.second.first].erase(it.first);
            graph[it.second.first].insert(it.second.second);
            graph[it.second.second].erase(it.first);
            graph[it.second.second].insert(it.second.first);
        }
    }
}

bool QuadExtractor::removeSingleEndpoints(std::vector<Vector3>* crossPoints,
    std::unordered_map<size_t, std::unordered_set<size_t>>* edgeConnectMap)
{
    bool removed = false;
    std::unordered_map<size_t, std::unordered_set<size_t>>& graph = *edgeConnectMap;
    std::vector<size_t> endpoints;
    for (auto it = graph.begin(); it != graph.end(); ++it) {
        if (it->second.size() != 1)
            continue;
        endpoints.push_back(it->first);
    }
    for (const auto& endpoint : endpoints) {
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

bool QuadExtractor::collapseTriangles(std::vector<Vector3>* crossPoints,
    std::unordered_map<size_t, std::unordered_set<size_t>>* edgeConnectMap)
{
    auto& graph = *edgeConnectMap;

    std::set<std::tuple<size_t, size_t, size_t>> triangles;
    for (const auto& level0It : graph) {
        const auto& level0 = level0It.first;
        for (const auto& level1 : level0It.second) {
            auto findLevel2 = graph.find(level1);
            if (findLevel2 == graph.end())
                continue;
            for (const auto& level2 : findLevel2->second) {
                if (level0 == level2)
                    continue;
                auto findLevel3 = graph.find(level2);
                if (findLevel3 == graph.end())
                    continue;
                if (findLevel3->second.end() == findLevel3->second.find(level0))
                    continue;
                std::vector<size_t> sorted = { level0, level1, level2 };
                std::sort(sorted.begin(), sorted.end());
                triangles.insert(std::make_tuple(sorted[0], sorted[1], sorted[2]));
            }
        }
    }

    if (triangles.empty())
        return false;

    // Collapse one edge per triangle, the shortest one. Merging every connected
    // triangle into a single point instead would drag all of their neighbors onto
    // that point and leave a star of slivers behind it
    bool collapsed = false;
    for (const auto& triangle : triangles) {
        const size_t corners[3] = { std::get<0>(triangle), std::get<1>(triangle), std::get<2>(triangle) };
        std::pair<size_t, size_t> shortestEdge;
        double shortestLength = std::numeric_limits<double>::max();
        bool stillATriangle = true;
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            auto findCorner = graph.find(corners[i]);
            if (findCorner == graph.end() || findCorner->second.end() == findCorner->second.find(corners[j])) {
                stillATriangle = false;
                break;
            }
            double length = ((*crossPoints)[corners[i]] - (*crossPoints)[corners[j]]).length();
            if (length < shortestLength) {
                shortestLength = length;
                shortestEdge = { corners[i], corners[j] };
            }
        }
        // An earlier collapse may have already taken this one apart
        if (!stillATriangle)
            continue;
        collapseEdge(crossPoints, edgeConnectMap, shortestEdge);
        collapsed = true;
    }

    return collapsed;
}

bool QuadExtractor::collapseShortEdges(std::vector<Vector3>* crossPoints,
    std::unordered_map<size_t, std::unordered_set<size_t>>* edgeConnectMap)
{
    double totalLength = 0.0;
    size_t edgeCount = 0;
    std::map<std::pair<size_t, size_t>, double> edgeLengths;
    for (const auto& it : *edgeConnectMap) {
        for (const auto& neighbor : it.second) {
            if (edgeLengths.end() != edgeLengths.find({ neighbor, it.first }))
                continue;
            double edgeLength = ((*crossPoints)[it.first] - (*crossPoints)[neighbor]).length();
            totalLength += edgeLength;
            edgeLengths.insert({ { it.first, neighbor }, edgeLength });
            ++edgeCount;
        }
    }
    if (0 == edgeCount)
        return false;
    double averageEdgeLength = totalLength / edgeCount;
    double collapsedLength = averageEdgeLength * 0.01;
    bool collapsed = false;
    for (const auto& it : edgeLengths) {
        if (it.second > collapsedLength)
            continue;
        collapseEdge(crossPoints, edgeConnectMap, it.first);
        collapsed = true;
    }
    return collapsed;
}

void QuadExtractor::collapseEdge(std::vector<Vector3>* crossPoints,
    std::unordered_map<size_t, std::unordered_set<size_t>>* edgeConnectMap,
    const std::pair<size_t, size_t>& edge)
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
    for (const auto& neighbor : firstNeighbors) {
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

void QuadExtractor::extractMesh(std::vector<Vector3>& points,
    const std::vector<size_t>& pointSourceTriangles,
    std::unordered_map<size_t, std::unordered_set<size_t>>& edgeConnectMap,
    std::vector<std::vector<size_t>>* quads)
{
    std::unordered_map<size_t, Vector3> triangleNormals;
    for (size_t pointIndex = 0; pointIndex < pointSourceTriangles.size(); ++pointIndex) {
        const auto& it = pointSourceTriangles[pointIndex];
        const auto& triangleVertices = (*m_triangles)[it];
        auto triangleNormal = Vector3::normal((*m_vertices)[triangleVertices[0]],
            (*m_vertices)[triangleVertices[1]],
            (*m_vertices)[triangleVertices[2]]);
        triangleNormals.insert({ pointIndex, triangleNormal });
    }

    auto calculateFaceNormal = [&](const std::vector<size_t>& corners) {
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

    auto calculateSide = [&](const std::vector<size_t>& corners) {
        auto ringNormal = calculateFaceNormal(corners);
        Vector3 originalNormal;
        for (const auto& it : corners)
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
    auto& halfEdges = m_halfEdges;
    auto isConerUsed = [&](size_t previous, size_t current, size_t next) {
        if (corners.end() != corners.find(std::make_tuple(previous, current, next)))
            return true;
        if (corners.end() != corners.find(std::make_tuple(next, current, previous)))
            return true;
        return false;
    };
    auto isFaceCornerExist = [&](const std::vector<size_t>& vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            size_t k = (i + 2) % vertices.size();
            if (isConerUsed(vertices[i], vertices[j], vertices[k]))
                return true;
        }
        return false;
    };
    auto addFaceCorners = [&](const std::vector<size_t>& vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            size_t k = (i + 2) % vertices.size();
            corners.insert(std::make_tuple(vertices[i], vertices[j], vertices[k]));
            corners.insert(std::make_tuple(vertices[k], vertices[j], vertices[i]));
        }
    };
    auto isFaceHalfEdgeExist = [&](const std::vector<size_t>& vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            if (halfEdges.end() != halfEdges.find({ vertices[i], vertices[j] }))
                return true;
        }
        return false;
    };
    auto addFaceHalfEdges = [&](const std::vector<size_t>& vertices) {
        for (size_t i = 0; i < vertices.size(); ++i) {
            size_t j = (i + 1) % vertices.size();
            halfEdges.insert({ vertices[i], vertices[j] });
        }
    };

    size_t triangleRound = 4;
    for (size_t round = 0; round < 5; ++round) {
        for (const auto& level0It : edgeConnectMap) {
            const auto& level0 = level0It.first;
            auto findLevel1 = edgeConnectMap.find(level0);
            if (findLevel1 == edgeConnectMap.end())
                continue;
            const auto& triangleVertices = (*m_triangles)[pointSourceTriangles[level0]];
            auto triangleNormal = Vector3::normal((*m_vertices)[triangleVertices[0]],
                (*m_vertices)[triangleVertices[1]],
                (*m_vertices)[triangleVertices[2]]);
            for (const auto& level1 : findLevel1->second) {
                auto findLevel2 = edgeConnectMap.find(level1);
                if (findLevel2 == edgeConnectMap.end())
                    continue;
                if (halfEdges.find({ level0, level1 }) != halfEdges.end() && halfEdges.find({ level1, level0 }) != halfEdges.end())
                    continue;
                for (const auto& level2 : findLevel2->second) {
                    if (level0 == level2)
                        continue;
                    auto findLevel3 = edgeConnectMap.find(level2);
                    if (findLevel3 == edgeConnectMap.end())
                        continue;
                    if (halfEdges.find({ level1, level2 }) != halfEdges.end() && halfEdges.find({ level2, level1 }) != halfEdges.end())
                        continue;
                    for (const auto& level3 : findLevel3->second) {
                        if (level0 == level3) {
                            if (triangleRound == round) {
                                if (!isFaceCornerExist({ level0, level1, level2 })) {
                                    auto side = calculateSide({ level0, level1, level2 });
                                    if (side > 0) {
                                        if (!isFaceHalfEdgeExist({ level0, level1, level2 })) {
                                            quads->push_back({ level0, level1, level2 });
                                            addFaceCorners({ level0, level1, level2 });
                                            addFaceHalfEdges({ level0, level1, level2 });
                                        }
                                    } else if (side < 0) {
                                        if (!isFaceHalfEdgeExist({ level2, level1, level0 })) {
                                            quads->push_back({ level2, level1, level0 });
                                            addFaceCorners({ level2, level1, level0 });
                                            addFaceHalfEdges({ level2, level1, level0 });
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
                        if (halfEdges.find({ level2, level3 }) != halfEdges.end() && halfEdges.find({ level3, level2 }) != halfEdges.end())
                            continue;
                        for (const auto& level4 : findLevel4->second) {
                            if (level0 != level4) {
                                if (level2 == level4 || level1 == level4)
                                    continue;
                                if (round < 1)
                                    continue;
                                auto findLevel5 = edgeConnectMap.find(level4);
                                if (findLevel5 == edgeConnectMap.end())
                                    continue;
                                if (halfEdges.find({ level3, level4 }) != halfEdges.end() && halfEdges.find({ level4, level3 }) != halfEdges.end())
                                    continue;
                                for (const auto& level5 : findLevel5->second) {
                                    if (level0 != level5) {
                                        if (level3 == level5 || level2 == level5 || level1 == level5)
                                            continue;
                                        if (round < 2)
                                            continue;
                                        auto findLevel6 = edgeConnectMap.find(level5);
                                        if (findLevel6 == edgeConnectMap.end())
                                            continue;
                                        if (halfEdges.find({ level4, level5 }) != halfEdges.end() && halfEdges.find({ level5, level4 }) != halfEdges.end())
                                            continue;
                                        for (const auto& level6 : findLevel6->second) {
                                            if (level0 != level6) {
                                                if (level4 == level6 || level3 == level6 || level2 == level6 || level1 == level6)
                                                    continue;
                                                if (round < 3)
                                                    continue;
                                                auto findLevel7 = edgeConnectMap.find(level6);
                                                if (findLevel7 == edgeConnectMap.end())
                                                    continue;
                                                if (halfEdges.find({ level5, level6 }) != halfEdges.end() && halfEdges.find({ level6, level5 }) != halfEdges.end())
                                                    continue;
                                                for (const auto& level7 : findLevel7->second) {
                                                    if (level0 != level7)
                                                        continue;
                                                    if (3 != round)
                                                        break;
                                                    if (!isFaceCornerExist({ level0, level1, level2, level3, level4, level5, level6 })) {
                                                        auto side = calculateSide({ level0, level1, level2, level3, level4, level5, level6 });
                                                        if (side > 0) {
                                                            if (!isFaceHalfEdgeExist({ level0, level1, level2, level3, level4, level5, level6 })) {
                                                                quads->push_back({ level0, level1, level2, level3, level4, level5, level6 });
                                                                addFaceCorners({ level0, level1, level2, level3, level4, level5, level6 });
                                                                addFaceHalfEdges({ level0, level1, level2, level3, level4, level5, level6 });
                                                            }
                                                        } else if (side < 0) {
                                                            if (!isFaceHalfEdgeExist({ level6, level5, level4, level3, level2, level1, level0 })) {
                                                                quads->push_back({ level6, level5, level4, level3, level2, level1, level0 });
                                                                addFaceCorners({ level6, level5, level4, level3, level2, level1, level0 });
                                                                addFaceHalfEdges({ level6, level5, level4, level3, level2, level1, level0 });
                                                            }
                                                        }
                                                    }
                                                    break;
                                                }
                                                continue;
                                            }
                                            if (2 != round)
                                                break;
                                            if (!isFaceCornerExist({ level0, level1, level2, level3, level4, level5 })) {
                                                auto side = calculateSide({ level0, level1, level2, level3, level4, level5 });
                                                if (side > 0) {
                                                    if (!isFaceHalfEdgeExist({ level0, level1, level2, level3, level4, level5 })) {
                                                        quads->push_back({ level0, level1, level2, level3, level4, level5 });
                                                        addFaceCorners({ level0, level1, level2, level3, level4, level5 });
                                                        addFaceHalfEdges({ level0, level1, level2, level3, level4, level5 });
                                                    }
                                                } else if (side < 0) {
                                                    if (!isFaceHalfEdgeExist({ level5, level4, level3, level2, level1, level0 })) {
                                                        quads->push_back({ level5, level4, level3, level2, level1, level0 });
                                                        addFaceCorners({ level5, level4, level3, level2, level1, level0 });
                                                        addFaceHalfEdges({ level5, level4, level3, level2, level1, level0 });
                                                    }
                                                }
                                            }
                                            break;
                                        }
                                        continue;
                                    }
                                    if (1 != round)
                                        break;
                                    if (!isFaceCornerExist({ level0, level1, level2, level3, level4 })) {
                                        auto side = calculateSide({ level0, level1, level2, level3, level4 });
                                        if (side > 0) {
                                            if (!isFaceHalfEdgeExist({ level0, level1, level2, level3, level4 })) {
                                                quads->push_back({ level0, level1, level2, level3, level4 });
                                                addFaceCorners({ level0, level1, level2, level3, level4 });
                                                addFaceHalfEdges({ level0, level1, level2, level3, level4 });
                                            }
                                        } else if (side < 0) {
                                            if (!isFaceHalfEdgeExist({ level4, level3, level2, level1, level0 })) {
                                                quads->push_back({ level4, level3, level2, level1, level0 });
                                                addFaceCorners({ level4, level3, level2, level1, level0 });
                                                addFaceHalfEdges({ level4, level3, level2, level1, level0 });
                                            }
                                        }
                                    }
                                    break;
                                }
                                continue;
                            }
                            if (0 != round)
                                break;
                            if (!isFaceCornerExist({ level0, level1, level2, level3 })) {
                                auto side = calculateSide({ level0, level1, level2, level3 });
                                if (side > 0) {
                                    if (!isFaceHalfEdgeExist({ level0, level1, level2, level3 })) {
                                        quads->push_back({ level0, level1, level2, level3 });
                                        addFaceCorners({ level0, level1, level2, level3 });
                                        addFaceHalfEdges({ level0, level1, level2, level3 });
                                    }
                                } else if (side < 0) {
                                    if (!isFaceHalfEdgeExist({ level3, level2, level1, level0 })) {
                                        quads->push_back({ level3, level2, level1, level0 });
                                        addFaceCorners({ level3, level2, level1, level0 });
                                        addFaceHalfEdges({ level3, level2, level1, level0 });
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

void QuadExtractor::extractConnections(std::vector<Vector3>* crossPoints,
    std::vector<size_t>* sourceTriangles,
    std::set<std::pair<size_t, size_t>>* connections)
{
    std::map<PositionKey, size_t> crossPointMap;

    m_connectionInfos.clear();
    m_addedConnections.clear();

    auto addCrossPoint = [&](const Vector3& position3, size_t triangleIndex) {
        auto insertResult = crossPointMap.insert({ position3, crossPoints->size() });
        if (insertResult.second) {
            crossPoints->push_back(position3);
            sourceTriangles->push_back(triangleIndex);
        }
        return insertResult.first->second;
    };
    auto addConnection = [&](size_t fromPointIndex, size_t toPointIndex,
                             size_t triangleIndex, int coordIndex, int integer) {
        if (fromPointIndex == toPointIndex)
            return;
        connections->insert({ fromPointIndex, toPointIndex });
        m_connectionInfos.insert({ { std::min(fromPointIndex, toPointIndex),
                                       std::max(fromPointIndex, toPointIndex) },
            ConnectionInfo { triangleIndex, coordIndex, integer } });
    };

    struct CrossPoint {
        Vector3 position3;
        Vector2 position2;
        int integer;
    };

    for (size_t triangleIndex = 0; triangleIndex < m_triangles->size(); ++triangleIndex) {
        //std::cerr << "Extract connections for triangle:" << triangleIndex << "..." << std::endl;

        const auto& cornerUvs = (*m_triangleUvs)[triangleIndex];
        const auto& cornerIndices = (*m_triangles)[triangleIndex];

        // Extract intersections of isolines with edges
        std::map<int, std::vector<std::vector<CrossPoint>>> lines[2];
        bool edgeCollapsed[2][3] = { { false, false, false },
            { false, false, false } };
        for (size_t i = 0; i < 2; ++i) {
            for (size_t j = 0; j < 3; ++j) {
                size_t k = (j + 1) % 3;
                const auto& current = cornerUvs[j];
                const auto& next = cornerUvs[k];
                if ((Double::isZero((double)(int)current[i] - current[i]) && Double::isZero(current[i] - next[i]))) {
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
                    lines[i][integer].push_back({ fromPoint, toPoint });
                }
            }
            std::map<int, std::vector<CrossPoint>> points;
            for (size_t j = 0; j < 3; ++j) {
                size_t k = (j + 1) % 3;
                const auto& current = cornerUvs[j];
                const auto& next = cornerUvs[k];
                double distance = std::abs(current[i] - next[i]);
                if ((int)current[i] != (int)next[i] || (current[i] > 0) != (next[i] > 0)) {
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
            for (const auto& it : points) {
                for (size_t pointIndex = 0; pointIndex < it.second.size(); ++pointIndex) {
                    size_t nextPointIndex = (pointIndex + 1) % it.second.size();
                    const auto& point = it.second[pointIndex];
                    const auto& nextPoint = it.second[nextPointIndex];
                    lines[i][it.first].push_back({ point, nextPoint });
                }
            }
        }

        //std::cerr << "Segment lines by isolines for triangle:" << triangleIndex << "..." << std::endl;

        // Segment lines by isolines
        for (size_t i = 0; i < 2; ++i) {
            size_t j = (i + 1) % 2;
            for (const auto& targetIt : lines[i]) {
                for (const auto& target : targetIt.second) {
                    std::vector<std::vector<CrossPoint>> segments = { target };
                    for (const auto& splitIt : lines[j]) {
                        const auto& split = splitIt.second.begin();
                        const auto& coordIndex = j;
                        double segmentPosition = split[0][0].position2[coordIndex];
                        for (int segmentIndex = (int)segments.size() - 1; segmentIndex >= 0; --segmentIndex) {
                            auto& segment = segments[segmentIndex];
                            double fromPosition;
                            double toPosition;
                            size_t fromIndex;
                            size_t toIndex;
                            const auto& uv0 = segment[0].position2;
                            const auto& uv1 = segment[1].position2;
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
                            if (segmentPosition < fromPosition || segmentPosition > toPosition)
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
                            segments.push_back({ newFromPoint, newToPoint });
                        }
                    }
                    for (const auto& segment : segments) {
                        addConnection(addCrossPoint(segment[0].position3, triangleIndex),
                            addCrossPoint(segment[1].position3, triangleIndex),
                            triangleIndex, (int)i, targetIt.first);
                    }
                }
            }
        }
    }
}

void QuadExtractor::holdSingularLines(std::vector<Vector3>* crossPoints,
    std::vector<size_t>* sourceTriangles,
    std::set<std::pair<size_t, size_t>>* connections)
{
    if (nullptr == m_singularVertices || m_singularVertices->empty())
        return;

    const size_t ringCount = 10;
    const size_t maxWalkSteps = 32;
    const double aheadCosineThreshold = std::cos(M_PI * 30.0 / 180.0);
    const double parallelCosineThreshold = 0.9;

    auto makeEdge = [](size_t first, size_t second) {
        return std::make_pair(std::min(first, second), std::max(first, second));
    };

    std::map<PositionKey, size_t> crossPointMap;
    for (size_t i = 0; i < crossPoints->size(); ++i)
        crossPointMap.insert({ (*crossPoints)[i], i });

    std::unordered_map<size_t, std::unordered_set<size_t>> branchesOfPoint;
    for (const auto& connection : *connections) {
        branchesOfPoint[connection.first].insert(connection.second);
        branchesOfPoint[connection.second].insert(connection.first);
    }

    std::unordered_map<size_t, std::vector<size_t>> trianglesAroundVertex;
    for (size_t triangleIndex = 0; triangleIndex < m_triangles->size(); ++triangleIndex) {
        for (const auto& vertexIndex : (*m_triangles)[triangleIndex])
            trianglesAroundVertex[vertexIndex].push_back(triangleIndex);
    }

    struct Crossing {
        Vector3 position;
        std::pair<size_t, size_t> edge;
        size_t triangleIndex;
    };

    size_t addedConnections = 0;

    auto splitAt = [&](const Crossing& crossing) {
        auto findInfo = m_connectionInfos.find(crossing.edge);
        if (findInfo == m_connectionInfos.end())
            return std::numeric_limits<size_t>::max();
        const auto info = findInfo->second;
        auto insertResult = crossPointMap.insert({ PositionKey(crossing.position), crossPoints->size() });
        if (!insertResult.second)
            return insertResult.first->second;
        const size_t newPointIndex = crossPoints->size();
        crossPoints->push_back(crossing.position);
        sourceTriangles->push_back(info.triangleIndex);
        connections->erase({ crossing.edge.first, crossing.edge.second });
        connections->erase({ crossing.edge.second, crossing.edge.first });
        m_connectionInfos.erase(crossing.edge);
        branchesOfPoint[crossing.edge.first].erase(crossing.edge.second);
        branchesOfPoint[crossing.edge.second].erase(crossing.edge.first);
        for (const auto& endpoint : { crossing.edge.first, crossing.edge.second }) {
            connections->insert({ endpoint, newPointIndex });
            m_connectionInfos.insert({ makeEdge(endpoint, newPointIndex), info });
            branchesOfPoint[endpoint].insert(newPointIndex);
            branchesOfPoint[newPointIndex].insert(endpoint);
        }
        return newPointIndex;
    };

    auto addWalkConnection = [&](size_t fromPointIndex, size_t toPointIndex) {
        if (fromPointIndex == toPointIndex)
            return;
        auto edge = makeEdge(fromPointIndex, toPointIndex);
        if (m_connectionInfos.end() != m_connectionInfos.find(edge))
            return;
        connections->insert({ fromPointIndex, toPointIndex });
        ConnectionInfo info;
        info.triangleIndex = (*sourceTriangles)[toPointIndex];
        info.coordIndex = -1;
        info.integer = 0;
        m_connectionInfos.insert({ edge, info });
        m_addedConnections.insert(edge);
        branchesOfPoint[fromPointIndex].insert(toPointIndex);
        branchesOfPoint[toPointIndex].insert(fromPointIndex);
        ++addedConnections;
    };

    size_t starvedCones = 0;
    size_t walkedCones = 0;
    for (const auto& singularVertexIndex : *m_singularVertices) {
        if (singularVertexIndex >= m_vertices->size())
            continue;
        const auto& singularPosition = (*m_vertices)[singularVertexIndex];

        size_t singularPointIndex = std::numeric_limits<size_t>::max();
        {
            auto findCrossPoint = crossPointMap.find(PositionKey(singularPosition));
            if (findCrossPoint == crossPointMap.end())
                continue;
            singularPointIndex = findCrossPoint->second;
        }
        auto findBranches = branchesOfPoint.find(singularPointIndex);
        if (findBranches == branchesOfPoint.end() || 1 != findBranches->second.size())
            continue;
        const size_t comingFromPointIndex = *findBranches->second.begin();
        ++starvedCones;

        std::unordered_set<size_t> neighborTriangles;
        {
            std::unordered_set<size_t> ringVertices = { singularVertexIndex };
            for (size_t ring = 0; ring < ringCount; ++ring) {
                std::unordered_set<size_t> nextRingVertices;
                for (const auto& vertexIndex : ringVertices) {
                    auto findTriangles = trianglesAroundVertex.find(vertexIndex);
                    if (findTriangles == trianglesAroundVertex.end())
                        continue;
                    for (const auto& triangleIndex : findTriangles->second) {
                        neighborTriangles.insert(triangleIndex);
                        for (const auto& corner : (*m_triangles)[triangleIndex])
                            nextRingVertices.insert(corner);
                    }
                }
                ringVertices = std::move(nextRingVertices);
            }
        }
        if (neighborTriangles.empty())
            continue;

        std::map<std::pair<size_t, size_t>, ConnectionInfo> localEdges;
        double totalEdgeLength = 0.0;
        for (const auto& connection : *connections) {
            auto edge = makeEdge(connection.first, connection.second);
            auto findInfo = m_connectionInfos.find(edge);
            if (findInfo == m_connectionInfos.end())
                continue;
            if (neighborTriangles.end() == neighborTriangles.find(findInfo->second.triangleIndex))
                continue;
            if (!localEdges.insert({ edge, findInfo->second }).second)
                continue;
            totalEdgeLength += ((*crossPoints)[edge.first] - (*crossPoints)[edge.second]).length();
        }
        if (localEdges.size() < 3)
            continue;
        const double averageEdgeLength = totalEdgeLength / localEdges.size();
        if (Double::isZero(averageEdgeLength))
            continue;
        const double tolerance = 0.5 * averageEdgeLength;
        const double nearbyRadius = 12.0 * averageEdgeLength;

        Vector3 coneNormal;
        {
            auto findTriangles = trianglesAroundVertex.find(singularVertexIndex);
            if (findTriangles == trianglesAroundVertex.end())
                continue;
            for (const auto& triangleIndex : findTriangles->second) {
                const auto& triangle = (*m_triangles)[triangleIndex];
                coneNormal += Vector3::normal((*m_vertices)[triangle[0]],
                    (*m_vertices)[triangle[1]], (*m_vertices)[triangle[2]]);
            }
            coneNormal = coneNormal.normalized();
            if (coneNormal.isZero())
                continue;
        }
        Vector3 seedTail = (*crossPoints)[comingFromPointIndex];
        {
            int coordIndex = -1;
            int integer = 0;
            auto findInfo = m_connectionInfos.find(makeEdge(singularPointIndex, comingFromPointIndex));
            if (findInfo != m_connectionInfos.end()) {
                coordIndex = findInfo->second.coordIndex;
                integer = findInfo->second.integer;
            }
            size_t previous = singularPointIndex;
            size_t current = comingFromPointIndex;
            for (size_t back = 0; back < maxWalkSteps; ++back) {
                auto findBranches = branchesOfPoint.find(current);
                if (findBranches == branchesOfPoint.end())
                    break;
                if (findBranches->second.size() > 2)
                    break;
                size_t next = std::numeric_limits<size_t>::max();
                for (const auto& neighbor : findBranches->second) {
                    if (neighbor == previous)
                        continue;
                    auto findNextInfo = m_connectionInfos.find(makeEdge(current, neighbor));
                    if (findNextInfo == m_connectionInfos.end())
                        continue;
                    if (coordIndex >= 0
                        && (findNextInfo->second.coordIndex != coordIndex
                            || findNextInfo->second.integer != integer))
                        continue;
                    next = neighbor;
                    break;
                }
                if (std::numeric_limits<size_t>::max() == next)
                    break;
                previous = current;
                current = next;
            }
            seedTail = (*crossPoints)[current];
        }
        Vector3 seedDirection = singularPosition - seedTail;
        seedDirection = (seedDirection - coneNormal * Vector3::dotProduct(seedDirection, coneNormal)).normalized();
        if (seedDirection.isZero())
            continue;

        const std::unordered_set<size_t> behindPoints = { singularPointIndex, comingFromPointIndex };

        auto findNodeAhead = [&](const Vector3& position, const Vector3& direction) {
            size_t nearest = std::numeric_limits<size_t>::max();
            double nearestDistance = nearbyRadius;
            for (const auto& it : localEdges) {
                for (const auto& endpoint : { it.first.first, it.first.second }) {
                    if (behindPoints.end() != behindPoints.find(endpoint))
                        continue;
                    auto findBranches = branchesOfPoint.find(endpoint);
                    if (findBranches == branchesOfPoint.end() || findBranches->second.size() <= 2)
                        continue;
                    Vector3 offset = (*crossPoints)[endpoint] - position;
                    double distance = offset.length();
                    if (Double::isZero(distance) || distance >= nearestDistance)
                        continue;
                    if (Vector3::dotProduct(offset / distance, direction) < aheadCosineThreshold)
                        continue;
                    nearest = endpoint;
                    nearestDistance = distance;
                }
            }
            return nearest;
        };

        auto findCrossing = [&](const Vector3& position, const Vector3& direction,
                                double limitDistance, size_t alsoBlockedPoint,
                                Crossing* crossing) {
            bool found = false;
            double nearestDistance = limitDistance;
            for (const auto& it : localEdges) {
                bool blocked = false;
                for (const auto& endpoint : { it.first.first, it.first.second }) {
                    if (endpoint == alsoBlockedPoint
                        || behindPoints.end() != behindPoints.find(endpoint)) {
                        blocked = true;
                        break;
                    }
                }
                if (blocked)
                    continue;
                const auto& from = (*crossPoints)[it.first.first];
                const auto& to = (*crossPoints)[it.first.second];
                Vector3 edgeVector = to - from;
                if (std::abs(Vector3::dotProduct(direction, edgeVector.normalized())) > parallelCosineThreshold)
                    continue;
                Vector3 offset = from - position;
                double a = Vector3::dotProduct(edgeVector, edgeVector);
                double b = Vector3::dotProduct(direction, edgeVector);
                double c = Vector3::dotProduct(direction, offset);
                double d = Vector3::dotProduct(edgeVector, offset);
                double denominator = a - b * b;
                double edgeRatio = Double::isZero(denominator) ? 0.0 : (b * c - d) / denominator;
                if (edgeRatio < 0.0)
                    edgeRatio = 0.0;
                else if (edgeRatio > 1.0)
                    edgeRatio = 1.0;
                Vector3 pointOnEdge = from + edgeVector * edgeRatio;
                double distance = Vector3::dotProduct(pointOnEdge - position, direction);
                if (distance <= tolerance || distance >= nearestDistance)
                    continue;
                Vector3 pointOnRay = position + direction * distance;
                Vector3 miss = pointOnEdge - pointOnRay;
                const auto& missTriangle = (*m_triangles)[it.second.triangleIndex];
                Vector3 missNormal = Vector3::normal((*m_vertices)[missTriangle[0]],
                    (*m_vertices)[missTriangle[1]], (*m_vertices)[missTriangle[2]]);
                if ((miss - missNormal * Vector3::dotProduct(miss, missNormal)).length() > tolerance)
                    continue;
                *crossing = Crossing { pointOnEdge, it.first, it.second.triangleIndex };
                nearestDistance = distance;
                found = true;
            }
            return found;
        };

        std::vector<Crossing> path;
        size_t nodePointIndex = std::numeric_limits<size_t>::max();
        {
            Vector3 walkPosition = singularPosition;
            Vector3 walkDirection = seedDirection;
            for (size_t step = 0; step < maxWalkSteps; ++step) {
                double limitDistance = std::numeric_limits<double>::max();
                if (std::numeric_limits<size_t>::max() == nodePointIndex)
                    nodePointIndex = findNodeAhead(walkPosition, walkDirection);
                if (std::numeric_limits<size_t>::max() != nodePointIndex) {
                    Vector3 offset = (*crossPoints)[nodePointIndex] - walkPosition;
                    limitDistance = offset.length();
                    walkDirection = offset.normalized();
                }
                Crossing crossing {};
                if (!findCrossing(walkPosition, walkDirection, limitDistance, nodePointIndex,
                        &crossing)) {
                    break;
                }
                path.push_back(crossing);
                walkPosition = crossing.position;
                const auto& triangle = (*m_triangles)[crossing.triangleIndex];
                Vector3 triangleNormal = Vector3::normal((*m_vertices)[triangle[0]],
                    (*m_vertices)[triangle[1]], (*m_vertices)[triangle[2]]);
                Vector3 flattened = walkDirection - triangleNormal * Vector3::dotProduct(walkDirection, triangleNormal);
                if (!flattened.isZero())
                    walkDirection = flattened.normalized();
            }
        }
        if (std::numeric_limits<size_t>::max() == nodePointIndex)
            continue;

        size_t previousPointIndex = singularPointIndex;
        for (const auto& crossing : path) {
            size_t pointIndex = splitAt(crossing);
            if (std::numeric_limits<size_t>::max() == pointIndex)
                continue;
            addWalkConnection(previousPointIndex, pointIndex);
            previousPointIndex = pointIndex;
        }
        addWalkConnection(previousPointIndex, nodePointIndex);
        ++walkedCones;
    }

    std::cerr << "Hold singular lines walked " << walkedCones << " of " << starvedCones
              << " starved cone(s), added " << addedConnections << " connection(s)" << std::endl;
}

bool QuadExtractor::testPointInTriangle(const std::vector<Vector3>& points,
    const std::vector<size_t>& triangle,
    const std::vector<size_t>& testPoints)
{
    Vector3 triangleNormal = Vector3::normal(points[triangle[0]],
        points[triangle[1]], points[triangle[2]]);
    std::vector<Vector3> pointsIn3d;
    for (const auto& it : triangle)
        pointsIn3d.push_back(points[it]);
    for (const auto& it : testPoints)
        pointsIn3d.push_back(points[it]);
    std::vector<Vector2> pointsIn2d;
    Vector3 origin = (points[triangle[0]] + points[triangle[1]] + points[triangle[2]]) / 3;
    Vector3 axis = (points[triangle[0]] - origin).normalized();
    Vector3::project(pointsIn3d, &pointsIn2d, triangleNormal, axis, origin);
    const Vector2& a = pointsIn2d[0];
    const Vector2& b = pointsIn2d[1];
    const Vector2& c = pointsIn2d[2];
    for (size_t i = 3; i < pointsIn2d.size(); ++i) {
        if (Vector2::isInTriangle(a, b, c, pointsIn2d[i]))
            return true;
    }
    return false;
}

void QuadExtractor::rebuildHalfEdges()
{
    m_halfEdges.clear();
    for (const auto& it : m_remeshedPolygons) {
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            m_halfEdges.insert({ it[i], it[j] });
        }
    }
}

void QuadExtractor::fixHoles()
{
    std::vector<std::vector<size_t>> loops;
    searchBoundaries(m_halfEdges, &loops);
    for (auto& loop : loops) {
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

void QuadExtractor::fixHoleWithQuads(std::vector<size_t>& hole, bool checkScore)
{
    auto recordHalfEdgesOfLastPolygon = [&]() {
        const auto& it = m_remeshedPolygons[m_remeshedPolygons.size() - 1];
        for (size_t i = 0; i < it.size(); ++i) {
            size_t j = (i + 1) % it.size();
            m_halfEdges.insert({ it[i], it[j] });
        }
    };

    for (;;) {
        if (hole.size() <= 2) {
            std::cerr << "fixHoleWithQuads cancel on edge length:" << hole.size() << std::endl;
            return;
        }

        if (3 == hole.size()) {
            m_remeshedPolygons.push_back({ (size_t)hole[2], (size_t)hole[1], (size_t)hole[0] });
            recordHalfEdgesOfLastPolygon();
            return;
        }

        if (4 == hole.size()) {
            m_remeshedPolygons.push_back({ (size_t)hole[3], (size_t)hole[2], (size_t)hole[1], (size_t)hole[0] });
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
            edgeScores.push_back({ i, Vector3::dotProduct(left, right) });
        }
        std::sort(edgeScores.begin(), edgeScores.end(), [](const std::pair<int, double>& first, const std::pair<int, double>& second) {
            return first.second < second.second;
        });
        bool holeChanged = false;
        for (int edgeIndex = edgeScores.size() - 1; edgeIndex >= 0; --edgeIndex) {
            const auto& score = edgeScores[edgeIndex];
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
            std::vector<size_t> candidate = { (size_t)hole[k], (size_t)hole[j], (size_t)hole[i], (size_t)hole[h] };
            if (m_halfEdges.end() != m_halfEdges.find({ candidate[0], candidate[1] }) || m_halfEdges.end() != m_halfEdges.find({ candidate[1], candidate[2] }) || m_halfEdges.end() != m_halfEdges.find({ candidate[2], candidate[3] }) || m_halfEdges.end() != m_halfEdges.find({ candidate[3], candidate[0] })) {
                std::cerr << "fixHoleWithQuads ignore score:" << score.second << " because conflicts with existed quads" << std::endl;
                continue;
            }
            std::vector<size_t> remainPoints;
            for (int w = 0; w < hole.size(); ++w) {
                if (w == i || w == j || w == h || w == k)
                    continue;
                remainPoints.push_back(hole[w]);
            }
            if (testPointInTriangle(m_remeshedVertices, { candidate[0], candidate[1], candidate[2] }, remainPoints) || testPointInTriangle(m_remeshedVertices, { candidate[2], candidate[3], candidate[0] }, remainPoints)) {
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

void QuadExtractor::searchBoundaries(const std::set<std::pair<size_t, size_t>>& halfEdges,
    std::vector<std::vector<size_t>>* loops)
{
    std::cerr << "Searching boundaries..." << std::endl;

    std::unordered_map<size_t, std::unordered_set<size_t>> nextMap;
    for (const auto& it : halfEdges) {
        if (halfEdges.end() != halfEdges.find({ it.second, it.first }))
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
        for (const auto& v : loop)
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
    m_remeshedPolygons = *std::max_element(quadsIslands.begin(), quadsIslands.end(), [&](const std::vector<std::vector<size_t>>& first, const std::vector<std::vector<size_t>>& second) {
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
    for (const auto& it : edgeToFaceMap) {
        if (edgeToFaceMap.end() != edgeToFaceMap.find({ it.first.second, it.first.first }))
            continue;
        vertexOpenBoundaryCountMap[it.first.first]++;
        vertexOpenBoundaryCountMap[it.first.second]++;
    }
    std::vector<std::vector<size_t>> manifoldFaces;
    for (const auto& it : m_remeshedPolygons) {
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

namespace {

    Vector3 closestPointOnTriangle(const Vector3& p,
        const Vector3& a, const Vector3& b, const Vector3& c)
    {
        const auto ab = b - a;
        const auto ac = c - a;
        const auto ap = p - a;
        double d1 = Vector3::dotProduct(ab, ap);
        double d2 = Vector3::dotProduct(ac, ap);
        if (d1 <= 0.0 && d2 <= 0.0)
            return a;

        const auto bp = p - b;
        double d3 = Vector3::dotProduct(ab, bp);
        double d4 = Vector3::dotProduct(ac, bp);
        if (d3 >= 0.0 && d4 <= d3)
            return b;

        double vc = d1 * d4 - d3 * d2;
        if (vc <= 0.0 && d1 >= 0.0 && d3 <= 0.0) {
            double denom = d1 - d3;
            double v = 0.0 != denom ? d1 / denom : 0.0;
            return a + v * ab;
        }

        const auto cp = p - c;
        double d5 = Vector3::dotProduct(ab, cp);
        double d6 = Vector3::dotProduct(ac, cp);
        if (d6 >= 0.0 && d5 <= d6)
            return c;

        double vb = d5 * d2 - d1 * d6;
        if (vb <= 0.0 && d2 >= 0.0 && d6 <= 0.0) {
            double denom = d2 - d6;
            double w = 0.0 != denom ? d2 / denom : 0.0;
            return a + w * ac;
        }

        double va = d3 * d6 - d5 * d4;
        if (va <= 0.0 && (d4 - d3) >= 0.0 && (d5 - d6) >= 0.0) {
            double denom = (d4 - d3) + (d5 - d6);
            double w = 0.0 != denom ? (d4 - d3) / denom : 0.0;
            return b + w * (c - b);
        }

        double denom = va + vb + vc;
        if (0.0 == denom)
            return a;
        double v = vb / denom;
        double w = vc / denom;
        return a + ab * v + ac * w;
    }

}

void QuadExtractor::smoothAndProject(size_t iterations,
    const std::unordered_set<size_t>* movableVertices)
{
    if (0 == iterations
        || m_remeshedVertices.empty()
        || m_remeshedPolygons.empty()
        || nullptr == m_vertices
        || nullptr == m_triangles
        || m_triangles->empty())
        return;

    std::vector<std::unordered_set<size_t>> neighbors(m_remeshedVertices.size());
    std::map<std::pair<size_t, size_t>, size_t> edgeUseCount;
    for (const auto& face : m_remeshedPolygons) {
        for (size_t i = 0; i < face.size(); ++i) {
            size_t j = (i + 1) % face.size();
            if (face[i] >= neighbors.size() || face[j] >= neighbors.size())
                continue;
            neighbors[face[i]].insert(face[j]);
            neighbors[face[j]].insert(face[i]);
            ++edgeUseCount[std::make_pair(std::min(face[i], face[j]),
                std::max(face[i], face[j]))];
        }
    }
    std::vector<bool> locked(m_remeshedVertices.size(), false);
    if (nullptr != movableVertices) {
        for (size_t i = 0; i < locked.size(); ++i)
            locked[i] = movableVertices->end() == movableVertices->find(i);
    }
    for (const auto& it : edgeUseCount) {
        if (1 == it.second) {
            locked[it.first.first] = true;
            locked[it.first.second] = true;
        }
    }

    std::vector<::Vector3> targetVertices;
    targetVertices.reserve(m_vertices->size());
    for (const auto& it : *m_vertices)
        targetVertices.push_back(::Vector3(it.x(), it.y(), it.z()));

    std::vector<AxisAlignedBoudingBox> triangleBoxes(m_triangles->size());
    std::vector<size_t> triangleIndices(m_triangles->size());
    AxisAlignedBoudingBox groupBox;
    for (size_t i = 0; i < m_triangles->size(); ++i) {
        const auto& triangle = (*m_triangles)[i];
        for (size_t k = 0; k < 3; ++k) {
            triangleBoxes[i].update(targetVertices[triangle[k]]);
            groupBox.update(targetVertices[triangle[k]]);
        }
        triangleBoxes[i].updateCenter();
        triangleIndices[i] = i;
    }
    groupBox.updateCenter();
    AxisAlignedBoudingBoxTree tree(&triangleBoxes, triangleIndices, groupBox);

    // Average quad edge length drives the initial search radius
    double totalEdgeLength = 0.0;
    size_t edgeNum = 0;
    for (const auto& it : edgeUseCount) {
        totalEdgeLength += (m_remeshedVertices[it.first.first] - m_remeshedVertices[it.first.second]).length();
        ++edgeNum;
    }
    if (0 == edgeNum)
        return;
    const double averageEdgeLength = totalEdgeLength / edgeNum;
    if (averageEdgeLength <= 0.0)
        return;

    auto projectToTargetMesh = [&](const Vector3& position, Vector3* projected) {
        for (double radius = averageEdgeLength; radius <= averageEdgeLength * 8.0; radius *= 2.0) {
            std::vector<AxisAlignedBoudingBox> queryBoxes(1);
            queryBoxes[0].update(::Vector3(position.x() - radius, position.y() - radius, position.z() - radius));
            queryBoxes[0].update(::Vector3(position.x() + radius, position.y() + radius, position.z() + radius));
            queryBoxes[0].updateCenter();
            AxisAlignedBoudingBoxTree queryTree(&queryBoxes, { 0 }, queryBoxes[0]);
            std::vector<std::pair<size_t, size_t>> pairs;
            tree.test(tree.root(), queryTree.root(), &queryBoxes, &pairs);
            double minDistance2 = std::numeric_limits<double>::max();
            for (const auto& it : pairs) {
                const auto& triangle = (*m_triangles)[it.first];
                const auto candidate = closestPointOnTriangle(position,
                    (*m_vertices)[triangle[0]],
                    (*m_vertices)[triangle[1]],
                    (*m_vertices)[triangle[2]]);
                double distance2 = (candidate - position).lengthSquared();
                if (distance2 < minDistance2) {
                    minDistance2 = distance2;
                    *projected = candidate;
                }
            }
            if (minDistance2 < std::numeric_limits<double>::max())
                return true;
        }
        return false;
    };

    // Both passes below already read one buffer and write another, and the
    // projection only reads the bounding box tree, so each vertex is independent
    // and the parallel result is the same as the serial one.
    const double smoothFactor = 0.5;
    for (size_t iteration = 0; iteration < iterations; ++iteration) {
        std::vector<Vector3> smoothedVertices(m_remeshedVertices);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, m_remeshedVertices.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (locked[i] || neighbors[i].empty())
                        continue;
                    Vector3 center;
                    for (const auto& neighbor : neighbors[i])
                        center += m_remeshedVertices[neighbor];
                    center /= (double)neighbors[i].size();
                    smoothedVertices[i] = m_remeshedVertices[i] + smoothFactor * (center - m_remeshedVertices[i]);
                }
            });
        tbb::parallel_for(tbb::blocked_range<size_t>(0, smoothedVertices.size()),
            [&](const tbb::blocked_range<size_t>& range) {
                for (size_t i = range.begin(); i != range.end(); ++i) {
                    if (locked[i] || neighbors[i].empty())
                        continue;
                    Vector3 projected;
                    if (projectToTargetMesh(smoothedVertices[i], &projected))
                        smoothedVertices[i] = projected;
                }
            });
        m_remeshedVertices = std::move(smoothedVertices);
    }
}

void QuadExtractor::splitSixEdgeFaces()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
    for (const auto& face : m_remeshedPolygons) {
        for (size_t i = 0; i < face.size(); ++i) {
            size_t j = (i + 1) % face.size();
            vertexNeighbors[face[i]].insert(face[j]);
            vertexNeighbors[face[j]].insert(face[i]);
        }
    }

    auto flowScoreAt = [&](size_t vertex, const Vector3& direction,
                           const std::unordered_set<size_t>& faceNeighbors) {
        double best = -1.0;
        const auto& findNeighbors = vertexNeighbors.find(vertex);
        if (vertexNeighbors.end() == findNeighbors)
            return 0.0;
        bool found = false;
        for (const auto& neighbor : findNeighbors->second) {
            if (faceNeighbors.end() != faceNeighbors.find(neighbor))
                continue;
            auto incoming = (m_remeshedVertices[vertex] - m_remeshedVertices[neighbor]).normalized();
            double score = Vector3::dotProduct(incoming, direction);
            if (score > best)
                best = score;
            found = true;
        }
        return found ? best : 0.0;
    };

    auto cornerScore = [&](const std::vector<size_t>& quad) {
        double total = 0.0;
        for (size_t i = 0; i < quad.size(); ++i) {
            size_t h = (i + quad.size() - 1) % quad.size();
            size_t j = (i + 1) % quad.size();
            auto left = (m_remeshedVertices[quad[h]] - m_remeshedVertices[quad[i]]).normalized();
            auto right = (m_remeshedVertices[quad[j]] - m_remeshedVertices[quad[i]]).normalized();
            total += std::abs(Vector3::dotProduct(left, right));
        }
        return -total / quad.size();
    };

    size_t splitNum = 0;
    std::vector<std::vector<size_t>> polygons;
    polygons.reserve(m_remeshedPolygons.size());
    for (const auto& face : m_remeshedPolygons) {
        if (6 != face.size()) {
            polygons.push_back(face);
            continue;
        }
        std::unordered_set<size_t> uniqueVertices(face.begin(), face.end());
        if (6 != uniqueVertices.size()) {
            polygons.push_back(face);
            continue;
        }

        int bestCorner = -1;
        double bestScore = 0.0;
        for (size_t i = 0; i < 3; ++i) {
            size_t a = face[i];
            size_t b = face[i + 3];
            const auto& findNeighbors = vertexNeighbors.find(a);
            if (vertexNeighbors.end() != findNeighbors
                && findNeighbors->second.end() != findNeighbors->second.find(b)) {
                continue;
            }
            auto direction = (m_remeshedVertices[b] - m_remeshedVertices[a]).normalized();
            std::unordered_set<size_t> aFaceNeighbors = {
                face[(i + 5) % 6], face[(i + 1) % 6], b
            };
            std::unordered_set<size_t> bFaceNeighbors = {
                face[(i + 2) % 6], face[(i + 4) % 6], a
            };
            double score = flowScoreAt(a, direction, aFaceNeighbors)
                + flowScoreAt(b, -direction, bFaceNeighbors);
            score += cornerScore({ face[i], face[(i + 1) % 6], face[(i + 2) % 6], face[i + 3] });
            score += cornerScore({ face[i + 3], face[(i + 4) % 6], face[(i + 5) % 6], face[i] });
            if (-1 == bestCorner || score > bestScore) {
                bestCorner = (int)i;
                bestScore = score;
            }
        }
        if (-1 == bestCorner) {
            std::cerr << "Six edge face kept, no diagonal available" << std::endl;
            polygons.push_back(face);
            continue;
        }

        size_t i = (size_t)bestCorner;
        polygons.push_back({ face[i], face[(i + 1) % 6], face[(i + 2) % 6], face[i + 3] });
        polygons.push_back({ face[i + 3], face[(i + 4) % 6], face[(i + 5) % 6], face[i] });
        ++splitNum;
        vertexNeighbors[face[i]].insert(face[i + 3]);
        vertexNeighbors[face[i + 3]].insert(face[i]);
    }

    if (0 == splitNum)
        return;

    std::cerr << "Split six edge faces:" << splitNum << std::endl;
    m_remeshedPolygons = std::move(polygons);
    rebuildHalfEdges();
}

void QuadExtractor::convertTriangleAndFiveEdgeFans()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    using Edge = std::pair<size_t, size_t>;
    const auto edgeOf = [](size_t a, size_t b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };
    const auto hasRepeatedVertex = [](const std::vector<size_t>& face) {
        std::unordered_set<size_t> vertices(face.begin(), face.end());
        return vertices.size() != face.size();
    };
    const size_t noVertex = std::numeric_limits<size_t>::max();
    Vector3 addedPosition;
    size_t addedVertex = noVertex;
    const auto positionOf = [&](size_t vertex) {
        return vertex == addedVertex ? addedPosition : m_remeshedVertices[vertex];
    };
    const auto faceNormal = [&](const std::vector<size_t>& face) {
        Vector3 normal;
        const auto origin = positionOf(face[0]);
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            normal += Vector3::crossProduct(positionOf(face[i]) - origin,
                positionOf(face[i + 1]) - origin);
        }
        return normal;
    };
    const size_t maxValence = 6;
    const size_t minFanValence = 4;
    const size_t maxFanValence = 6;

    size_t convertNum = 0;
    std::unordered_set<size_t> convertedVertices;
    for (;;) {
        std::map<Edge, std::vector<size_t>> edgeFaces;
        std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
        std::unordered_map<size_t, std::vector<size_t>> vertexFaces;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            for (size_t i = 0; i < face.size(); ++i) {
                const size_t j = (i + 1) % face.size();
                edgeFaces[edgeOf(face[i], face[j])].push_back(faceIndex);
                vertexNeighbors[face[i]].insert(face[j]);
                vertexNeighbors[face[j]].insert(face[i]);
            }
            for (const auto& vertex : face)
                vertexFaces[vertex].push_back(faceIndex);
        }

        std::unordered_set<size_t> borderVertices;
        for (const auto& it : edgeFaces) {
            if (2 == it.second.size())
                continue;
            borderVertices.insert(it.first.first);
            borderVertices.insert(it.first.second);
        }

        const auto fanAround = [&](size_t vertex, std::vector<size_t>* fan) {
            fan->clear();
            const auto& findFaces = vertexFaces.find(vertex);
            if (vertexFaces.end() == findFaces || findFaces->second.empty())
                return false;
            const size_t startFace = findFaces->second.front();
            size_t currentFace = startFace;
            for (;;) {
                fan->push_back(currentFace);
                if (fan->size() > findFaces->second.size())
                    return false;
                const auto& face = m_remeshedPolygons[currentFace];
                size_t at = face.size();
                for (size_t i = 0; i < face.size(); ++i) {
                    if (vertex == face[i]) {
                        at = i;
                        break;
                    }
                }
                if (at >= face.size())
                    return false;
                const auto& incident = edgeFaces[edgeOf(vertex, face[(at + 1) % face.size()])];
                if (2 != incident.size())
                    return false;
                currentFace = incident[0] == currentFace ? incident[1] : incident[0];
                if (currentFace == startFace)
                    break;
            }
            return fan->size() == findFaces->second.size();
        };

        std::unordered_set<size_t> touchedVertices;
        size_t roundConvertNum = 0;
        std::set<size_t> fanVertices;
        for (const auto& it : vertexNeighbors) {
            if (it.second.size() >= minFanValence && it.second.size() <= maxFanValence)
                fanVertices.insert(it.first);
        }
        for (const auto& vertex : fanVertices) {
            if (touchedVertices.end() != touchedVertices.find(vertex)
                || borderVertices.end() != borderVertices.find(vertex))
                continue;
            std::vector<size_t> fan;
            if (!fanAround(vertex, &fan) || fan.size() != vertexNeighbors[vertex].size())
                continue;

            size_t trianglePosition = fan.size();
            size_t pentagonPosition = fan.size();
            bool wellFormed = true;
            for (size_t i = 0; i < fan.size() && wellFormed; ++i) {
                const auto& face = m_remeshedPolygons[fan[i]];
                if (hasRepeatedVertex(face)) {
                    wellFormed = false;
                } else if (3 == face.size()) {
                    if (trianglePosition < fan.size())
                        wellFormed = false;
                    trianglePosition = i;
                } else if (5 == face.size()) {
                    if (pentagonPosition < fan.size())
                        wellFormed = false;
                    pentagonPosition = i;
                } else if (4 != face.size()) {
                    wellFormed = false;
                }
            }
            if (!wellFormed || trianglePosition >= fan.size() || pentagonPosition >= fan.size())
                continue;

            const size_t forwardGap = (pentagonPosition + fan.size() - trianglePosition) % fan.size();
            const size_t backwardGap = fan.size() - forwardGap;
            if (2 != std::min(forwardGap, backwardGap))
                continue;
            const size_t runStart = forwardGap <= backwardGap ? trianglePosition : pentagonPosition;
            std::vector<size_t> run;
            run.reserve(3);
            for (size_t i = 0; i < 3; ++i)
                run.push_back(fan[(runStart + i) % fan.size()]);

            std::set<std::pair<size_t, size_t>> directedEdges;
            for (const auto& faceIndex : run) {
                const auto& face = m_remeshedPolygons[faceIndex];
                for (size_t i = 0; i < face.size(); ++i)
                    directedEdges.insert({ face[i], face[(i + 1) % face.size()] });
            }
            std::unordered_map<size_t, size_t> boundaryNext;
            std::vector<Edge> buriedEdges;
            bool simpleBoundary = true;
            for (const auto& it : directedEdges) {
                if (directedEdges.end() != directedEdges.find({ it.second, it.first })) {
                    if (it.first < it.second)
                        buriedEdges.push_back(edgeOf(it.first, it.second));
                    continue;
                }
                if (!boundaryNext.insert({ it.first, it.second }).second) {
                    simpleBoundary = false;
                    break;
                }
            }
            if (!simpleBoundary || 8 != boundaryNext.size() || 2 != buriedEdges.size())
                continue;
            bool buriedAtFanVertex = true;
            for (const auto& edge : buriedEdges) {
                if (vertex != edge.first && vertex != edge.second) {
                    buriedAtFanVertex = false;
                    break;
                }
            }
            if (!buriedAtFanVertex)
                continue;

            std::vector<size_t> octagon;
            octagon.reserve(8);
            size_t walk = vertex;
            for (size_t i = 0; i < 8; ++i) {
                octagon.push_back(walk);
                const auto& findNext = boundaryNext.find(walk);
                if (boundaryNext.end() == findNext)
                    break;
                walk = findNext->second;
            }
            if (8 != octagon.size() || walk != vertex || hasRepeatedVertex(octagon))
                continue;
            bool touched = false;
            for (const auto& corner : octagon) {
                if (touchedVertices.end() != touchedVertices.find(corner)) {
                    touched = true;
                    break;
                }
            }
            if (touched)
                continue;

            bool lopsided = false;
            for (size_t i = 0; i < 8 && !lopsided; i += 2) {
                size_t valence = vertexNeighbors[octagon[i]].size();
                for (const auto& edge : buriedEdges) {
                    if (octagon[i] == edge.first || octagon[i] == edge.second)
                        --valence;
                }
                if (++valence > maxValence)
                    lopsided = true;
            }
            if (lopsided)
                continue;

            addedVertex = m_remeshedVertices.size();
            addedPosition = Vector3();
            for (size_t i = 0; i < 8; i += 2)
                addedPosition += m_remeshedVertices[octagon[i]];
            addedPosition /= 4.0;

            std::vector<std::vector<size_t>> quads;
            quads.reserve(4);
            for (size_t i = 0; i < 8; i += 2) {
                quads.push_back({ octagon[i], octagon[(i + 1) % 8],
                    octagon[(i + 2) % 8], addedVertex });
            }

            Vector3 oldNormal;
            for (const auto& faceIndex : run)
                oldNormal += faceNormal(m_remeshedPolygons[faceIndex]);
            bool shaped = true;
            for (const auto& quad : quads) {
                if (Vector3::dotProduct(oldNormal, faceNormal(quad)) <= 0.0) {
                    shaped = false;
                    break;
                }
            }
            if (!shaped) {
                addedVertex = noVertex;
                continue;
            }

            m_remeshedVertices.push_back(addedPosition);
            for (size_t i = 0; i < quads.size(); ++i) {
                if (i < run.size())
                    m_remeshedPolygons[run[i]] = quads[i];
                else
                    m_remeshedPolygons.push_back(quads[i]);
            }
            touchedVertices.insert(octagon.begin(), octagon.end());
            convertedVertices.insert(octagon.begin(), octagon.end());
            convertedVertices.insert(addedVertex);
            addedVertex = noVertex;
            ++roundConvertNum;
        }

        if (0 == roundConvertNum)
            break;
        convertNum += roundConvertNum;
    }

    if (0 == convertNum)
        return;

    std::cerr << "Convert triangle and five edge fans:" << convertNum << std::endl;
    rebuildHalfEdges();

    smoothAroundVertices(convertedVertices, 3, 5);
}

void QuadExtractor::collapseThreeValenceDiagonals()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    using Edge = std::pair<size_t, size_t>;
    const auto edgeOf = [](size_t a, size_t b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };
    const auto hasRepeatedVertex = [](const std::vector<size_t>& face) {
        std::unordered_set<size_t> vertices(face.begin(), face.end());
        return vertices.size() != face.size();
    };
    const auto canonicalFace = [](const std::vector<size_t>& face) {
        std::vector<size_t> best;
        std::vector<size_t> reversed(face.rbegin(), face.rend());
        for (const std::vector<size_t>* winding : {
                 &face, static_cast<const std::vector<size_t>*>(&reversed) }) {
            for (size_t start = 0; start < winding->size(); ++start) {
                std::vector<size_t> candidate;
                candidate.reserve(winding->size());
                for (size_t i = 0; i < winding->size(); ++i)
                    candidate.push_back((*winding)[(start + i) % winding->size()]);
                if (best.empty() || candidate < best)
                    best = std::move(candidate);
            }
        }
        return best;
    };
    const size_t noVertex = std::numeric_limits<size_t>::max();
    const size_t noFace = std::numeric_limits<size_t>::max();
    Vector3 addedPosition;
    size_t addedVertex = noVertex;
    const auto positionOf = [&](size_t vertex) {
        return vertex == addedVertex ? addedPosition : m_remeshedVertices[vertex];
    };
    const auto faceNormal = [&](const std::vector<size_t>& face) {
        Vector3 normal;
        const auto origin = positionOf(face[0]);
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            normal += Vector3::crossProduct(positionOf(face[i]) - origin,
                positionOf(face[i + 1]) - origin);
        }
        return normal;
    };
    // The quad closes up along the diagonal, so the two corners left standing each
    // hand one neighbor over to the point in the middle. Three neighbors is as low
    // as a corner may go before it turns into a doublet, so the sides need four
    const size_t minSideValence = 4;

    // Two three valence points facing each other across a quad are the two halves of
    // a single point which the extraction split apart, the quad between them is the
    // gap. Collapsing the diagonal puts a point back in the middle carrying the four
    // neighbors the pair had between them, and takes the quad away along with both
    // singularities, the sides paying for it with one neighbor each
    std::set<Edge> rejectedDiagonals;
    std::unordered_set<size_t> collapsedVertices;
    size_t collapseCount = 0;
    for (;;) {
        std::map<Edge, std::vector<size_t>> edgeFaces;
        std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
        std::unordered_map<size_t, size_t> vertexFaceCounts;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            for (size_t i = 0; i < face.size(); ++i) {
                const size_t j = (i + 1) % face.size();
                edgeFaces[edgeOf(face[i], face[j])].push_back(faceIndex);
                vertexNeighbors[face[i]].insert(face[j]);
                vertexNeighbors[face[j]].insert(face[i]);
            }
            for (const auto& vertex : face)
                ++vertexFaceCounts[vertex];
        }

        // A three valence point on a border is what a border looks like, not a defect
        std::unordered_set<size_t> borderVertices;
        for (const auto& it : edgeFaces) {
            if (2 == it.second.size())
                continue;
            borderVertices.insert(it.first.first);
            borderVertices.insert(it.first.second);
        }

        size_t collapsingFace = noFace;
        size_t first = noVertex;
        size_t second = noVertex;
        size_t left = noVertex;
        size_t right = noVertex;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size()
            && noFace == collapsingFace;
            ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            if (4 != face.size() || hasRepeatedVertex(face))
                continue;
            bool onBorder = false;
            for (const auto& vertex : face) {
                if (borderVertices.end() != borderVertices.find(vertex)) {
                    onBorder = true;
                    break;
                }
            }
            if (onBorder)
                continue;
            for (size_t i = 0; i < 2; ++i) {
                const size_t diagonalFirst = face[i];
                const size_t diagonalSecond = face[i + 2];
                if (rejectedDiagonals.end() != rejectedDiagonals.find(edgeOf(diagonalFirst, diagonalSecond)))
                    continue;
                if (3 != vertexNeighbors[diagonalFirst].size()
                    || 3 != vertexNeighbors[diagonalSecond].size())
                    continue;
                // A closed fan of three faces is the only shape a three valence point
                // may have here, anything else is not the pair this is looking for
                if (3 != vertexFaceCounts[diagonalFirst] || 3 != vertexFaceCounts[diagonalSecond])
                    continue;
                const size_t diagonalLeft = face[(i + 1) % 4];
                const size_t diagonalRight = face[(i + 3) % 4];
                if (vertexNeighbors[diagonalLeft].size() < minSideValence
                    || vertexNeighbors[diagonalRight].size() < minSideValence)
                    continue;
                // The two fans are only allowed to meet at the sides of the quad, any
                // other point shared between them, an edge included, would fold the
                // faces of the merged point over each other
                if (vertexNeighbors[diagonalFirst].end() != vertexNeighbors[diagonalFirst].find(diagonalSecond))
                    continue;
                size_t sharedNum = 0;
                for (const auto& neighbor : vertexNeighbors[diagonalFirst]) {
                    if (vertexNeighbors[diagonalSecond].end() != vertexNeighbors[diagonalSecond].find(neighbor))
                        ++sharedNum;
                }
                if (2 != sharedNum)
                    continue;
                collapsingFace = faceIndex;
                first = diagonalFirst;
                second = diagonalSecond;
                left = diagonalLeft;
                right = diagonalRight;
                break;
            }
        }
        if (noFace == collapsingFace)
            break;

        addedVertex = m_remeshedVertices.size();
        addedPosition = (m_remeshedVertices[first] + m_remeshedVertices[second]) * 0.5;

        std::vector<std::vector<size_t>> rewritten;
        std::vector<bool> affected;
        rewritten.reserve(m_remeshedPolygons.size());
        affected.reserve(m_remeshedPolygons.size());
        bool valid = true;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size() && valid; ++faceIndex) {
            if (collapsingFace == faceIndex)
                continue;
            const auto& face = m_remeshedPolygons[faceIndex];
            bool faceAffected = false;
            for (const auto& vertex : face) {
                if (first == vertex || second == vertex) {
                    faceAffected = true;
                    break;
                }
            }
            if (!faceAffected) {
                rewritten.push_back(face);
                affected.push_back(false);
                continue;
            }
            std::vector<size_t> candidate;
            candidate.reserve(face.size());
            for (const auto& vertex : face) {
                const size_t rewrittenVertex = first == vertex || second == vertex
                    ? addedVertex
                    : vertex;
                if (candidate.empty() || candidate.back() != rewrittenVertex)
                    candidate.push_back(rewrittenVertex);
            }
            if (candidate.size() > 1 && candidate.front() == candidate.back())
                candidate.pop_back();
            // The quad is the only face allowed to disappear, the fans around the pair
            // keep every side they came in with
            if (candidate.size() != face.size() || hasRepeatedVertex(candidate)) {
                valid = false;
                break;
            }
            if (Vector3::dotProduct(faceNormal(face), faceNormal(candidate)) <= 0.0) {
                valid = false;
                break;
            }
            rewritten.push_back(std::move(candidate));
            affected.push_back(true);
        }

        if (valid) {
            std::set<std::vector<size_t>> uniqueFaces;
            for (size_t faceIndex = 0; faceIndex < rewritten.size(); ++faceIndex) {
                if (!affected[faceIndex])
                    uniqueFaces.insert(canonicalFace(rewritten[faceIndex]));
            }
            std::map<Edge, size_t> addedEdgeCounts;
            for (size_t faceIndex = 0; faceIndex < rewritten.size() && valid; ++faceIndex) {
                const auto& face = rewritten[faceIndex];
                if (affected[faceIndex]
                    && !uniqueFaces.insert(canonicalFace(face)).second) {
                    valid = false;
                    break;
                }
                for (size_t i = 0; i < face.size(); ++i) {
                    const Edge edge = edgeOf(face[i], face[(i + 1) % face.size()]);
                    if (addedVertex != edge.first && addedVertex != edge.second)
                        continue;
                    if (++addedEdgeCounts[edge] > 2) {
                        valid = false;
                        break;
                    }
                }
            }
        }
        if (!valid) {
            rejectedDiagonals.insert(edgeOf(first, second));
            addedVertex = noVertex;
            continue;
        }

        m_remeshedVertices.push_back(addedPosition);
        m_remeshedPolygons = std::move(rewritten);
        collapsedVertices.insert(addedVertex);
        collapsedVertices.insert(left);
        collapsedVertices.insert(right);
        addedVertex = noVertex;
        ++collapseCount;
    }

    if (0 == collapseCount)
        return;

    // The two collapsed points are left with no face of their own
    std::map<size_t, size_t> oldToNew;
    std::vector<Vector3> compactedVertices;
    for (const auto& face : m_remeshedPolygons) {
        for (const auto vertex : face) {
            if (oldToNew.end() != oldToNew.find(vertex))
                continue;
            oldToNew[vertex] = compactedVertices.size();
            compactedVertices.push_back(m_remeshedVertices[vertex]);
        }
    }
    for (auto& face : m_remeshedPolygons) {
        for (auto& vertex : face)
            vertex = oldToNew.at(vertex);
    }
    m_remeshedVertices = std::move(compactedVertices);
    std::unordered_set<size_t> compactedCollapsedVertices;
    for (const auto& vertex : collapsedVertices) {
        const auto& findNew = oldToNew.find(vertex);
        if (oldToNew.end() != findNew)
            compactedCollapsedVertices.insert(findNew->second);
    }

    std::cerr << "Collapse three valence diagonals:" << collapseCount << std::endl;
    rebuildHalfEdges();

    // The point in the middle came from the diagonal, not from the source mesh, pull
    // the patch which closed up around it back onto the surface
    smoothAroundVertices(compactedCollapsedVertices, 3, 5);
}

void QuadExtractor::mergeDoubleSharedEdgeQuads()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    const auto hasRepeatedVertex = [](const std::vector<size_t>& face) {
        std::unordered_set<size_t> vertices(face.begin(), face.end());
        return vertices.size() != face.size();
    };
    const auto canonicalFace = [](const std::vector<size_t>& face) {
        std::vector<size_t> best;
        std::vector<size_t> reversed(face.rbegin(), face.rend());
        for (const std::vector<size_t>* winding : {
                 &face, static_cast<const std::vector<size_t>*>(&reversed) }) {
            for (size_t start = 0; start < winding->size(); ++start) {
                std::vector<size_t> candidate;
                candidate.reserve(winding->size());
                for (size_t i = 0; i < winding->size(); ++i)
                    candidate.push_back((*winding)[(start + i) % winding->size()]);
                if (best.empty() || candidate < best)
                    best = std::move(candidate);
            }
        }
        return best;
    };
    const auto indexOfVertex = [](const std::vector<size_t>& face, size_t vertex) {
        for (size_t i = 0; i < face.size(); ++i) {
            if (vertex == face[i])
                return i;
        }
        return face.size();
    };
    const auto faceNormal = [&](const std::vector<size_t>& face) {
        Vector3 normal;
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            normal += Vector3::crossProduct(
                m_remeshedVertices[face[i]] - m_remeshedVertices[face[0]],
                m_remeshedVertices[face[i + 1]] - m_remeshedVertices[face[0]]);
        }
        return normal;
    };

    size_t mergeNum = 0;
    std::unordered_set<size_t> mergedVertices;
    for (;;) {
        std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
        std::unordered_map<size_t, std::vector<size_t>> vertexFaces;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            for (size_t i = 0; i < face.size(); ++i) {
                const size_t j = (i + 1) % face.size();
                vertexNeighbors[face[i]].insert(face[j]);
                vertexNeighbors[face[j]].insert(face[i]);
            }
            for (const auto& vertex : face)
                vertexFaces[vertex].push_back(faceIndex);
        }

        std::set<size_t> middleVertices;
        for (const auto& it : vertexNeighbors) {
            if (2 != it.second.size())
                continue;
            const auto& findFaces = vertexFaces.find(it.first);
            if (vertexFaces.end() == findFaces || 2 != findFaces->second.size())
                continue;
            middleVertices.insert(it.first);
        }

        std::set<std::vector<size_t>> existingFaces;
        for (const auto& face : m_remeshedPolygons)
            existingFaces.insert(canonicalFace(face));

        std::unordered_set<size_t> touchedVertices;
        std::unordered_map<size_t, std::vector<size_t>> replacedFaces;
        std::unordered_set<size_t> removedFaces;
        size_t roundMergeNum = 0;
        for (const auto& middle : middleVertices) {
            const auto& faces = vertexFaces[middle];
            const size_t firstFaceIndex = faces[0];
            const size_t secondFaceIndex = faces[1];
            if (firstFaceIndex == secondFaceIndex)
                continue;
            const auto& firstFace = m_remeshedPolygons[firstFaceIndex];
            const auto& secondFace = m_remeshedPolygons[secondFaceIndex];
            if (4 != firstFace.size() || 4 != secondFace.size())
                continue;
            if (hasRepeatedVertex(firstFace) || hasRepeatedVertex(secondFace))
                continue;
            const size_t firstAt = indexOfVertex(firstFace, middle);
            const size_t secondAt = indexOfVertex(secondFace, middle);
            if (firstAt >= firstFace.size() || secondAt >= secondFace.size())
                continue;
            const size_t previous = firstFace[(firstAt + 3) % 4];
            const size_t next = firstFace[(firstAt + 1) % 4];
            if (previous != secondFace[(secondAt + 1) % 4]
                || next != secondFace[(secondAt + 3) % 4])
                continue;
            const size_t firstApex = firstFace[(firstAt + 2) % 4];
            const size_t secondApex = secondFace[(secondAt + 2) % 4];
            if (firstApex == secondApex)
                continue;
            bool touched = false;
            for (const auto& vertex : { middle, previous, next, firstApex, secondApex }) {
                if (touchedVertices.end() != touchedVertices.find(vertex)) {
                    touched = true;
                    break;
                }
            }
            if (touched)
                continue;

            std::vector<size_t> merged { firstApex, previous, secondApex, next };
            if (hasRepeatedVertex(merged))
                continue;
            if (existingFaces.end() != existingFaces.find(canonicalFace(merged)))
                continue;
            if (Vector3::dotProduct(faceNormal(firstFace) + faceNormal(secondFace),
                    faceNormal(merged))
                <= 0.0)
                continue;

            existingFaces.insert(canonicalFace(merged));
            replacedFaces.insert({ firstFaceIndex, merged });
            removedFaces.insert(secondFaceIndex);
            touchedVertices.insert(merged.begin(), merged.end());
            touchedVertices.insert(middle);
            mergedVertices.insert(merged.begin(), merged.end());
            ++roundMergeNum;
        }

        if (0 == roundMergeNum)
            break;

        std::vector<std::vector<size_t>> rewritten;
        rewritten.reserve(m_remeshedPolygons.size() - roundMergeNum);
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            if (removedFaces.end() != removedFaces.find(faceIndex))
                continue;
            const auto& findReplaced = replacedFaces.find(faceIndex);
            if (replacedFaces.end() != findReplaced) {
                rewritten.push_back(findReplaced->second);
                continue;
            }
            rewritten.push_back(m_remeshedPolygons[faceIndex]);
        }
        m_remeshedPolygons = std::move(rewritten);
        mergeNum += roundMergeNum;
    }

    if (0 == mergeNum)
        return;

    std::map<size_t, size_t> oldToNew;
    std::vector<Vector3> compactedVertices;
    for (const auto& face : m_remeshedPolygons) {
        for (const auto vertex : face) {
            if (oldToNew.end() != oldToNew.find(vertex))
                continue;
            oldToNew[vertex] = compactedVertices.size();
            compactedVertices.push_back(m_remeshedVertices[vertex]);
        }
    }
    for (auto& face : m_remeshedPolygons) {
        for (auto& vertex : face)
            vertex = oldToNew.at(vertex);
    }
    m_remeshedVertices = std::move(compactedVertices);
    std::unordered_set<size_t> compactedMergedVertices;
    for (const auto& vertex : mergedVertices) {
        const auto& findNew = oldToNew.find(vertex);
        if (oldToNew.end() != findNew)
            compactedMergedVertices.insert(findNew->second);
    }

    std::cerr << "Merge double shared edge quads:" << mergeNum << std::endl;
    rebuildHalfEdges();

    smoothAroundVertices(compactedMergedVertices, 3, 5);
}

void QuadExtractor::switchHighValenceEdges()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    using Edge = std::pair<size_t, size_t>;
    const auto edgeOf = [](size_t a, size_t b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };
    const auto canonicalFace = [](const std::vector<size_t>& face) {
        std::vector<size_t> best;
        std::vector<size_t> reversed(face.rbegin(), face.rend());
        for (const std::vector<size_t>* winding : {
                 &face, static_cast<const std::vector<size_t>*>(&reversed) }) {
            for (size_t start = 0; start < winding->size(); ++start) {
                std::vector<size_t> candidate;
                candidate.reserve(winding->size());
                for (size_t i = 0; i < winding->size(); ++i)
                    candidate.push_back((*winding)[(start + i) % winding->size()]);
                if (best.empty() || candidate < best)
                    best = std::move(candidate);
            }
        }
        return best;
    };
    const auto findDirectedEdge = [](const std::vector<size_t>& face, size_t from, size_t to) {
        for (size_t i = 0; i < face.size(); ++i) {
            if (from == face[i] && to == face[(i + 1) % face.size()])
                return i;
        }
        return face.size();
    };
    const auto faceNormal = [&](const std::vector<size_t>& face) {
        Vector3 normal;
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            normal += Vector3::crossProduct(
                m_remeshedVertices[face[i]] - m_remeshedVertices[face[0]],
                m_remeshedVertices[face[i + 1]] - m_remeshedVertices[face[0]]);
        }
        return normal;
    };
    const auto cornerScore = [&](const std::vector<size_t>& quad) {
        double total = 0.0;
        for (size_t i = 0; i < quad.size(); ++i) {
            size_t h = (i + quad.size() - 1) % quad.size();
            size_t j = (i + 1) % quad.size();
            auto left = (m_remeshedVertices[quad[h]] - m_remeshedVertices[quad[i]]).normalized();
            auto right = (m_remeshedVertices[quad[j]] - m_remeshedVertices[quad[i]]).normalized();
            total += std::abs(Vector3::dotProduct(left, right));
        }
        return -total / quad.size();
    };
    // How far a point is from the four neighbors a quad point wants
    const auto valenceScore = [](size_t valence) {
        return valence > 4 ? (int)(valence - 4) : (int)(4 - valence);
    };

    // Two quads sharing an edge make a hexagon with three diagonals, the shared
    // edge being one of them. Switching to another diagonal takes one neighbor
    // away from each end of the shared edge and hands one to each end of the new
    // diagonal, no point is added, removed or moved
    size_t switchNum = 0;
    std::unordered_set<size_t> switchedVertices;
    for (;;) {
        std::map<Edge, std::vector<size_t>> edgeFaces;
        std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            for (size_t i = 0; i < face.size(); ++i) {
                const size_t j = (i + 1) % face.size();
                edgeFaces[edgeOf(face[i], face[j])].push_back(faceIndex);
                vertexNeighbors[face[i]].insert(face[j]);
                vertexNeighbors[face[j]].insert(face[i]);
            }
        }

        // Four neighbors is the wrong target on a border, leave those alone
        std::unordered_set<size_t> borderVertices;
        for (const auto& it : edgeFaces) {
            if (2 == it.second.size())
                continue;
            borderVertices.insert(it.first.first);
            borderVertices.insert(it.first.second);
        }

        std::set<std::vector<size_t>> existingFaces;
        for (const auto& face : m_remeshedPolygons)
            existingFaces.insert(canonicalFace(face));

        // A second switch reaching into the same hexagon would work off the face
        // indices and the valences the first one left behind
        std::unordered_set<size_t> touchedVertices;
        size_t roundSwitchNum = 0;
        for (const auto& it : edgeFaces) {
            if (2 != it.second.size())
                continue;

            const size_t a = it.first.first;
            const size_t b = it.first.second;
            const size_t aValence = vertexNeighbors[a].size();
            const size_t bValence = vertexNeighbors[b].size();
            if (aValence <= 4 || bValence <= 4)
                continue;
            if (borderVertices.end() != borderVertices.find(a)
                || borderVertices.end() != borderVertices.find(b))
                continue;

            size_t first = it.second[0];
            size_t second = it.second[1];
            if (4 != m_remeshedPolygons[first].size()
                || 4 != m_remeshedPolygons[second].size())
                continue;
            size_t firstEntry = findDirectedEdge(m_remeshedPolygons[first], a, b);
            if (firstEntry >= m_remeshedPolygons[first].size()) {
                std::swap(first, second);
                firstEntry = findDirectedEdge(m_remeshedPolygons[first], a, b);
            }
            const size_t secondEntry = findDirectedEdge(m_remeshedPolygons[second], b, a);
            if (firstEntry >= m_remeshedPolygons[first].size()
                || secondEntry >= m_remeshedPolygons[second].size())
                continue;

            // The hexagon runs b, c, d, a, e, f, keeping the winding of both quads
            const auto& firstFace = m_remeshedPolygons[first];
            const auto& secondFace = m_remeshedPolygons[second];
            const size_t c = firstFace[(firstEntry + 2) % 4];
            const size_t d = firstFace[(firstEntry + 3) % 4];
            const size_t e = secondFace[(secondEntry + 2) % 4];
            const size_t f = secondFace[(secondEntry + 3) % 4];
            const std::unordered_set<size_t> hexagon = { a, b, c, d, e, f };
            if (6 != hexagon.size())
                continue;
            bool touched = false;
            for (const auto& vertex : hexagon) {
                if (touchedVertices.end() != touchedVertices.find(vertex)) {
                    touched = true;
                    break;
                }
            }
            if (touched)
                continue;

            const Vector3 oldNormal = faceNormal(firstFace) + faceNormal(secondFace);
            const int oldValenceScore = valenceScore(aValence) + valenceScore(bValence);

            int bestGain = 0;
            double bestShape = 0.0;
            std::vector<size_t> bestFace1;
            std::vector<size_t> bestFace2;
            for (size_t candidate = 0; candidate < 2; ++candidate) {
                const size_t x = 0 == candidate ? c : d;
                const size_t y = 0 == candidate ? e : f;
                if (borderVertices.end() != borderVertices.find(x)
                    || borderVertices.end() != borderVertices.find(y))
                    continue;
                // The new diagonal would land on an edge which is already there
                if (vertexNeighbors[x].end() != vertexNeighbors[x].find(y))
                    continue;
                const size_t xValence = vertexNeighbors[x].size();
                const size_t yValence = vertexNeighbors[y].size();
                const int gain = oldValenceScore + valenceScore(xValence) + valenceScore(yValence)
                    - (valenceScore(aValence - 1) + valenceScore(bValence - 1)
                        + valenceScore(xValence + 1) + valenceScore(yValence + 1));
                if (gain <= 0)
                    continue;
                std::vector<size_t> face1 = 0 == candidate
                    ? std::vector<size_t> { c, d, a, e }
                    : std::vector<size_t> { d, a, e, f };
                std::vector<size_t> face2 = 0 == candidate
                    ? std::vector<size_t> { e, f, b, c }
                    : std::vector<size_t> { f, b, c, d };
                if (Vector3::dotProduct(oldNormal, faceNormal(face1)) <= 0.0
                    || Vector3::dotProduct(oldNormal, faceNormal(face2)) <= 0.0)
                    continue;
                if (existingFaces.end() != existingFaces.find(canonicalFace(face1))
                    || existingFaces.end() != existingFaces.find(canonicalFace(face2)))
                    continue;
                const double shape = cornerScore(face1) + cornerScore(face2);
                if (gain < bestGain || (gain == bestGain && shape <= bestShape))
                    continue;
                bestGain = gain;
                bestShape = shape;
                bestFace1 = std::move(face1);
                bestFace2 = std::move(face2);
            }
            if (bestFace1.empty())
                continue;

            existingFaces.erase(canonicalFace(firstFace));
            existingFaces.erase(canonicalFace(secondFace));
            existingFaces.insert(canonicalFace(bestFace1));
            existingFaces.insert(canonicalFace(bestFace2));
            m_remeshedPolygons[first] = std::move(bestFace1);
            m_remeshedPolygons[second] = std::move(bestFace2);
            touchedVertices.insert(hexagon.begin(), hexagon.end());
            switchedVertices.insert(hexagon.begin(), hexagon.end());
            ++roundSwitchNum;
        }

        if (0 == roundSwitchNum)
            break;
        switchNum += roundSwitchNum;
    }

    if (0 == switchNum)
        return;

    std::cerr << "Switch high valence edges:" << switchNum << std::endl;
    rebuildHalfEdges();

    // The switched quads kept their points, pull the reconnected patches back
    // into shape
    smoothAroundVertices(switchedVertices, 3, 5);
}

void QuadExtractor::smoothAroundVertices(const std::unordered_set<size_t>& seedVertices,
    size_t rings, size_t iterations)
{
    if (seedVertices.empty() || m_remeshedPolygons.empty())
        return;

    std::unordered_map<size_t, std::vector<size_t>> vertexFaces;
    for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
        for (const auto& vertex : m_remeshedPolygons[faceIndex])
            vertexFaces[vertex].push_back(faceIndex);
    }

    // Rings of faces grown from the seed points, the vertices sitting on the outer
    // border of the patch anchor the smoothing
    std::unordered_set<size_t> patchFaces;
    std::unordered_set<size_t> frontier;
    for (const auto& vertex : seedVertices) {
        if (vertexFaces.end() == vertexFaces.find(vertex))
            continue;
        frontier.insert(vertex);
    }
    for (size_t ring = 0; ring < rings && !frontier.empty(); ++ring) {
        std::unordered_set<size_t> nextFrontier;
        for (const auto& vertex : frontier) {
            for (const auto& faceIndex : vertexFaces[vertex]) {
                if (!patchFaces.insert(faceIndex).second)
                    continue;
                for (const auto& neighborVertex : m_remeshedPolygons[faceIndex])
                    nextFrontier.insert(neighborVertex);
            }
        }
        frontier = std::move(nextFrontier);
    }

    std::unordered_set<size_t> movableVertices;
    for (const auto& faceIndex : patchFaces) {
        for (const auto& vertex : m_remeshedPolygons[faceIndex])
            movableVertices.insert(vertex);
    }
    for (auto it = movableVertices.begin(); it != movableVertices.end();) {
        bool onPatchBorder = false;
        for (const auto& faceIndex : vertexFaces[*it]) {
            if (patchFaces.end() == patchFaces.find(faceIndex)) {
                onPatchBorder = true;
                break;
            }
        }
        if (onPatchBorder)
            it = movableVertices.erase(it);
        else
            ++it;
    }

    if (movableVertices.empty())
        return;

    smoothAndProject(iterations, &movableVertices);
}

void QuadExtractor::cleanupTriangles()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    using Edge = std::pair<size_t, size_t>;
    const auto edgeOf = [](size_t a, size_t b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };
    const auto faceHasRepeatedVertex = [](const std::vector<size_t>& face) {
        std::set<size_t> vertices(face.begin(), face.end());
        return vertices.size() != face.size();
    };
    const auto canonicalFace = [](const std::vector<size_t>& face) {
        std::vector<size_t> best;
        std::vector<size_t> reversed(face.rbegin(), face.rend());
        for (const std::vector<size_t>* winding : {
                 &face, static_cast<const std::vector<size_t>*>(&reversed) }) {
            for (size_t start = 0; start < winding->size(); ++start) {
                std::vector<size_t> candidate;
                candidate.reserve(winding->size());
                for (size_t i = 0; i < winding->size(); ++i)
                    candidate.push_back((*winding)[(start + i) % winding->size()]);
                if (best.empty() || candidate < best)
                    best = std::move(candidate);
            }
        }
        return best;
    };
    const auto polygonNormal = [](const std::vector<Vector3>& positions) {
        Vector3 normal;
        for (size_t i = 1; i + 1 < positions.size(); ++i) {
            normal += Vector3::crossProduct(positions[i] - positions[0],
                positions[i + 1] - positions[0]);
        }
        return normal;
    };

    // The route of a triangle is the ladder of edges crossing the quad strip that
    // leads to the nearest sink, a border or another non quad face. Every rung of
    // the ladder collapses in the same step: the strip closes up, the sink loses
    // one side, and the valence of the vertices along the way is left untouched
    // because each pair of side edges merges together with its rung.
    // Collapsing one rung at a time instead lets the surviving vertex take part in
    // the next collapse as well, which grows a fan of slivers around a single point
    const size_t maxRouteLength = 20;
    const size_t noFace = std::numeric_limits<size_t>::max();

    std::set<Edge> rejectedEdges;
    std::unordered_set<size_t> collapsedVertices;
    size_t collapseCount = 0;
    for (;;) {
        std::map<Edge, std::vector<size_t>> edgeFaces;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            for (size_t i = 0; i < face.size(); ++i)
                edgeFaces[edgeOf(face[i], face[(i + 1) % face.size()])].push_back(faceIndex);
        }

        auto walkRoute = [&](size_t startFace, const Edge& startEdge,
                             std::vector<Edge>* rungs,
                             std::set<size_t>* dissolvedFaces,
                             size_t* sinkFace) {
            rungs->clear();
            dissolvedFaces->clear();
            dissolvedFaces->insert(startFace);
            *sinkFace = noFace;
            std::set<size_t> rungVertices;
            size_t currentFace = startFace;
            Edge rung = startEdge;
            for (;;) {
                if (rungs->size() >= maxRouteLength)
                    return false;
                // Two rungs sharing a vertex would collapse into each other
                if (!rungVertices.insert(rung.first).second)
                    return false;
                if (!rungVertices.insert(rung.second).second)
                    return false;
                rungs->push_back(rung);
                const auto& incident = edgeFaces[rung];
                if (1 == incident.size())
                    return true;
                if (2 != incident.size())
                    return false;
                const size_t neighbor = incident[0] == currentFace
                    ? incident[1]
                    : incident[0];
                if (dissolvedFaces->end() != dissolvedFaces->find(neighbor))
                    return false;
                const auto& neighborFace = m_remeshedPolygons[neighbor];
                if (4 != neighborFace.size()) {
                    *sinkFace = neighbor;
                    return true;
                }
                size_t entry = neighborFace.size();
                for (size_t i = 0; i < neighborFace.size(); ++i) {
                    if (edgeOf(neighborFace[i], neighborFace[(i + 1) % neighborFace.size()]) == rung) {
                        entry = i;
                        break;
                    }
                }
                if (entry >= neighborFace.size())
                    return false;
                dissolvedFaces->insert(neighbor);
                currentFace = neighbor;
                rung = edgeOf(neighborFace[(entry + 2) % 4], neighborFace[(entry + 3) % 4]);
            }
        };

        std::vector<Edge> route;
        std::set<size_t> routeFaces;
        size_t routeSink = noFace;
        for (size_t startFace = 0; startFace < m_remeshedPolygons.size(); ++startFace) {
            const auto& triangle = m_remeshedPolygons[startFace];
            if (3 != triangle.size() || faceHasRepeatedVertex(triangle))
                continue;
            for (size_t i = 0; i < 3; ++i) {
                const Edge startEdge = edgeOf(triangle[i], triangle[(i + 1) % 3]);
                if (rejectedEdges.end() != rejectedEdges.find(startEdge))
                    continue;
                std::vector<Edge> candidateRoute;
                std::set<size_t> candidateFaces;
                size_t candidateSink = noFace;
                if (!walkRoute(startFace, startEdge, &candidateRoute, &candidateFaces, &candidateSink))
                    continue;
                // The shorter the ladder, the less of the surrounding mesh it takes with it
                if (route.empty() || candidateRoute.size() < route.size()) {
                    route = std::move(candidateRoute);
                    routeFaces = std::move(candidateFaces);
                    routeSink = candidateSink;
                }
            }
        }
        if (route.empty())
            break;

        std::unordered_map<size_t, size_t> mergedInto;
        std::unordered_map<size_t, Vector3> mergedPositions;
        for (const auto& rung : route) {
            mergedInto.insert({ rung.second, rung.first });
            mergedPositions.insert({ rung.first,
                (m_remeshedVertices[rung.first] + m_remeshedVertices[rung.second]) * 0.5 });
        }
        const auto rewriteVertex = [&](size_t vertex) {
            const auto& findMerged = mergedInto.find(vertex);
            return mergedInto.end() == findMerged ? vertex : findMerged->second;
        };
        const auto rewritePosition = [&](size_t vertex) {
            const auto& findPosition = mergedPositions.find(vertex);
            return mergedPositions.end() == findPosition
                ? m_remeshedVertices[vertex]
                : findPosition->second;
        };

        std::vector<std::vector<size_t>> rewritten;
        rewritten.reserve(m_remeshedPolygons.size());
        std::set<std::vector<size_t>> touchedFaces;
        std::map<Edge, size_t> touchedEdgeCounts;
        bool valid = true;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            std::vector<size_t> candidate;
            candidate.reserve(face.size());
            bool touched = false;
            for (const auto& vertex : face) {
                const size_t rewrittenVertex = rewriteVertex(vertex);
                if (rewrittenVertex != vertex
                    || mergedPositions.end() != mergedPositions.find(vertex))
                    touched = true;
                if (candidate.empty() || candidate.back() != rewrittenVertex)
                    candidate.push_back(rewrittenVertex);
            }
            if (candidate.size() > 1 && candidate.front() == candidate.back())
                candidate.pop_back();
            // The strip faces and a triangle sink are meant to disappear, everything
            // else has to come out of the collapse with the shape it went in with,
            // apart from the sink which gives up exactly one side
            const bool dissolving = routeFaces.end() != routeFaces.find(faceIndex)
                || (faceIndex == routeSink && 3 == face.size());
            if (dissolving) {
                if (candidate.size() >= 3) {
                    valid = false;
                    break;
                }
                continue;
            }
            const size_t expectedSize = faceIndex == routeSink ? face.size() - 1 : face.size();
            if (candidate.size() != expectedSize || faceHasRepeatedVertex(candidate)) {
                valid = false;
                break;
            }
            if (touched) {
                std::vector<Vector3> before;
                before.reserve(face.size());
                for (const auto& vertex : face)
                    before.push_back(m_remeshedVertices[vertex]);
                std::vector<Vector3> after;
                after.reserve(candidate.size());
                for (const auto& vertex : candidate)
                    after.push_back(rewritePosition(vertex));
                if (Vector3::dotProduct(polygonNormal(before), polygonNormal(after)) <= 0.0) {
                    valid = false;
                    break;
                }
                // A face that duplicates another one must share every vertex with it,
                // so both of them are among the faces touched by the collapse
                if (!touchedFaces.insert(canonicalFace(candidate)).second) {
                    valid = false;
                    break;
                }
                for (size_t i = 0; i < candidate.size(); ++i) {
                    const Edge edge = edgeOf(candidate[i], candidate[(i + 1) % candidate.size()]);
                    if (mergedPositions.end() == mergedPositions.find(edge.first)
                        && mergedPositions.end() == mergedPositions.find(edge.second))
                        continue;
                    if (++touchedEdgeCounts[edge] > 2) {
                        valid = false;
                        break;
                    }
                }
                if (!valid)
                    break;
            }
            rewritten.push_back(std::move(candidate));
        }
        if (!valid) {
            rejectedEdges.insert(route.front());
            continue;
        }

        for (const auto& it : mergedPositions) {
            m_remeshedVertices[it.first] = it.second;
            collapsedVertices.insert(it.first);
        }
        m_remeshedPolygons = std::move(rewritten);
        ++collapseCount;
    }

    if (0 == collapseCount)
        return;

    std::map<size_t, size_t> oldToNew;
    std::vector<Vector3> compactedVertices;
    for (const auto& face : m_remeshedPolygons) {
        for (const auto vertex : face) {
            if (oldToNew.end() != oldToNew.find(vertex))
                continue;
            oldToNew[vertex] = compactedVertices.size();
            compactedVertices.push_back(m_remeshedVertices[vertex]);
        }
    }
    for (auto& face : m_remeshedPolygons) {
        for (auto& vertex : face)
            vertex = oldToNew.at(vertex);
    }
    m_remeshedVertices = std::move(compactedVertices);
    std::unordered_set<size_t> compactedCollapsedVertices;
    for (const auto& vertex : collapsedVertices) {
        const auto& findNew = oldToNew.find(vertex);
        if (oldToNew.end() != findNew)
            compactedCollapsedVertices.insert(findNew->second);
    }

    std::cerr << "Cleanup triangle faces:" << collapseCount << std::endl;
    rebuildHalfEdges();

    // The rungs met halfway, pull the closed up strips back onto the source mesh
    smoothAroundVertices(compactedCollapsedVertices, 3, 5);
}

void QuadExtractor::splitSevenEdgeFaces()
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
    for (const auto& face : m_remeshedPolygons) {
        for (size_t i = 0; i < face.size(); ++i) {
            size_t j = (i + 1) % face.size();
            vertexNeighbors[face[i]].insert(face[j]);
            vertexNeighbors[face[j]].insert(face[i]);
        }
    }

    auto flowScoreAt = [&](size_t vertex, const Vector3& direction,
                           const std::unordered_set<size_t>& faceNeighbors) {
        double best = -1.0;
        const auto& findNeighbors = vertexNeighbors.find(vertex);
        if (vertexNeighbors.end() == findNeighbors)
            return 0.0;
        bool found = false;
        for (const auto& neighbor : findNeighbors->second) {
            if (faceNeighbors.end() != faceNeighbors.find(neighbor))
                continue;
            auto incoming = (m_remeshedVertices[vertex] - m_remeshedVertices[neighbor]).normalized();
            double score = Vector3::dotProduct(incoming, direction);
            if (score > best)
                best = score;
            found = true;
        }
        return found ? best : 0.0;
    };

    auto cornerScore = [&](const std::vector<size_t>& polygon) {
        double total = 0.0;
        for (size_t i = 0; i < polygon.size(); ++i) {
            size_t h = (i + polygon.size() - 1) % polygon.size();
            size_t j = (i + 1) % polygon.size();
            auto left = (m_remeshedVertices[polygon[h]] - m_remeshedVertices[polygon[i]]).normalized();
            auto right = (m_remeshedVertices[polygon[j]] - m_remeshedVertices[polygon[i]]).normalized();
            total += std::abs(Vector3::dotProduct(left, right));
        }
        return -total / polygon.size();
    };

    size_t splitNum = 0;
    std::vector<std::vector<size_t>> polygons;
    polygons.reserve(m_remeshedPolygons.size());
    for (const auto& face : m_remeshedPolygons) {
        if (7 != face.size()) {
            polygons.push_back(face);
            continue;
        }
        std::unordered_set<size_t> uniqueVertices(face.begin(), face.end());
        if (7 != uniqueVertices.size()) {
            polygons.push_back(face);
            continue;
        }

        int bestCorner = -1;
        double bestScore = 0.0;
        for (size_t i = 0; i < 7; ++i) {
            size_t a = face[i];
            size_t b = face[(i + 3) % 7];
            const auto& findNeighbors = vertexNeighbors.find(a);
            if (vertexNeighbors.end() != findNeighbors
                && findNeighbors->second.end() != findNeighbors->second.find(b)) {
                continue;
            }
            auto direction = (m_remeshedVertices[b] - m_remeshedVertices[a]).normalized();
            std::unordered_set<size_t> aFaceNeighbors = {
                face[(i + 6) % 7], face[(i + 1) % 7], b
            };
            std::unordered_set<size_t> bFaceNeighbors = {
                face[(i + 2) % 7], face[(i + 4) % 7], a
            };
            double score = flowScoreAt(a, direction, aFaceNeighbors)
                + flowScoreAt(b, -direction, bFaceNeighbors);
            score += cornerScore({ face[i], face[(i + 1) % 7], face[(i + 2) % 7], face[(i + 3) % 7] });
            score += cornerScore({ face[(i + 3) % 7], face[(i + 4) % 7], face[(i + 5) % 7], face[(i + 6) % 7], face[i] });
            if (-1 == bestCorner || score > bestScore) {
                bestCorner = (int)i;
                bestScore = score;
            }
        }
        if (-1 == bestCorner) {
            std::cerr << "Seven edge face kept, no diagonal available" << std::endl;
            polygons.push_back(face);
            continue;
        }

        size_t i = (size_t)bestCorner;
        polygons.push_back({ face[i], face[(i + 1) % 7], face[(i + 2) % 7], face[(i + 3) % 7] });
        polygons.push_back({ face[(i + 3) % 7], face[(i + 4) % 7], face[(i + 5) % 7], face[(i + 6) % 7], face[i] });
        ++splitNum;
        vertexNeighbors[face[i]].insert(face[(i + 3) % 7]);
        vertexNeighbors[face[(i + 3) % 7]].insert(face[i]);
    }

    if (0 == splitNum)
        return;

    std::cerr << "Split seven edge faces:" << splitNum << std::endl;
    m_remeshedPolygons = std::move(polygons);
    rebuildHalfEdges();
}

void QuadExtractor::mergeSharedFiveEdgeFaces(const ProgressHandler* progressHandler)
{
    if (m_remeshedVertices.empty() || m_remeshedPolygons.empty())
        return;

    // Every merge consumes a pair of pentagons, so half the pentagon count is a
    // real ceiling on how many rounds can succeed and makes an honest
    // denominator for the fraction reported below.
    size_t pentagonCount = 0;
    for (const auto& face : m_remeshedPolygons) {
        if (5 == face.size())
            ++pentagonCount;
    }
    const size_t mergeCeiling = std::max<size_t>(1, pentagonCount / 2);

    using Edge = std::pair<size_t, size_t>;
    const auto edgeOf = [](size_t a, size_t b) {
        return std::make_pair(std::min(a, b), std::max(a, b));
    };
    const auto faceHasRepeatedVertex = [](const std::vector<size_t>& face) {
        std::set<size_t> vertices(face.begin(), face.end());
        return vertices.size() != face.size();
    };
    const auto canonicalFace = [](const std::vector<size_t>& face) {
        std::vector<size_t> best;
        std::vector<size_t> reversed(face.rbegin(), face.rend());
        for (const std::vector<size_t>* winding : {
                 &face, static_cast<const std::vector<size_t>*>(&reversed) }) {
            for (size_t start = 0; start < winding->size(); ++start) {
                std::vector<size_t> candidate;
                candidate.reserve(winding->size());
                for (size_t i = 0; i < winding->size(); ++i)
                    candidate.push_back((*winding)[(start + i) % winding->size()]);
                if (best.empty() || candidate < best)
                    best = std::move(candidate);
            }
        }
        return best;
    };
    const auto positionOf = [&](size_t vertex, size_t movedVertex, const Vector3& movedPosition) {
        return vertex == movedVertex ? movedPosition : m_remeshedVertices[vertex];
    };
    const auto faceNormal = [&](const std::vector<size_t>& face, size_t movedVertex,
                                const Vector3& movedPosition) {
        Vector3 normal;
        const auto origin = positionOf(face[0], movedVertex, movedPosition);
        for (size_t i = 1; i + 1 < face.size(); ++i) {
            normal += Vector3::crossProduct(
                positionOf(face[i], movedVertex, movedPosition) - origin,
                positionOf(face[i + 1], movedVertex, movedPosition) - origin);
        }
        return normal;
    };

    // The merged point inherits the neighbors of both ends of the collapsed edge.
    // The pair of pentagons is worth trading for a six valence point, past that
    // the singularity left behind is a worse defect than the faces it replaces
    const size_t maxMergedValence = 6;

    std::set<Edge> rejectedEdges;
    std::unordered_set<size_t> mergedVertices;
    size_t mergeCount = 0;
    for (;;) {
        if (nullptr != progressHandler && *progressHandler) {
            (*progressHandler)(std::min(0.99f, (float)mergeCount / mergeCeiling),
                "Merging shared five edge faces");
        }
        std::map<Edge, std::vector<size_t>> edgeFaces;
        std::unordered_map<size_t, std::unordered_set<size_t>> vertexNeighbors;
        for (size_t faceIndex = 0; faceIndex < m_remeshedPolygons.size(); ++faceIndex) {
            const auto& face = m_remeshedPolygons[faceIndex];
            for (size_t i = 0; i < face.size(); ++i) {
                const size_t j = (i + 1) % face.size();
                edgeFaces[edgeOf(face[i], face[j])].push_back(faceIndex);
                vertexNeighbors[face[i]].insert(face[j]);
                vertexNeighbors[face[j]].insert(face[i]);
            }
        }

        const auto mergedValence = [&](const Edge& edge) {
            std::unordered_set<size_t> neighbors = vertexNeighbors[edge.first];
            const auto& secondNeighbors = vertexNeighbors[edge.second];
            neighbors.insert(secondNeighbors.begin(), secondNeighbors.end());
            neighbors.erase(edge.first);
            neighbors.erase(edge.second);
            return neighbors.size();
        };

        Edge sharedEdge { 0, 0 };
        bool foundShared = false;
        for (const auto& it : edgeFaces) {
            if (2 != it.second.size())
                continue;
            if (rejectedEdges.end() != rejectedEdges.find(it.first))
                continue;
            bool bothFiveEdges = true;
            for (const auto& faceIndex : it.second) {
                const auto& face = m_remeshedPolygons[faceIndex];
                std::unordered_set<size_t> uniqueVertices(face.begin(), face.end());
                if (5 != face.size() || 5 != uniqueVertices.size()) {
                    bothFiveEdges = false;
                    break;
                }
            }
            if (!bothFiveEdges)
                continue;
            if (mergedValence(it.first) > maxMergedValence)
                continue;
            sharedEdge = it.first;
            foundShared = true;
            break;
        }
        if (!foundShared)
            break;

        const size_t keep = sharedEdge.first;
        const size_t remove = sharedEdge.second;
        const size_t unmovedVertex = m_remeshedVertices.size();
        const Vector3 keepPosition = (m_remeshedVertices[keep]
                                         + m_remeshedVertices[remove])
            * 0.5;
        std::vector<std::vector<size_t>> rewritten;
        std::vector<bool> affected;
        rewritten.reserve(m_remeshedPolygons.size());
        affected.reserve(m_remeshedPolygons.size());
        bool valid = true;
        for (const auto& face : m_remeshedPolygons) {
            bool faceAffected = false;
            for (const auto vertex : face) {
                if (keep == vertex || remove == vertex) {
                    faceAffected = true;
                    break;
                }
            }
            std::vector<size_t> candidate;
            candidate.reserve(face.size());
            for (const auto vertex : face) {
                const size_t rewrittenVertex = vertex == remove ? keep : vertex;
                if (candidate.empty() || candidate.back() != rewrittenVertex)
                    candidate.push_back(rewrittenVertex);
            }
            if (candidate.size() > 1 && candidate.front() == candidate.back())
                candidate.pop_back();
            if (faceAffected) {
                // The two five edge faces become quads, no other face is allowed to
                // degrade, otherwise the merge is trading one defect for another
                if (candidate.size() < 3
                    || (face.size() >= 4 && candidate.size() < 4)) {
                    valid = false;
                    break;
                }
                if (faceHasRepeatedVertex(candidate)) {
                    valid = false;
                    break;
                }
                const auto oldNormal = faceNormal(face, unmovedVertex, Vector3());
                const auto newNormal = faceNormal(candidate, keep, keepPosition);
                if (Vector3::dotProduct(oldNormal, newNormal) <= 0.0) {
                    valid = false;
                    break;
                }
            }
            rewritten.push_back(std::move(candidate));
            affected.push_back(faceAffected);
        }

        if (valid) {
            std::set<std::vector<size_t>> uniqueFaces;
            for (size_t faceIndex = 0; faceIndex < rewritten.size(); ++faceIndex) {
                if (!affected[faceIndex])
                    uniqueFaces.insert(canonicalFace(rewritten[faceIndex]));
            }
            std::map<Edge, size_t> keepEdgeCounts;
            for (size_t faceIndex = 0; faceIndex < rewritten.size() && valid; ++faceIndex) {
                const auto& face = rewritten[faceIndex];
                if (affected[faceIndex]
                    && !uniqueFaces.insert(canonicalFace(face)).second) {
                    valid = false;
                    break;
                }
                for (size_t i = 0; i < face.size(); ++i) {
                    const Edge edge = edgeOf(face[i], face[(i + 1) % face.size()]);
                    if (keep != edge.first && keep != edge.second)
                        continue;
                    if (++keepEdgeCounts[edge] > 2) {
                        valid = false;
                        break;
                    }
                }
            }
        }
        if (!valid) {
            rejectedEdges.insert(sharedEdge);
            continue;
        }

        m_remeshedVertices[keep] = keepPosition;
        m_remeshedPolygons = std::move(rewritten);
        mergedVertices.insert(keep);
        ++mergeCount;
    }

    if (0 == mergeCount)
        return;

    std::map<size_t, size_t> oldToNew;
    std::vector<Vector3> compactedVertices;
    for (const auto& face : m_remeshedPolygons) {
        for (const auto vertex : face) {
            if (oldToNew.end() != oldToNew.find(vertex))
                continue;
            oldToNew[vertex] = compactedVertices.size();
            compactedVertices.push_back(m_remeshedVertices[vertex]);
        }
    }
    for (auto& face : m_remeshedPolygons) {
        for (auto& vertex : face)
            vertex = oldToNew.at(vertex);
    }
    m_remeshedVertices = std::move(compactedVertices);
    std::unordered_set<size_t> compactedMergedVertices;
    for (const auto& vertex : mergedVertices) {
        const auto& findNew = oldToNew.find(vertex);
        if (oldToNew.end() != findNew)
            compactedMergedVertices.insert(findNew->second);
    }

    std::cerr << "Merge shared five edge faces:" << mergeCount << std::endl;
    rebuildHalfEdges();

    // The two pentagons closed up around the merged point, pull the patch back
    // onto the source mesh
    smoothAroundVertices(compactedMergedVertices, 3, 5);
}

}
