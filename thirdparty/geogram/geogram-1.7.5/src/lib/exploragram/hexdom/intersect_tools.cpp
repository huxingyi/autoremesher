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

#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/mesh_utils.h>
#define FPG_UNCERTAIN_VALUE 0
#include <geogram/numerics/predicates/orient3d.h>
#include <geogram/mesh/triangle_intersection.h>
#include <geogram/mesh/mesh_io.h>
namespace {
    using namespace GEO;
    
    struct IndexPointCmp {
        IndexPointCmp(vector<vec3>& p_data, index_t p_dim)
   	    : dim(p_dim), data(p_data) {}
        bool operator()(const index_t A, const index_t B) {
            return data[A][dim] > data[B][dim];
        }
        index_t dim;
        vector<vec3>& data;
    };
}

namespace GEO {


    const index_t quad_rand_split[2][3] = { { 0, 1, 2 },{ 0, 2, 3 } };    // random triangulation
    const index_t quad_split[4][3] = { { 0, 1, 2 },{ 0, 2, 3 },{ 1, 2, 3 },{ 1, 3, 0 } }; // convex hull
    const index_t diamon_split[12][3] = {                                 // surface and inside
        { 4, 5, 0 },{ 4, 5, 1 },{ 4, 5, 2 },{ 4, 5, 3 },
        { 0, 1, 4 },{ 1, 2, 4 },{ 2, 3, 4 },{ 3, 0, 4 },
        { 0, 1, 5 },{ 1, 2, 5 },{ 2, 3, 5 },{ 3, 0, 5 } // orientation doesn't matter
    };

    const index_t diamon_quad_split[16][3] = {           // surface and inside
        { 4, 5, 0 },{ 4, 5, 1 },{ 4, 5, 2 },{ 4, 5, 3 },
        { 0, 1, 4 },{ 1, 2, 4 },{ 2, 3, 4 },{ 3, 0, 4 },
        { 0, 1, 5 },{ 1, 2, 5 },{ 2, 3, 5 },{ 3, 0, 5 }, // orientation doesn't matter
        { 0, 1, 2 },{ 0, 2, 3 },{ 1, 2, 3 },{ 1, 3, 0 }
    };
    



    bool BBox::intersect(const BBox& b) const {
	FOR(d, 3) {
	    if (min[d] > b.max[d] || max[d] < b.min[d]) {
		return false;
	    }
	}
	return true;
    }

    bool BBox::contains(const vec3& v) const {
	FOR(d, 3) {
	    if (min[d] > v[d] || max[d] < v[d]) {
		return false;
	    }
	}
	return true;
    }

    bool BBox::is_null() const {
	FOR(d, 3) {
	    if (max[d] - min[d] < 0) {
		return true;
	    }
	}
	return false;
    }

    void BBox::add(const BBox& b) {
	if (b.is_null()) return;
	add(b.min);
	add(b.max);
    }

    void BBox::add(const vec3& P) {
	FOR(d, 3) {
	    min[d] = std::min(min[d], P[d]);
	    max[d] = std::max(max[d], P[d]);
	}
    }

    vec3 BBox::bary() const {
	return 0.5*(min + max);
    }


    /**********************************************************/

    inline unsigned int mylog2( unsigned int x ) {
	unsigned int ans = 0 ;
	while( x>>=1 ) ans++;
	return ans ;
    }

    
    void HBoxes::init(vector<BBox>& inboxes) {
	vector<vec3> G(inboxes.size());
	tree_pos_to_org.resize(inboxes.size());
	FOR(p, G.size())  G[p] = inboxes[p].bary();
	FOR(p, G.size())  tree_pos_to_org[p] = p;
	sort(G, 0, tree_pos_to_org.size());

	offset = index_t(pow(2.0, 1.0 + mylog2(G.size()))) - 1;
	tree.resize(offset + G.size());
	FOR(i, G.size())  tree[offset + i] = inboxes[tree_pos_to_org[i]];
	for (int i = int(offset) - 1; i >= 0; i--) {
	    for (int son = 2 * i + 1; son < 2 * i + 3; son++)
		if (son < int(tree.size())) tree[i].add(tree[son]);
	}

	STAT_nb_visits = 0;
	STAT_nb_leafs = 0;
	STAT_nb_requests = 0;
    }

    void HBoxes::sort(vector<vec3> &G, index_t org, index_t dest) {

	// find the best dim to cut
	index_t dim = 2;
	BBox b;
	for (index_t i = org; i < dest; i++) b.add(G[tree_pos_to_org[i]]);
	FOR(d, 2) if (b.max[d] - b.min[d] > b.max[dim] - b.min[dim]) dim = d;
	// sort
	IndexPointCmp cmp(G, dim);
	std::sort(tree_pos_to_org.begin() + int(org), tree_pos_to_org.begin() + int(dest), cmp);
	if (dest - org <= 2) return;
	index_t m = org + index_t(pow(2.0, int(mylog2(dest - org - 1))));
	sort(G, org, m);
	sort(G, m, dest);
    }

