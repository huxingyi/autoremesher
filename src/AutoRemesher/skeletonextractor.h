#ifndef AUTO_REMESHER_SKELETON_EXTRACTOR_H
#define AUTO_REMESHER_SKELETON_EXTRACTOR_H
#include <AutoRemesher/Vector3>
#include <vector>

namespace AutoRemesher
{
    
class SkeletonExtractor
{
public:
    SkeletonExtractor(const std::vector<Vector3> &vertices,
            const std::vector<std::vector<size_t>> &faces);
    void extract();
private:
    std::vector<Vector3> m_vertices;
    std::vector<std::vector<size_t>> m_faces;
};
    
}

#endif
