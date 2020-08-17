/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000-2015 INRIA - Project ALICE
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact for Graphite: Bruno Levy - Bruno.Levy@inria.fr
 *  Contact for this Plugin: Nicolas Ray - nicolas.ray@inria.fr
 *
 *     Project ALICE
 *     LORIA, INRIA Lorraine,
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs.
 *
 * As an exception to the GPL, Graphite can be linked with the following
 * (non-GPL) libraries:
 *     Qt, tetgen, SuperLU, WildMagic and CGAL
 */

#ifndef H_HEXDOM_ALGO_HEX_CRUNCHER_H
#define H_HEXDOM_ALGO_HEX_CRUNCHER_H

#include <exploragram/basic/common.h>
#include <geogram/mesh/mesh.h>

namespace GEO {
    void EXPLORAGRAM_API bourrin_subdivide_hexes(Mesh* m);
    void EXPLORAGRAM_API bourrin_quadrangulate_facets(Mesh* m);
    bool EXPLORAGRAM_API next_crunch(Mesh* m, Mesh* newhex, bool newmodel =false, int nb_max_punch = 1000000);
    void EXPLORAGRAM_API prepare_crunch(Mesh* m, bool subdivide, bool revert);
    void EXPLORAGRAM_API punch_and_cut(Mesh* m, Mesh* newhex, int nb_iter, int nb_punch_per_iter, bool check_validity);

	void EXPLORAGRAM_API hex_crunch(Mesh* m, Mesh* hex);


	void EXPLORAGRAM_API quadrangulate_easy_boundary(Mesh* m);

}
#endif
