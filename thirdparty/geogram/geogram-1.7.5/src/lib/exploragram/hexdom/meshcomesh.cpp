#include <exploragram/hexdom/quad_dominant.h> 
#include <exploragram/hexdom/meshcomesh.h> 
#include <exploragram/hexdom/PGP.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/permutation.h>
#include <algorithm>
#include <geogram/mesh/triangle_intersection.h>

#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/delaunay/delaunay.h>

#include <geogram/points/nn_search.h>
#include <geogram/points/colocate.h>
#include <queue>

#include <exploragram/hexdom/mesh_inspector.h>
#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/polygon.h>

namespace GEO {

    bool find_self_intersections(Mesh* facets_with_quads_and_tri_only, vector<index_t> &intersections) {
        intersections.resize(0);
        Mesh* m = facets_with_quads_and_tri_only;
        vector<BBox> inboxes(m->facets.nb());
        FOR(f, m->facets.nb()) {
            index_t nbv = m->facets.nb_vertices(f);
            geo_assert(4 == nbv || 3 == nbv);
            FOR(fv, nbv) {
                inboxes[f].add(X(m)[m->facets.vertex(f, fv)]);
            }
        }
        HBoxes hb(inboxes);

        bool conflict_detected = false;
        FOR(f, m->facets.nb()) {
            vector<index_t> primitives;
            hb.intersect(inboxes[f], primitives);
            FOR(i, primitives.size()) {
                index_t opp_f = primitives[i];
                if (f==opp_f) continue;
                vector<vec3> P;
                FOR(fv, m->facets.nb_vertices(f)) {
                    P.push_back(X(m)[m->facets.vertex(f, fv)]);
                }
                vector<vec3> Q;
                FOR(fv, m->facets.nb_vertices(opp_f)) {
                    Q.push_back(X(m)[m->facets.vertex(opp_f, fv)]);
                }

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
                    }
                }

                if (conflict) {
                    intersections.push_back(f);
                    intersections.push_back(opp_f);
                }
                conflict_detected = conflict_detected || conflict;
            }
        }
        sort( intersections.begin(), intersections.end() );
        intersections.erase( unique( intersections.begin(), intersections.end() ), intersections.end() );
        geo_assert((conflict_detected && intersections.size()) || (!conflict_detected && !intersections.size()));
        return  conflict_detected;
    }


