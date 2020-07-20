#include <map>
#include <cassert>
#include <iostream>
#include <set>
#include <unordered_set>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/Parametrization>
#include <AutoRemesher/Radians>
#include <AutoRemesher/MeshSegmenter>
#include <AutoRemesher/HeatMapGenerator>

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
        const std::vector<std::vector<size_t>> &triangles,
        const std::unordered_map<size_t, Vector3> &guidelineVertices) :
    m_vertexRemovalCostPriorityQueue(m_vertexRemovalCostComparer)
{
    MeshSegmenter meshSegmenter(&vertices, &triangles);
    meshSegmenter.segment();
    const std::vector<size_t> &segmentIds = meshSegmenter.triangleSegmentIds();
    
    std::vector<Vertex *> halfEdgeVertices(vertices.size());
    for (size_t i = 0; i < vertices.size(); ++i) {
        Vertex *vertex = allocVertex();
        vertex->index = i;
        vertex->position = vertices[i];
        halfEdgeVertices[i] = vertex;
    }
    
    std::vector<Face *> halfEdgeFaces(triangles.size());
    for (size_t i = 0; i < triangles.size(); ++i) {
        halfEdgeFaces[i] = allocFace();
    }
    
    std::map<std::pair<size_t, size_t>, HalfEdge *> halfEdgeIndexMap;
    for (size_t i = 0; i < triangles.size(); ++i) {
        auto &face = halfEdgeFaces[i];
        
        face->segmentId = segmentIds[i];
        
        const auto &triangleIndices = triangles[i];
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
            auto &vertex = halfEdgeVertices[vertexIndex];
            const auto &nextVertex = halfEdgeVertices[nextVertexIndex];
            auto findGuideline = guidelineVertices.find(vertexIndex);
            if (guidelineVertices.end() != findGuideline) {
                face->isGuideline = true;
                face->guidelineDirection = findGuideline->second;
            }
            vertex->anyHalfEdge = halfEdge;
            ++vertex->halfEdgeCount;
            halfEdge->startVertex = vertex;
            halfEdge->previousHalfEdge = halfEdges[h];
            halfEdge->nextHalfEdge = halfEdges[k];
            halfEdge->leftFace = face;
            halfEdge->length2 = (vertex->position - nextVertex->position).lengthSquared();
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
        calculateFaceNormals();
        calculateAnglesBetweenFaces();
    
        HeatMapGenerator heatMapGenerator(&vertices, &triangles);
        std::unordered_set<size_t> heatMapSources;
        for (HalfEdge *halfEdge = m_firstHalfEdge; nullptr != halfEdge; halfEdge = halfEdge->_next) {
            if (halfEdge->leftFace->segmentId != halfEdge->oppositeHalfEdge->leftFace->segmentId/* ||
                    halfEdge->degreesBetweenFaces >= 80*/) {
                heatMapSources.insert(halfEdge->startVertex->index);
                heatMapSources.insert(halfEdge->oppositeHalfEdge->startVertex->index);
            }
        }
        heatMapGenerator.generate(heatMapSources);
        const std::vector<double> &vertexHeatMap = heatMapGenerator.vertexHeatMap();
        for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next)
            vertex->heat = vertexHeatMap[vertex->index];
        
        for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next)
            vertex->fineCurvature = calculateVertexCurvature(vertex);
        for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
            vertex->removalCost = calculateVertexRemovalCost(vertex);
            m_vertexRemovalCostPriorityQueue.push({vertex, vertex->removalCost, vertex->version});
        }
    }
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
    vertex->version = std::numeric_limits<uint32_t>::max();
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

