#include <AutoRemesher/GuidelineGenerator>
#include <AutoRemesher/SkeletonExtractor>
#include <unordered_set>
#include <cstdio>
#include <iostream>

namespace AutoRemesher
{
    
GuidelineGenerator::GuidelineGenerator(const std::vector<Vector3> *vertices,
        const std::vector<std::vector<size_t>> *faces) :
    m_vertices(vertices),
    m_faces(faces)
{
}

void GuidelineGenerator::generate()
{
    SkeletonExtractor skeletonExtractor(m_vertices, m_faces);
    skeletonExtractor.extract();
    std::vector<std::vector<Skeleton::vertex_descriptor>> strokes = skeletonExtractor.convertToStrokes();
    const auto &meshPropertyMap = skeletonExtractor.meshPropertyMap();
    std::unordered_set<size_t> guidelineVertices;
    for (const auto &stroke: strokes) {
        std::vector<std::vector<Mesh::Vertex_index>> seam = skeletonExtractor.calculateStrokeSeam(stroke);
        if (seam.empty())
            continue;
        for (const auto &seamVertices: seam) {
            for (const auto &v: seamVertices)
                guidelineVertices.insert(meshPropertyMap[v]);
        }
    }
    skeletonExtractor.debugExportStrokes("C:\\Users\\Jeremy\\Desktop\\test-skeleton-strokes.obj", strokes);
    debugExportPly("C:\\Users\\Jeremy\\Desktop\\test-skeleton-guideline.ply", guidelineVertices);
}

void GuidelineGenerator::debugExportPly(const char *filename, const std::unordered_set<size_t> &guidelineVertices)
{
    FILE *fp = fopen(filename, "wb");
    fprintf(fp, "ply\n");
    fprintf(fp, "format ascii 1.0\n");
    fprintf(fp, "element vertex %zu\n", m_vertices->size());
    fprintf(fp, "property float x\n");
    fprintf(fp, "property float y\n");
    fprintf(fp, "property float z\n");
    fprintf(fp, "property uchar red\n");
    fprintf(fp, "property uchar green\n");
    fprintf(fp, "property uchar blue\n");
    fprintf(fp, "element face %zu\n", m_faces->size());
    fprintf(fp, "property list uchar uint vertex_indices\n");
    fprintf(fp, "end_header\n");
    for (size_t i = 0; i < m_vertices->size(); ++i) {
        const auto &it = (*m_vertices)[i];
        int r, g, b;
        r = g = b = 0;
        if (guidelineVertices.end() != guidelineVertices.find(i))
            r = 255;
        fprintf(fp, "%f %f %f %d %d %d\n", 
            it.x(), it.y(), it.z(),
            r, g, b);
    }
    for (const auto &it: *m_faces) {
        fprintf(fp, "3 %zu %zu %zu\n",
            it[0], it[1], it[2]);
    }
    fclose(fp);
}

}