/*
    // test for self-intersection
    // input : a surface with only triangles and quads allowed
    // output : push_back into regions_to_lock BBoxes of self-intersections
    bool lock_self_intersecting_regions(Mesh* mesh, Attribute<bool> &verts_to_remove, Attribute<index_t> &undo) {
        vector<index_t> intersections;
        bool conflict = find_self_intersections(mesh, intersections);
        FOR(f, intersections.size()) {
            if (undo[intersections[f]]!=NOT_AN_ID) verts_to_remove[undo[intersections[f]]] = false;
        }
        geo_assert(!conflict || intersections.size()>0);
        return intersections.size()>0;
    }

    bool lock_self_intersecting_regions(Mesh* mesh, vector<BBox>& regions_to_lock) {
        Attribute<bool> conflict(mesh->facets.attributes(), "conflict");
        vector<index_t> intersections;
        find_self_intersections(mesh, intersections);
//      sort( intersections.begin(), intersections.end() );
//      intersections.erase( unique( intersections.begin(), intersections.end() ), intersections.end() );

        FOR(f, intersections.size()) {
            BBox inbox;
            index_t nbv = mesh->facets.nb_vertices(intersections[f]);
            geo_assert(4 == nbv || 3 == nbv);
            FOR(fv, nbv) {
                inbox.add(X(mesh)[mesh->facets.vertex(intersections[f], fv)]);
            }
            regions_to_lock.push_back(inbox);
            conflict[intersections[f]] = true;
        }
        std::cerr << "[CONFLICTING FACETS] " << intersections.size() << "\n";
        return intersections.size()>0;
    }
*/
    // compute light connectivity (vertex to facets)
    vector<vector<index_t> > generate_v2f(Mesh *m) {
        vector<vector<index_t> > v2f(m->vertices.nb());
        FOR(f, m->facets.nb()) {
	    geo_assert(m->facets.nb_vertices(f) == 3);
            FOR(lc, m->facets.nb_corners(f)) {
                v2f[m->facets.vertex(f, lc)].push_back(f);
            }
        }
        return v2f;
    }

    static void fan_asserts(Mesh *m, vector<vector<index_t> > &v2f, Attribute<index_t> &chart) {
        std::cerr << "sanity check: triangulated surface with 3 distinct vertid for each triangle...";
        FOR(f, m->facets.nb()) {
            geo_assert(3 == m->facets.nb_corners(f));
            FOR(c1, m->facets.nb_corners(f)) {
                FOR(c2, m->facets.nb_corners(f)) {
                    if (c1 == c2) continue;
                    geo_assert(m->facets.vertex(f, c1) != m->facets.vertex(f, c2));
                }
            }
        }
        std::cerr << "ok\n";

        std::cerr << "sanity check: all fans are correct and for complete fans valency >= 3...";
        geo_assert(v2f.size()==m->vertices.nb());
        FOR(v, m->vertices.nb()) {
            if (!v2f[v].size()) continue;
            TrFan fan = TrFan(v, m, v2f, chart);
            geo_assert(fan.incomplete_ || v2f[v].size() >= 3);

        }
        std::cerr << "ok\n";
    }

    bool try_simplify(Mesh* m, Attribute<index_t> &chart, Attribute<bool> &verts_to_remove, Attribute<index_t> &undo) {
//        std::cerr << "NVERTS= " << m->vertices.nb() << "\n";
        vector<index_t> to_kill(m->facets.nb(), false);
        vector<vector<index_t> > v2f = generate_v2f(m);
        bool mesh_is_modified = false;

        fan_asserts(m, v2f, chart);

        FOR(v, m->vertices.nb()) {
            if (!verts_to_remove[v] || !v2f[v].size()) continue;
            TrFan fan = TrFan(v, m, v2f, chart);

            fan.triangulate();
//            if (!fan.triangulate()) continue;

            { // block of geometrical and topological sanity checks, allows to reject the triangulation
                bool non_manifold_edge = false;
                FOR(ichart, fan.ncharts()) {
                    vector<index_t> &tri = fan.triangles[ichart];
                    FOR(t, tri.size()/3) {
                        FOR(iv, 3) {
                            index_t a = tri[3*t + iv];
                            index_t b = tri[3*t + (iv+1) % 3];

                            // (a,b) is an edge in new triangulation, let us check whether it is on the boundary of the chart
                            bool fan_boundary = false;
                            FOR(i, fan.nb_fan_triangles()) {
                                fan_boundary = fan_boundary || (a == fan[i].org && b == fan[i].dest);
                            }
                            if (fan_boundary) continue;

                            // here we know that (a,b) is an interior edge
                            FOR(f, v2f[a].size()) {
                                FOR(c, m->facets.nb_corners(v2f[a][f])) {
                                    non_manifold_edge = non_manifold_edge || (b == m->facets.vertex(v2f[a][f], c));
                                }
                            }
                        }
                    }
                }
                if (non_manifold_edge) {
                    error("this triangulation would introduce a non manifold edge, rejecting");
                    continue;
                }
            }

            mesh_is_modified = true;

            FOR(ichart, fan.ncharts()) {
                vector<index_t> &tri = fan.triangles[ichart];
                FOR(t, tri.size() / 3) {
                    index_t new_f = m->facets.create_triangle(tri[3*t + 0], tri[3*t + 1], tri[3*t + 2]);
                    m->facets.attributes().copy_item(new_f, fan[fan.chart_offset[ichart]].f);
                    undo[new_f] = v;
                    to_kill.push_back(false);

                    FOR(lc, 3) {
                        v2f[m->facets.vertex(new_f, lc)].push_back(new_f);
                    }
                }
            }

            // update light connection info
            FOR(i, v2f[fan.v_].size()) {
                to_kill[v2f[fan.v_][i]] = true;
            }
            v2f[fan.v_].clear();

            FOR(i, fan.nb_fan_triangles()) {
                index_t f = fan[i].f;
                index_t org = fan[i].org;
                index_t dest = fan[i].dest;

                index_t vopp[2] = { org, dest };

                FOR(j, 2) {
                    FOR(fi, v2f[vopp[j]].size()) {
                        if (v2f[vopp[j]][fi] != f) continue;
                        v2f[vopp[j]][fi] = v2f[vopp[j]].back();
                        v2f[vopp[j]].pop_back();
                        break;
                    }
                }
            }
        }

        fan_asserts(m, v2f, chart);

//        std::cerr << "NVERTS= " << m->vertices.nb() << "\n";
        m->facets.delete_elements(to_kill, false);
//        kill_isolated_vertices(m);
//      std::cerr << "NVERTS= " << m->vertices.nb() << "\n";
//      std::cerr << "modified " << mesh_is_modified << "\n";
        return mesh_is_modified;
    }

}
