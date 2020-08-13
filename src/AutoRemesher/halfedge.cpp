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
#include <map>
#include <cassert>
#include <iostream>
#include <set>
#include <unordered_map>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/Radians>
#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

namespace AutoRemesher
{

namespace HalfEdge
{
    
inline void makeLinkedHalfEdges(HalfEdge *previous, HalfEdge *next) 
{
    previous->nextHalfEdge = next;
    next->previousHalfEdge = previous;
}

Mesh::Mesh(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles)
{
    std::unordered_map<size_t, Vertex *> vertexMap;
    auto createVertex = [&](size_t index) {
        auto findVertex = vertexMap.find(index);
        if (findVertex != vertexMap.end())
            return findVertex->second;
        Vertex *vertex = allocVertex();
        vertex->index = vertexMap.size();
        vertex->position = vertices[index];
        vertexMap.insert({index, vertex});
        return vertex;
    };
    
    std::map<std::pair<size_t, size_t>, HalfEdge *> halfEdgeIndexMap;
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto &triangleIndices = triangles[i];
        bool foundRepeatedHalfEdge = false;
        for (size_t j = 0; j < 3; ++j) {
            size_t k = (j + 1) % 3;
            const auto &vertexIndex = triangleIndices[j];
            const auto &nextVertexIndex = triangleIndices[k];
            if (halfEdgeIndexMap.end() != halfEdgeIndexMap.find({vertexIndex, nextVertexIndex})) {
                std::cerr << "Found repeated halfedge" << std::endl;
                foundRepeatedHalfEdge = true;
                break;
            }
        }
        if (foundRepeatedHalfEdge)
            continue;
        
        auto face = allocFace();
        std::vector<HalfEdge *> halfEdges = {
            allocHalfEdge(),
            allocHalfEdge(),
            allocHalfEdge()
        };
        face->anyHalfEdge = halfEdges[0];
        for (size_t j = 0; j < 3; ++j) {
            size_t k = (j + 1) % 3;
            size_t h = (k + 1) % 3;
            auto &halfEdge = halfEdges[j];
            const auto &vertexIndex = triangleIndices[j];
            const auto &nextVertexIndex = triangleIndices[k];
            Vertex *vertex = createVertex(vertexIndex);
            vertex->anyHalfEdge = halfEdge;
            ++vertex->halfEdgeCount;
            halfEdge->startVertex = vertex;
            halfEdge->previousHalfEdge = halfEdges[h];
            halfEdge->nextHalfEdge = halfEdges[k];
            halfEdge->leftFace = face;
            auto insertResult = halfEdgeIndexMap.insert({{vertexIndex, nextVertexIndex}, halfEdge});
            if (!insertResult.second) {
                std::cerr << "Found repeated halfedge" << std::endl;
                ++m_repeatedHalfEdges;
            }
        }
    }
    for (auto &it: halfEdgeIndexMap) {
        auto findOppositeResult = halfEdgeIndexMap.find({it.first.second, it.first.first});
        if (findOppositeResult == halfEdgeIndexMap.end()) {
            std::cerr << "Found alone halfedge" << std::endl;
            ++m_aloneHalfEdges;
            continue;
        }
        it.second->oppositeHalfEdge = findOppositeResult->second;
    }
    
