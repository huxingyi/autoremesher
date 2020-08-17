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
