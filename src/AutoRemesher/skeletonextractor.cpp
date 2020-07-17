#include <CGAL/Simple_cartesian.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/extract_mean_curvature_flow_skeleton.h>
#include <CGAL/boost/graph/split_graph_into_polylines.h>
#include <CGAL/Surface_mesh.h>
#include <AutoRemesher/SkeletonExtractor>

typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
typedef Kernel::Point_3                                         Point;
typedef CGAL::Surface_mesh<Point>                               Mesh;
typedef CGAL::Mean_curvature_flow_skeletonization<Mesh>         Skeletonization;
typedef Skeletonization::Skeleton                               Skeleton;
typedef Skeleton::vertex_descriptor                             Skeleton_vertex;
typedef Skeleton::edge_descriptor                               Skeleton_edge;

namespace AutoRemesher
{

SkeletonExtractor::SkeletonExtractor(const std::vector<Vector3> &vertices,
        const std::vector<std::vector<size_t>> &faces) :
    m_vertices(vertices),
    m_faces(faces)
{
}

void SkeletonExtractor::extract()
{
    Mesh mesh;
    std::vector<Mesh::Vertex_index> vertices;
    vertices.reserve(m_vertices.size());
    for (const auto &position: m_vertices)
        vertices.push_back(mesh.add_vertex(Point(position.x(), position.y(), position.z())));
    for (const auto &face: m_faces)
        mesh.add_face(vertices[face[0]], vertices[face[1]], vertices[face[2]]);
    
    Skeleton skeleton;
    CGAL::extract_mean_curvature_flow_skeleton(mesh, skeleton);
    
    std::ofstream output("C:\\Users\\Jeremy\\Desktop\\test-skeleton.obj");
    std::vector<std::pair<size_t, size_t>> polylines;
    size_t vertexNum = 0;
    for (Skeleton_edge e : CGAL::make_range(edges(skeleton))) {
        const Point &s = skeleton[source(e, skeleton)].point;
        const Point &t = skeleton[target(e, skeleton)].point;
        output << "v " << s << std::endl;
        output << "v " << t << std::endl;
        polylines.push_back({vertexNum + 1, vertexNum + 2});
        vertexNum += 2;
    }
    for (const auto &it: polylines) {
        output << "l " << it.first << " " << it.second << std::endl;
    }
}

}