#include <map>
#include <cassert>
#include <iostream>
#include <AutoRemesher/HalfEdge>

namespace AutoRemesher
{

namespace HalfEdge
{

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
    for (size_t i = 0; i < triangles.size(); ++i) {
        const auto &triangleIndices = triangles[i];
        for (size_t j = 0; j < 3; ++j) {
            size_t k = (j + 1) % 3;
            const auto &vertexIndex = triangleIndices[j];
            const auto &nextVertexIndex = triangleIndices[k];
            auto findResult = halfEdgeIndexMap.find({vertexIndex, nextVertexIndex});
            auto &halfEdge = findResult->second;
            auto findOppositeResult = halfEdgeIndexMap.find({nextVertexIndex, vertexIndex});
            if (findOppositeResult == halfEdgeIndexMap.end()) {
                std::cerr << "Found alone halfedge" << std::endl;
                ++m_aloneHalfEdges;
                continue;
            }
            auto &oppositeHalfEdge = findOppositeResult->second;
            halfEdge->oppositeHalfEdge = oppositeHalfEdge;
            oppositeHalfEdge->oppositeHalfEdge = halfEdge;
        }
    }
    
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next)
        vertex->fineCurvature = calculateVertexCurvature(vertex);
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->removalCost = calculateVertexRemovalCost(vertex);
        //std::cerr << "removalCost[" << vertex->index << "]:" << vertex->removalCost << std::endl;
        m_vertexRemovalCostPriorityQueue.push({vertex, vertex->removalCost, vertex->version});
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
    return halfEdge;
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
    std::map<Vertex *, std::vector<std::pair<Vertex *, Vertex *>>> vertexCones;
    HalfEdge *halfEdge = shortestHalfEdge;
    do {
        ringVertices.push_back(halfEdge->nextHalfEdge->startVertex);
        vertexCones.insert({halfEdge->nextHalfEdge->startVertex,
            collectConesAroundVertexExclude(halfEdge->nextHalfEdge->startVertex, vertex)});
        halfEdge = halfEdge->nextHalfEdge->nextHalfEdge->oppositeHalfEdge;
    } while (halfEdge != shortestHalfEdge);
    
    std::vector<std::vector<Vertex *>> triangles;
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (shortestHalfEdge->nextHalfEdge->startVertex->position - vertex->position).normalized();
    if (!delaunayTriangulate(ringVertices, projectNormal, projectAxis, &triangles))
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
        std::vector<std::vector<Vertex *>> *triangles) const
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
    Vector3::project(ringPoints, &ringPointsIn2d, projectNormal, projectAxis);
    
    size_t a = 0;
    size_t i = 1;
    for (; i + 2 < ringPointsIn2d.size(); i += 2) {
        size_t b = i;
        size_t c = i + 1;
        size_t d = i + 2;
        if (ringPointsIn2d[d].isInCircle(ringPointsIn2d[a], ringPointsIn2d[b], ringPointsIn2d[c])) {
            triangles->push_back({ringVertices[a], ringVertices[b], ringVertices[d]});
            triangles->push_back({ringVertices[b], ringVertices[c], ringVertices[d]});
        } else {
            triangles->push_back({ringVertices[a], ringVertices[b], ringVertices[c]});
            triangles->push_back({ringVertices[a], ringVertices[c], ringVertices[d]});
        }
    }
    // Add the last triangle if there is
    if (i + 1 < ringPointsIn2d.size()) {
        size_t b = i;
        size_t c = i + 1;
        triangles->push_back({ringVertices[a], ringVertices[b], ringVertices[c]});
    }
    
    return true;
}

