#ifndef AUTO_REMESHER_MESH_SEGMENTER_H
#define AUTO_REMESHER_MESH_SEGMENTER_H
#include <AutoRemesher/Vector3>
#include <vector>

namespace AutoRemesher
{
    
class MeshSegmenter
{
public:
    MeshSegmenter(const std::vector<Vector3> *vertices,
        const std::vector<std::vector<size_t>> *triangles);
    void segment();
    const std::vector<size_t> &triangleSegmentIds();
private:
    const std::vector<Vector3> *m_vertices = nullptr;
    const std::vector<std::vector<size_t>> *m_triangles = nullptr;
    std::vector<size_t> m_triangleSegmentIds;
};
    
}

#endif