    if (isWatertight()) {
        removeZeroAngleTriangles();
    }
    
#if AUTO_REMESHER_DEBUG
    qDebug() << "calculateFaceNormals";
#endif
    calculateFaceNormals();
#if AUTO_REMESHER_DEBUG
    qDebug() << "calculateVertexNormals";
#endif
    calculateVertexNormals();
#if AUTO_REMESHER_DEBUG
    qDebug() << "calculateVertexAverageNormals";
#endif
    calculateVertexAverageNormals();
#if AUTO_REMESHER_DEBUG
    qDebug() << "calculateVertexRelativeHeights";
#endif
    calculateVertexRelativeHeights();
#if AUTO_REMESHER_DEBUG
    qDebug() << "normalizeVertexRelativeHeights";
#endif
    normalizeVertexRelativeHeights();
#if AUTO_REMESHER_DEBUG
    qDebug() << "HalfEdge created";
#endif
}

Mesh::~Mesh()
{
    while (nullptr != m_lastVertex)
        freeVertex(m_lastVertex);
    while (nullptr != m_lastFace)
        freeFace(m_lastFace);
    while (nullptr != m_lastHalfEdge)
        freeHalfEdge(m_lastHalfEdge);
    while (nullptr != m_firstDeferedRemovalVertex) {
        auto vertex = m_firstDeferedRemovalVertex;
        m_firstDeferedRemovalVertex = vertex->_next;
        delete vertex;
    }
    while (nullptr != m_firstDeferedRemovalHalfEdge) {
        auto halfEdge = m_firstDeferedRemovalHalfEdge;
        m_firstDeferedRemovalHalfEdge = halfEdge->_next;
        delete halfEdge;
    }
    while (nullptr != m_firstDeferedRemovalFace) {
        auto face = m_firstDeferedRemovalFace;
        m_firstDeferedRemovalFace = face->_next;
        delete face;
    }
}

void Mesh::deferedFreeVertex(Vertex *vertex)
{
    if (vertex == m_firstVertex)
        m_firstVertex = vertex->_next;
    if (vertex == m_lastVertex)
        m_lastVertex = vertex->_previous;
    if (nullptr != vertex->_next)
        vertex->_next->_previous = vertex->_previous;
    if (nullptr != vertex->_previous)
        vertex->_previous->_next = vertex->_next;
    --m_vertexCount;
    vertex->_next = m_firstDeferedRemovalVertex;
    m_firstDeferedRemovalVertex = vertex;
}

void Mesh::freeVertex(Vertex *vertex)
{
    if (vertex == m_firstVertex)
        m_firstVertex = vertex->_next;
    if (vertex == m_lastVertex)
        m_lastVertex = vertex->_previous;
    if (nullptr != vertex->_next)
        vertex->_next->_previous = vertex->_previous;
    if (nullptr != vertex->_previous)
        vertex->_previous->_next = vertex->_next;
    --m_vertexCount;
    delete vertex;
}

void Mesh::freeFace(Face *face)
{
    if (face == m_firstFace)
        m_firstFace = face->_next;
    if (face == m_lastFace)
        m_lastFace = face->_previous;
    if (nullptr != face->_next)
        face->_next->_previous = face->_previous;
    if (nullptr != face->_previous)
        face->_previous->_next = face->_next;
    --m_faceCount;
    delete face;
}

void Mesh::deferedFreeFace(Face *face)
{
    if (face == m_firstFace)
        m_firstFace = face->_next;
    if (face == m_lastFace)
        m_lastFace = face->_previous;
    if (nullptr != face->_next)
        face->_next->_previous = face->_previous;
    if (nullptr != face->_previous)
        face->_previous->_next = face->_next;
    --m_faceCount;
    face->_next = m_firstDeferedRemovalFace;
    m_firstDeferedRemovalFace = face;
}

void Mesh::deferedFreeHalfEdge(HalfEdge *halfEdge)
{
    halfEdge->leftFace = nullptr;
    if (halfEdge == m_firstHalfEdge)
        m_firstHalfEdge = halfEdge->_next;
    if (halfEdge == m_lastHalfEdge)
        m_lastHalfEdge = halfEdge->_previous;
    if (nullptr != halfEdge->_next)
        halfEdge->_next->_previous = halfEdge->_previous;
    if (nullptr != halfEdge->_previous)
        halfEdge->_previous->_next = halfEdge->_next;
    halfEdge->_next = m_firstDeferedRemovalHalfEdge;
    m_firstDeferedRemovalHalfEdge = halfEdge;
}

void Mesh::freeHalfEdge(HalfEdge *halfEdge)
{
    if (halfEdge == m_firstHalfEdge)
        m_firstHalfEdge = halfEdge->_next;
    if (halfEdge == m_lastHalfEdge)
        m_lastHalfEdge = halfEdge->_previous;
    if (nullptr != halfEdge->_next)
        halfEdge->_next->_previous = halfEdge->_previous;
    if (nullptr != halfEdge->_previous)
        halfEdge->_previous->_next = halfEdge->_next;
    delete halfEdge;
}

Vertex *Mesh::allocVertex()
{
    Vertex *vertex = new Vertex;
    vertex->_previous = m_lastVertex;
    if (nullptr != m_lastVertex)
        m_lastVertex->_next = vertex;
    else
        m_firstVertex = vertex;
    m_lastVertex = vertex;
    ++m_vertexCount;
    return vertex;
}

Face *Mesh::allocFace()
{
    Face *face = new Face;
    face->_previous = m_lastFace;
    if (nullptr != m_lastFace)
        m_lastFace->_next = face;
    else
        m_firstFace = face;
    m_lastFace = face;
    ++m_faceCount;
    return face;
}

HalfEdge *Mesh::allocHalfEdge()
{
    HalfEdge *halfEdge = new HalfEdge;
    halfEdge->_previous = m_lastHalfEdge;
    if (nullptr != m_lastHalfEdge)
        m_lastHalfEdge->_next = halfEdge;
    else
        m_firstHalfEdge = halfEdge;
    m_lastHalfEdge = halfEdge;
    halfEdge->index = m_halfEdgeCount;
    ++m_halfEdgeCount;
    return halfEdge;
}

const size_t &Mesh::vertexCount() const
{
    return m_vertexCount;
}

const size_t &Mesh::faceCount() const
{
    return m_faceCount;
}

Vertex *Mesh::firstVertex() const
{
    return m_firstVertex;
}

Face *Mesh::firstFace() const
{
    return m_firstFace;
}

bool Mesh::isWatertight()
{
    return m_faceCount >= 4 && 
        0 == m_repeatedHalfEdges && 
        0 == m_aloneHalfEdges;
}

bool Mesh::flip(HalfEdge *halfEdge)
{
    // cf. <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 6
    
    auto &hflip = halfEdge;
    auto hflip_x = hflip->oppositeHalfEdge;
    auto ha = hflip->previousHalfEdge;
    auto hb = hflip_x->previousHalfEdge;
    auto hc = hflip->nextHalfEdge;
    auto hd = hflip_x->nextHalfEdge;
    
    auto faceA = hflip->leftFace;
    auto faceB = hflip_x->leftFace;
    
    faceA->anyHalfEdge = hflip_x;
    faceB->anyHalfEdge = hflip;
    
    ha->leftFace = faceB;
    hb->leftFace = faceA;
    
    --hflip->startVertex->halfEdgeCount;
    if (hflip->startVertex->anyHalfEdge == hflip)
        hflip->startVertex->anyHalfEdge = hd;
    
    --hflip_x->startVertex->halfEdgeCount;
    if (hflip_x->startVertex->anyHalfEdge == hflip_x)
        hflip_x->startVertex->anyHalfEdge = hc;
    
    ++hb->startVertex->halfEdgeCount;
    ++ha->startVertex->halfEdgeCount;
    
    hflip->startVertex = hb->startVertex;
    hflip_x->startVertex = ha->startVertex;
    
    std::swap(hflip->leftFace, hflip_x->leftFace);
    
    makeLinkedHalfEdges(hc, hflip_x);
    makeLinkedHalfEdges(hflip_x, hb);
    makeLinkedHalfEdges(hb, hc);
    
    makeLinkedHalfEdges(hflip, ha);
    makeLinkedHalfEdges(ha, hd);
    makeLinkedHalfEdges(hd, hflip);
    
    return true;
}

void Mesh::calculateFaceNormals()
{
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        face->normal = Vector3::normal(h0->startVertex->position,
            h1->startVertex->position,
            h2->startVertex->position);
    }
}