bool Mesh::removeVertex(Vertex *target)
{
    HalfEdge *shortestHalfEdge = findShortestHalfEdgeAroundVertex(target);
    
    std::vector<HalfEdge *> halfEdgesToBeRemoval;
    
    std::map<std::pair<Vertex *, Vertex *>, HalfEdge *> ringOppositeHalfEdges;
    std::vector<Vertex *> ringVertices;
    HalfEdge *halfEdge = shortestHalfEdge;
    do {
        auto insertResult = ringOppositeHalfEdges.insert({
            {halfEdge->nextHalfEdge->nextHalfEdge->startVertex, 
                halfEdge->nextHalfEdge->startVertex},
            halfEdge->nextHalfEdge->oppositeHalfEdge
        });
        if (!insertResult.second)
            return false;
        ringVertices.push_back(halfEdge->nextHalfEdge->startVertex);
        halfEdgesToBeRemoval.push_back(halfEdge);
        halfEdge = halfEdge->nextHalfEdge->nextHalfEdge->oppositeHalfEdge;
    } while (halfEdge != shortestHalfEdge);

    std::vector<std::vector<Vertex *>> triangles;
    Vector3 projectNormal = calculateVertexNormal(target);
    Vector3 projectAxis = (shortestHalfEdge->nextHalfEdge->startVertex->position - target->position).normalized();
    if (!delaunayTriangulate(ringVertices, projectNormal, projectAxis, &triangles)) {
        return false;
    }
    
    for (const auto &triangle: triangles) {
        for (size_t j = 0; j < 3; ++j) {
            size_t k = (j + 1) % 3;
            const auto &currentVertex = triangle[j];
            const auto &nextVertex = triangle[k];
            if (ringOppositeHalfEdges.end() != ringOppositeHalfEdges.find({currentVertex, nextVertex})) {
                return false;
            }
        }
    }
    //exportObj("C:\\Users\\Jeremy\\Desktop\\test-ring.obj", ringVertices);
    //exportObj("C:\\Users\\Jeremy\\Desktop\\test-delaunay.obj", triangles);
    
    for (const auto &halfEdge: halfEdgesToBeRemoval) {
        auto &ringVertex = halfEdge->nextHalfEdge->startVertex;
        ringVertex->anyHalfEdge = halfEdge->nextHalfEdge->oppositeHalfEdge->nextHalfEdge;
    }
    
    //std::cerr << "ringVertices.size():" << ringVertices.size() << 
    //    " triangles.size():" << triangles.size() << 
    //    " ringOppositeHalfEdges.size():" << ringOppositeHalfEdges.size() <<
    //    std::endl;
    
    // Free removed faces, halfedges and vertex
    std::cerr << "Free removed faces, halfedges and vertex..." << std::endl;
    for (const auto &halfEdge: halfEdgesToBeRemoval) {
        freeFace(halfEdge->leftFace);
        freeHalfEdge(halfEdge->nextHalfEdge);
        freeHalfEdge(halfEdge->oppositeHalfEdge);
        freeHalfEdge(halfEdge);
    }
    deferedFreeVertex(target);
    
    // Create new halfedges
    std::cerr << "Create new halfedges..." << std::endl;
    for (const auto &triangle: triangles) {
        Face *face = allocFace();
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
            halfEdge->startVertex = triangle[j];
            halfEdge->leftFace = face;
            halfEdge->nextHalfEdge = halfEdges[k];
            halfEdge->previousHalfEdge = halfEdges[h];
            const auto &currentVertex = triangle[j];
            const auto &nextVertex = triangle[k];
            halfEdge->length2 = (currentVertex->position - nextVertex->position).lengthSquared();
            auto insertResult = ringOppositeHalfEdges.insert({
                {currentVertex, nextVertex}, halfEdge
            });
            assert(insertResult.second && "ringOppositeHalfEdges.insert");
            if (!insertResult.second)
                std::cerr << "ringOppositeHalfEdges.insert failed" << std::endl;
        }
    }
    std::cerr << "Link new halfedges..." << std::endl;
    for (auto &it: ringOppositeHalfEdges) {
        auto findOpposite = ringOppositeHalfEdges.find(std::make_pair(it.first.second, it.first.first));
        assert(findOpposite != ringOppositeHalfEdges.end() && "ringOppositeHalfEdges.find");
        if (findOpposite == ringOppositeHalfEdges.end())
            std::cerr << "ringOppositeHalfEdges.find failed" << std::endl;
        findOpposite->second->oppositeHalfEdge = it.second;
        it.second->oppositeHalfEdge = findOpposite->second;
    }
    std::cerr << "Update removal cost for vertices:" << ringOppositeHalfEdges.size() << "..." << std::endl;
    for (auto &it: ringOppositeHalfEdges) {
        it.second->startVertex->removalCost = calculateVertexRemovalCost(it.second->startVertex);
        ++it.second->startVertex->version;
        m_vertexRemovalCostPriorityQueue.push({it.second->startVertex, 
            it.second->startVertex->removalCost, 
            it.second->startVertex->version});
    }
    std::cerr << "Done." << std::endl;
    
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
        if (removeVertex(vertex)) {
            std::cerr << "Vertex " << vertex->index << " removed" << std::endl;
            std::cerr << "Vertices reduced from:" << vertexCountBeforeDecimation << " to:" << m_vertexCount << std::endl;
        }
    }
    std::cerr << "decimated vertices:" << (vertexCountBeforeDecimation - m_vertexCount) << std::endl;
    return vertexCountBeforeDecimation - m_vertexCount > 0;
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
    double maxCurvature = 0.0;
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        if (vertex->removalCost > maxCurvature)
            maxCurvature = vertex->removalCost;
    }
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->index = index++;
        int color = vertex->removalCost / maxCurvature * 255;
        fprintf(fp, "%f %f %f %d %d %d\n", 
            vertex->position.x(), vertex->position.y(), vertex->position.z(),
            color, 0, 0);
    }
    for (Face *face = m_firstFace; nullptr != face; face = face->_next) {
        HalfEdge *h0 = face->anyHalfEdge;
        HalfEdge *h1 = h0->nextHalfEdge;
        HalfEdge *h2 = h1->nextHalfEdge;
        fprintf(fp, "3 %zu %zu %zu\n",
            h0->startVertex->index, 
            h1->startVertex->index, 
            h2->startVertex->index);
    }
    fclose(fp);
}
 
}
    
}
