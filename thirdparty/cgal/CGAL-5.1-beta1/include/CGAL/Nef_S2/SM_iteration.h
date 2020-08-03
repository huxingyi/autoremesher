// Copyright (c) 1997-2002  Max-Planck-Institute Saarbruecken (Germany).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org).
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Nef_S2/include/CGAL/Nef_S2/SM_iteration.h $
// $Id: SM_iteration.h 0779373 2020-03-26T13:31:46+01:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Michael Seel  <seel@mpi-sb.mpg.de>
//               : Peter Hachenberger <hachenberger@mpi-sb.mpg.de>

#ifndef CGAL_SM_ITERATION_H
#define CGAL_SM_ITERATION_H

#include <CGAL/license/Nef_S2.h>


#undef CGAL_forall_iterators
#define CGAL_forall_iterators(x,S)\
for(x = (S).begin(); x != (S).end(); ++x)

#undef CGAL_forall_svertices
#define CGAL_forall_svertices(x,SM)\
for(x = (SM).svertices_begin(); x != (SM).svertices_end(); ++x)

#undef CGAL_forall_shalfedges
#define CGAL_forall_shalfedges(x,SM)\
for(x = (SM).shalfedges_begin(); x != (SM).shalfedges_end(); ++x)

#undef CGAL_forall_sedges
#define CGAL_forall_sedges(x,SM)\
for(x = (SM).shalfedges_begin(); x != (SM).shalfedges_end(); ++(++x))

#undef CGAL_forall_shalfloops
#define CGAL_forall_shalfloops(x,SM)\
for(x = (SM).shalfloops_begin(); x != (SM).shalfloops_end(); ++x)

#undef CGAL_forall_sfaces
#define CGAL_forall_sfaces(x,SM)\
for(x = (SM).sfaces_begin(); x != (SM).sfaces_end(); ++x)

#undef CGAL_forall_sface_cycles_of
#define CGAL_forall_sface_cycles_of(x,F)\
for(x = (F)->sface_cycles_begin(); x != (F)->sface_cycles_end(); ++x)

#endif //CGAL_SM_ITERATION_H
