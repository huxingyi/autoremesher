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

namespace AutoRemesher
{

bool QuadExtractor::extract()
{
    std::vector<Vector3> crossPoints;
    std::set<std::pair<size_t, size_t>> links;
    std::unordered_map<size_t, Intersection> intersections;
    extractConnections(&crossPoints, &links, &intersections);
    
    std::unordered_map<size_t, std::unordered_set<size_t>> edgeConnectMap;
    extractEdges(intersections, links, &edgeConnectMap);
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-edges.obj", "wb");
        for (size_t i = 0; i < crossPoints.size(); ++i) {
            const auto &vertex = crossPoints[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: edgeConnectMap) {
            for (const auto &it2: it.second)
                fprintf(fp, "l %zu %zu\n", 1 + it.first, 1 + it2);
        }
        fclose(fp);
    }
#endif
    
    extractMesh(crossPoints, intersections, edgeConnectMap, &m_remeshedQuads);
    
    m_remeshedVertices.reserve(intersections.size());
    std::unordered_map<size_t, size_t> intersectionOldToNewMap;
    for (const auto &it: intersections) {
        intersectionOldToNewMap.insert({it.first, m_remeshedVertices.size()});
        m_remeshedVertices.push_back(crossPoints[it.first]);
    }
    for (auto &it: m_remeshedQuads) {
        it[0] = intersectionOldToNewMap[it[0]];
        it[1] = intersectionOldToNewMap[it[1]];
        it[2] = intersectionOldToNewMap[it[2]];
        it[3] = intersectionOldToNewMap[it[3]];
    }
    
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

void QuadExtractor::extractEdges(const std::unordered_map<size_t, Intersection> &intersections,
        const std::set<std::pair<size_t, size_t>> &links,
        std::unordered_map<size_t, std::unordered_set<size_t>> *edgeConnectMap)
{
    std::unordered_map<size_t, std::unordered_set<size_t>> nextPointMap;
    for (const auto &it: links) {
        nextPointMap[it.first].insert(it.second);
        nextPointMap[it.second].insert(it.first);
    }

    for (const auto &intersectionIt: intersections) {
        const auto &intersection = intersectionIt.first;
        //std::cerr << "Processing intersection:" << intersection << std::endl;
        auto findAroundIntersection = nextPointMap.find(intersection);
        if (findAroundIntersection == nextPointMap.end())
            continue;
        for (const auto &pointAround: findAroundIntersection->second) {
            size_t pointIndex = pointAround;
            if (intersections.end() == intersections.find(pointIndex)) {
                std::unordered_set<size_t> visited;
                visited.insert(intersection);
                for (;;) {
                    visited.insert(pointIndex);
                    //std::cerr << "Loop pointIndex:" << pointIndex << std::endl;
                    auto findNext = nextPointMap.find(pointIndex);
                    if (findNext == nextPointMap.end()) {
                        //std::cerr << "Failed to find next of pointIndex:" << pointIndex << std::endl;
                        break;
                    }
                    bool foundIntersection = false;
                    std::vector<size_t> neighbors;
                    for (const auto &it: findNext->second) {
                        if (visited.end() != visited.find(it))
                            continue;
                        if (intersections.end() != intersections.find(it)) {
                            //std::cerr << "Found next insersection of pointIndex:" << pointIndex << " result:" << it << std::endl;
                            pointIndex = it;
                            foundIntersection = true;
                            break;
                        }
                        neighbors.push_back(it);
                    }
                    if (foundIntersection)
                        break;
                    if (1 != neighbors.size()) {
                        std::cerr << "Found multiple next neighbors of pointIndex:" << pointIndex << " num:" << neighbors.size() << std::endl;
                        break;
                    }
                    pointIndex = neighbors[0];
                }
            }
            if (intersection != pointIndex && 
                    intersections.end() != intersections.find(pointIndex)) {
                //std::cerr << "Halfedge:" << intersection << "~" << pointIndex << std::endl;
                (*edgeConnectMap)[intersection].insert(pointIndex);
                (*edgeConnectMap)[pointIndex].insert(intersection);
            }
        }
    }
}

void QuadExtractor::extractMesh(const std::vector<Vector3> &points,
        const std::unordered_map<size_t, Intersection> &intersections,
        const std::unordered_map<size_t, std::unordered_set<size_t>> &edgeConnectMap,
        std::vector<std::vector<size_t>> *quads)
{
    quads->reserve(intersections.size());
    std::set<std::tuple<size_t, size_t, size_t, size_t>> candidates;
    for (const auto &intersectionIt: intersections) {
        const auto &level0 = intersectionIt.first;
        const auto &intersection = intersectionIt.second;
        auto findLevel1 = edgeConnectMap.find(level0);
        if (findLevel1 == edgeConnectMap.end())
            continue;
        const auto &triangleVertices = (*m_triangles)[intersection.sourceTriangleIndex];
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
                        if (level0 != level4)
                            continue;
                        std::vector<size_t> allLevels = {level0, level1, level2, level3};
                        std::sort(allLevels.begin(), allLevels.end());
                        auto insertResult = candidates.insert({allLevels[0], allLevels[1], allLevels[2], allLevels[3]});
                        if (insertResult.second) {
                            auto quadNormal = Vector3::normal(points[level0], points[level1], points[level2]);
                            if (Vector3::dotProduct(quadNormal, triangleNormal) > 0) {
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
}

void QuadExtractor::extractConnections(std::vector<Vector3> *positions, 
    std::set<std::pair<size_t, size_t>> *links,
    std::unordered_map<size_t, Intersection> *intersections)
{
    struct CrossContext
    {
        size_t crossPositionIndex;
        Vector2 uvPosition;
        int integer;
    };
    
    std::vector<Vector3> &crossPoints = *positions;
    std::map<PositionKey, size_t> crossPointMap;
    std::set<std::pair<size_t, size_t>> intersectionLinks;
    std::set<std::pair<size_t, size_t>> canceledLinks;
    
    auto addCrossPoint = [&](const Vector3 &position3, size_t triangleIndex) {
        auto insertResult = crossPointMap.insert({position3, crossPoints.size()});
        if (insertResult.second)
            crossPoints.push_back(position3);
        return insertResult;
    };
    
    auto findCrossPoints = [&](std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> &crossLinks, 
            size_t coordIndex,
            const std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> *perpendicularCrossLinks=nullptr) {
        for (size_t triangleIndex = 0; triangleIndex < m_triangles->size(); ++triangleIndex) {
            const auto &cornerUvs = (*m_triangleUvs)[triangleIndex];
            const auto &cornerIndices = (*m_triangles)[triangleIndex];
            std::unordered_map<int, std::vector<CrossContext>> integerLink;
            //std::cerr << "======== Processing triangleIndex:" << triangleIndex << " ==============" << std::endl;
            for (size_t j = 0; j < 3; ++j) {
                size_t k = (j + 1) % 3;
                const auto &current = cornerUvs[j];
                const auto &next = cornerUvs[k];
                if ((Double::isZero((double)(int)current[coordIndex] - current[coordIndex]) && 
                        Double::isZero(current[coordIndex] - next[coordIndex]))) {
                    // Triangle edge collapse with isoline
                    int integer = (int)current[coordIndex];
                    //std::cerr << "j:" << j << " Triangle edge collapse with isoline integer:" << integer << std::endl;
                    {
                        const Vector3 &position3 = (*m_vertices)[cornerIndices[j]];
                        const Vector2 &position2 = cornerUvs[j];
                        auto insertResult = addCrossPoint(position3, triangleIndex);
                        //std::cerr << "  first: integer:" << integer << " crossPoint:" << insertResult.first->second << " position2:" << position2 << std::endl;
                        integerLink[integer].push_back({insertResult.first->second, position2, integer});
                    }
                    {
                        const Vector3 &position3 = (*m_vertices)[cornerIndices[k]];
                        const Vector2 &position2 = cornerUvs[k];
                        auto insertResult = addCrossPoint(position3, triangleIndex);
                        //std::cerr << "  second: integer:" << integer << " crossPoint:" << insertResult.first->second << " position2:" << position2 << std::endl;
                        integerLink[integer].push_back({insertResult.first->second, position2, integer});
                    }
                    break;
                }
            }
            if (integerLink.empty()) {
                bool isCornerIntegers[3] = {false, false, false};
                for (size_t j = 0; j < 3; ++j) {
                    size_t k = (j + 1) % 3;
                    const auto &current = cornerUvs[j];
                    const auto &next = cornerUvs[k];
                    double distance = std::abs(current[coordIndex] - next[coordIndex]);
                    if ((int)current[coordIndex] != (int)next[coordIndex] ||
                            (current[coordIndex] > 0) != (next[coordIndex] > 0)) {
                        int lowInteger, highInteger;
                        double fromPosition;
                        double toPosition;
                        size_t fromIndex, toIndex;
                        if (current[coordIndex] < next[coordIndex]) {
                            lowInteger = (int)current[coordIndex];
                            highInteger = (int)next[coordIndex];
                            fromPosition = current[coordIndex];
                            toPosition = next[coordIndex];
                            fromIndex = j;
                            toIndex = k;
                        } else {
                            lowInteger = (int)next[coordIndex];
                            highInteger = (int)current[coordIndex];
                            fromPosition = next[coordIndex];
                            toPosition = current[coordIndex];
                            fromIndex = k;
                            toIndex = j;
                        }
                        for (int integer = lowInteger; integer <= highInteger; ++integer) {
                            double ratio = (integer - fromPosition) / distance;
                            if (ratio < 0 || ratio > 1)
                                continue;
                            if (Double::isZero(ratio))
                                isCornerIntegers[fromIndex] = true;
                            if (Double::isZero(ratio - 1.0))
                                isCornerIntegers[toIndex] = true;
                            Vector3 position3 = (*m_vertices)[cornerIndices[fromIndex]] * (1 - ratio) +
                                (*m_vertices)[cornerIndices[toIndex]] * ratio;
                            Vector2 position2 = cornerUvs[fromIndex] * (1 - ratio) + cornerUvs[toIndex] * ratio;
                            auto insertResult = addCrossPoint(position3, triangleIndex);
                            //std::cerr << "j:" << j << " integer:" << integer << " crossPoint:" << insertResult.first->second << " position2:" << position2 << std::endl;
                            integerLink[integer].push_back({insertResult.first->second, position2, integer});
                        }
                    }
                }
                if (isCornerIntegers[0] && isCornerIntegers[1] && isCornerIntegers[2])
                    integerLink.clear();
            }
            const std::vector<std::pair<CrossContext, CrossContext>> *perpendicularLines = nullptr;
            if (nullptr != perpendicularCrossLinks) {
                auto findResult = perpendicularCrossLinks->find(triangleIndex);
                if (findResult != perpendicularCrossLinks->end())
                    perpendicularLines = &findResult->second;
            }
            for (const auto &it: integerLink) {
                //if (it.second.size() >= 3) {
                //    std::cerr << "triangleIndex:" << triangleIndex << " integerLink size:" << it.second.size() << " integer:" << it.first << std::endl;
                //    for (size_t i = 0; i < it.second.size(); ++i) {
                //        std::cerr << "["<<i<<"]:" << "crossPoint:" << it.second[i].crossPositionIndex << " uvPosition:" << it.second[i].uvPosition << std::endl;
                //    }
                //}
                if (it.second.size() >= 2) {
                    for (size_t pointIndex = 0; pointIndex < it.second.size(); ++pointIndex) {
                        size_t nextPointIndex = (pointIndex + 1) % it.second.size();
                        crossLinks[triangleIndex].push_back({it.second[pointIndex], it.second[nextPointIndex]});
                        if (nullptr != perpendicularLines) {
                            const auto &uv0 = it.second[pointIndex].uvPosition;
                            const auto &uv1 = it.second[nextPointIndex].uvPosition;
                            auto perpendicularCoordIndex = (coordIndex + 1) % 2;
                            double distance = std::abs(uv0[perpendicularCoordIndex] - uv1[perpendicularCoordIndex]);
                            for (const auto &perpendicularLine: *perpendicularLines) {
                                double segmentPosition = perpendicularLine.first.uvPosition[perpendicularCoordIndex];
                                double fromPosition;
                                double toPosition;
                                size_t fromIndex;
                                size_t toIndex;
                                if (uv0[perpendicularCoordIndex] < uv1[perpendicularCoordIndex]) {
                                    fromPosition = uv0[perpendicularCoordIndex];
                                    toPosition = uv1[perpendicularCoordIndex];
                                    fromIndex = pointIndex;
                                    toIndex = nextPointIndex;
                                } else {
                                    fromPosition = uv1[perpendicularCoordIndex];
                                    toPosition = uv0[perpendicularCoordIndex];
                                    fromIndex = nextPointIndex;
                                    toIndex = pointIndex;
                                }
                                if (segmentPosition < fromPosition || 
                                        segmentPosition > toPosition)
                                    continue;
                                double ratio = distance > 0 ? (segmentPosition - fromPosition) / distance : 0.5;
                                Vector3 position3 = crossPoints[it.second[fromIndex].crossPositionIndex] * (1 - ratio) +
                                    crossPoints[it.second[toIndex].crossPositionIndex] * ratio;
                                //std::cerr << "Intersection ratio:" << ratio << " position3:" << position3 << std::endl;
                                auto insertResult = addCrossPoint(position3, triangleIndex);
                                auto newPositionIndex = insertResult.first->second;
                                intersections->insert({newPositionIndex, {triangleIndex}});
                                intersectionLinks.insert({it.second[pointIndex].crossPositionIndex, newPositionIndex});
                                intersectionLinks.insert({newPositionIndex, it.second[nextPointIndex].crossPositionIndex});
                                if (it.second[pointIndex].crossPositionIndex != newPositionIndex &&
                                        newPositionIndex != it.second[nextPointIndex].crossPositionIndex) {
                                    canceledLinks.insert({it.second[pointIndex].crossPositionIndex, it.second[nextPointIndex].crossPositionIndex});
                                }
                                intersectionLinks.insert({perpendicularLine.first.crossPositionIndex, newPositionIndex});
                                intersectionLinks.insert({newPositionIndex, perpendicularLine.second.crossPositionIndex});
                                if (perpendicularLine.first.crossPositionIndex != newPositionIndex &&
                                        newPositionIndex != perpendicularLine.second.crossPositionIndex) {
                                    canceledLinks.insert({perpendicularLine.first.crossPositionIndex, perpendicularLine.second.crossPositionIndex});
                                }
                            }
                        }
                    }
                }
            }
        }
    };
    std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> xCrossLinks;
    std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> yCrossLinks;
    findCrossPoints(xCrossLinks, 0);
    findCrossPoints(yCrossLinks, 1, &xCrossLinks);

    auto &mergedLinks = *links;
    mergedLinks = intersectionLinks;
    auto mergeLink = [&](const std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> &crossLinks) {
        for (const auto &it: crossLinks) {
            for (const auto &subIt: it.second) {
                if (subIt.first.crossPositionIndex == subIt.second.crossPositionIndex)
                    continue;
                mergedLinks.insert({subIt.first.crossPositionIndex, subIt.second.crossPositionIndex});
            }
        }
    };
    mergeLink(xCrossLinks);
    mergeLink(yCrossLinks);
    for (const auto &it: canceledLinks)
        mergedLinks.erase(it);
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-connections.obj", "wb");
        for (size_t i = 0; i < crossPoints.size(); ++i) {
            const auto &vertex = crossPoints[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: mergedLinks) {
            fprintf(fp, "l %zu %zu\n", 1 + it.first, 1 + it.second);
        }
        fclose(fp);
    }
#endif
}
    
}