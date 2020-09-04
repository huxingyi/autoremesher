#ifndef H_MESHCOMESH_H
#define H_MESHCOMESH_H

#include <exploragram/basic/common.h>
#include <geogram/mesh/mesh.h>
#include <exploragram/hexdom/polygon.h>
#include <exploragram/hexdom/intersect_tools.h>

    /**
     * \file exploragram/hexdom/MESHCOMESH.h
     */

namespace GEO {
    struct TrFan {
        struct TrFanElt {
            TrFanElt(Mesh* m, index_t v, index_t p_f, index_t p_chart) : f(p_f), chart(p_chart) {
                geo_assert(m->facets.nb_vertices(f) == 3);
                FOR (c, 3) {
                    if (m->facets.vertex(f, c) != v) continue;
                    org  = m->facets.vertex(f, (c + 1) % 3);
                    dest = m->facets.vertex(f, (c + 2) % 3);
                    break;
                }
            }

            index_t f;
            index_t chart;
            index_t org;
            index_t dest;
        };

        TrFan(index_t v, Mesh *m, vector<vector<index_t> > &v2f, Attribute<index_t> &chart) : m_(m), v_(v), incomplete_(false) {
            if (!v2f[v].size()) return;

            FOR(i, v2f[v].size()) { // collect triangles around v
                index_t f = v2f[v][i];
                geo_assert(3 == m->facets.nb_vertices(f));
                fan.push_back(TrFanElt(m, v, f, chart[f]));
            }

            FOR(i, fan.size()) { // find boundary fan element and put it in the first position
                bool boundary = true;
                FOR(j, fan.size()) {
                    if (fan[i].org == fan[j].dest) {
                        geo_assert(i != j);
                        boundary = false;
                        break;
                    }
                }
                if (boundary) {
                    std::swap(fan[0], fan[i]);
                    incomplete_ = true;
                    break;
                }
            }

            FOR(f, fan.size()-1) { // sort triangles in circular order, first triangle is not moved
                for (index_t i = f+2; i<fan.size(); i++) {
                    if (fan[f].dest == fan[i].org) {
                        std::swap(fan[f + 1], fan[i]);
                        break;
                    }
                }
            }

            FOR(f, fan.size()) {
                if (fan[f].dest == fan[(f+1) % fan.size()].org) continue;
                if (f+1==fan.size() && incomplete_) continue;
                GEO::Logger::out("HexDom")  << "Fan around vertex " << v_ << " is not valid" <<  std::endl;
                FOR(ff, fan.size()) GEO::Logger::out("HexDom")  << "fan[ff].org = " << fan[ff].org << "\tfan[ff].dest = " << fan[ff].dest <<  std::endl;
                geo_assert_not_reached;
            }

            if (!incomplete_) {
                index_t rotate = 0; // find a boundary between charts (any boundary will do); if no boundary rotate will be zero
                FOR(f, fan.size()) {
                    if (fan[rotate].chart != fan[(rotate-1+fan.size())%fan.size()].chart) break;
                    rotate++;
                }
                geo_assert(rotate <= fan.size());
                std::rotate(fan.begin(), fan.begin() + int(rotate), fan.end());
            }

            chart_offset.push_back(0);
            FOR(f, fan.size()-1) {
                if (fan[f].chart == fan[f+1].chart) continue;
                chart_offset.push_back(int(f + 1));
            }
        }

        index_t ncharts() {
            return index_t(chart_offset.size());
        }

        bool triangulate() {
            // 2 charts max, and incomplete fan must not have more than 1 chart
            Attribute<bool> selection(m_->vertices.attributes(), "selection");
            if (2<ncharts() || (incomplete_ && 1!=ncharts())) { 
                selection[v_] = true;
                return false;
            }

            bool result = true;
            FOR(ichart, ncharts()) {
                vector<index_t> vidx;
                int off1 = chart_offset[ichart];
                int off2 = ichart+1 < ncharts() ? chart_offset[ichart+1] : int(fan.size());
                for (int ivert=off1; ivert<off2; ivert++) {
                    vidx.push_back(fan[ivert].org);
                }
                if (incomplete_ || 1<ncharts()) {
                    vidx.push_back(fan[off2-1].dest);
                }
                triangles.push_back(vector<index_t>());
                if (3>vidx.size()) continue;

                vector<vec3> pts3d;
                FOR(ivert, vidx.size()) {
                    pts3d.push_back(X(m_)[vidx[ivert]]);
                }

                pts3d.push_back(X(m_)[v_]);

                vec3 nrm = Poly3d(pts3d).normal();
                if (nrm.length()<1e-10) nrm = vec3(0,0,1); // ça va, si c'est un polygon degenere, prenons un truc au pif

                Basis3d b(nrm);
                pts3d.pop_back(); // v_ was useful for computing the normal, removing it for the triangulation

                vector<vec2> pts2d;
                FOR(ivert, pts3d.size()) { 
                    pts2d.push_back(b.project_xy(pts3d[ivert]));
                }

                vector<index_t> tri_local_indices;
                if (!Poly2d(pts2d).try_triangulate_minweight(tri_local_indices)) {
                    error("was not able to triangulate");
//                    tri_local_indices.clear();
                    result = false;
                }

                FOR(ivert, tri_local_indices.size()) {
                    triangles.back().push_back(vidx[tri_local_indices[ivert]]);
                }

                geo_assert(0 == triangles.back().size() % 3);
            }
            return result;
        }

        TrFanElt& operator[](int i) {
            geo_assert(i >= 0 && i < int(fan.size()));
            return fan[i];
        }

        TrFanElt& operator[](index_t i) {
            geo_assert(i < fan.size());
            return fan[i];
        }

        index_t nb_fan_triangles() {
            return index_t(fan.size());
        }

        vector<vector<index_t> > triangles;
        vector<TrFanElt> fan;
        vector<int> chart_offset;

        Mesh *m_;
        index_t v_;
        bool incomplete_;
    };

    vector<vector<index_t> > generate_v2f(Mesh *m);
    bool find_self_intersections(Mesh* facets_with_quads_and_tri_only, vector<index_t> &intersections);
    bool lock_self_intersecting_regions(Mesh* facets_with_quads_and_tri_only, vector<BBox>& regions_to_lock);
    bool lock_self_intersecting_regions(Mesh* facets_with_quads_and_tri_only, Attribute<bool> &verts_to_remove, Attribute<index_t> &undo);
    bool try_simplify(Mesh* m, Attribute<index_t> &chart, Attribute<bool> &verts_to_remove, Attribute<index_t> &undo);
}

#endif
