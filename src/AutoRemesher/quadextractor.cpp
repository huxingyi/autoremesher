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
    std::unordered_set<size_t> intersections;
    extractConnections(&crossPoints, &links, &intersections);
    
    std::unordered_map<size_t, std::unordered_set<size_t>> nextPointMap;
    for (const auto &it: links) {
        nextPointMap[it.first].insert(it.second);
        nextPointMap[it.second].insert(it.first);
    }
    
    std::vector<Vector3> intersectionVertices;
    intersectionVertices.reserve(intersections.size());
    std::unordered_map<size_t, size_t> intersectionOldToNewMap;
    for (const auto &it: intersections) {
        intersectionOldToNewMap.insert({it, intersectionVertices.size()});
        intersectionVertices.push_back(crossPoints[it]);
    }
    
    std::vector<std::pair<size_t, size_t>> halfEdges;
    for (const auto &intersection: intersections) {
        //std::cerr << "Processing intersection:" << intersection << std::endl;
        auto findAroundIntersection = nextPointMap.find(intersection);
        if (findAroundIntersection == nextPointMap.end())
            continue;
        for (const auto &pointAround: findAroundIntersection->second) {
            size_t pointIndex = pointAround;
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
            if (intersection != pointIndex && 
                    intersections.end() != intersections.find(pointIndex)) {
                //std::cerr << "Halfedge:" << intersection << "~" << pointIndex << std::endl;
                halfEdges.push_back({intersectionOldToNewMap[intersection], intersectionOldToNewMap[pointIndex]});
            }
        }
    }
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor-halfedges.obj", "wb");
        for (size_t i = 0; i < intersectionVertices.size(); ++i) {
            const auto &vertex = intersectionVertices[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: halfEdges) {
            fprintf(fp, "l %zu %zu\n", 1 + it.first, 1 + it.second);
        }
        fclose(fp);
    }
#endif

    return true;
}

void QuadExtractor::extractConnections(std::vector<Vector3> *positions, 
    std::set<std::pair<size_t, size_t>> *links,
    std::unordered_set<size_t> *intersections)
{
    struct CrossContext
    {
        size_t crossPositionIndex;
        Vector2 uvPosition;
    };
    
    std::vector<Vector3> &crossPoints = *positions;
    std::map<PositionKey, size_t> crossPointMap;
    std::set<std::pair<size_t, size_t>> intersectionLinks;
    std::set<std::pair<size_t, size_t>> canceledLinks;
    auto findCrossPoints = [&](std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> &crossLinks, 
            size_t coordIndex,
            const std::map<size_t, std::vector<std::pair<CrossContext, CrossContext>>> *perpendicularCrossLinks=nullptr) {
        for (size_t triangleIndex = 0; triangleIndex < m_triangles->size(); ++triangleIndex) {
            const auto &cornerUvs = (*m_triangleUvs)[triangleIndex];
            const auto &cornerIndices = (*m_triangles)[triangleIndex];
            std::unordered_map<int, std::vector<CrossContext>> integerLink;
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
                        Vector3 position3 = (*m_vertices)[cornerIndices[fromIndex]] * (1 - ratio) +
                            (*m_vertices)[cornerIndices[toIndex]] * ratio;
                        Vector2 position2 = cornerUvs[fromIndex] * (1 - ratio) + cornerUvs[toIndex] * ratio;
                        auto insertResult = crossPointMap.insert({position3, crossPoints.size()});
                        if (insertResult.second)
                            crossPoints.push_back(position3);
                        integerLink[integer].push_back({insertResult.first->second, position2});
                    }
                }
            }
            const std::vector<std::pair<CrossContext, CrossContext>> *perpendicularLines = nullptr;
            if (nullptr != perpendicularCrossLinks) {
                auto findResult = perpendicularCrossLinks->find(triangleIndex);
                if (findResult != perpendicularCrossLinks->end())
                    perpendicularLines = &findResult->second;
            }
            for (const auto &it: integerLink) {
                if (it.second.size() >= 2) {
                    bool foundIntersection = false;
                    if (nullptr != perpendicularLines) {
                        const auto &uv0 = it.second[0].uvPosition;
                        const auto &uv1 = it.second[1].uvPosition;
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
                                fromIndex = 0;
                                toIndex = 1;
                            } else {
                                fromPosition = uv1[perpendicularCoordIndex];
                                toPosition = uv0[perpendicularCoordIndex];
                                fromIndex = 1;
                                toIndex = 0;
                            }
                            if (segmentPosition < fromPosition || segmentPosition > toPosition)
                                continue;
                            double ratio = distance > 0 ? (segmentPosition - fromPosition) / distance : 0.5;
                            Vector3 position3 = crossPoints[it.second[fromIndex].crossPositionIndex] * (1 - ratio) +
                                crossPoints[it.second[toIndex].crossPositionIndex] * ratio;
                            //std::cerr << "Intersection ratio:" << ratio << " position3:" << position3 << std::endl;
                            auto insertResult = crossPointMap.insert({position3, crossPoints.size()});
                            if (insertResult.second)
                                crossPoints.push_back(position3);
                            auto newPositionIndex = insertResult.first->second;
                            intersections->insert(newPositionIndex);
                            intersectionLinks.insert({it.second[0].crossPositionIndex, newPositionIndex});
                            intersectionLinks.insert({newPositionIndex, it.second[1].crossPositionIndex});
                            intersectionLinks.insert({perpendicularLine.first.crossPositionIndex, newPositionIndex});
                            intersectionLinks.insert({newPositionIndex, perpendicularLine.second.crossPositionIndex});
                            if (perpendicularLine.first.crossPositionIndex != newPositionIndex &&
                                    newPositionIndex != perpendicularLine.second.crossPositionIndex)
                                canceledLinks.insert({perpendicularLine.first.crossPositionIndex, perpendicularLine.second.crossPositionIndex});
                            foundIntersection = true;
                        }
                    }
                    if (!foundIntersection)
                        crossLinks[triangleIndex].push_back({it.second[0], it.second[1]});
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