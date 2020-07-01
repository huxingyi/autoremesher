#include <map>
#include <AutoRemesher/HalfEdge>

namespace AutoRemesher
{

namespace HalfEdge
{

Mesh::Mesh(const std::vector<Vector3> &vertices,
        std::vector<std::vector<size_t>> &triangles)
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
            if (!insertResult.second)
                ++m_repeatedHalfEdges;
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
}

Mesh::~Mesh()
{
    while (nullptr != m_lastVertex)
        freeVertex(m_lastVertex);
    while (nullptr != m_lastFace)
        freeFace(m_lastFace);
    while (nullptr != m_lastHalfEdge)
        freeHalfEdge(m_lastHalfEdge);
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
                halfEdge->previousHalfEdge->startVertex != exclude) {
            cones.push_back({halfEdge->nextHalfEdge->startVertex,
                halfEdge->previousHalfEdge->startVertex});
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
            halfEdge->previousHalfEdge->startVertex->position);
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != vertex->anyHalfEdge);
    return sumOfNormal.normalized();
}

double Mesh::calculateVertexRemovalCost(Vertex *vertex) const
{
    HalfEdge *shortestHalfEdge = findShortestHalfEdgeAroundVertex(vertex);
    
    // Collect fan points
    std::vector<Vertex *> fanVertices;
    std::vector<Vector3> fanPoints;
    std::map<Vertex *, std::vector<std::pair<Vertex *, Vertex *>>> vertexCones;
    HalfEdge *halfEdge = shortestHalfEdge->oppositeHalfEdge->nextHalfEdge;
    fanVertices.push_back(shortestHalfEdge->nextHalfEdge->startVertex);
    vertexCones.insert({shortestHalfEdge->nextHalfEdge->startVertex,
        collectConesAroundVertexExclude(shortestHalfEdge->nextHalfEdge->startVertex, vertex)});
    fanPoints.push_back(shortestHalfEdge->nextHalfEdge->startVertex->position);
    do {
        fanVertices.push_back(halfEdge->nextHalfEdge->startVertex);
        vertexCones.insert({halfEdge->nextHalfEdge->startVertex,
            collectConesAroundVertexExclude(halfEdge->nextHalfEdge->startVertex, vertex)});
        fanPoints.push_back(halfEdge->nextHalfEdge->startVertex->position);
        halfEdge = halfEdge->oppositeHalfEdge->nextHalfEdge;
    } while (halfEdge != shortestHalfEdge);
    
    // Project to 2D plane
    Vector3 projectNormal = calculateVertexNormal(vertex);
    Vector3 projectAxis = (shortestHalfEdge->startVertex->position - vertex->position).normalized();
    std::vector<Vector2> fanPointsIn2d;
    Vector3::project(fanPoints, &fanPointsIn2d, projectNormal, projectAxis);
    
    // Edge flip Delaunay triangulation. 
    // cf. https://dl.acm.org/doi/pdf/10.1145/2980179.2982413 
    // <Interactively Controlled Quad Remeshing of High Resolution 3D Models> Figure 4
    //std::vector<std::pair<Vertex *, Vertex *>> edges;
    std::vector<std::vector<Vertex *>> triangles;
    const auto &a = fanPointsIn2d[0];
    size_t i = 1;
    for (; i + 2 < fanPointsIn2d.size(); ++i) {
        const auto &b = fanPointsIn2d[i];
        const auto &c = fanPointsIn2d[i + 1];
        const auto &d = fanPointsIn2d[i + 2];
        if (d.isInCircle(a, b, c)) {
            // Point is in the circle created by the three points of a triangle, 
            // the triangualtion is not local delaunary
            
            // Flip ac as bd
            //edges.push_back({fanVertices[i], fanVertices[i + 2]});
            triangles.push_back(std::vector<Vertex *> {
                fanVertices[i],
                fanVertices[i + 1],
                fanVertices[i + 2]
            });
            
            // Swap for next loop's b
            std::swap(fanPointsIn2d[i], fanPointsIn2d[i + 1]);
            std::swap(fanVertices[i], fanVertices[i + 1]);
        } else {
            //edges.push_back({fanVertices[0], fanVertices[i + 1]});
            triangles.push_back(std::vector<Vertex *> {
                fanVertices[0],
                fanVertices[i],
                fanVertices[i + 1]
            });
        }
    }
    if (i + 1 < fanPointsIn2d.size()) {
        // Add the last triangle
        triangles.push_back(std::vector<Vertex *> {
            fanVertices[0],
            fanVertices[i],
            fanVertices[i + 1]
        });
    }
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
        oneRingCurvature += std::abs(2.0 * M_PI - sumOfAngle) - it.first->fineCurvature;
    }

    return vertex->fineCurvature + oneRingCurvature;
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
        if (vertex->fineCurvature > maxCurvature)
            maxCurvature = vertex->fineCurvature;
    }
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->index = index++;
        int color = vertex->fineCurvature / maxCurvature * 255;
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
