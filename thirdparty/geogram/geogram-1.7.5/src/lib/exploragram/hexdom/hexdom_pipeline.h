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

#ifndef H_HEXDOM_ALGO_PIPELINE_H
#define H_HEXDOM_ALGO_PIPELINE_H

#include <exploragram/basic/common.h>

namespace GEO {

    class Mesh;
    
    namespace HexdomPipeline {
	
        bool EXPLORAGRAM_API SetConstraints(Mesh*m, std::string& msg, bool hilbert_sort = true,
                bool relaxed = false);
	
        void EXPLORAGRAM_API FrameField(Mesh*m, bool smooth);

        //{algo} = {0: CubeCover, 1 : PGP with correction, 2 PGP}
        void EXPLORAGRAM_API Parameterization(Mesh*m, int algo=0, double PGP_max_scale_corr =0.3);
	
        void EXPLORAGRAM_API HexCandidates(Mesh*m, Mesh* result);

        bool EXPLORAGRAM_API QuadDominant(Mesh*m, Mesh* chartmesh);

        void EXPLORAGRAM_API Hexahedrons(Mesh* quaddominant, Mesh* hexcandidates, Mesh* result);
	
        bool EXPLORAGRAM_API Cavity(Mesh* quaddominant, Mesh* hexahedrons, Mesh* result);

        void EXPLORAGRAM_API HexDominant(Mesh* cavity, Mesh* hexahedrons, Mesh* result, bool with_pyramid=false, bool baudoin_carrier=false,bool vertex_puncher =false);
    }
}


#endif
