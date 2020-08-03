// Copyright (c) 2006 Foundation for Research and Technology-Hellas (Greece).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Voronoi_diagram_2/include/CGAL/Regular_triangulation_adaptation_policies_2.h $
// $Id: Regular_triangulation_adaptation_policies_2.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Menelaos Karavelas <mkaravel@iacm.forth.gr>

#ifndef CGAL_REGULAR_TRIANGULATION_ADAPTATION_POLICIES_2_H
#define CGAL_REGULAR_TRIANGULATION_ADAPTATION_POLICIES_2_H 1

#include <CGAL/license/Voronoi_diagram_2.h>


#include <CGAL/Voronoi_diagram_2/basic.h>
#include <CGAL/Voronoi_diagram_2/Regular_triangulation_degeneracy_testers.h>
#include <CGAL/Voronoi_diagram_2/Policy_base.h>
#include <CGAL/Voronoi_diagram_2/Default_site_inserters.h>
#include <CGAL/Voronoi_diagram_2/Identity_rejectors.h>

#include <CGAL/Identity_policy_2.h>

namespace CGAL {

//=========================================================================
//=========================================================================

template<class RT2>
struct Regular_triangulation_degeneracy_removal_policy_2
  : public CGAL_VORONOI_DIAGRAM_2_INS::Policy_base
  <RT2,
   CGAL_VORONOI_DIAGRAM_2_INS::Regular_triangulation_edge_tester_2<RT2>,
   CGAL_VORONOI_DIAGRAM_2_INS::Identity_face_rejector<RT2>,
   CGAL_VORONOI_DIAGRAM_2_INS::Default_site_inserter
   <typename RT2::Geom_traits::Weighted_point_2,RT2>,
   CGAL_VORONOI_DIAGRAM_2_INS::Default_site_remover<RT2> >
{
  typedef typename RT2::Geom_traits::Weighted_point_2  Site_2;
};


//=========================================================================
//=========================================================================


template<class RT2>
struct Regular_triangulation_caching_degeneracy_removal_policy_2
  : public CGAL_VORONOI_DIAGRAM_2_INS::Caching_policy_base
  <RT2,
   CGAL_VORONOI_DIAGRAM_2_INS::Regular_triangulation_edge_tester_2<RT2>,
   CGAL_VORONOI_DIAGRAM_2_INS::Identity_face_rejector<RT2>,
   CGAL_VORONOI_DIAGRAM_2_INS::Default_site_inserter
   <typename RT2::Geom_traits::Weighted_point_2,RT2>,
   CGAL_VORONOI_DIAGRAM_2_INS::Default_site_remover<RT2> >
{
  typedef typename RT2::Geom_traits::Weighted_point_2  Site_2;
};

//=========================================================================
//=========================================================================

} //namespace CGAL

#endif // CGAL_REGULAR_TRIANGULATION_ADAPTATION_POLICIES_2_H
