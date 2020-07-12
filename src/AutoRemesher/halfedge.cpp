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
    
const bool Mesh::m_enableDecimationLog = false;
    
typedef QEx::TransitionFunctionDouble Transition;
    
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
    while (nullptr != m_lastDecimationLog)
        freeDecimationLog(m_lastDecimationLog);
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

void Mesh::freeDecimationLog(DecimationLog *decimationLog)
{
    if (decimationLog == m_firstDecimationLog)
        m_firstDecimationLog = decimationLog->_next;
    if (decimationLog == m_lastDecimationLog)
        m_lastDecimationLog = decimationLog->_previous;
    if (nullptr != decimationLog->_next)
        decimationLog->_next->_previous = decimationLog->_previous;
    if (nullptr != decimationLog->_previous)
        decimationLog->_previous->_next = decimationLog->_next;
    delete decimationLog;
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

DecimationLog *Mesh::allocDecimationLog()
{
    DecimationLog *decimationLog = new DecimationLog;
    decimationLog->_previous = m_lastDecimationLog;
    if (nullptr != m_lastDecimationLog)
        m_lastDecimationLog->_next = decimationLog;
    else
        m_firstDecimationLog = decimationLog;
    m_lastDecimationLog = decimationLog;
    return decimationLog;
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
    
    // The following two checks are necessary, unless non-manifold vertex will be created because of collapse 
    if (shortestHalfEdge->nextHalfEdge->nextHalfEdge->startVertex->halfEdgeCount < 4)
        return std::numeric_limits<double>::max();
    if (shortestHalfEdge->oppositeHalfEdge->previousHalfEdge->startVertex->halfEdgeCount < 4)
        return std::numeric_limits<double>::max();
    
    std::vector<Vertex *> ringVertices;
    std::set<Vertex *> verticesAroundTarget;
    std::map<Vertex *, std::vector<std::pair<Vertex *, Vertex *>>> vertexCones;
    std::set<std::pair<Vertex *, Vertex *>> halfEdges;
    HalfEdge *halfEdge = shortestHalfEdge;
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
    } while (halfEdge != shortestHalfEdge);
    
    if (verticesAroundTarget.size() < 4)
        return std::numeric_limits<double>::max();
    
    if (vertex->halfEdgeCount != verticesAroundTarget.size()) {
        //std::cerr << "vertex halfEdgeCount:" << vertex->halfEdgeCount << " verticesAroundTarget:" << verticesAroundTarget.size() << std::endl;
        //vertex->g = 255;
        //exportPly("C:\\Users\\Jeremy\\Desktop\\test-halfedge.ply");
        //exit(0);
        return std::numeric_limits<double>::max();
    }
    
    std::vector<std::vector<Vertex *>> triangles;
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (shortestHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
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
    
    auto k = halfEdgesPointToTarget.size();
    
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (shortestHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
    std::vector<Vector2> ringPointsIn2d;
    Vector3::project(ringPoints, &ringPointsIn2d, projectNormal, projectAxis, vertex->position);
    
    if (m_enableDecimationLog) {
        
        const Vector2 origin2d = Vector2(0.0, 0.0);
        int alphaIndex = -1;
        for (size_t i = 1; i <= k - 2; ++i) {
            if (origin2d.isOnLeft(ringPointsIn2d[0], ringPointsIn2d[i]) != 
                    origin2d.isOnLeft(ringPointsIn2d[0], ringPointsIn2d[i + 1])) {
                alphaIndex = i;
                break;
            }
        }
        if (-1 == alphaIndex) {
            //std::cerr << "No barycentricCoordinates found" << std::endl;
            return false;
        }
        
        /*
        static int s_count = 0;
        ++s_count;
        extern int gDebugIndex;
        if (gDebugIndex == s_count) {
            {
                std::vector<std::vector<Vector3>> debugFaces;
                for (size_t i = 0; i < ringPoints.size(); ++i) {
                    debugFaces.push_back({
                        ringPoints[i], vertex->position, ringPoints[(i + 1) % ringPoints.size()]
                    });
                }
                exportObj("C:\\Users\\Jeremy\\Desktop\\test-debug-0.obj", debugFaces);
            }
            {
                std::vector<std::vector<Vector2>> debugFaces;
                for (size_t i = 0; i < ringPointsIn2d.size(); ++i) {
                    debugFaces.push_back({
                        ringPointsIn2d[i], origin2d, ringPointsIn2d[(i + 1) % ringPointsIn2d.size()]
                    });
                }
                exportObj("C:\\Users\\Jeremy\\Desktop\\test-debug-1.obj", debugFaces);
            }
            {
                std::vector<std::vector<Vector2>> debugFaces;
                for (size_t i = 2; i < ringPointsIn2d.size(); ++i) {
                    debugFaces.push_back({
                        ringPointsIn2d[i - 1], ringPointsIn2d[0], ringPointsIn2d[i]
                    });
                }
                exportObj("C:\\Users\\Jeremy\\Desktop\\test-debug-2.obj", debugFaces);
            }
            exit(0);
        }
        */
    
    
        DecimationLog *collapseLog = allocDecimationLog();
        collapseLog->k = k;
        collapseLog->h = halfEdgesPointToTarget;
        collapseLog->h_x.reserve(collapseLog->h.size());
        collapseLog->ring.reserve(collapseLog->h.size());
        for (size_t i = 0; i < collapseLog->h.size(); ++i) {
            collapseLog->h_x.push_back(collapseLog->h[i]->oppositeHalfEdge);
            collapseLog->ring.push_back(collapseLog->h[i]->previousHalfEdge);
        }
        Vector2 uv = Vector2::barycentricCoordinates(ringPointsIn2d[alphaIndex], ringPointsIn2d[0], ringPointsIn2d[alphaIndex + 1], origin2d);
        collapseLog->alpha = uv.x();
        collapseLog->beta = uv.y();
        collapseLog->gamma = 1.0 - (uv.x() + uv.y());
        collapseLog->alpha_i = alphaIndex;
        collapseLog->gamma_i = alphaIndex + 1;
    }
    
    //std::cerr << "uv:" << uv << " alpha:" << alphaIndex << " k:" << k << std::endl;

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
                if (m_enableDecimationLog) {
                    DecimationLog *flipLog = allocDecimationLog();
                    flipLog->k = 0;
                    flipLog->hflip = hflip;
                    flipLog->hflip_x = hflip_x;
                    flipLog->ha = hflip->previousHalfEdge;
                    flipLog->hb = hflip_x->previousHalfEdge;
                    flipLog->hc = hflip->nextHalfEdge;
                    flipLog->hd = hflip_x->nextHalfEdge;
                }
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

static inline Vector2 transformedPoint(const Transition &t, Vector2 uv) 
{
    t.transform_point(uv);
    return uv;
}

void Mesh::unFlip(HalfEdge *hflip, HalfEdge *hflip_x, 
        HalfEdge *ha, HalfEdge *hb, HalfEdge *hc, HalfEdge *hd)
{
    // cf. <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 6
    
    auto fjb = hc->startVertexUv;
    auto fjc = hflip_x->startVertexUv;
    auto fjflip_x = hb->startVertexUv;
    
    auto fja = hd->startVertexUv;
    auto fjd = hflip->startVertexUv;
    auto fjflip = ha->startVertexUv;
    
    auto &fib = hflip_x->startVertexUv;
    //auto &fiflip_x = hd->startVertexUv;
    auto &fid = hb->startVertexUv;
    
    auto &fia = hflip->startVertexUv;
    //auto &fiflip = hc->startVertexUv;
    auto &fic = ha->startVertexUv;

    Transition tjflip;
    tjflip.estimate_from_point_pair<Vector2>(fjd, fjflip_x, fjflip, fjc);
    
    //fiflip = fjb;
    //fiflip_x = fja;
    
    fia = transformedPoint(tjflip, fja);
    
    fib = transformedPoint(tjflip.inverse(), fjb);
    
    fic = fjc;
    fid = fjd;
    
    /*
    exportObj("C:\\Users\\Jeremy\\Desktop\\test-unflip-before.obj", std::vector<std::vector<Vector2>> {
        {fjb, fjc, fjflip_x},
        {fja, fjd, fjflip},
    });
    exportObj("C:\\Users\\Jeremy\\Desktop\\test-unflip-after.obj", std::vector<std::vector<Vector2>> {
        {fib, fiflip_x, fid},
        {fia, fiflip, fic}
    });
    */
}

void Mesh::unCollapse(int k,
        std::vector<HalfEdge *> &h, std::vector<HalfEdge *> &h_x,
        std::vector<HalfEdge *> &ring, 
        double alpha, double beta, double gamma,
        int alpha_i, int gamma_i)
{
    static int s_count = 0;
    ++s_count;
    extern int gDebugIndex;
        
    // cf. <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 7
    double averageEdgeLengthBefore = 0.0;
    if (-1 == gDebugIndex) {
        if (4 == k) {
            std::cerr << "Good debug index:" << s_count << std::endl;
            exit(0);
        }
        double edgeCount = 0;
        for (int i = 2; i < k - 2; ++i) {
            //int j = i + 1;
            //if (h_x[i]->startVertexUv != h_x[j]->startVertexUv) {
            //    std::cerr << "Good debug index:" << s_count << std::endl;
            //    exit(0);
            //}
            averageEdgeLengthBefore += (h_x[i]->startVertexUv - ring[i - 1]->startVertexUv).length();
            ++edgeCount;
        }
        averageEdgeLengthBefore /= edgeCount;
    }
    
    if (gDebugIndex == s_count) {
        std::cerr << "k:" << k << std::endl;
        std::cerr << "alpha:" << alpha << std::endl;
        std::cerr << "beta:" << beta << std::endl;
        std::cerr << "gamma:" << gamma << std::endl;
        std::cerr << "alpha_i:" << alpha_i << std::endl;
        std::cerr << "gamma_i:" << gamma_i << std::endl;
        
        {
            std::vector<std::vector<Vector2>> debugFaces;
            debugFaces.push_back({
                h_x[2]->startVertexUv,
                ring[1]->startVertexUv,
                ring[0]->startVertexUv
            });
            for (int i = 2; i < k - 2; ++i) {
                debugFaces.push_back({
                    h[i]->startVertexUv,
                    h_x[i + 1]->startVertexUv,
                    ring[i]->startVertexUv
                });
            }
            debugFaces.push_back({
                h[k - 2]->startVertexUv,
                ring[k - 1]->startVertexUv,
                ring[k - 2]->startVertexUv
            });
            exportObj("C:\\Users\\Jeremy\\Desktop\\test-uncollapse-before.obj", debugFaces);
        }
    }
    
    std::vector<Transition> tj_x(k, Transition::IDENTITY);
    for (int i = 2; i <= k - 2; ++i) {
        tj_x[i].estimate_from_point_pair<Vector2>(
            h_x[i]->startVertexUv, 
            i + 1 <= k - 2 ? h_x[i + 1]->startVertexUv : ring[k - 1]->startVertexUv,
            ring[i - 1]->startVertexUv, 
            h[i]->startVertexUv);
        if (gDebugIndex == s_count) {
            std::cerr << "tj_x[" << i << "]:" << tj_x[i] << std::endl;
        }
    }
    
    //std::cerr << "alpha_i:" << alpha_i << " k:" << k << std::endl;
    Vector2 unCollapsedVertexUv;
    if (alpha_i < 2) {
        unCollapsedVertexUv = alpha * ring[alpha_i - 1]->startVertexUv +
            beta * h_x[2]->startVertexUv +
            gamma * ring[alpha_i]->startVertexUv;
        h_x[2]->startVertexUv = unCollapsedVertexUv;
        //std::cerr << "h_x[" << 2 << "]:" << h_x[2]->startVertexUv << std::endl;
        {
            auto uv = unCollapsedVertexUv;
            for (int i = 2; i <= k - 2; ++i) {
                uv = transformedPoint(tj_x[i], uv);
                h_x[i + 1]->startVertexUv = uv;
                //std::cerr << "h_x[" << (i + 1) << "]:" << h_x[i + 1]->startVertexUv << std::endl;
            }
        }
    } else if (alpha_i >= k - 2) {
        unCollapsedVertexUv = alpha * h[alpha_i]->startVertexUv +
            beta * ring[k - 1]->startVertexUv +
            gamma * ring[k - 2]->startVertexUv;
        h_x[k - 1]->startVertexUv = unCollapsedVertexUv;
        //std::cerr << "unCollapsedVertexUv:" << unCollapsedVertexUv << std::endl;
        {
            auto uv = unCollapsedVertexUv;
            for (int i = k - 2; i >= 2; --i) {
                uv = transformedPoint(tj_x[i].inverse(), uv);
                h_x[i]->startVertexUv = uv;
                //std::cerr << "h_x[" << i << "]:" << h_x[i]->startVertexUv << std::endl;
            }
        }
    } else {
        unCollapsedVertexUv = alpha * h[alpha_i]->startVertexUv +
            beta * h_x[gamma_i]->startVertexUv +
            gamma * ring[alpha_i]->startVertexUv;
        h_x[gamma_i]->startVertexUv = unCollapsedVertexUv;
        //std::cerr << "h_x[" << gamma_i << "]:" << h_x[gamma_i]->startVertexUv << std::endl;
        {
            auto uv = unCollapsedVertexUv;
            for (int i = gamma_i; i <= k - 2; ++i) {
                uv = transformedPoint(tj_x[i], uv);
                h_x[i + 1]->startVertexUv = uv;
                //std::cerr << "h_x[" << (i + 1) << "]:" << h_x[i + 1]->startVertexUv << std::endl;
            }
        }
        {
            auto uv = unCollapsedVertexUv;
            for (int i = gamma_i - 1; i >= 2; --i) {
                uv = transformedPoint(tj_x[i].inverse(), uv);
                h_x[i]->startVertexUv = uv;
                //std::cerr << "h_x[" << i << "]:" << h_x[i]->startVertexUv << std::endl;
            }
        }
    }
    
    h_x[0]->startVertexUv = h_x[k - 1]->startVertexUv;
    h_x[1]->startVertexUv = h_x[2]->startVertexUv;
    
    {
        auto uv = ring[k - 1]->startVertexUv;
        for (int i = k - 2; i >= 2; --i) {
            uv = transformedPoint(tj_x[i].inverse(), uv);
        }
        h[0]->startVertexUv = uv;
    }
    h[k - 1]->startVertexUv = ring[k - 2]->startVertexUv;
    h[1]->startVertexUv = ring[0]->startVertexUv;
    
    double averageEdgeLengthAfter = 0.0;
    if (-1 == gDebugIndex) {
        double edgeCount = 0;
        for (int i = 2; i < k - 2; ++i) {
            averageEdgeLengthAfter += (h_x[i]->startVertexUv - ring[i - 1]->startVertexUv).length();
            ++edgeCount;
        }
        averageEdgeLengthAfter /= edgeCount;
        if (averageEdgeLengthAfter > averageEdgeLengthBefore * 3) {
            std::cerr << "averageEdgeLengthAfter:" << averageEdgeLengthAfter << " averageEdgeLengthBefore:" << averageEdgeLengthBefore << std::endl;
            std::cerr << "Good debug index:" << s_count << std::endl;
            exit(0);
        }
    }
    
    {
        if (gDebugIndex == s_count) {
            std::vector<std::vector<Vector2>> debugFaces;
            for (int i = 0; i <= k - 1; ++i) {
                debugFaces.push_back({
                    h[i]->startVertexUv,
                    h_x[(i + 1) % k]->startVertexUv,
                    ring[i]->startVertexUv
                });
            }
            exportObj("C:\\Users\\Jeremy\\Desktop\\test-uncollapse-after.obj", debugFaces);
            exit(0);
        }
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
        if (decimate(vertex)) {
            //std::cerr << "Vertex " << vertex->index << " decimated at cost:" << vertex->removalCost << std::endl;
            //std::cerr << "Vertices reduced from:" << vertexCountBeforeDecimation << " to:" << m_vertexCount << std::endl;
        }
    }
    std::cerr << "decimated vertices:" << (vertexCountBeforeDecimation - m_vertexCount) << " from:" << vertexCountBeforeDecimation << std::endl;
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
    size_t i = 0;
    while (nullptr != m_lastDecimationLog) {
        ++i;
        auto &decimationLog = m_lastDecimationLog;
        if (0 == decimationLog->k) {
            //std::cerr << "[" << i << "]:unFlip..." << std::endl;
            unFlip(decimationLog->hflip, 
                decimationLog->hflip_x, 
                decimationLog->ha, 
                decimationLog->hb, 
                decimationLog->hc, 
                decimationLog->hd);
        } else {
            //std::cerr << "[" << i << "]:unCollapse..." << std::endl;
            unCollapse(decimationLog->k,
                decimationLog->h, 
                decimationLog->h_x,
                decimationLog->ring, 
                decimationLog->alpha, 
                decimationLog->beta, 
                decimationLog->gamma,
                decimationLog->alpha_i, 
                decimationLog->gamma_i);
        }
        freeDecimationLog(decimationLog);
    }
    std::cerr << "Coarse to fine map done" << std::endl;
    return true;
}

void Mesh::exportObj(const char *filename, std::vector<Vector2> &face)
{
    FILE *fp = fopen(filename, "wb");
    for (size_t i = 0; i < face.size(); ++i) {
        fprintf(fp, "v %f %f %f\n", 
            face[i].x(), 
            0.0f, 
            face[i].y());
    }
    fprintf(fp, "f");
    for (size_t i = 0; i < face.size(); ++i) {
        fprintf(fp, " %zu", 1 + i);
    }
    fprintf(fp, "\n");
    fclose(fp);
}

void Mesh::exportObj(const char *filename, std::vector<std::vector<Vector2>> &faces)
{
    FILE *fp = fopen(filename, "wb");
    for (const auto &face: faces) {
        for (size_t i = 0; i < face.size(); ++i) {
            fprintf(fp, "v %f %f %f\n", 
                face[i].x(), 
                0.0f,
                face[i].y());
        }
    }
    size_t vertexIndex = 0;
    for (const auto &face: faces) {
        fprintf(fp, "f");
        for (size_t i = 0; i < face.size(); ++i) {
            fprintf(fp, " %zu", 1 + (vertexIndex++));
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
}

void Mesh::exportObj(const char *filename, std::vector<std::vector<Vector3>> &faces)
{
    FILE *fp = fopen(filename, "wb");
    for (const auto &face: faces) {
        for (size_t i = 0; i < face.size(); ++i) {
            fprintf(fp, "v %f %f %f\n", 
                face[i].x(), 
                face[i].z(), 
                face[i].y());
        }
    }
    size_t vertexIndex = 0;
    for (const auto &face: faces) {
        fprintf(fp, "f");
        for (size_t i = 0; i < face.size(); ++i) {
            fprintf(fp, " %zu", 1 + (vertexIndex++));
        }
        fprintf(fp, "\n");
    }
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

void Mesh::exportObj(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    
    size_t index = 0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->outputIndex = index++;
        fprintf(fp, "v %f %f %f\n", 
            (double)vertex->position.x(), 
            (double)vertex->position.y(), 
            (double)vertex->position.z());
    }
    
    size_t count = 0;
    for (const Face *face = firstFace(); nullptr != face; face = face->_next) {
        ++count;
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        fprintf(fp, "f %zu %zu %zu\n",
            1 + h0->startVertex->outputIndex, 
            1 + h1->startVertex->outputIndex, 
            1 + h2->startVertex->outputIndex);
    }
    if (count != m_faceCount) {
        std::cerr << "Face count:" << m_faceCount << " but output count:" << count << std::endl;
    }
    
    fclose(fp);
}

void Mesh::exportPly(const char *filename) const
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