    void HBoxes::intersect(BBox& b, vector<index_t>& primitives, index_t node) {
	if (node == 0) STAT_nb_requests++;
	geo_assert(node < tree.size());
	STAT_nb_visits++;
	if (!tree[node].intersect(b)) return;
	if (node >= offset) {
	    STAT_nb_leafs++;
	    primitives.push_back(tree_pos_to_org[node - offset]);
	} else {
	    for (index_t son = 2 * node + 1; son < 2 * node + 3; son++)
		if (son < tree.size())
		    intersect(b, primitives, son);
	}
    }
    
    /**********************************************************/

    void DynamicHBoxes::init(vector<BBox>& inboxes) {
	hbox.init(inboxes);
	moved.clear();
	movedbbox.clear();
    }

    void DynamicHBoxes::intersect(BBox& b, vector<index_t>& primitives) {
		hbox.intersect(b, primitives);
		FOR(i, moved.size()) {
	    if (movedbbox[i].intersect(b))
			primitives.push_back(moved[i]);
		}
    }

    void DynamicHBoxes::update_bbox(index_t id, BBox b) {
	geo_assert(id<hbox.tree_pos_to_org.size());
	FOR(i, moved.size()) {
	    if (moved[i] == id) {
		movedbbox[i] = b;
		return;
	    }
	}
	moved.push_back(id);
	movedbbox.push_back(b);
    }




	static double tetra_volume(vec3 A, vec3 B, vec3 C, vec3 D) {
		return dot(cross(B - A, C - A), D - A);
	}

	double tetra_volume_sign(vec3 A, vec3 B, vec3 C, vec3 D) {
		double res = tetra_volume(A, B, C, D);
		if (std::abs(res) > 1e-15) return res;
		return dot(normalize(cross(normalize(B - A), normalize(C - A))), normalize(D - A));
	}

	bool same_sign(double a, double b) { return (a > 0) == (b > 0); }



	 vector<BBox> facets_bbox(Mesh* m) {
		vector<BBox> inboxes(m->facets.nb());
		FOR(f, m->facets.nb()) {
			index_t nbv = m->facets.nb_vertices(f);
			FOR(fv, nbv)  inboxes[f].add(X(m)[m->facets.vertex(f, fv)]);
		}
		return inboxes;
	 }




		 FacetIntersect::FacetIntersect(Mesh* p_m) { m = p_m;
			inboxes = facets_bbox(m);
			hb.init(inboxes);
		}

		 static void save_conflict(std::string name, vec3 A0, vec3 B0, vec3 C0, vec3 A1, vec3 B1, vec3 C1) {
			 Mesh conflict;
			 conflict.vertices.create_vertices(6);
			 conflict.facets.create_triangles(2);
			 X(&conflict)[0] = A0; X(&conflict)[1] = B0; X(&conflict)[2] = C0;
			 X(&conflict)[3] = A1; X(&conflict)[4] = B1; X(&conflict)[5] = C1;
			 FOR(f, 2) FOR(lv, 3) conflict.facets.set_vertex(f, lv, 3 * f + lv);
			 mesh_save(conflict, "C:/DATA/debug/" + name + "conflict.geogram");
		 }
    
		 static bool polyintersect_both_triangulation(vector<vec3>& P, vector<vec3>& Q) {
			 bool conflict = false;
			 FOR(trP, 4) {
				 FOR(trQ, 4) {
					 if (trP > 0 && P.size() == 3) continue;
					 if (trQ > 0 && Q.size() == 3) continue;
					 vector<TriangleIsect> trash;
					 conflict = conflict || triangles_intersections(
						 P[quad_split[trP][0]], P[quad_split[trP][1]], P[quad_split[trP][2]],
						 Q[quad_split[trQ][0]], Q[quad_split[trQ][1]], Q[quad_split[trQ][2]],
						 trash
					 );
					 FOR(i, trash.size()) {
						 //if (trash[i].first > 2 || trash[i].second > 2) conflict = true;
					 }
					 static int nb_intersects = 0;
					 if (conflict) {
						 nb_intersects++;
						 //save_conflict("gna"+ String::to_string(nb_intersects), P[quad_split[trP][0]], P[quad_split[trP][1]], P[quad_split[trP][2]],
						// Q[quad_split[trQ][0]], Q[quad_split[trQ][1]], Q[quad_split[trQ][2]]);
						 return true;
					 }
				 }
			 }
			 return false;
		 }
		 bool polyintersect(vector<vec3>& P, vector<vec3>& Q) {
			 geo_assert(P.size() == 3 || P.size() == 4);
			 geo_assert(Q.size() == 3 || Q.size() == 4);
			 
			 // check for same facet			 
			 if (P.size() == Q.size()) FOR(off, P.size()) {
				 bool is_same = true;
				 FOR(v, P.size()) {
					 if ((P[v] - Q[(v + off) % P.size()]).length2() != 0) {
						 is_same = false;
						 break;
					 }
				 }
				 if (is_same) return false;
			 }
			 return polyintersect_both_triangulation(P, Q);
		 }

