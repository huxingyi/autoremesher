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


#include <exploragram/hexdom/quads_from_boundary.h>
#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/polygon.h>

namespace GEO {


    struct QuadsFromBoundry {
        index_t n;
        vector<vec2>& pts;
        double ave;
        vector<index_t> global_vid[2]; // vertex indices of the sub problem
        vector<index_t>& quads;

        QuadsFromBoundry(vector<vec2>& p_pts, vector<index_t>& p_quads) : pts(p_pts), quads(p_quads){}

        double angle(int i) {
            vec2 P[3];
            FOR(p, 3) P[p] = aupp(i + int(p) - 1, pts);
            return (180. / M_PI)*atan2(det(P[2] - P[1], P[0] - P[1]), dot(P[2] - P[1], P[0] - P[1]));
        }

	double angle(index_t i) {
	    return angle(int(i));
	}

        vec2 point(int i) {
            return aupp(i , pts);
        }
	
        vec2 point(index_t i) {
            return aupp(i , pts);
        }

            void init() {
                n = pts.size();
                ave = 0;
                FOR(p, pts.size()) ave += (pts[p] - aupp(p + 1, pts)).length();
                ave  /= double(pts.size());
            }


            bool solve_and_merge_subproblems() {
                plop("recurs");
                // solve on two halves
                vector<vec2> poly[2];
                FOR(i, 2) FOR(fv, global_vid[i].size()) poly[i].push_back(pts[global_vid[i][fv]]);

                vector<index_t> poly_quad[2];

                FOR(i, 2) {
                    QuadsFromBoundry sub(poly[i], poly_quad[i]);
                    if (!sub.apply()) return false;
                    plop(pts.size());
                    plop(global_vid[i].size());
                }
                // add new pts to global
                FOR(i, 2) for (index_t d = global_vid[i].size(); d < poly[i].size(); d++) {
                    global_vid[i].push_back(pts.size());
                    pts.push_back(poly[i][d]);
                }
                FOR(i, 2) FOR(qu, poly_quad[i].size()) quads.push_back(global_vid[i][poly_quad[i][qu]]);

                return true;

            }


        bool apply() {
            init();
            if (n == 4) { FOR(v, 4) quads.push_back(v); return true; }




            // try to find a quad to close
            FOR(v, n) {
                double alpha[2] = { angle(v),angle((v + 1) % n) };
                double tolerance = 45;
                if (alpha[0] > 90 - tolerance && alpha[0] < 90 + tolerance
                    &&  alpha[1] > 90 - tolerance && alpha[1] < 90 + tolerance) {
                    FOR(i, n) if (i != v && (i != ((v + 1) % n))) global_vid[0].push_back(i);
                    FOR(i, 4) global_vid[1].push_back((v + i + n - 1) % n);
                    return solve_and_merge_subproblems();
                    //goto split_done;
                }
            }


            // try to find a new vertex to punch
            FOR(it, 2)
                FOR(v, n) {
                double alpha[3] = { angle(v),angle((v + 1) % n),angle((v + 2) % n) };
                double tolerance = 45;
                bool can_punch;
                if (it == 0)
                    can_punch = alpha[0] > 90 - tolerance  && alpha[0] < 90 + tolerance
                    && alpha[1] > 0 - tolerance  && alpha[1] < 0 + tolerance
                    && alpha[2] > 90 - tolerance  && alpha[2] < 90 + tolerance
                    ;
                else //(it == 1)
                    can_punch = alpha[0] > 90 - tolerance  && alpha[0] < 90 + tolerance;
                if (can_punch) {// more or less 90 degree ;)
                    vec2 nvP;
                    vec2 vec[2] = { point(v + 1) - point(v),point(v - 1) - point(v)};
                    mat2 mat;
                    FOR(i, 2)FOR(j, 2) mat(i, j) = vec[i][j];
                    mat2 inv = mat.inverse();
                    vec2 b(vec[0].length2(), vec[1].length2());
                    mult(inv, b.data(), nvP.data());
                    nvP = nvP + point(v);
                    nvP = point(v) + vec[0] + vec[1];

                    // check that there is no existing vertex 
                    bool geometric_issue = false;
                    {
                        vec2 new_quad[4] = { point(v - 1),point(v),point(v + 1),nvP };
                        for (double du = 0; du < .3; du += .2)
                            for (double dv = .8; dv < 1; dv += .2) {
                                vec2 pixel =
                                    du*dv*new_quad[0]
                                    + du*(1. - dv)*new_quad[1]
                                    + (1. - du)*dv*new_quad[3]
                                    + (1. - du)*(1. - dv)*new_quad[2];
                                for (double c = 0; c < 1.; c += .2) FOR(vv, n) 
                                    if (((c*aupp(vv + 1, pts) + (1. - c)*point(vv)) - pixel).length() < .5*ave) geometric_issue = true;
                            }
                    }
                    if (geometric_issue) continue;


                    FOR(i, n) global_vid[0].push_back((i != v) ? ((i) % n) : pts.size());
                    FOR(i, 3) global_vid[1].push_back((v + i + n - 1) % n);
                    global_vid[1].push_back(pts.size());
                    pts.push_back(nvP);
                    return solve_and_merge_subproblems();
                    //goto split_done;
                }
            }

            return true;
      //  split_done:
        }
    };


