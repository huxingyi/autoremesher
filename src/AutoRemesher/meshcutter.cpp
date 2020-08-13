/*
 *  Copyright (c) 2020 Jeremy HU <jeremy-at-dust3d dot org>. All rights reserved. 
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:

 *  The above copyright notice and this permission notice shall be included in all
 *  copies or substantial portions of the Software.

 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 *  SOFTWARE.
 */
#include <AutoRemesher/MeshCutter>
#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>
#include <CGAL/boost/graph/graph_traits_Surface_mesh.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <boost/function_output_iterator.hpp>
#if AUTO_REMESHER_DEBUG
#include <QDebug>
#endif

typedef CGAL::Exact_predicates_inexact_constructions_kernel     Kernel;
typedef Kernel::Point_3                                         Point;
typedef CGAL::Surface_mesh<Kernel::Point_3>                     Mesh;
typedef boost::graph_traits<Mesh>::halfedge_descriptor          halfedge_descriptor;
typedef boost::graph_traits<Mesh>::edge_descriptor              edge_descriptor;
typedef boost::graph_traits<Mesh>::vertex_iterator              vertex_iterator;
typedef boost::graph_traits<Mesh>::vertex_descriptor            vertex_descriptor;

namespace AutoRemesher
{

const double g_leftBoxVertices[8][3] = {
    {0, 10000.000000, -10000.000000},
    {0, -10000.000000, -10000.000000},
    {0, 10000.000000, 10000.000000},
    {0, -10000.000000, 10000.000000},
    {-10000.000000, 10000.000000, -10000.000000},
    {-10000.000000, -10000.000000, -10000.000000},
    {-10000.000000, 10000.000000, 10000.000000},
    {-10000.000000, -10000.000000, 10000.000000},
};

const double g_rightBoxVertices[8][3] = {
    {10000.000000, 10000.000000, -10000.000000},
    {10000.000000, -10000.000000, -10000.000000},
    {10000.000000, 10000.000000, 10000.000000},
    {10000.000000, -10000.000000, 10000.000000},
    {0, 10000.000000, -10000.000000},
    {0, -10000.000000, -10000.000000},
    {0, 10000.000000, 10000.000000},
    {0, -10000.000000, 10000.000000},
};

const double g_upBoxVertices[8][3] = {
    {10000.000000, 10000.000000, -10000.000000},
    {10000.000000, 0.000000, -10000.000000},
    {10000.000000, 10000.000000, 10000.000000},
    {10000.000000, 0.000000, 10000.000000},
    {-10000.000000, 10000.000000, -10000.000000},
    {-10000.000000, 0.000000, -10000.000000},
    {-10000.000000, 10000.000000, 10000.000000},
    {-10000.000000, 0.000000, 10000.000000},
};

static void initializeBoxMesh(Mesh *mesh, const double boxVertices[8][3])
{
    std::vector<Mesh::Vertex_index> newVertices;
    newVertices.reserve(8);
    for (size_t i = 0; i < 8; ++i)
        newVertices.push_back(mesh->add_vertex(Point(boxVertices[i][0], boxVertices[i][1], boxVertices[i][2])));
    mesh->add_face(newVertices[4], newVertices[2], newVertices[0]);
    mesh->add_face(newVertices[2], newVertices[7], newVertices[3]);
    mesh->add_face(newVertices[6], newVertices[5], newVertices[7]);
    mesh->add_face(newVertices[1], newVertices[7], newVertices[5]);
    mesh->add_face(newVertices[0], newVertices[3], newVertices[1]);
    mesh->add_face(newVertices[4], newVertices[1], newVertices[5]);
    mesh->add_face(newVertices[4], newVertices[6], newVertices[2]);
    mesh->add_face(newVertices[2], newVertices[6], newVertices[7]);
    mesh->add_face(newVertices[6], newVertices[4], newVertices[5]);
    mesh->add_face(newVertices[1], newVertices[3], newVertices[7]);
    mesh->add_face(newVertices[0], newVertices[2], newVertices[3]);
    mesh->add_face(newVertices[4], newVertices[0], newVertices[1]);
}
    
void MeshCutter::cut()
{
    Mesh mesh;
    
    std::vector<Mesh::Vertex_index> newVertices;
    newVertices.reserve(m_vertices.size());
    for (const auto &position: m_vertices)
        newVertices.push_back(mesh.add_vertex(Point(position.x(), position.y(), position.z())));
    for (const auto &face: m_triangles)
        mesh.add_face(newVertices[face[0]], newVertices[face[1]], newVertices[face[2]]);
    
    Mesh firstMesh;
    Mesh secondMesh;
    initializeBoxMesh(&firstMesh, g_upBoxVertices);
    initializeBoxMesh(&secondMesh, g_rightBoxVertices);
    
    CGAL::Polygon_mesh_processing::corefine_and_compute_intersection(firstMesh, mesh, firstMesh);
    CGAL::Polygon_mesh_processing::corefine_and_compute_intersection(secondMesh, mesh, secondMesh);
    
    auto fetchFromMesh = [&](Mesh &mesh, std::vector<Vector3> *resultVertices, std::vector<std::vector<size_t>> *resultTriangles) {
        Mesh::Property_map<Mesh::Vertex_index, size_t> meshPropertyMap;
        bool created;
        boost::tie(meshPropertyMap, created) = mesh.add_property_map<Mesh::Vertex_index, size_t>("v:source", 0);
        for (auto vertexIt = mesh.vertices_begin(); vertexIt != mesh.vertices_end(); vertexIt++) {
            auto point = mesh.point(*vertexIt);
            meshPropertyMap[*vertexIt] = resultVertices->size();
            resultVertices->push_back(Vector3 {
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
            resultTriangles->push_back(faceIndices);
        }
    };
    
    fetchFromMesh(firstMesh, &m_firstHalfVertices, &m_firstHalfTriangles);
    fetchFromMesh(secondMesh, &m_secondHalfVertices, &m_secondHalfTriangles);
}

}