void Mesh::calculateVertexNormals()
{
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        HalfEdge *halfEdge = vertex->anyHalfEdge;
        if (nullptr == halfEdge)
            continue;
        do {
            vertex->normal += halfEdge->leftFace->normal;
            if (nullptr == halfEdge->oppositeHalfEdge)
                break;
            halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
        } while (halfEdge && halfEdge != vertex->anyHalfEdge);
        vertex->normal.normalize();
    }
}

void Mesh::calculateVertexAverageNormals()
{
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        HalfEdge *halfEdge = vertex->anyHalfEdge;
        if (nullptr == halfEdge)
            continue;
        do {
            if (nullptr == halfEdge->oppositeHalfEdge)
                break;
            vertex->averageNormal += halfEdge->oppositeHalfEdge->startVertex->normal;
            halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
        } while (halfEdge && halfEdge != vertex->anyHalfEdge);
        vertex->averageNormal.normalize();
        if (vertex->averageNormal.isZero())
            vertex->averageNormal = vertex->normal;
    }
}

void Mesh::calculateVertexRelativeHeights()
{
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        HalfEdge *halfEdge = vertex->anyHalfEdge;
        if (nullptr == halfEdge)
            continue;
        double low = 0.0;
        double high = 0.0;
        bool isBoundary = false;
        auto project = [&](const Vector3 &position) {
            double projectedTo = Vector3::dotProduct((position - vertex->position).normalized(), vertex->averageNormal);
            if (projectedTo < low)
                low = projectedTo;
            if (projectedTo > high)
                high = projectedTo;
        };
        do {
            if (nullptr == halfEdge->oppositeHalfEdge) {
                isBoundary = true;
                break;
            }
            Vertex *neighborVertex = halfEdge->oppositeHalfEdge->startVertex;
            project(neighborVertex->position);
            HalfEdge *neighborHalfEdge = neighborVertex->anyHalfEdge;
            if (nullptr != neighborHalfEdge) {
                do {
                    if (nullptr == neighborHalfEdge->oppositeHalfEdge) {
                        isBoundary = true;
                        break;
                    }
                    if (neighborHalfEdge->oppositeHalfEdge->startVertex != vertex)
                        project(neighborHalfEdge->oppositeHalfEdge->startVertex->position);
                    neighborHalfEdge = neighborHalfEdge->oppositeHalfEdge->nextHalfEdge;
                } while (neighborHalfEdge && neighborHalfEdge != neighborVertex->anyHalfEdge);
            }
            halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
        } while (halfEdge && halfEdge != vertex->anyHalfEdge);
        if (!isBoundary) {
            vertex->relativeHeight = high - low;
            vertex->hasRelativeHeight = true;
        } else {
            vertex->relativeHeight = std::numeric_limits<double>::max();
        }
    }
}

