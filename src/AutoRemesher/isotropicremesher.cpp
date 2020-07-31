#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/remesh.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <boost/function_output_iterator.hpp>
#include <AutoRemesher/Vector3>
#include <AutoRemesher/IsotropicRemesher>

typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
typedef Kernel::Point_3                                         Point;
typedef CGAL::Surface_mesh<Kernel::Point_3>                     Mesh;
typedef boost::graph_traits<Mesh>::halfedge_descriptor          halfedge_descriptor;
typedef boost::graph_traits<Mesh>::edge_descriptor              edge_descriptor;

struct halfedge2edge
{
    halfedge2edge(const Mesh &m, std::vector<edge_descriptor> &edges)
        : m_mesh(m), m_edges(edges)
    {}
    void operator()(const halfedge_descriptor &h) const
    {
        m_edges.push_back(edge(h, m_mesh));
    }
    const Mesh &m_mesh;
    std::vector<edge_descriptor> &m_edges;
};

namespace AutoRemesher
{

bool IsotropicRemesher::remesh()
{
    Mesh mesh;
    
    std::vector<Mesh::Vertex_index> vertices;
    vertices.reserve(m_vertices.size());
    for (const auto &position: m_vertices)
        vertices.push_back(mesh.add_vertex(Point(position.x(), position.y(), position.z())));
    for (const auto &face: m_triangles)
        mesh.add_face(vertices[face[0]], vertices[face[1]], vertices[face[2]]);
    
    auto ecm = mesh.add_property_map<edge_descriptor, bool>("ecm").first;
    CGAL::Polygon_mesh_processing::detect_sharp_edges(mesh, m_sharpEdgeDegrees, ecm);
    
    std::vector<edge_descriptor> border;
    for (edge_descriptor e: edges(mesh)) {
        if (ecm[e])
            border.push_back(e);
    }
    CGAL::Polygon_mesh_processing::split_long_edges(border, 
        m_targetEdgeLength, mesh, CGAL::Polygon_mesh_processing::parameters::edge_is_constrained_map(ecm));
    
    CGAL::Polygon_mesh_processing::isotropic_remeshing(faces(mesh),
        m_targetEdgeLength,
        mesh,
        CGAL::Polygon_mesh_processing::parameters::number_of_iterations(m_remeshIterations)
        .protect_constraints(true)
        .edge_is_constrained_map(ecm));
    
    Mesh::Property_map<Mesh::Vertex_index, size_t> meshPropertyMap;
    bool created;
    boost::tie(meshPropertyMap, created) = mesh.add_property_map<Mesh::Vertex_index, size_t>("v:source", 0);
    
    for (auto vertexIt = mesh.vertices_begin(); vertexIt != mesh.vertices_end(); vertexIt++) {
        auto point = mesh.point(*vertexIt);
        meshPropertyMap[*vertexIt] = m_remeshedVertices.size();
        m_remeshedVertices.push_back(Vector3 {
            CGAL::to_double(point.x()),
            CGAL::to_double(point.y()),
            CGAL::to_double(point.z()),
        });
    }
    
    for (const auto &faceIt: mesh.faces()) {
        CGAL::Vertex_around_face_iterator<Mesh> vbegin, vend;
        std::vector<size_t> faceIndices;
        for (boost::tie(vbegin, vend) = CGAL::vertices_around_face(mesh.halfedge(faceIt), mesh);
                vbegin != vend;
                ++vbegin) {
            faceIndices.push_back(meshPropertyMap[*vbegin]);
        }
        m_remeshedTriangles.push_back(faceIndices);
    }
        
    return true;
}

void IsotropicRemesher::debugExportObj(const char *filename)
{
    FILE *fp = fopen(filename, "wb");
    for (const auto &it: m_remeshedVertices) {
        fprintf(fp, "v %f %f %f\n",
            it[0], it[1], it[2]);
    }
    for (const auto &it: m_remeshedTriangles) {
        fprintf(fp, "f %zu %zu %zu\n",
            it[0] + 1, it[1] + 1, it[2] + 1);
    }
    fclose(fp);
}

}
