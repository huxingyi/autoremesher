// Copyright (c) 1997-2002  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Nef_3/include/CGAL/Nef_3/SNC_iteration.h $
// $Id: SNC_iteration.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Seel    <seel@mpi-sb.mpg.de>
//                 Miguel Granados <granados@mpi-sb.mpg.de>
//                 Susan Hert      <hert@mpi-sb.mpg.de>
//                 Lutz Kettner    <kettner@mpi-sb.mpg.de>
#ifndef CGAL_SNC_ITERATION_H
#define CGAL_SNC_ITERATION_H

#include <CGAL/license/Nef_3.h>


#include <CGAL/Nef_S2/SM_iteration.h>

#define CGAL_forall_vertices(x,SNC)\
for(x = (SNC).vertices_begin(); x != (SNC).vertices_end(); ++x)

#define CGAL_forall_halfedges(x,SNC)\
for(x = (SNC).halfedges_begin(); x != (SNC).halfedges_end(); ++x)

#define CGAL_forall_edges(x,SNC)\
for(x = (SNC).halfedges_begin(); x != (SNC).halfedges_end(); ++x) \
if ( x->is_twin() ) continue; else

#define CGAL_forall_halffacets(x,SNC)\
for(x = (SNC).halffacets_begin(); x != (SNC).halffacets_end(); ++x)

#define CGAL_forall_facets(x,SNC)\
for(x = (SNC).halffacets_begin(); x != (SNC).halffacets_end(); ++x) \
if ( x->is_twin() ) continue; else

#define CGAL_forall_volumes(x,SNC)\
for(x = (SNC).volumes_begin(); x != (SNC).volumes_end(); ++x)

#define CGAL_forall_facet_cycles_of(x,F)\
for(x = (F)->facet_cycles_begin(); x != (F)->facet_cycles_end(); ++x)

#define CGAL_forall_shells_of(x,C)\
for(x = (C)->shells_begin(); x != (C)->shells_end(); ++x)

#define CGAL_forall_svertices_of(x,V)\
for(x = (V)->svertices_begin(); x != (V)->svertices_end(); ++x)

#define CGAL_forall_sedges_of(x,V)\
for(x = (V)->shalfedges_begin(); x != (V)->shalfedges_end(); ++(++x))

#define CGAL_forall_shalfedges_of(x,V)\
for(x = (V)->shalfedges_begin(); x != (V)->shalfedges_end(); ++x)

#define CGAL_forall_sfaces_of(x,V)\
for(x = (V)->sfaces_begin(); x != (V)->sfaces_end(); ++x)

#endif //CGAL_SNC_ITERATION_H
