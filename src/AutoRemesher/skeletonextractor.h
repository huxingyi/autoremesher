#ifndef AUTO_REMESHER_SKELETON_EXTRACTOR_H
#define AUTO_REMESHER_SKELETON_EXTRACTOR_H
#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/extract_mean_curvature_flow_skeleton.h>
#include <CGAL/boost/graph/split_graph_into_polylines.h>
#include <CGAL/Surface_mesh.h>
#include <AutoRemesher/Vector3>
#include <vector>
#include <Eigen/Dense>

namespace AutoRemesher
{

typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
typedef Kernel::Point_3                                         Point;
typedef CGAL::Surface_mesh<Point>                               Mesh;
typedef CGAL::Mean_curvature_flow_skeletonization<Mesh>         Skeletonization;
typedef Skeletonization::Skeleton                               Skeleton;
typedef Skeleton::vertex_descriptor                             Skeleton_vertex;
typedef Skeleton::edge_descriptor                               Skeleton_edge;

class SkeletonExtractor
{
public:
    SkeletonExtractor(const std::vector<Vector3> *vertices,
            const std::vector<std::vector<size_t>> *faces);
    void extract();
    std::vector<std::vector<Skeleton::vertex_descriptor>> convertToStrokes() const;
    Vector3 calculateStrokeTraverseDirection(const std::vector<Skeleton::vertex_descriptor> &stroke) const;
    std::vector<std::vector<Mesh::Vertex_index>> calculateStrokeSeam(const std::vector<Skeleton::vertex_descriptor> &stroke, const Vector3 &traverseDirection) const;
    Mesh &mesh();
    Skeleton &skeleton();
    Mesh::Property_map<Mesh::Vertex_index, size_t> &meshPropertyMap();
    void debugExportPolylines(const char *filename);
    void debugExportStrokes(const char *filename, const std::vector<std::vector<Skeleton::vertex_descriptor>> &strokes);
private:
    const std::vector<Vector3> *m_vertices = nullptr;
    const std::vector<std::vector<size_t>> *m_faces = nullptr;
    Mesh m_mesh;
    Skeleton m_skeleton;
    Mesh::Property_map<Mesh::Vertex_index, size_t> m_meshPropertyMap;
    
    Vector3 calculateStrokeBaseNormal(const std::vector<Skeleton::vertex_descriptor> &stroke, const Vector3 &traverseDirection) const;
};
    
}

#endif
