// Copyright (c) 2004  INRIA Sophia-Antipolis (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Mesher_level/include/CGAL/Meshes/Simple_set_container.h $
// $Id: Simple_set_container.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: LGPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Laurent RINEAU

#ifndef CGAL_MESHES_SIMPLE_SET_CONTAINER_H
#define CGAL_MESHES_SIMPLE_SET_CONTAINER_H

#include <set>

namespace CGAL {

  namespace Meshes {

    template <typename Element>
    class Simple_set_container
    {
    public:
      typedef std::set<Element> Set;
      typedef typename Set::size_type size_type;

    protected:
      // --- protected datas ---
      Set s;

    public:
      bool no_longer_element_to_refine_impl() const
      {
        return s.empty();
      }

      Element get_next_element_impl()
      {
        CGAL_assertion(!s.empty());

        return *(s.begin());

      }

      void add_bad_element(const Element& e)
      {
        s.insert(e);
      }

      void pop_next_element_impl()
      {
        s.erase(s.begin());
      }

      void remove_element(const Element& e)
      {
        s.erase(e);
      }

      size_type size() const
      {
        return s.size();
      }
    }; // end Simple_set_container

  } // end namespace Meshes
} // end namespace CGAL

#endif // CGAL_MESHES_SIMPLE_SET_CONTAINER_H
