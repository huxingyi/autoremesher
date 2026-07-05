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

#ifndef H_HEXDOM_ALGO_INTERSECT_TOOLS_H
#define H_HEXDOM_ALGO_INTERSECT_TOOLS_H

#include <exploragram/basic/common.h>
#include <exploragram/hexdom/basic.h>
#include <geogram/mesh/mesh.h>
namespace GEO {

    extern const index_t EXPLORAGRAM_API quad_rand_split[2][3];
    extern const index_t EXPLORAGRAM_API quad_split[4][3];
    extern const index_t EXPLORAGRAM_API diamon_split[12][3];
    extern const index_t EXPLORAGRAM_API diamon_quad_split[16][3];


	
	struct EXPLORAGRAM_API BBox {
        BBox() {
            min = vec3(1e20, 1e20, 1e20);
            max = vec3(-1e20, -1e20, -1e20);
        }

        bool intersect(const BBox& b) const;
        bool contains(const vec3& v) const;
        bool is_null() const;
        void add(const BBox& b);
        void add(const vec3& P);
		void dilate(double eps) {
			min -= vec3(eps, eps, eps);
			max += vec3(eps, eps, eps);
		}
        vec3 bary() const;

        vec3 min;
        vec3 max;
    };

    struct EXPLORAGRAM_API HBoxes {
        HBoxes() {
	}

        HBoxes(vector<BBox>& inboxes) {
            init(inboxes);
        }

        void init(vector<BBox>& inboxes);

        ~HBoxes() {
        }

        void sort(vector<vec3> &G, index_t org, index_t dest);
        void intersect(BBox& b, vector<index_t>& primitives, index_t node = 0);
	
        int STAT_nb_visits;
        int STAT_nb_leafs;
        int STAT_nb_requests;

        index_t offset;
        vector<index_t> tree_pos_to_org;
        vector<BBox> tree;
    };


    struct EXPLORAGRAM_API DynamicHBoxes {
        void init(vector<BBox>& inboxes);
	
        void intersect(BBox& b, vector<index_t>& primitives);

        void update_bbox(index_t id, BBox b = BBox());

        HBoxes          hbox;
        vector<index_t> moved;
        vector<BBox>    movedbbox;
    };
	


	// double tetra_volume(vec3 A, vec3 B, vec3 C, vec3 D);
	 double tetra_volume_sign(vec3 A, vec3 B, vec3 C, vec3 D);
	 bool same_sign(double a, double b);

	//bool naive_tri_tri_intersect(vec3 v0, vec3 v1, vec3 v2, vec3 u0, vec3 u1, vec3 u2);

	 vector<BBox> facets_bbox(Mesh* m);
	struct FacetIntersect {
		FacetIntersect(Mesh* p_m);
		vector<index_t> get_intersections(vector<vec3>& P);
		vector<index_t> get_intersections(index_t& f);
		vector<BBox> inboxes;
		DynamicHBoxes hb;
		Mesh* m;
	};


	bool polyintersect(vector<vec3>& P, vector<vec3>& Q);
	vector<index_t> get_intersecting_faces(Mesh* m);
	void check_no_intersecting_faces(Mesh* m,bool allow_duplicated = false);
}
#endif