void Mesh::normalizeVertexRelativeHeights()
{
    double maxHeight = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (vertex->hasRelativeHeight && vertex->relativeHeight > maxHeight)
            maxHeight = vertex->relativeHeight;
    }
    if (Double::isZero(maxHeight))
        return;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (vertex->hasRelativeHeight)
            vertex->relativeHeight /= maxHeight;
    }
}

void Mesh::removeZeroAngleTriangles()
{
    std::vector<HalfEdge *> candidates;
    for (HalfEdge *halfEdge = m_firstHalfEdge; nullptr != halfEdge; halfEdge = halfEdge->_next) {
        auto v1 = halfEdge->nextHalfEdge->startVertex->position - halfEdge->startVertex->position;
        auto v2 = halfEdge->previousHalfEdge->startVertex->position - halfEdge->startVertex->position;
        auto degrees = Radians::toDegrees(Vector3::angle(v1, v2));
        if (degrees <= 179.9)
            continue;
        candidates.push_back(halfEdge);
    }
    
    for (auto &halfEdge: candidates) {
        if (nullptr == halfEdge->leftFace)
            continue;
        
        auto v1 = halfEdge->nextHalfEdge->startVertex->position - halfEdge->startVertex->position;
        auto v2 = halfEdge->previousHalfEdge->startVertex->position - halfEdge->startVertex->position;
        auto degrees = Radians::toDegrees(Vector3::angle(v1, v2));
        if (degrees <= 179.9)
            continue;
        
        flip(halfEdge->nextHalfEdge);
    }
}

void Mesh::orderVertexByFlatness()
{
    m_vertexOrderedByFlatness.resize(m_vertexCount);
    size_t vertexIndex = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        m_vertexOrderedByFlatness[vertexIndex] = vertex;
        ++vertexIndex;
    }
    std::sort(m_vertexOrderedByFlatness.begin(), m_vertexOrderedByFlatness.end(), 
            [](const Vertex *first, const Vertex *second) {
        return first->relativeHeight < second->relativeHeight;
    });
}

const std::vector<Vertex *> &Mesh::vertexOrderedByFlatness()
{
    return m_vertexOrderedByFlatness;
}

#if AUTO_REMESHER_DEV

void Mesh::debugExportRelativeHeightPly(const char *filename)
{
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->debugColor = (1.0 - vertex->relativeHeight) * 255;
    }
    debugExportPly(filename);
}

void Mesh::debugExportLimitRelativeHeightPly(const char *filename, const std::pair<double, double> &limitRelativeHeight)
{
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->debugColor = (vertex->relativeHeight < limitRelativeHeight.first ||
                vertex->relativeHeight > limitRelativeHeight.second) ? 0 : 127 + vertex->relativeHeight * 100;
    }
    debugExportPly(filename);
}

void Mesh::debugExportPly(const char *filename)
{
    std::cerr << "debugExportPly:" << filename << std::endl;
    
    FILE *fp = fopen(filename, "wb");
    fprintf(fp, "ply\n");
    fprintf(fp, "format ascii 1.0\n");
    fprintf(fp, "element vertex %zu\n", m_vertexCount);
    fprintf(fp, "property float x\n");
    fprintf(fp, "property float y\n");
    fprintf(fp, "property float z\n");
    fprintf(fp, "property uchar red\n");
    fprintf(fp, "property uchar green\n");
    fprintf(fp, "property uchar blue\n");
    fprintf(fp, "element face %zu\n", m_faceCount);
    fprintf(fp, "property list uchar uint vertex_indices\n");
    fprintf(fp, "end_header\n");
    size_t index = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = index++;
        int c = vertex->debugColor > 255 ? 255 : vertex->debugColor;
        fprintf(fp, "%f %f %f %d %d %d\n", 
            vertex->position.x(), vertex->position.y(), vertex->position.z(),
            c, c, c);
    }
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        fprintf(fp, "3 %zu %zu %zu\n",
            h0->startVertex->outputIndex, 
            h1->startVertex->outputIndex, 
            h2->startVertex->outputIndex);
    }
    fclose(fp);
}

#endif
 
}
    
}
