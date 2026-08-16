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
#include <AutoRemesher/MeshSeparator>
#include <algorithm>
#include <limits>
#include <queue>
#include <unordered_set>
#if defined(__has_include)
#if __has_include(<oneapi/tbb/parallel_sort.h>)
#include <oneapi/tbb/parallel_sort.h>
#else
#include <tbb/parallel_sort.h>
#endif
#else
#include <tbb/parallel_sort.h>
#endif

namespace AutoRemesher {

namespace {

    // A directed edge packed into one key, so the adjacency lookup can be a
    // sorted array instead of a red black tree with a node per edge.  Vertex
    // indices above 2^32 cannot be packed, and the caller falls back to the tree
    // in that case.
    const size_t maximumPackableVertexIndex = 0xffffffffu;

    inline uint64_t packDirectedEdge(size_t from, size_t to)
    {
        return ((uint64_t)from << 32) | (uint64_t)to;
    }

    bool buildPackedEdgeToFaceTable(const std::vector<std::vector<size_t>>& faces,
        std::vector<std::pair<uint64_t, size_t>>& table)
    {
        size_t edgeCount = 0;
        for (const auto& face : faces)
            edgeCount += face.size();
        table.clear();
        table.reserve(edgeCount);
        for (size_t index = 0; index < faces.size(); ++index) {
            const auto& face = faces[index];
            for (size_t i = 0; i < face.size(); i++) {
                size_t j = (i + 1) % face.size();
                if (face[i] > maximumPackableVertexIndex || face[j] > maximumPackableVertexIndex)
                    return false;
                table.push_back({ packDirectedEdge(face[i], face[j]), index });
            }
        }
        // Sorting by (edge, face) leaves the highest face index for a repeated
        // directed edge last, which is the face the map form used to keep: it
        // assigned in ascending face order and let the last write win.
        tbb::parallel_sort(table.begin(), table.end());
        return true;
    }

    size_t findFaceOfDirectedEdge(const std::vector<std::pair<uint64_t, size_t>>& table,
        size_t from, size_t to)
    {
        const uint64_t key = packDirectedEdge(from, to);
        auto it = std::upper_bound(table.begin(), table.end(), key,
            [](uint64_t value, const std::pair<uint64_t, size_t>& entry) {
                return value < entry.first;
            });
        if (it == table.begin())
            return std::numeric_limits<size_t>::max();
        --it;
        if (it->first != key)
            return std::numeric_limits<size_t>::max();
        return it->second;
    }

}

void MeshSeparator::splitToIslands(const std::vector<std::vector<size_t>>& faces,
    std::vector<std::vector<std::vector<size_t>>>& islands)
{
    std::vector<std::pair<uint64_t, size_t>> packedEdgeToFace;
    std::map<std::pair<size_t, size_t>, size_t> edgeToFaceMap;
    const bool packed = buildPackedEdgeToFaceTable(faces, packedEdgeToFace);
    if (!packed)
        buildEdgeToFaceMap(faces, edgeToFaceMap);

    const auto oppositeFaceOf = [&](size_t from, size_t to) {
        if (packed)
            return findFaceOfDirectedEdge(packedEdgeToFace, from, to);
        auto found = edgeToFaceMap.find({ from, to });
        return found == edgeToFaceMap.end() ? std::numeric_limits<size_t>::max() : found->second;
    };

    // The flood fill stays serial and visits faces in the same order as before,
    // so the islands and the faces inside them come out unchanged.
    std::vector<char> processedFaces(faces.size(), 0);
    std::queue<size_t> waitFaces;
    for (size_t indexInGroup = 0; indexInGroup < faces.size(); ++indexInGroup) {
        if (processedFaces[indexInGroup])
            continue;
        waitFaces.push(indexInGroup);
        std::vector<std::vector<size_t>> island;
        while (!waitFaces.empty()) {
            size_t index = waitFaces.front();
            waitFaces.pop();
            if (processedFaces[index])
                continue;
            const auto& face = faces[index];
            for (size_t i = 0; i < face.size(); i++) {
                size_t j = (i + 1) % face.size();
                const size_t oppositeFace = oppositeFaceOf(face[j], face[i]);
                if (std::numeric_limits<size_t>::max() == oppositeFace)
                    continue;
                waitFaces.push(oppositeFace);
            }
            island.push_back(faces[index]);
            processedFaces[index] = 1;
        }
        if (island.empty())
            continue;
        islands.push_back(std::move(island));
    }
}

void MeshSeparator::buildEdgeToFaceMap(const std::vector<std::vector<size_t>>& faces,
    std::map<std::pair<size_t, size_t>, size_t>& edgeToFaceMap)
{
    edgeToFaceMap.clear();
    for (size_t index = 0; index < faces.size(); ++index) {
        const auto& face = faces[index];
        for (size_t i = 0; i < face.size(); i++) {
            size_t j = (i + 1) % face.size();
            edgeToFaceMap[{ face[i], face[j] }] = index;
        }
    }
}

}
