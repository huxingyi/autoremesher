#include <map>
#include <cassert>
#include <iostream>
#include <set>
#include <TransitionFunction.hh>
#include <AutoRemesher/HalfEdge>
#include <AutoRemesher/Parametrization>

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
        std::vector<std::vector<size_t>> &triangles) :
    m_vertexRemovalCostPriorityQueue(m_vertexRemovalCostComparer)
{
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
            vertex->anyHalfEdge = halfEdge;
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
        for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next)
            vertex->fineCurvature = calculateVertexCurvature(vertex);
        for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
            vertex->removalCost = calculateVertexRemovalCost(vertex);
            //std::cerr << "removalCost[" << vertex->index << "]:" << vertex->removalCost << std::endl;
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
    HalfEdge *shortest = halfEdge;
    do {
        if (halfEdge->length2 < shortestLength2) {
            shortest = halfEdge;
            shortestLength2 = halfEdge->length2;
        }
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return shortest;
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
    // TODO: Cache vertex normal
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

double Mesh::calculateVertexRemovalCost(Vertex *vertex) const
{
    HalfEdge *shortestHalfEdge = findShortestHalfEdgeAroundVertex(vertex);
    
    std::vector<Vertex *> ringVertices;
    std::set<Vertex *> verticesAroundTarget;
    std::map<Vertex *, std::vector<std::pair<Vertex *, Vertex *>>> vertexCones;
    HalfEdge *halfEdge = shortestHalfEdge;
    do {
        auto insertResult = verticesAroundTarget.insert(halfEdge->oppositeHalfEdge->startVertex);
        if (!insertResult.second) {
            return std::numeric_limits<double>::max();
        }
        ringVertices.push_back(halfEdge->oppositeHalfEdge->startVertex);
        vertexCones.insert({halfEdge->oppositeHalfEdge->startVertex,
            collectConesAroundVertexExclude(halfEdge->oppositeHalfEdge->startVertex, vertex)});
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != shortestHalfEdge);
    
    std::vector<std::vector<Vertex *>> triangles;
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (shortestHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
    if (!delaunayTriangulate(ringVertices, projectNormal, projectAxis, &triangles, vertex->position))
        return std::numeric_limits<double>::max();
    
    for (const auto &it: triangles) {
        for (size_t i = 0; i < 3; ++i) {
            size_t j = (i + 1) % 3;
            size_t k = (i + 2) % 3;
            vertexCones[it[j]].push_back({it[i], it[k]});
        }
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
        if (ringPointsIn2d[d].isInCircle(ringPointsIn2d[a], ringPointsIn2d[c], ringPointsIn2d[b])) {
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

bool Mesh::decimate(Vertex *vertex)
{
    // cf. <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 7
    
    HalfEdge *shortestHalfEdge = findShortestHalfEdgeAroundVertex(vertex);
    
    std::vector<HalfEdge *> halfEdgesPointToTarget;
    std::set<Vertex *> ringVertices;
    std::vector<Vector3> ringPoints;
    HalfEdge *halfEdge = shortestHalfEdge;
    do {
        auto insertResult = ringVertices.insert(halfEdge->oppositeHalfEdge->startVertex);
        if (!insertResult.second) {
            return false;
        }
        ringPoints.push_back(halfEdge->oppositeHalfEdge->startVertex->position);
        halfEdgesPointToTarget.push_back(halfEdge->oppositeHalfEdge);
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != shortestHalfEdge);
    
    if (halfEdgesPointToTarget.size() < 4)
        return false;
    
    Vector3 origin = vertex->position;
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (shortestHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
    
    // Project to 2D plane
    std::vector<Vector2> ringPointsIn2d;
    Vector3::project(ringPoints, &ringPointsIn2d, projectNormal, projectAxis, origin);
    
    std::cerr << "collapse halfEdgesPointToTarget.size:" << halfEdgesPointToTarget.size() << std::endl;
 
    if (!collapse(vertex, halfEdgesPointToTarget))
        return false;
    vertex = nullptr;
    
    size_t a = 0;
    size_t i = 1;
    for (; i + 2 < ringPointsIn2d.size(); i += 2) {
        size_t b = i;
        size_t c = i + 1;
        size_t d = i + 2;

        if (ringPointsIn2d[d].isInCircle(ringPointsIn2d[a], ringPointsIn2d[c], ringPointsIn2d[b])) {
            const auto &hflip_x = halfEdgesPointToTarget[i + 1];
            const auto &hflip = hflip_x->oppositeHalfEdge;
            const auto &ha = hflip->previousHalfEdge;
            const auto &hb = hflip_x->previousHalfEdge;
            /*
            std::cerr << "edge:" <<
                hflip->startVertex->position << " " << 
                hflip_x->startVertex->position << " " << 
                ha->startVertex->position << " " << 
                hb->startVertex->position << " " <<  
                std::endl;
                
            std::cerr << "point:" <<
                ringPoints[a] << " " << 
                ringPoints[c] << " " << 
                ringPoints[b] << " " << 
                ringPoints[d] << " " <<  
                std::endl;
                
            std::cerr << "projectNormal:" <<
                projectNormal <<  
                std::endl;
                
            std::cerr << "projectAxis:" <<
                projectAxis <<  
                std::endl;
                
            std::cerr << "origin:" <<
                origin <<  
                std::endl;
                
            static int s_count = 0;
            ++s_count;
            if (2 == s_count) {
                hflip->startVertex->r = 255;
                hflip_x->startVertex->g = 125;
                ha->startVertex->r = 255;
                hb->startVertex->b = 255;
                exportPly("C:\\Users\\Jeremy\\Desktop\\test-before-flip.ply");
                if (!flip(hflip))
                    return false;
                exportPly("C:\\Users\\Jeremy\\Desktop\\test-after-flip.ply");
                exportObj("C:\\Users\\Jeremy\\Desktop\\test-2d.obj", std::vector<Vector2> {
                    ringPointsIn2d[a],
                    ringPointsIn2d[b],
                    ringPointsIn2d[c],
                    ringPointsIn2d[d]
                });
                exit(0);
            }
            */
            if (!flip(hflip))
                return false;
        }
    }
    
    /*
    for (size_t i = 2; i + 1 < h.size(); i += 2) {
        const auto &hflip_x = h[i];
        const auto &hflip = hflip_x->oppositeHalfEdge;
        const auto &ha = hflip->previousHalfEdge;
        const auto &hb = hflip_x->previousHalfEdge;
        std::vector<Vector3> abcd = {
            hflip->startVertex->position,
            ha->startVertex->position,
            hflip_x->startVertex->position,
            hb->startVertex->position
        };
        std::vector<Vector2> abcdIn2d;
        Vector3::project(abcd, &abcdIn2d, projectNormal, projectAxis, origin);
        if (abcdIn2d[3].isInCircle(abcdIn2d[0], abcdIn2d[1], abcdIn2d[2])) {
            static int s_count = 0;
            ++s_count;
            if (2 == s_count) {
                hflip->startVertex->g = 255;
                hflip_x->startVertex->g = 255;
                ha->startVertex->g = 255;
                hb->startVertex->b = 255;
                exportPly("C:\\Users\\Jeremy\\Desktop\\test-before-flip.ply");
                if (!flip(hflip))
                    return false;
                exportPly("C:\\Users\\Jeremy\\Desktop\\test-after-flip.ply");
                exit(0);
            }
            if (!flip(hflip))
                return false;
        }
    }
    */
    
    for (auto &it: ringVertices) {
        it->removalCost = calculateVertexRemovalCost(it);
        ++it->version;
        m_vertexRemovalCostPriorityQueue.push({it, 
            it->removalCost, 
            it->version});
    }
    
    return true;
}

void Mesh::updateVertexRemovalCostToColor()
{
    double maxRemovalCost = 0.0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (Double::isEqual(vertex->removalCost, std::numeric_limits<double>::max()))
            continue;
        if (vertex->removalCost > maxRemovalCost)
            maxRemovalCost = vertex->removalCost;
    }
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        auto cost = std::min(vertex->removalCost, maxRemovalCost);
        vertex->r = cost * 255 / maxRemovalCost;
    }
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
    
    if (hflip->startVertex->anyHalfEdge == hflip)
        hflip->startVertex->anyHalfEdge = hd;
    
    if (hflip_x->startVertex->anyHalfEdge == hflip_x)
        hflip_x->startVertex->anyHalfEdge = hc;
    
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
    
    if (collapseToVertex->anyHalfEdge == h0)
        collapseToVertex->anyHalfEdge = c;
    
    if (hk_1->startVertex->anyHalfEdge == hk_1)
        hk_1->startVertex->anyHalfEdge = hk_1->oppositeHalfEdge->nextHalfEdge;
    if (h1->startVertex->anyHalfEdge == h1)
        h1->startVertex->anyHalfEdge = h1->oppositeHalfEdge->nextHalfEdge;
    
    a->leftFace = h1->leftFace;
    c->leftFace = hk_1->oppositeHalfEdge->leftFace;
    
    if (hk_1->oppositeHalfEdge->leftFace->anyHalfEdge == hk_1->oppositeHalfEdge)
        hk_1->oppositeHalfEdge->leftFace->anyHalfEdge = hk_1->oppositeHalfEdge->nextHalfEdge;
    if (h1->leftFace->anyHalfEdge == h1)
        h1->leftFace->anyHalfEdge = b;
    
    freeFace(hk_1->leftFace);
    freeFace(h1->oppositeHalfEdge->leftFace);
    
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
        if (decimate(vertex)) {
            std::cerr << "Vertex " << vertex->index << " decimated" << std::endl;
            std::cerr << "Vertices reduced from:" << vertexCountBeforeDecimation << " to:" << m_vertexCount << std::endl;
        }
    }
    std::cerr << "decimated vertices:" << (vertexCountBeforeDecimation - m_vertexCount) << std::endl;
    return true;
}

bool Mesh::parametrize(double gradientSize)
{
    Parametrization::Parameters parameters;
    parameters.gradientSize = gradientSize;
    if (!Parametrization::miq(*this, parameters))
        return false;
    return true;
}

bool Mesh::coarseToFineMap()
{
    // QEx::TransitionFunctionT<double> transitionFlipI;
    
    return true;
}

void Mesh::exportObj(const char *filename, std::vector<Vector2> &face)
{
    FILE *fp = fopen(filename, "wb");
    for (size_t i = 0; i < face.size(); ++i) {
        fprintf(fp, "v %f %f %f\n", 
            face[i].x(), 
            face[i].y(), 
            0.0f);
    }
    fprintf(fp, "f");
    for (size_t i = 0; i < face.size(); ++i) {
        fprintf(fp, " %zu", 1 + i);
    }
    fprintf(fp, "\n");
    fclose(fp);
}

void Mesh::exportObj(const char *filename, std::vector<std::vector<Vertex *>> &faces)
{
    std::map<Vertex *, size_t> vertexToIndexMap;
    std::vector<Vertex *> vertices;
    for (const auto &face: faces) {
        for (const auto &vertex: face) {
            auto insertResult = vertexToIndexMap.insert({vertex, vertices.size()});
            if (insertResult.second) {
                vertices.push_back(vertex);
            }
        }
    }
    
    FILE *fp = fopen(filename, "wb");
    for (size_t i = 0; i < vertices.size(); ++i) {
        fprintf(fp, "v %f %f %f\n", 
            vertices[i]->position.x(), 
            vertices[i]->position.y(), 
            vertices[i]->position.z());
    }
    for (const auto &face: faces) {
        fprintf(fp, "f");
        for (const auto &vertex: face) {
            fprintf(fp, " %zu", 1 + vertexToIndexMap[vertex]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Mesh::exportObj(const char *filename, std::vector<Vertex *> &face)
{
    std::vector<std::vector<Vertex *>> faces = {face};
    exportObj(filename, faces);
}

void Mesh::exportPly(const char *filename)
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
    size_t index = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = index++;
        fprintf(fp, "%f %f %f %d %d %d\n", 
            vertex->position.x(), vertex->position.y(), vertex->position.z(),
            vertex->r, vertex->g, vertex->b);
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
 
}
    
}
