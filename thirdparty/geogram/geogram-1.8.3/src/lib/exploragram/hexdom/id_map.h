/*
 *  Copyright (c) 2000-2022 Inria
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
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
