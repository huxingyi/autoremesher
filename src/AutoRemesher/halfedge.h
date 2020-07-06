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
    double fineCurvature = 0.0;
    double removalCost = 0.0;
    uint32_t version = 0;
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
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
    double length2 = 0.0;
    Vector2 startVertexUv;
};

struct Face
{
    Face *_previous = nullptr;
    Face *_next = nullptr;
    HalfEdge *anyHalfEdge = nullptr;
};

class Mesh
{
public:
    Mesh(const std::vector<Vector3> &vertices,
        std::vector<std::vector<size_t>> &triangles);
    ~Mesh();
    Vertex *allocVertex();
    Face *allocFace();
    HalfEdge *allocHalfEdge();
    void freeVertex(Vertex *vertex);
    void deferedFreeVertex(Vertex *vertex);
    void freeFace(Face *face);
    void freeHalfEdge(HalfEdge *halfEdge);
    void deferedFreeHalfEdge(HalfEdge *halfEdge);
    double calculateVertexCurvature(Vertex *vertex) const;
    double calculateVertexRemovalCost(Vertex *vertex) const;
    bool collapse(Vertex *vertex, std::vector<HalfEdge *> &halfEdgesAroundVertex);
    bool flip(HalfEdge *halfEdge);
    Vector3 calculateVertexNormal(Vertex *vertex) const;
    HalfEdge *findShortestHalfEdgeAroundVertex(Vertex *vertex) const;
    std::vector<std::pair<Vertex *, Vertex *>> collectConesAroundVertexExclude(Vertex *vertex, Vertex *exclude) const;
    void setTargetVertexCount(size_t targetVertexCount);
    bool decimate();
    bool decimate(Vertex *vertex);
    bool parametrize(double gradientSize);
    bool coarseToFineMap();
    bool isWatertight();
    bool delaunayTriangulate(std::vector<Vertex *> &ringVertices,
        const Vector3 &projectNormal, const Vector3 &projectAxis,
        std::vector<std::vector<Vertex *>> *triangles) const;
    void exportPly(const char *filename);
    void exportObj(const char *filename, std::vector<std::vector<Vertex *>> &faces);
    void exportObj(const char *filename, std::vector<Vertex *> &face);
    const size_t &vertexCount() const;
    const size_t &faceCount() const;
    Vertex *firstVertex() const;
    Face *firstFace() const;
    
private:

    Vertex *m_firstVertex = nullptr;
    Vertex *m_lastVertex = nullptr;
    Vertex *m_firstDeferedRemovalVertex = nullptr;
    Face *m_firstFace = nullptr;
    Face *m_lastFace = nullptr;
    HalfEdge *m_firstHalfEdge = nullptr;
    HalfEdge *m_lastHalfEdge = nullptr;
    HalfEdge *m_firstDeferedRemovalHalfEdge = nullptr;
    size_t m_repeatedHalfEdges = 0;
    size_t m_aloneHalfEdges = 0;
    size_t m_vertexCount = 0;
    size_t m_faceCount = 0;
    size_t m_halfEdgeCount = 0;
    size_t m_targetVertexCount = 1000;
    
    struct VertexRemovalCost
    {
        Vertex *vertex;
        double cost;
        uint32_t version;
    };
    
    struct vertexRemovalCostComparer
    {
        bool operator()(const VertexRemovalCost &lhs, const VertexRemovalCost &rhs) const
        {
            return lhs.cost > rhs.cost;
        }
    };
    vertexRemovalCostComparer m_vertexRemovalCostComparer;
    
    std::priority_queue<VertexRemovalCost, std::vector<VertexRemovalCost>, vertexRemovalCostComparer> m_vertexRemovalCostPriorityQueue;
};

}
    
}

#endif
