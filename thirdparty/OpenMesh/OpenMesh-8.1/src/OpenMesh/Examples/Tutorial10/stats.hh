#ifndef STATS_HH
#define STATS_HH

template <typename Mesh>
void mesh_stats( Mesh& _m, const std::string& prefix = "" )
{
  std::cout << prefix
            << _m.n_vertices() << " vertices, "
            << _m.n_edges() << " edges, "
            << _m.n_faces() << " faces\n";
}

template <typename Mesh>
void mesh_property_stats(Mesh& _m)
{
  std::cout << "Current set of properties:\n";
  _m.property_stats(std::cout);
}

#endif