HalfEdge *Mesh::findShortestHalfEdgeAroundVertex(Vertex *vertex) const
{
    HalfEdge *halfEdge = vertex->anyHalfEdge;
    double shortestLength2 = std::numeric_limits<double>::max();
    HalfEdge *selected = halfEdge;
    do {
        if (halfEdge->length2 < shortestLength2) {
            selected = halfEdge;
            shortestLength2 = halfEdge->length2;
        }
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return selected;
}

double Mesh::calculateVertexCurvature(Vertex *vertex) const
{
    double sumOfAngle = 0.0;
    HalfEdge *halfEdge = vertex->anyHalfEdge;
    do {
        sumOfAngle += Vector3::angle(halfEdge->nextHalfEdge->startVertex->position - vertex->position,
            halfEdge->previousHalfEdge->startVertex->position - vertex->position);
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return std::abs(2.0 * M_PI - sumOfAngle);
}

std::vector<std::pair<Vertex *, Vertex *>> Mesh::collectConesAroundVertexExclude(Vertex *vertex, Vertex *exclude) const
{
    std::vector<std::pair<Vertex *, Vertex *>> cones;
    HalfEdge *halfEdge = vertex->anyHalfEdge;
    do {
        if (halfEdge->nextHalfEdge->startVertex != exclude &&
                halfEdge->nextHalfEdge->nextHalfEdge->startVertex != exclude) {
            cones.push_back({halfEdge->nextHalfEdge->startVertex,
                halfEdge->nextHalfEdge->nextHalfEdge->startVertex});
        }
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return cones;
}

Vector3 Mesh::calculateVertexNormal(Vertex *vertex) const
{
    HalfEdge *halfEdge = vertex->anyHalfEdge;
    Vector3 sumOfNormal;
    do {
        sumOfNormal += Vector3::normal(vertex->position,
            halfEdge->nextHalfEdge->startVertex->position,
            halfEdge->nextHalfEdge->nextHalfEdge->startVertex->position);
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return sumOfNormal.normalized();
}

bool Mesh::isWatertight()
{
    return m_faceCount >= 4 && 
        0 == m_repeatedHalfEdges && 
        0 == m_aloneHalfEdges;
}

bool Mesh::isVertexConstrained(Vertex *vertex) const
{
    HalfEdge *halfEdge = vertex->anyHalfEdge;
    do {
        if (0 != halfEdge->featured)
            return true;
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return false;
}

double Mesh::calculateVertexRemovalCost(Vertex *vertex) const
{
    HalfEdge *collapsedHalfEdge = findShortestHalfEdgeAroundVertex(vertex);
    
    // The following two checks are necessary, unless non-manifold vertex will be created because of collapse 
    if (collapsedHalfEdge->nextHalfEdge->nextHalfEdge->startVertex->halfEdgeCount < 4)
        return std::numeric_limits<double>::max();
    if (collapsedHalfEdge->oppositeHalfEdge->previousHalfEdge->startVertex->halfEdgeCount < 4)
        return std::numeric_limits<double>::max();
    
    if (vertex->halfEdgeCount < 4)
        return std::numeric_limits<double>::max();
    
    std::vector<Vertex *> ringVertices;
    std::set<Vertex *> verticesAroundTarget;
    std::map<Vertex *, std::vector<std::pair<Vertex *, Vertex *>>> vertexCones;
    std::set<std::pair<Vertex *, Vertex *>> halfEdges;
    HalfEdge *halfEdge = collapsedHalfEdge;
    do {
        auto insertVertexResult = verticesAroundTarget.insert(halfEdge->oppositeHalfEdge->startVertex);
        if (!insertVertexResult.second) {
            return std::numeric_limits<double>::max();
        }
        auto insertEdgeResult = halfEdges.insert({halfEdge->nextHalfEdge->nextHalfEdge->startVertex,
            halfEdge->nextHalfEdge->startVertex});
        if (!insertEdgeResult.second) {
            return std::numeric_limits<double>::max();
        }
        ringVertices.push_back(halfEdge->oppositeHalfEdge->startVertex);
        vertexCones.insert({halfEdge->oppositeHalfEdge->startVertex,
            collectConesAroundVertexExclude(halfEdge->oppositeHalfEdge->startVertex, vertex)});
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != collapsedHalfEdge);
    
    if (vertex->halfEdgeCount != verticesAroundTarget.size())
        return std::numeric_limits<double>::max();

    if (isVertexConstrained(vertex))
        return std::numeric_limits<double>::max();
    
    std::vector<std::vector<Vertex *>> triangles;
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (collapsedHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
    if (!delaunayTriangulate(ringVertices, projectNormal, projectAxis, &triangles, vertex->position))
        return std::numeric_limits<double>::max();
    
    for (const auto &it: triangles) {
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            size_t k = (i + 2) % 3;
            auto insertEdgeResult = halfEdges.insert({it[i], it[j]});
            if (!insertEdgeResult.second)
                return std::numeric_limits<double>::max();
            vertexCones[it[j]].push_back({it[i], it[k]});
        }
    }
    for (const auto &it: halfEdges) {
        if (halfEdges.end() == halfEdges.find({it.second, it.first}))
            return std::numeric_limits<double>::max();
    }
    double oneRingCurvature = 0.0;
    for (const auto &it: vertexCones) {
        double sumOfAngle = 0.0;
        for (const auto &cone: it.second) {
            sumOfAngle += Vector3::angle(cone.first->position - it.first->position, 
                cone.second->position - it.first->position);
        }
        oneRingCurvature += std::abs(std::abs(2.0 * M_PI - sumOfAngle) - it.first->fineCurvature);
    }

    return vertex->fineCurvature + oneRingCurvature;
}

bool Mesh::delaunayTriangulate(std::vector<Vertex *> &ringVertices,
        const Vector3 &projectNormal, const Vector3 &projectAxis,
        std::vector<std::vector<Vertex *>> *triangles,
        const Vector3 &origin) const
{
    // cf. https://dl.acm.org/doi/pdf/10.1145/2980179.2982413 
    // <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 4
    
    if (ringVertices.size() < 4)
        return false;
    
    std::vector<Vector3> ringPoints(ringVertices.size());
    for (size_t i = 0; i < ringVertices.size(); ++i) {
        ringPoints[i] = ringVertices[i]->position;
    }
    
    // Project to 2D plane
    std::vector<Vector2> ringPointsIn2d;
    Vector3::project(ringPoints, &ringPointsIn2d, projectNormal, projectAxis, origin);
    
    size_t a = 0;
    size_t i = 1;
    for (; i + 2 < ringPointsIn2d.size(); i += 2) {
        size_t b = i;
        size_t c = i + 1;
        size_t d = i + 2;
        if (ringPointsIn2d[d].isInCircle(ringPointsIn2d[a], ringPointsIn2d[c], ringPointsIn2d[b]) &&
                ringVertices[a]->halfEdgeCount >= 4 && ringVertices[c]->halfEdgeCount >= 4) {
            triangles->push_back({ringVertices[a], ringVertices[d], ringVertices[b]});
            triangles->push_back({ringVertices[c], ringVertices[b], ringVertices[d]});
        } else {
            triangles->push_back({ringVertices[a], ringVertices[c], ringVertices[b]});
            triangles->push_back({ringVertices[a], ringVertices[d], ringVertices[c]});
        }
    }
    // Add the last triangle if there is
    if (i + 1 < ringPointsIn2d.size()) {
        size_t b = i;
        size_t c = i + 1;
        triangles->push_back({ringVertices[a], ringVertices[c], ringVertices[b]});
    }
    
    return true;
}

HalfEdge *Mesh::findHalfEdgeBetweenVertices(Vertex *firstVertex, Vertex *secondVertex)
{
    HalfEdge *halfEdge = firstVertex->anyHalfEdge;
    do {
        if (halfEdge->oppositeHalfEdge->startVertex == secondVertex)
            return halfEdge;
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != firstVertex->anyHalfEdge);
    return nullptr;
}

bool Mesh::isVertexMixed(Vertex *vertex) const
{
    HalfEdge *halfEdge = vertex->anyHalfEdge;
    bool isGuideline = halfEdge->leftFace->isGuideline;
    do {
        if (halfEdge->leftFace->isGuideline != isGuideline)
            return true;
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return false;
}

bool Mesh::decimate(Vertex *vertex)
{
    // cf. <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 7
    
    HalfEdge *collapsedHalfEdge = findShortestHalfEdgeAroundVertex(vertex);
    
    // The following two checks are necessary, unless non-manifold vertex will be created because of collapse 
    if (collapsedHalfEdge->nextHalfEdge->nextHalfEdge->startVertex->halfEdgeCount < 4)
        return false;
    if (collapsedHalfEdge->oppositeHalfEdge->previousHalfEdge->startVertex->halfEdgeCount < 4)
        return false;
    
    std::vector<HalfEdge *> halfEdgesPointToTarget;
    std::set<Vertex *> ringVertices;
    std::vector<Vector3> ringPoints;
    HalfEdge *halfEdge = collapsedHalfEdge;
    do {
        auto insertResult = ringVertices.insert(halfEdge->oppositeHalfEdge->startVertex);
        if (!insertResult.second) {
            return false;
        }
        ringPoints.push_back(halfEdge->oppositeHalfEdge->startVertex->position);
        halfEdgesPointToTarget.push_back(halfEdge->oppositeHalfEdge);
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != collapsedHalfEdge);

    if (halfEdgesPointToTarget.size() < 4)
        return false;
    
    auto k = halfEdgesPointToTarget.size();
    
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (collapsedHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
    std::vector<Vector2> ringPointsIn2d;
    Vector3::project(ringPoints, &ringPointsIn2d, projectNormal, projectAxis, vertex->position);
    
    for (size_t i = 2; i <= k - 2; ++i) {
        if (nullptr != findHalfEdgeBetweenVertices(halfEdgesPointToTarget[0]->startVertex, halfEdgesPointToTarget[i]->startVertex))
            return false;
    }

    if (!collapse(vertex, halfEdgesPointToTarget))
        return false;
    vertex = nullptr;
    
    {
        size_t a = 0;
        size_t i = 1;
        for (; i + 2 < k; i += 2) {
            size_t b = i;
            size_t c = i + 1;
            size_t d = i + 2;
            if (ringPointsIn2d[d].isInCircle(ringPointsIn2d[a], ringPointsIn2d[c], ringPointsIn2d[b])) {
                const auto &hflip_x = halfEdgesPointToTarget[i + 1];
                const auto &hflip = hflip_x->oppositeHalfEdge;
                if (hflip->startVertex->halfEdgeCount < 4)
                    continue;
                if (hflip_x->startVertex->halfEdgeCount < 4)
                    continue;
                if (nullptr != findHalfEdgeBetweenVertices(hflip->previousHalfEdge->startVertex, hflip_x->previousHalfEdge->startVertex))
                    continue;
                if (!flip(hflip))
                    return false;
            }
        }
    }
    
    for (auto &it: ringVertices) {
        it->removalCost = calculateVertexRemovalCost(it);
        ++it->version;
        m_vertexRemovalCostPriorityQueue.push({it, 
            it->removalCost, 
            it->version});
    }
    
    return true;
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
    
    hflip->length2 = (hflip->startVertex->position - hflip_x->startVertex->position).lengthSquared();
    hflip_x->length2 = hflip->length2;
    
    return true;
}

bool Mesh::collapse(Vertex *vertex, std::vector<HalfEdge *> &h)
{
    // cf. <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 7
    
    auto &h0 = h[0];
    auto k = h.size();
    
    auto &h1 = h[1];
    auto &hk_1 = h[k - 1];
    
    auto &a = h0->previousHalfEdge;
    auto &b = h1->previousHalfEdge;
    auto &c = h0->oppositeHalfEdge->nextHalfEdge;
    
    auto &collapseToVertex = h0->startVertex;
    for (size_t i = 2; i + 1 < k; ++i) {
        h[i]->oppositeHalfEdge->startVertex = collapseToVertex;
    }
    
    collapseToVertex->halfEdgeCount += k - 4;
    if (collapseToVertex->anyHalfEdge == h0)
        collapseToVertex->anyHalfEdge = c;
    
    --hk_1->startVertex->halfEdgeCount;
    if (hk_1->startVertex->anyHalfEdge == hk_1)
        hk_1->startVertex->anyHalfEdge = hk_1->oppositeHalfEdge->nextHalfEdge;
    
    --h1->startVertex->halfEdgeCount;
    if (h1->startVertex->anyHalfEdge == h1)
        h1->startVertex->anyHalfEdge = h1->oppositeHalfEdge->nextHalfEdge;
    
    a->leftFace = h1->leftFace;
    c->leftFace = hk_1->oppositeHalfEdge->leftFace;
    
    if (hk_1->oppositeHalfEdge->leftFace->anyHalfEdge == hk_1->oppositeHalfEdge)
        hk_1->oppositeHalfEdge->leftFace->anyHalfEdge = hk_1->oppositeHalfEdge->nextHalfEdge;
    if (h1->leftFace->anyHalfEdge == h1)
        h1->leftFace->anyHalfEdge = b;
    
    deferedFreeFace(hk_1->leftFace);
    deferedFreeFace(h1->oppositeHalfEdge->leftFace);
    
    makeLinkedHalfEdges(h[k-2], c);
    makeLinkedHalfEdges(c, hk_1->oppositeHalfEdge->nextHalfEdge);
    
    makeLinkedHalfEdges(a, h[2]->oppositeHalfEdge);
    makeLinkedHalfEdges(b, a);
    
    deferedFreeHalfEdge(hk_1->oppositeHalfEdge);
    deferedFreeHalfEdge(hk_1);
    deferedFreeHalfEdge(h1->oppositeHalfEdge);
    deferedFreeHalfEdge(h1);
    deferedFreeHalfEdge(h0->oppositeHalfEdge);
    deferedFreeHalfEdge(h0);
    
    deferedFreeVertex(vertex);
    
    return true;
}

void Mesh::setTargetVertexCount(size_t targetVertexCount)
{
    m_targetVertexCount = targetVertexCount;
}

bool Mesh::decimate()
{
    if (!isWatertight()) {
        std::cerr << "Mesh is not watertight, cannot continue to decimate" << std::endl;
        return false;
    }
    auto vertexCountBeforeDecimation = m_vertexCount;
    while (m_vertexCount > m_targetVertexCount && !m_vertexRemovalCostPriorityQueue.empty()) {
        const auto &item = m_vertexRemovalCostPriorityQueue.top();
        Vertex *vertex = item.vertex;
        if (item.version != vertex->version) {
            m_vertexRemovalCostPriorityQueue.pop();
            continue;
        }
        m_vertexRemovalCostPriorityQueue.pop();
        decimate(vertex);
    }
    std::cerr << "Postchecking begin" << std::endl;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (vertex->halfEdgeCount <= 2)
            std::cerr << "Found invalid vertex" << std::endl;
    }
    std::cerr << "Postchecking end" << std::endl;
    return true;
}

void Mesh::markGuidelineEdgesAsFeatured()
{
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        if (0 == face->isGuideline)
            continue;
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        HalfEdge *halfEdges[] = {
            h0, h1, h2
        };
        bool vertexMixedChecks[] = {
            isVertexMixed(h0->startVertex),
            isVertexMixed(h1->startVertex),
            isVertexMixed(h2->startVertex)
        };
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            if (!vertexMixedChecks[i] || !vertexMixedChecks[j])
                continue;
            Vector3 edgeDirection = (halfEdges[j]->startVertex->position - halfEdges[i]->startVertex->position).normalized();
            if (std::abs(Vector3::dotProduct(face->guidelineDirection, edgeDirection)) >= 0.966) { //<=15 degrees
                halfEdges[i]->featured = 1;
                halfEdges[i]->oppositeHalfEdge->featured = 1;
            }
        }
    }
}

bool Mesh::parametrize(double gradientSize, double constraintStength)
{
    Parametrization::Parameters parameters;
    parameters.gradientSize = gradientSize;
    parameters.constraintStength = constraintStength;
    if (!Parametrization::miq(*this, parameters))
        return false;
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

void Mesh::calculateAnglesBetweenFaces()
{
    for (HalfEdge *halfEdge = m_firstHalfEdge; nullptr != halfEdge; halfEdge = halfEdge->_next) {
        if (-1 != halfEdge->degreesBetweenFaces)
            continue;
        halfEdge->degreesBetweenFaces = std::round(Radians::toDegrees(Vector3::angle(halfEdge->leftFace->normal, 
            halfEdge->oppositeHalfEdge->leftFace->normal)));
        halfEdge->oppositeHalfEdge->degreesBetweenFaces = halfEdge->degreesBetweenFaces;
    }
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

void Mesh::debugExportEdgeAnglesPly(const char *filename)
{
    debugResetColor();
    
    int maxDegrees = 0;
    for (HalfEdge *halfEdge = m_firstHalfEdge; nullptr != halfEdge; halfEdge = halfEdge->_next) {
        if (-1 == halfEdge->degreesBetweenFaces)
            continue;
        if (halfEdge->degreesBetweenFaces > maxDegrees)
            maxDegrees = halfEdge->degreesBetweenFaces;
    }
    std::cerr << "maxDegrees:" << maxDegrees << std::endl;
    for (HalfEdge *halfEdge = m_firstHalfEdge; nullptr != halfEdge; halfEdge = halfEdge->_next) {
        if (-1 == halfEdge->degreesBetweenFaces)
            continue;
        if (halfEdge->degreesBetweenFaces < 60)
            continue;
        //auto degrees = 255 * halfEdge->degreesBetweenFaces / maxDegrees;
        halfEdge->startVertex->debugColor = 255;
        halfEdge->oppositeHalfEdge->startVertex->debugColor = 255;
    }
    
    debugExportPly(filename);
}

void Mesh::debugExportVertexHeatMapPly(const char *filename)
{
    debugResetColor();
    
    double maxHeat = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (vertex->heat > maxHeat)
            maxHeat = vertex->heat;
    }
    std::cerr << "maxHeat:" << maxHeat << std::endl;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        const int maxLevel = 30;
        int level = (int)(maxLevel * vertex->heat / maxHeat);
        vertex->debugColor = 255 * level / maxLevel;
        if (level % 2 == 0)
            vertex->debugColor = 0xff;
        else
            vertex->debugColor = 0xcc;
    }
    
    debugExportPly(filename);
}

void Mesh::debugExportSegmentEdgesPly(const char *filename)
{
    debugResetColor();
    
    for (HalfEdge *halfEdge = m_firstHalfEdge; nullptr != halfEdge; halfEdge = halfEdge->_next) {
        if (halfEdge->leftFace->segmentId == halfEdge->oppositeHalfEdge->leftFace->segmentId)
            continue;
        halfEdge->startVertex->debugColor = 255;
        halfEdge->oppositeHalfEdge->startVertex->debugColor = 255;
    }
    
    debugExportPly(filename);
}

void Mesh::debugResetColor()
{
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->debugColor = 0;
    }
}

void Mesh::debugExportCurvaturePly(const char *filename)
{
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
    double maxCurvature = std::numeric_limits<double>::lowest();
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (vertex->fineCurvature > maxCurvature) {
            maxCurvature = vertex->fineCurvature;
        }
    }
    size_t index = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = index++;
        int r = 255 * vertex->fineCurvature / maxCurvature;
        fprintf(fp, "%f %f %f %d %d %d\n", 
            vertex->position.x(), vertex->position.y(), vertex->position.z(),
            r, 0, 0);
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

void Mesh::debugExportUvObj(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        fprintf(fp, "v %f %f %f\n",
            h0->startVertexUv.x(), 
            0.0, 
            h0->startVertexUv.y());
        fprintf(fp, "v %f %f %f\n",
            h1->startVertexUv.x(), 
            0.0, 
            h1->startVertexUv.y());
        fprintf(fp, "v %f %f %f\n",
            h2->startVertexUv.x(), 
            0.0, 
            h2->startVertexUv.y());
    }
    size_t index = 0;
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        fprintf(fp, "f %zu %zu %zu\n",
            index + 1, 
            index + 2, 
            index + 3);
        index += 3;
    }
    fclose(fp);
}

void Mesh::debugExportGuidelinePly(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    fprintf(fp, "ply\n");
    fprintf(fp, "format ascii 1.0\n");
    fprintf(fp, "element vertex %zu\n", m_faceCount * 3);
    fprintf(fp, "property float x\n");
    fprintf(fp, "property float y\n");
    fprintf(fp, "property float z\n");
    fprintf(fp, "property uchar red\n");
    fprintf(fp, "property uchar green\n");
    fprintf(fp, "property uchar blue\n");
    fprintf(fp, "element face %zu\n", m_faceCount);
    fprintf(fp, "property list uchar uint vertex_indices\n");
    fprintf(fp, "end_header\n");
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        HalfEdge *halfEdges[] = {
            h0, h1, h2
        };
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            int r = halfEdges[i]->featured ? 255 : 0;
            fprintf(fp, "%f %f %f %d %d %d\n", 
                halfEdges[i]->startVertex->position.x(), halfEdges[i]->startVertex->position.y(), halfEdges[i]->startVertex->position.z(),
                r, 0, 0);
        }
    }
    size_t index = 0;
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        fprintf(fp, "3 %zu %zu %zu\n",
            index + 0, 
            index + 1, 
            index + 2);
        index += 3;
    }
    fclose(fp);
}
 
}
    
}
