#include <map>
#include <AutoRemesher/HalfEdge>

namespace AutoRemesher
{

namespace HalfEdge
{
    
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

Mesh::Mesh(const std::vector<Vector3> &vertices,
        std::vector<std::vector<size_t>> &triangles) :
    m_flatVertexPointers(m_vertexCurvatureComparer)
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
            vertex->anyHalfEdge = halfEdge;
            halfEdge->startVertex = vertex;
            halfEdge->previousHalfEdge = halfEdges[h];
            halfEdge->nextHalfEdge = halfEdges[k];
            halfEdge->leftFace = face;
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
    
    if (0 == m_repeatedHalfEdges && 0 == m_aloneHalfEdges) {
        for (Face *face = m_firstFace; nullptr != face; face = face->_next)
            calculateFaceArea(face);
        for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
            calculateVertexCurvature(vertex);
            m_flatVertexPointers.push(vertex);
        }
    }
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
        if (vertex->curvature > maxCurvature)
            maxCurvature = vertex->curvature;
    }
    for (Vertex *vertex = m_firstVertex; nullptr != vertex; vertex = vertex->_next) {
        vertex->index = index++;
        int color = vertex->curvature / maxCurvature * 255;
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
