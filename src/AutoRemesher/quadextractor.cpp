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
#include <map>

namespace AutoRemesher
{

bool QuadExtractor::extract()
{
    struct CrossContext
    {
        size_t crossPositionIndex;
        Vector2 uvPosition;
    };
    
    std::vector<Vector3> crossPoints;
    std::map<PositionKey, size_t> crossPointMap;
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
                if ((int)current[coordIndex] != (int)next[coordIndex]) {
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
                    crossLinks[triangleIndex].push_back({it.second[0], it.second[1]});
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
                            double ratio = (segmentPosition - fromPosition) / distance;
                            Vector3 position3 = crossPoints[it.second[fromIndex].crossPositionIndex] * (1 - ratio) +
                                crossPoints[it.second[toIndex].crossPositionIndex] * ratio;
                            auto insertResult = crossPointMap.insert({position3, crossPoints.size()});
                            if (insertResult.second)
                                crossPoints.push_back(position3);
                            auto newPositionIndex = insertResult.first->second;
                            std::cerr << "newPositionIndex:" << newPositionIndex << std::endl;
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
    
#if AUTO_REMESHER_DEV
    {
        FILE *fp = fopen("debug-quadextractor.obj", "wb");
        for (size_t i = 0; i < crossPoints.size(); ++i) {
            const auto &vertex = crossPoints[i];
            fprintf(fp, "v %f %f %f\n", vertex.x(), vertex.y(), vertex.z());
        }
        for (const auto &it: xCrossLinks) {
            for (const auto &subIt: it.second)
                fprintf(fp, "l %zu %zu\n", 1 + subIt.first.crossPositionIndex, 1 + subIt.second.crossPositionIndex);
        }
        for (const auto &it: yCrossLinks) {
            for (const auto &subIt: it.second)
                fprintf(fp, "l %zu %zu\n", 1 + subIt.first.crossPositionIndex, 1 + subIt.second.crossPositionIndex);
        }
        fclose(fp);
    }
#endif
}
    
}