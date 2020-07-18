#ifndef AUTO_REMESHER_GUIDELINE_GENERATOR_H
#define AUTO_REMESHER_GUIDELINE_GENERATOR_H
#include <AutoRemesher/Vector3>
#include <vector>
#include <unordered_map>

namespace AutoRemesher
{
    
class GuidelineGenerator
{
public:
    GuidelineGenerator(const std::vector<Vector3> *vertices,
            const std::vector<std::vector<size_t>> *faces);
    void generate();
    const std::unordered_map<size_t, Vector3> &guidelineVertices() const;
    void debugExportPly(const char *filename, const std::unordered_map<size_t, Vector3> &guidelineVertices) const;
private:
    const std::vector<Vector3> *m_vertices = nullptr;
    const std::vector<std::vector<size_t>> *m_faces = nullptr;
    std::unordered_map<size_t, Vector3> m_guidelineVertices;
};
    
}

#endif