		vector<index_t> FacetIntersect::get_intersections(vector<vec3>& P) {
			vector<index_t> res; 
			vector<index_t> primitives;
			BBox request_bbox;
			FOR(v, P.size())  request_bbox.add(P[v]);
			request_bbox.dilate(1e-15);
			hb.intersect(request_bbox, primitives);
			FOR(i, primitives.size()) {
				index_t opp_f = primitives[i];
				vector<vec3> Q;
				FOR(fv, m->facets.nb_vertices(opp_f)) 
					Q.push_back(X(m)[m->facets.vertex(opp_f, fv)]);
				if (polyintersect(P, Q) || polyintersect(Q,P)) 
					res.push_back(opp_f);
			}
			return res;
		}
		vector<index_t> FacetIntersect::get_intersections(index_t& f) {
			vector<vec3> verts(m->facets.nb_vertices(f));
			FOR(v, m->facets.nb_vertices(f)) verts[v] = X(m)[m->facets.vertex(f,v)];
			return get_intersections(verts);
		}


	 vector<index_t> get_intersecting_faces(Mesh* m) {
		 vector<index_t> res;
		 FacetIntersect finter(m);
		 FOR(f, m->facets.nb()) {
			// if ((f%50) ==0)plop(double(f) / double(m->facets.nb()));
			 vector<index_t> opp = finter.get_intersections(f);
			 FOR(i, opp.size()) {
				 res.push_back(f);
				 res.push_back(opp[i]);
			 }
		 }
	 return res;
	}




	 void check_no_intersecting_faces(Mesh* m, bool allow_duplicated ) {
		 vector<index_t> intersect = get_intersecting_faces(m);
		 if (allow_duplicated) {
			 index_t offpair = 0;
			 while (offpair < intersect.size()) {
				 index_t f0 = intersect[offpair];
				 index_t f1 = intersect[offpair + 1];
				 if (f0 != f1 && (facet_bary(m, f0) - facet_bary(m, f1)).length2() < 1e-15) {
					 FOR(d, 2) std::swap(intersect[offpair + d], intersect[intersect.size() - 2 + d]);
					 FOR(d, 2) intersect.pop_back();
				 }
				 else  offpair += 2;
			 }
		 }
		if (intersect.empty()) return;
		 Attribute<int> intersection(m->facets.attributes(), "intersection");
		 FOR(f, m->facets.nb()) intersection[f] = 0;
		 FOR(i, intersect.size()) intersection[intersect[i]] = 1;
		 intersection[intersect[0]] = 2;
		 intersection[intersect[1]] = 3;
		 FOR(i, 2) plop(m->facets.nb_vertices(intersect[i]));
		 FOR(i, 2) FOR(lv, m->facets.nb_vertices(intersect[i])) plop(m->facets.vertex(intersect[i], lv));
		 FOR(i, 2) FOR(lv, m->facets.nb_vertices(intersect[i])) plop(X(m)[m->facets.vertex(intersect[i], lv)]);
		 mesh_save(*m, "C:/DATA/debug/intersectingsurface.geogram");
		 
		 save_conflict("first",
			 X(m)[m->facets.vertex(intersect[0], 0)],
			 X(m)[m->facets.vertex(intersect[0], 1)],
			 X(m)[m->facets.vertex(intersect[0], 2)],
			 X(m)[m->facets.vertex(intersect[1], 0)],
			 X(m)[m->facets.vertex(intersect[1], 1)],
			 X(m)[m->facets.vertex(intersect[1], 2)]
			 );
		 Mesh conflict;
		 conflict.vertices.create_vertices(6);
		 conflict.facets.create_triangles(2);
		 FOR(f, 2) FOR(lv, 3) X(&conflict)[3*f+lv] = X(m)[m->facets.vertex(intersect[f], lv)];
		 FOR(f, 2) FOR(lv, 3) conflict.facets.set_vertex(f, lv, 3 * f + lv);
		 mesh_save(conflict, "C:/DATA/debug/conflict.geogram");
		 geo_assert_not_reached;
	 }

    
}
