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
#ifndef AUTO_REMESHER_HALF_EDGE_H
#define AUTO_REMESHER_HALF_EDGE_H
#include <vector>
#include <cstddef>
#include <queue>
#include <limits>
#include <AutoRemesher/Vector3>
#include <AutoRemesher/Vector2>

namespace AutoRemesher
{
    
namespace HalfEdge
{
    
struct Vertex;
struct HalfEdge;
struct Face;

struct Vertex
{
    Vertex *_previous = nullptr;
    Vertex *_next = nullptr;
    size_t index;
    size_t outputIndex;
    Vector3 position;
    HalfEdge *anyHalfEdge = nullptr;
    size_t halfEdgeCount = 0;
    Vector3 normal;
    Vector3 averageNormal;
    bool hasRelativeHeight = false;
    double relativeHeight = 0.0;
#if AUTO_REMESHER_DEV
    unsigned char debugColor = 0;
#endif
};

struct HalfEdge
{
    HalfEdge *_previous = nullptr;
    HalfEdge *_next = nullptr;
    size_t index;
    Vertex *startVertex = nullptr;
    Face *leftFace = nullptr;
    HalfEdge *previousHalfEdge = nullptr;
    HalfEdge *nextHalfEdge = nullptr;
    HalfEdge *oppositeHalfEdge = nullptr;
    Vector2 startVertexUv;
};

struct Face
{
    Face *_previous = nullptr;
    Face *_next = nullptr;
    HalfEdge *anyHalfEdge = nullptr;
    Vector3 normal;
};

class Mesh
{
public:
    Mesh(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles);
    ~Mesh();
    Vertex *allocVertex();
    Face *allocFace();
    HalfEdge *allocHalfEdge();
    void freeVertex(Vertex *vertex);
    void deferedFreeVertex(Vertex *vertex);
    void freeFace(Face *face);
    void freeHalfEdge(HalfEdge *halfEdge);
    void deferedFreeHalfEdge(HalfEdge *halfEdge);
    void deferedFreeFace(Face *face);
    bool flip(HalfEdge *halfEdge);
    bool isWatertight();
    const size_t &vertexCount() const;
    const size_t &faceCount() const;
    Vertex *firstVertex() const;
    Face *firstFace() const;
    void calculateFaceNormals();
    void calculateVertexNormals();
    void calculateVertexAverageNormals();
    void calculateVertexRelativeHeights();
    void normalizeVertexRelativeHeights();
    void removeZeroAngleTriangles();
    void orderVertexByFlatness();
    const std::vector<Vertex *> &vertexOrderedByFlatness();
#if AUTO_REMESHER_DEV
    void debugExportRelativeHeightPly(const char *filename);
    void debugExportLimitRelativeHeightPly(const char *filename, const std::pair<double, double> &limitRelativeHeight);
    void debugExportPly(const char *filename);
#endif
    
private:

    Vertex *m_firstVertex = nullptr;
    Vertex *m_lastVertex = nullptr;
    Vertex *m_firstDeferedRemovalVertex = nullptr;
    Face *m_firstFace = nullptr;
    Face *m_lastFace = nullptr;
    HalfEdge *m_firstHalfEdge = nullptr;
    HalfEdge *m_lastHalfEdge = nullptr;
    HalfEdge *m_firstDeferedRemovalHalfEdge = nullptr;
    Face *m_firstDeferedRemovalFace = nullptr;
    size_t m_repeatedHalfEdges = 0;
    size_t m_aloneHalfEdges = 0;
    size_t m_vertexCount = 0;
    size_t m_faceCount = 0;
    size_t m_halfEdgeCount = 0;
    std::vector<Vertex *> m_vertexOrderedByFlatness;
};

}
    
}

#endif
