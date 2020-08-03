// Copyright (c) 2020 GeometryFactory (France) and Telecom Paris (France).
// All rights reserved.
//
// This file is part of CGAL (www.cgal.org)
//
// $URL: https://github.com/CGAL/cgal/blob/releases/CGAL-5.1-beta1/Tetrahedral_remeshing/include/CGAL/Tetrahedral_remeshing/Remeshing_cell_base_3.h $
// $Id: Remeshing_cell_base_3.h 8b3c1ee 2020-05-27T16:24:25+02:00 Sébastien Loriot
// SPDX-License-Identifier: GPL-3.0-or-later OR LicenseRef-Commercial
//
//
// Author(s)     : Jane Tournois, Noura Faraj

#ifndef CGAL_TET_ADAPTIVE_REMESHING_CELL_BASE_3_H
#define CGAL_TET_ADAPTIVE_REMESHING_CELL_BASE_3_H

#include <CGAL/license/Tetrahedral_remeshing.h>

#include <CGAL/Mesh_cell_base_3.h>

namespace CGAL
{
namespace Tetrahedral_remeshing
{
namespace internal
{
struct Fake_MD_C
{
  typedef int Subdomain_index;
  typedef int Surface_patch_index;
  typedef int Index;
};
}

/*!
\ingroup PkgTetrahedralRemeshingClasses

The class `Remeshing_cell_base_3` is a model of the concept `MeshCellBase_3`.
It is designed to serve as cell base class for the 3D triangulation
used in the tetrahedral remeshing process.

\tparam Gt is the geometric traits class.
It has to be a model of the concept `RemeshingTriangulationTraits_3`.

\tparam Cb is a cell base class from which `Remeshing_cell_base_3` derives.
It must be a model of the `TriangulationCellBase_3` concept.
It has the default value `Triangulation_cell_base_3<Gt>`.

\cgalModels `MeshCellBase_3`

*/
#ifndef DOXYGEN_RUNNING
template<typename Gt,
         typename Cb = CGAL::Triangulation_cell_base_3<Gt> >
using Remeshing_cell_base_3
  = CGAL::Mesh_cell_base_3<Gt, internal::Fake_MD_C, Cb>;
#else
template<typename Gt,
         typename Cb = CGAL::Triangulation_cell_base_3<Gt> >
class Remeshing_cell_base_3;
#endif

}//end namespace Tetrahedral_remeshing
}//end namespace CGAL

#endif //CGAL_TET_ADAPTIVE_REMESHING_CELL_BASE_3_H
