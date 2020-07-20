#include <CGAL/extract_mean_curvature_flow_skeleton.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/Heat_method_3/Surface_mesh_geodesic_distances_3.h>
#include <AutoRemesher/HeatMapGenerator>

typedef CGAL::Exact_predicates_inexact_constructions_kernel                     Kernel;
typedef Kernel::Point_3                                                         Point_3;
typedef CGAL::Surface_mesh<Point_3>                                             Triangle_mesh;
typedef boost::graph_traits<Triangle_mesh>::vertex_descriptor                   vertex_descriptor;
typedef Triangle_mesh::Property_map<vertex_descriptor,double>                   Vertex_distance_map;
typedef CGAL::Heat_method_3::Surface_mesh_geodesic_distances_3<Triangle_mesh>   Heat_method;

namespace AutoRemesher
{
    
HeatMapGenerator::HeatMapGenerator(const std::vector<Vector3> *vertices,
        const std::vector<std::vector<size_t>> *triangles) :
    m_vertices(vertices),
    m_triangles(triangles)
{
}

const std::vector<double> &HeatMapGenerator::vertexHeatMap() const
{
    return m_vertexHeatMap;
}

void HeatMapGenerator::generate(const std::unordered_set<size_t> &sourceVertices)
{
    if (sourceVertices.empty())
        return;
    
    Triangle_mesh mesh;
    
    std::vector<Triangle_mesh::Vertex_index> addedVertices;
    addedVertices.reserve(m_vertices->size());
    for (const auto &position: *m_vertices)
        addedVertices.push_back(mesh.add_vertex(Point_3(position.x(), position.y(), position.z())));
    for (const auto &face: *m_triangles)
        mesh.add_face(addedVertices[face[0]], addedVertices[face[1]], addedVertices[face[2]]);
    
    Triangle_mesh::Property_map<Triangle_mesh::Vertex_index, size_t> meshVertexToIndexMap;
    bool created;
    boost::tie(meshVertexToIndexMap, created) = mesh.add_property_map<Triangle_mesh::Vertex_index, size_t>("v:source", 0);
    for (size_t i = 0; i < addedVertices.size(); ++i)
        meshVertexToIndexMap[addedVertices[i]] = i;

    Vertex_distance_map vertex_distance = mesh.add_property_map<vertex_descriptor, double>("v:distance", 0).first;
    Heat_method hm(mesh);
    for (const auto &it: sourceVertices)
        hm.add_source(addedVertices[it]);
    hm.estimate_geodesic_distances(vertex_distance);
    m_vertexHeatMap.resize(m_vertices->size());
    for (vertex_descriptor vd : vertices(mesh)){
        m_vertexHeatMap[get(meshVertexToIndexMap, vd)] = get(vertex_distance, vd);
    }
}
    
}