    //double angle(vector<vec2>& pts, int i) {
    //    vec2 P[3];
    //    FOR(p, 3) P[p] = aupp(i + p - 1, pts);
    //    return (180. / M_PI)*atan2(det(P[2] - P[1], P[0] - P[1]), dot(P[2] - P[1], P[0] - P[1]));
    //}
    //double ave_length(vector<vec2>& pts) {
    //    double res = 0;
    //    FOR(p, pts.size()) res += (pts[p] - aupp(p + 1, pts)).length();
    //    return res/double(pts.size());
    //}



    //bool try_quadrangulate_with_punch_vertex(vector<vec2>& P, vector<index_t>& quads) {
    //    int n = P.size();
    //    if (n == 4) { FOR(v, 4) quads.push_back(v); return true; }
    //    vector<index_t> global_vid[2];


    //    // try to find a quad to close
    //    FOR(v, n) {
    //        double alpha[2] = { angle(P, v),angle(P, (v + 1) % n) };
    //        double tolerance = 45;
    //        if (alpha[0] > 90-tolerance && alpha[0] < 90 + tolerance
    //            &&  alpha[1] > 90 - tolerance && alpha[1] < 90 + tolerance) {
    //            // check that there is no existing vertex 
    //            //bool geometric_issue = false;
    //            //for (double c = 0; c < 1.; c += .2) FOR(vv, n) if (((c*aupp(vv + 1, P) + (1. - c)*P[vv]) - nvP).length() < .5*ave_length(P)) geometric_issue = true;
    //            //if (geometric_issue) continue;
    //            plop("go");
    //            FOR(i, n) if (i != v && (i != ((v + 1) % n))) global_vid[0].push_back(i);
    //            FOR(i, 4) global_vid[1].push_back((v + i + n - 1) % n);
    //            goto split_done;
    //        }
    //    }


    //    // try to find a new vertex to punch
    //    FOR(it,2)
    //    FOR(v, n) {
    //        double alpha[3] = { angle(P, v),angle(P, (v + 1) % n),angle(P, (v + 2) % n) };
    //        double tolerance = 45;
    //        bool can_punch ;
    //        if (it == 0)
    //            can_punch = alpha[0] > 90 - tolerance  && alpha[0] < 90 + tolerance
    //            && alpha[1] > 0 - tolerance  && alpha[1] < 0 + tolerance
    //            && alpha[2] > 90 - tolerance  && alpha[2] < 90 + tolerance
    //            ;
    //        if (it == 1)
    //            can_punch = alpha[0] > 90 - tolerance  && alpha[0] < 90 + tolerance;
    //        if (can_punch) {// more or less 90 degree ;)
    //            vec2 nvP;
    //            vec2 vec[2] = { aupp(v + 1, P) - P[v],aupp(v - 1, P) - P[v] };
    //            mat2 mat;
    //            FOR(i,2)FOR(j,2) mat(i, j) = vec[i][j];
    //            mat2 inv = mat.inverse();
    //            vec2 b (vec[0].length2(), vec[1].length2());
    //            mult(inv, b.data(), nvP.data());
    //            nvP = nvP + P[v];
    //            nvP = P[v] + vec[0] + vec[1];

    //            // check that there is no existing vertex 
    //            bool geometric_issue = false;
    //            {
    //                double ave = ave_length(P);
    //                vec2 new_quad[4] = { aupp(v - 1, P),aupp(v , P),aupp(v + 1, P),nvP };
    //                for (double du = 0; du < .3; du += .2)
    //                    for (double dv = .8; dv < 1; dv += .2) {
    //                        vec2 pixel = 
    //                            du*dv*new_quad[0] 
    //                            + du*(1. - dv)*new_quad[1]
    //                            + (1. - du)*dv*new_quad[3] 
    //                            + (1. - du)*(1. - dv)*new_quad[2];
    //                        for (double c = 0; c < 1.; c += .2) FOR(vv, n) if (((c*aupp(vv + 1, P) + (1. - c)*P[vv]) - pixel).length() < .5*ave) geometric_issue = true;
    //                    }
    //            }
    //            if (geometric_issue) continue;

    //        
    //            FOR(i, n) global_vid[0].push_back((i != v) ? ((i)%n) : P.size());
    //            FOR(i, 3) global_vid[1].push_back((v+i+n-1)%n);
    //            global_vid[1].push_back(P.size());
    //            P.push_back(nvP);
    //            goto split_done;
    //        }
    //    }

    //    return true;
    //    split_done:
    //    // solve on two halves
    //    vector<vec2> poly[2];
    //    FOR(i, 2) FOR(fv, global_vid[i].size()) poly[i].push_back(P[global_vid[i][fv]]);

    //    vector<index_t> poly_quad[2];
    //    FOR(i, 2) if (!try_quadrangulate_with_punch_vertex(poly[i],poly_quad[i])) return false;
    //        
    //    // add new pts to global
    //    FOR(i, 2) for (int d = global_vid[i].size(); d < poly[i].size(); d++) {
    //        global_vid[i].push_back(P.size());
    //        P.push_back(poly[i][d]);
    //    }
    //    FOR(i, 2) FOR(qu, poly_quad[i].size()) quads.push_back(global_vid[i][poly_quad[i][qu]]);

    //    return true;
    //}






    bool try_quadrangulate(vector<vec2>& pts, vector<index_t>& quads) {

        return Poly2d(pts).try_quadrangulate(quads);

        //QuadsFromBoundry sub(pts, quads);
        //return sub.apply();

    }
}
