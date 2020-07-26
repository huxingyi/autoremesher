#ifndef AUTO_REMESHER_HALF_EDGE_H
#define AUTO_REMESHER_HALF_EDGE_H
#include <vector>
#include <cstddef>
#include <queue>
#include <limits>
#include <stack>
#include <unordered_map>
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
    double fineCurvature = 0.0;
    double removalCost = 0.0;
    uint32_t version = 0;
    uint8_t debugColor = 0;
    double heat = 0;
    double heat2 = 0;
    Vector3 normal;
    Vector3 averageNormal;
    double relativeHeight = 0.0;
    double nextRelativeHeight = 0.0;
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
    uint8_t featured = 0;
    Vector2 startVertexUv;
    int degreesBetweenFaces = -1;
};

struct Face
{
    Face *_previous = nullptr;
    Face *_next = nullptr;
    uint8_t isGuideline = false;
    Vector3 guidelineDirection;
    HalfEdge *anyHalfEdge = nullptr;
    Vector3 normal;
    size_t segmentId = 0;
};

class Mesh
{
public:
    Mesh(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &triangles,
        const std::unordered_map<size_t, Vector3> &guidelineVertices=std::unordered_map<size_t, Vector3>());
    ~Mesh();
    Vertex *allocVertex();
    Face *allocFace();
    HalfEdge *allocHalfEdge();
    bool isVertexConstrained(Vertex *vertex) const;
    void freeVertex(Vertex *vertex);
    void deferedFreeVertex(Vertex *vertex);
    void freeFace(Face *face);
    void freeHalfEdge(HalfEdge *halfEdge);
    void deferedFreeHalfEdge(HalfEdge *halfEdge);
    void deferedFreeFace(Face *face);
    double calculateVertexCurvature(Vertex *vertex) const;
    double calculateVertexRemovalCost(Vertex *vertex) const;
    bool collapse(Vertex *vertex, std::vector<HalfEdge *> &halfEdgesAroundVertex);
    bool flip(HalfEdge *halfEdge);
    void unFlip(HalfEdge *hflip, HalfEdge *hflip_x, 
        HalfEdge *ha, HalfEdge *hb, HalfEdge *hc, HalfEdge *hd);
    void unCollapse(int k,
        std::vector<HalfEdge *> &h, std::vector<HalfEdge *> &h_x,
        std::vector<HalfEdge *> &ring, 
        double alpha, double beta, double gamma,
        int alpha_i, int gamma_i);
    Vector3 calculateVertexNormal(Vertex *vertex) const;
    HalfEdge *findShortestHalfEdgeAroundVertex(Vertex *vertex) const;
    std::vector<std::pair<Vertex *, Vertex *>> collectConesAroundVertexExclude(Vertex *vertex, Vertex *exclude) const;
    void setTargetVertexCount(size_t targetVertexCount);
    bool decimate();
    bool decimate(Vertex *vertex);
    bool parametrize(double gradientSize, double constraintStength);
    bool isWatertight();
    bool delaunayTriangulate(std::vector<Vertex *> &ringVertices,
        const Vector3 &projectNormal, const Vector3 &projectAxis,
        std::vector<std::vector<Vertex *>> *triangles,
        const Vector3 &origin) const;
    const size_t &vertexCount() const;
    const size_t &faceCount() const;
    Vertex *firstVertex() const;
    Face *firstFace() const;
    HalfEdge *findHalfEdgeBetweenVertices(Vertex *firstVertex, Vertex *secondVertex);
    void markGuidelineEdgesAsFeatured();
    void calculateAnglesBetweenFaces();
    void calculateFaceNormals();
    void calculateVertexNormals();
    void calculateVertexAverageNormals();
    void calculateVertexRelativeHeights();
    void expandVertexRelativeHeights();
    void normalizeVertexRelativeHeights();
    bool isVertexMixed(Vertex *vertex) const;
    void debugResetColor();
    void debugExportGuidelinePly(const char *filename);
    void debugExportPly(const char *filename);
    void debugExportUvObj(const char *filename);
    void debugExportCurvaturePly(const char *filename);
    void debugExportEdgeAnglesPly(const char *filename);
    void debugExportSegmentEdgesPly(const char *filename);
    void debugExportVertexHeatMapPly(const char *filename);
    void debugExportVertexRelativeHeightPly(const char *filename);
    
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
    size_t m_targetVertexCount = 5000;
    
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
