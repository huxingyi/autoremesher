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

#ifndef H_HEXDOM_ALGO_ID_MAP_H
#define H_HEXDOM_ALGO_ID_MAP_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h>
#include <geogram/basic/memory.h>

namespace IdMap {

    using namespace GEO;
    
    inline void make_identity(vector<index_t>& ind, int size = -1){
	if (size >= 0) ind.resize(index_t(size));
	for (index_t i = 0; i < ind.size(); i++)
	    ind[i] = i;
    }

    inline void connect_to_root(vector<index_t>& ind, index_t i){
	while (ind[i] != ind[ind[i]]) ind[i] = ind[ind[i]];
    }

    inline void merge(vector<index_t>& ind, index_t a, index_t b){
	connect_to_root(ind, a);
	connect_to_root(ind, b);
	ind[ind[a]] = ind[b];
    }
    
    inline void tree_to_map(vector<index_t>& ind){
	for (index_t i = 0; i < ind.size(); i++)
	    connect_to_root(ind, i);
    }
    
    inline void compress_id2grp(vector<index_t>& ind, index_t & nb_groups){
	vector<index_t> old_grp_2_new_grp(ind.size(), index_t(-1));
	nb_groups = 0;
	for (index_t i = 0; i < ind.size(); i++)
	    geo_assert(ind[i] == ind[ind[i]]);

	for (index_t i = 0; i < ind.size(); i++)if (i == ind[i]) {
		old_grp_2_new_grp[i] = nb_groups;
		nb_groups++;
	    }
	for (index_t i = 0; i < ind.size(); i++)
	    ind[i] = old_grp_2_new_grp[ind[i]];
    }
}

namespace IdMapSigned {

    using namespace GEO;
    
    inline void make_identity(vector<index_t>& ind, vector<bool>& sign, int size = -1){
	if (size >= 0) {
	    ind.resize(index_t(size));
	    sign.resize(index_t(size));
	}
	geo_assert(sign.size() == ind.size());
	for (index_t i = 0; i < ind.size(); i++){
	    ind[i] = i;
	    sign[i] = true;
	}
    }

    inline void connect_to_root(vector<index_t>& ind, vector<bool>& sign, index_t i){
	while (ind[i] != ind[ind[i]]) {
	    sign[i] = (sign[i] == sign[ind[i]]);
	    ind[i] = ind[ind[i]];
	}
    }

    inline void merge(vector<index_t>& ind, vector<bool>& sign, index_t a, index_t b, bool ab_link_sign){
	connect_to_root(ind, sign, a);
	connect_to_root(ind, sign, b);
	sign[ind[a]] = ((sign[a] == sign[b]) == ab_link_sign);
	ind[ind[a]] = ind[b];
    }
    
    inline void tree_to_map(vector<index_t>& ind, vector<bool>& sign){
	for (index_t i = 0; i < ind.size(); i++)
	    connect_to_root(ind, sign, i);
    }
    
    inline void compress_id2grp(vector<index_t>& ind, index_t & nb_groups){
	vector<index_t> old_grp_2_new_grp(ind.size(), index_t(-1));
	nb_groups = 0;
	FOR(i,ind.size()) geo_assert(ind[i] == ind[ind[i]]);

	FOR(i,ind.size())if (i == ind[i]) {
	    old_grp_2_new_grp[i] = nb_groups;
	    nb_groups++;
	}
	FOR(i, ind.size()) ind[i] = old_grp_2_new_grp[ind[i]];
    }
}

#endif
