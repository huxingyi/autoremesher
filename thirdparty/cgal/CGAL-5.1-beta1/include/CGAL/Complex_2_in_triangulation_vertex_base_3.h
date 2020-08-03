// Copyright (c) 2003-2005  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Surface_mesher/include/CGAL/Complex_2_in_triangulation_vertex_base_3.h $
// $Id: Complex_2_in_triangulation_vertex_base_3.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Steve Oudot, David Rey, Mariette Yvinec, Laurent Rineau, Andreas Fabri




#ifndef CGAL_COMPLEX_2_IN_TRIANGULATION_VERTEX_BASE_3_H
#define CGAL_COMPLEX_2_IN_TRIANGULATION_VERTEX_BASE_3_H

#include <CGAL/license/Surface_mesher.h>



#include <CGAL/Triangulation_vertex_base_3.h>

namespace CGAL {

  template < class GT, class Vb = Triangulation_vertex_base_3 <GT> >
  class Complex_2_in_triangulation_vertex_base_3 : public Vb {

  public:
    typedef Complex_2_in_triangulation_vertex_base_3 <GT, Vb> Self;

    template < class TDS3 >
    struct Rebind_TDS {
      typedef typename Vb::template Rebind_TDS<TDS3>::Other  Vb3;
      typedef Complex_2_in_triangulation_vertex_base_3 <GT, Vb3> Other;
    };

    typedef typename Vb::Triangulation_data_structure Tds;
    typedef typename Tds::Vertex_handle Vertex_handle;
    typedef typename Tds::Cell_handle Cell_handle;
    typedef typename Tds::Facet Facet;

  private:
    int number_of_incident_facets_;
    int number_of_components_; // number of components in the adjacency
                               // graph of incident facets (in complex)
    bool cache_validity;
  public:
    // Constructors

    Complex_2_in_triangulation_vertex_base_3()
      : Vb(),
        number_of_incident_facets_(0),
        number_of_components_(0),
        cache_validity(false)
    {}

    bool is_c2t3_cache_valid() const {
      return cache_validity;
    }

    void invalidate_c2t3_cache()
    {
      cache_validity = false;
    }

    void set_c2t3_cache(const int i, const int j)
    {
      number_of_incident_facets_ = i;
      number_of_components_ = j;
      cache_validity = true;
    }

    int cached_number_of_incident_facets() const
    {
      return number_of_incident_facets_;
    }

    int cached_number_of_components() const
    {
      return number_of_components_;
    }

#ifdef CGAL_MESH_3_IO_H
    static
    std::string io_signature()
    {
      return Get_io_signature<Vb>()();
    }
#endif

  };  // end Complex_2_in_triangulation_vertex_base_3


}  // namespace CGAL


#endif // CGAL_COMPLEX_2_IN_TRIANGULATION_VERTEX_BASE_3_H
