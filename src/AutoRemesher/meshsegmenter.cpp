#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_items_with_id_3.h>
#include <CGAL/Simple_cartesian.h>
#include <CGAL/extract_mean_curvature_flow_skeleton.h>
#include <CGAL/mesh_segmentation.h>
#include <AutoRemesher/MeshSegmenter>

// https://doc.cgal.org/latest/Surface_mesh_skeletonization/Surface_mesh_skeletonization_2segmentation_example_8cpp-example.html#a5

typedef CGAL::Simple_cartesian<double>                               Kernel;
typedef Kernel::Point_3                                              Point;
typedef CGAL::Polyhedron_3<Kernel, CGAL::Polyhedron_items_with_id_3> Polyhedron;
typedef Polyhedron::HalfedgeDS                                       HalfedgeDS;
typedef boost::graph_traits<Polyhedron>::vertex_descriptor           vertex_descriptor;
typedef boost::graph_traits<Polyhedron>::halfedge_descriptor         halfedge_descriptor;
typedef boost::graph_traits<Polyhedron>::face_descriptor             face_descriptor;
typedef CGAL::Mean_curvature_flow_skeletonization<Polyhedron>        Skeletonization;
typedef Skeletonization::Skeleton                                    Skeleton;
typedef Skeleton::vertex_descriptor                                  Skeleton_vertex;

namespace AutoRemesher
{
    
// Property map associating a facet with an integer as id to an
// element in a vector stored internally
template<class ValueType>
struct Facet_with_id_pmap
    : public boost::put_get_helper<ValueType&,
             Facet_with_id_pmap<ValueType> >
{
    typedef face_descriptor key_type;
    typedef ValueType value_type;
    typedef value_type& reference;
    typedef boost::lvalue_property_map_tag category;
    Facet_with_id_pmap(
      std::vector<ValueType>& internal_vector
    ) : internal_vector(internal_vector) { }
    reference operator[](key_type key) const
    { return internal_vector[key->id()]; }
private:
    std::vector<ValueType>& internal_vector;
};

template <class HDS>
class Build_mesh : public CGAL::Modifier_base<HDS> 
{
public:
    Build_mesh(const std::vector<Vector3> *vertices,
            const std::vector<std::vector<size_t>> *faces) :
        m_vertices(vertices),
        m_faces(faces)
    {
    }
    
    void operator()( HDS& hds) 
    {
        CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, false);
        B.begin_surface(m_vertices->size(), m_faces->size());
        typedef typename HDS::Vertex   Vertex;
        typedef typename Vertex::Point Point;
        for (const auto &it: *m_vertices)
            B.add_vertex(Point(it.x(), it.y(), it.z()));
        for (const auto &it: *m_faces) {
            B.begin_facet();
            B.add_vertex_to_facet(it[0]);
            B.add_vertex_to_facet(it[1]);
            B.add_vertex_to_facet(it[2]);
            B.end_facet();
        }
        B.end_surface();
    }
    
private:
    const std::vector<Vector3> *m_vertices = nullptr;
    const std::vector<std::vector<size_t>> *m_faces = nullptr;
};
    
MeshSegmenter::MeshSegmenter(const std::vector<Vector3> *vertices,
        const std::vector<std::vector<size_t>> *triangles) :
    m_vertices(vertices),
    m_triangles(triangles)
{
}

const std::vector<size_t> &MeshSegmenter::triangleSegmentIds()
{
    return m_triangleSegmentIds;
}

void MeshSegmenter::segment()
{
    if (m_triangles->empty())
        return;
    
    Polyhedron mesh;
    Skeleton skeleton;
    
    Build_mesh<HalfedgeDS> meshBuilder(m_vertices, m_triangles);
    mesh.delegate(meshBuilder);
    
    CGAL::extract_mean_curvature_flow_skeleton(mesh, skeleton);
    
    // init the polyhedron simplex indices
    CGAL::set_halfedgeds_items_id(mesh);
    //for each input vertex compute its distance to the skeleton
    std::vector<double> distances(num_vertices(mesh));
    for (Skeleton_vertex v : CGAL::make_range(vertices(skeleton))) {
        const Point& skel_pt = skeleton[v].point;
        for(vertex_descriptor mesh_v : skeleton[v].vertices) {
            const Point& mesh_pt = mesh_v->point();
            distances[mesh_v->id()] = std::sqrt(CGAL::squared_distance(skel_pt, mesh_pt));
        }
    }
    // create a property-map for sdf values
    std::vector<double> sdf_values(num_faces(mesh));
    Facet_with_id_pmap<double> sdf_property_map(sdf_values);
    // compute sdf values with skeleton
    for (face_descriptor f : faces(mesh)) {
        double dist = 0;
        for(halfedge_descriptor hd : halfedges_around_face(halfedge(f, mesh), mesh))
            dist += distances[target(hd, mesh)->id()];
        sdf_property_map[f] = dist / 3.;
    }
    // post-process the sdf values
    CGAL::sdf_values_postprocessing(mesh, sdf_property_map);
    // create a property-map for segment-ids (it is an adaptor for this case)
    std::vector<std::size_t> segment_ids(num_faces(mesh));
    Facet_with_id_pmap<std::size_t> segment_property_map(segment_ids);
    
    auto segments = CGAL::segmentation_from_sdf_values(mesh, sdf_property_map, segment_property_map);

    m_triangleSegmentIds.reserve(m_triangles->size());
    for (face_descriptor f : faces(mesh)) {
        m_triangleSegmentIds.push_back(segment_property_map[f]);
    }
}

}
