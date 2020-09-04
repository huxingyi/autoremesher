
#include <exploragram/hexdom/quad_dominant.h> 
#include <exploragram/hexdom/meshcomesh.h> 
#include <exploragram/hexdom/PGP.h>
#include <exploragram/hexdom/basic.h>
#include <exploragram/hexdom/extra_connectivity.h>
#include <exploragram/hexdom/mesh_inspector.h>
#include <exploragram/hexdom/intersect_tools.h>
#include <exploragram/hexdom/polygon.h>

#include <geogram/numerics/matrix_util.h>
#include <geogram/basic/permutation.h>
#include <geogram/mesh/triangle_intersection.h>
#include <geogram/mesh/mesh_tetrahedralize.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/points/nn_search.h>
#include <geogram/points/colocate.h>
#include <geogram/mesh/mesh_repair.h>
#include <geogram/mesh/mesh_fill_holes.h>
#include <geogram/mesh/mesh_geometry.h>


#include <algorithm>
#include <queue>
#include <stack>
#include <map>

namespace GEO {

    void export_boundary_with_uv(Mesh* m, Mesh* hex, const char* uv_name, const char* singular_name) {
        Attribute<GEO::vec2> uv(hex->facet_corners.attributes(), uv_name);
        Attribute<index_t> singtri(hex->facets.attributes(), singular_name);

        Attribute<bool> has_param(m->cell_facets.attributes(), "has_param");
        Attribute<vec3> UC(m->cell_corners.attributes(), "U");

        // STEP 1: compute number of boundary vertices AND create a lookup table tet mesh vertex -> boundary surface vertex
        vector<index_t> tetV_to_facetV(m->vertices.nb(), NOT_AN_ID);
        index_t nb_boundary_V = 0;
        FOR(c, m->cells.nb()) FOR(cf, m->cells.nb_facets(c)) {
            if (NO_CELL != m->cells.adjacent(c, cf)) continue;
            FOR(cfv, m->cells.facet_nb_vertices(c, cf)) {
                index_t v = m->cells.facet_vertex(c, cf, cfv);
                if (NOT_AN_ID == tetV_to_facetV[v]) {
                    tetV_to_facetV[v] = nb_boundary_V++;
                }
            }
        }

        // STEP 2: copy boundary vertices to the new mesh
        hex->vertices.create_vertices(nb_boundary_V);
        FOR(v, m->vertices.nb())
            if (NOT_AN_ID != tetV_to_facetV[v])
                hex->vertices.point(tetV_to_facetV[v]) = m->vertices.point(v);

        // STEP 3: extract triangles and their parameterization (when possible)
        FOR(c, m->cells.nb()) FOR(cf, m->cells.nb_facets(c)) {
            if (m->cells.adjacent(c, cf) != NO_CELL) continue;
            index_t tet_verts[3];
            FOR(cfv, 3) {
                tet_verts[cfv] = m->cells.facet_vertex(c, cf, cfv);
            }

            index_t tet_corners[3];
            FOR(cfv, 3) FOR(test, 4) if (m->cell_corners.vertex(m->cells.corner(c, test)) == tet_verts[cfv])
                tet_corners[cfv] = m->cells.corner(c, test);


            vec3 lX[3], lU[3];
            FOR(cfv, 3) {
                lX[cfv] = m->vertices.point(tet_verts[cfv]);
                lU[cfv] = UC[tet_corners[cfv]];
            }

            // STEP 2.2: non singular boundary triangles are easy to extract
            index_t fid = hex->facets.create_triangle(
                    tetV_to_facetV[tet_verts[0]],
                    tetV_to_facetV[tet_verts[1]],
                    tetV_to_facetV[tet_verts[2]]
                    );

            bool has_valid_2d_param = false;
            if (has_param[m->cells.facet(c, cf)]) {
                FOR(dim, 3) { // we are looking for the param dimension that goes inside the volume
                    if (lU[0][dim] != lU[1][dim] || lU[0][dim] != lU[2][dim]) continue;
                    has_valid_2d_param = true;
                    FOR(lv, 3) {
                        uv[hex->facets.corner(fid, lv)] = vec2(lU[lv][(dim + 1) % 3], lU[lv][(dim + 2) % 3]);
                    }
                }
#if 0                
                // mirror the copy when necessary
                if (det(uv[hex->facets.corner(fid, 1)] - uv[hex->facets.corner(fid, 0)],
                            uv[hex->facets.corner(fid, 2)] - uv[hex->facets.corner(fid, 0)]) < 0)
                    FOR(lv, 3) uv[hex->facets.corner(fid, lv)][0] *= -1.;
#endif
            }

            if (has_valid_2d_param) { // check param quality and invalidate it, if necessary
                TrglGradient grd(lX[0], lX[1], lX[2]);
                vec3 grduv[2];
                FOR(d, 2) grduv[d] = grd.gradient_3d(uv[hex->facets.corner(fid, 0)][d], uv[hex->facets.corner(fid, 1)][d], uv[hex->facets.corner(fid, 2)][d]);
	        FOR(d, 2) FOR(dd, 3) if (Numeric::is_nan(grduv[d][dd])) has_valid_2d_param = false;
#if 0                
              if (grduv[0].length() > 10. * grduv[1].length()) has_valid_2d_param = false;
              if (grduv[1].length() > 10. * grduv[0].length()) has_valid_2d_param = false;
              if (std::abs(dot(normalize(grduv[0]), normalize(grduv[1]))) > cos(M_PI / 4.)) has_valid_2d_param = false;
#endif
            }

            singtri[fid] = !has_valid_2d_param;
        }
    }

    static bool triangulate_surface_preserve_attributes(Mesh* m) {
        bool result = true;
        plop("triangulating surface after embedding isoUV");
        vector<index_t> to_kill(m->facets.nb(), false);
        FOR(f, m->facets.nb()) {
            geo_assert(m->facets.nb_corners(f) >= 3);
            if (m->facets.nb_corners(f) == 3) continue;

            vector<vec3> pts;
            FOR(lc, m->facets.nb_corners(f)) {
                pts.push_back(X(m)[m->facets.vertex(f, lc)]);
            }

            vector<index_t> triangles;
            bool success = Poly3d(pts).try_triangulate_minweight(triangles);
            result = result && success;

            geo_assert(0 == triangles.size() % 3);

            FOR(new_face, triangles.size() / 3) {
                index_t new_f = m->facets.create_triangle(
                        m->facets.vertex(f, triangles[3 * new_face + 0]),
                        m->facets.vertex(f, triangles[3 * new_face + 1]),
                        m->facets.vertex(f, triangles[3 * new_face + 2]));

                to_kill.push_back(false);
                m->facets.attributes().copy_item(new_f, f);
                FOR(nfc,3) {
                    m->facet_corners.attributes().copy_item(m->facets.corner(new_f, nfc), m->facets.corner(f, triangles[3 * new_face + nfc]));
                }
            }
            to_kill[f] = true;
        }
        m->facets.delete_elements(to_kill);
        return result;
    }


    /**
     * INPUT: facets with xyz geometry and uv coordinates s.t. no edge is of size 0
     singular bool per triangle
     *         integer values of uv --- interpolated along edge 'e' ---  matches on both side of 'e'
     on presuppose que les U ont été pre-snappés sur la grille entiere
     * OUTPUT: facets with uv coordinates s.t.
     *               a new vertex is inserted (with interpolated xyz and uv) at every integer values of uv along edges
     *               nothing prevents some vertices around a facet to share the same uv's
     ATTENTION: it modifies the parameterization when snaps grid corners on edges!
    */

    void split_edges_by_iso_uvs(Mesh* m, const char *uv_name, const char *singular_name) {
        Attribute<GEO::vec2> uv(m->facet_corners.attributes(), uv_name);
        Attribute<index_t> singular(m->facets.attributes(), singular_name);

        index_t nb_init_facets = m->facets.nb();
        Attribute<bool> added_vertices(m->vertices.attributes(), "added_vertices");
        Attribute<index_t> orig_tri_fid(m->facets.attributes(), "orig_tri_fid");
        Attribute<index_t> resp_facet(m->vertices.attributes(), "resp_facet");

        typedef std::pair<index_t /*v_id*/, vec2 /*uv*/> NewCorner;
        vector<vector<NewCorner> > new_corners(m->facet_corners.nb());

        reach("create vertices and init arrays of NewCorner to insert on edges");
        {
            FacetsExtraConnectivity fec(m);
            FOR(h, m->facet_corners.nb()) {
                index_t opp = fec.opposite(h);

                if (singular[fec.facet(h)]) continue; // the face is responsible for the edge if opp<h or if its opposite is singular
                if (NOT_AN_ID != opp && !singular[fec.facet(opp)] && opp > h) continue;

                vec2 lU[2] = { uv[h], uv[fec.next(h)] };

#if 0
                bool trans_func_found = false;
                index_t R;
                vec2 T;
                if (NOT_AN_ID != opp && !singular[fec.facet(opp)]) {
                    vec2 A = lU[1]-lU[0], B = uv[opp]-uv[fec.next(opp)], C = lU[0];
                    if (std::abs(A.length()-B.length())<1e-3) {
                        double mindist = std::numeric_limits<double>::max();
                        FOR(r,4) {
                            if ((A-B).length()<mindist) {
                                R = r;
                                mindist = (A-B).length();
                            }
                            A = vec2(-A[1], A[0]);
                        }
                        FOR(r, R) {
                            A = vec2(-A[1], A[0]);
                            C = vec2(-C[1], C[0]);
                        }
                        plop(mindist);
                        vec2 Ttmp = uv[fec.next(opp)] - C;
                        T = vec2(round(Ttmp[0]), round(Ttmp[1]));
                        trans_func_found = (mindist<1e-3 && (T-Ttmp).length()<1e-3);
                    } else {
                        plop("GNA?!");
                    }
                    if (!trans_func_found && NOT_AN_ID!=opp && !singular[fec.facet(opp)]) {
                        plop("ATTENTION, grids do not match");
                        singular[fec.facet(h)] = true;
                        singular[fec.facet(opp)] = true;
                        continue;
                    }
                }
#endif
                vector<double> coeff;
                FOR(coord, 2) { // find barycentric coordinates of both u and v integer isos
                    double v[2] = { lU[0][coord], lU[1][coord] };
                    double from = floor(std::min(v[0], v[1])) + 1;
                    double to   = std::max(v[0], v[1]);
                    if (to-from > 1000) continue;
                    for (double iso = from; iso < to; iso += 1.) {
                        double c = (iso - v[0]) / (v[1] - v[0]); // v[0] is far from v[1] (U was pre-snapped to integers with .05 tolerance)
                        if (!Numeric::is_nan(c) && c > 0 && c < 1) {
                            // vec2 u = lU[0] + c*(lU[1] - lU[0]);
                            // u[coord] = iso;
                            coeff.push_back(c);
                        }
                    }
                }
                std::sort(coeff.begin(), coeff.end(), std::less<double>()); // we need to sort in order to merge close values

                vector<vec3> pts;
                vector<vec2> lu;
                vector<vec2> luopp;

                FOR(i, coeff.size()) {
                    // a + c*(b-a) returns exactly a when a==b and no NaNs involved
                    // no need to worry about the cases when c==0 and c==1, because of the presnapping of the parameterization
                    vec3 pt = X(m)[fec.org(h)] + coeff[i] * (X(m)[fec.dest(h)] - X(m)[fec.org(h)]);
                    vec2 u  = lU[0] + coeff[i] * (lU[1] - lU[0]);
                    vec2 uopp;
                    if (NOT_AN_ID != opp) uopp = uv[fec.next(opp)] + coeff[i] * (uv[opp] - uv[fec.next(opp)]);

                    FOR(d, 2) { // we must guarantee that new vertices have (at least) one integer component
                        if (std::abs(u[d] - round(u[d])) < 1e-10) {
                            u[d] = round(u[d]);
                        }

                        if (NOT_AN_ID != opp && std::abs(uopp[d] - round(uopp[d])) < 1e-10) {
                            uopp[d] = round(uopp[d]);
                        }
                    }

                    pts.push_back(pt);
                    lu.push_back(u);
                    if (NOT_AN_ID != opp) luopp.push_back(uopp);
                }

                // create vertices
                index_t off = m->vertices.create_vertices(pts.size());
                FOR(i, pts.size()) {
                    added_vertices[i+off] = true;
                    resp_facet[i+off] = orig_tri_fid[fec.facet(h)];
                }
                FOR(i, pts.size()) {
                    m->vertices.point(off + i) = pts[i];
                    new_corners[h].push_back(NewCorner(off + i, lu[i]));
                    if (NOT_AN_ID != opp)
                        new_corners[opp].push_back(NewCorner(off + i, luopp[i]));
                }
                if (NOT_AN_ID != opp)
                    std::reverse(new_corners[opp].begin(), new_corners[opp].end());
            }
        }

        // now we have all the corners to insert, we create new facets for all the surface, old facets are to delete

        reach("split edges");
        FOR(f, nb_init_facets) {
            vector<index_t> polyV;
            vector<vec2> poly_uv;
            FOR(fc, m->facets.nb_corners(f)) {
                polyV.push_back(m->facets.vertex(f, fc));
                index_t c = m->facets.corner(f, fc);
                poly_uv.push_back(uv[c]);
                FOR(i, new_corners[c].size()) {
                    polyV.push_back(new_corners[c][i].first);
                    poly_uv.push_back(new_corners[c][i].second);
                }
            }
            index_t nf = m->facets.create_polygon(polyV);
            m->facets.attributes().copy_item(nf ,f);
            FOR(fc, m->facets.nb_corners(nf))
                uv[m->facets.corner(nf, fc)] = poly_uv[fc];
        }

        reach("kill facets");
        vector<index_t> to_kill(nb_init_facets, true); // kill old (pre-split) facets
        to_kill.resize(m->facets.nb(), false);
        m->facets.delete_elements(to_kill);
    }


    /**
     * INPUT:        facets with uv coordinates, where many vertices have integer values in uv
     * OUTPUT:       facets with uv coordinates, inclusing edges that have one coordinate of uv that is constant and integer valued
     *
     * remark:  some polylines of these new edges are likely to be pre-images of edges of the regular grid...
     */


    void find_degenerate_facets(Mesh* m, vector<index_t> &degenerate) {
        degenerate.clear();
        FOR(f, m->facets.nb()) {
            index_t nbv = m->facets.nb_corners(f);
//            geo_assert(3 == m->facets.nb_corners(f));
            FOR (c1, nbv) {
                FOR (c2, nbv) {
                    if (c1 == c2) continue;
                    index_t v1 = m->facets.vertex(f, c1);
                    index_t v2 = m->facets.vertex(f, c2);

                    geo_assert( v1!= v2 );
                    if (X(m)[v1][0] == X(m)[v2][0] && X(m)[v1][1] == X(m)[v2][1] && X(m)[v1][2] == X(m)[v2][2]) {
                        degenerate.push_back(f);
                    }
                }
            }
        }
    }

    void imprint(Mesh* m, const char *uv_name, const char *singular_name) {
        {
            Attribute<index_t> singular(m->facets.attributes(), singular_name);
            Attribute<index_t> resp_facet(m->vertices.attributes(), "resp_facet");
            Attribute<index_t> orig_tri_fid(m->facets.attributes(), "orig_tri_fid");
            FOR(i, m->facets.nb()  ) orig_tri_fid[i] = i;
            FOR(v, m->vertices.nb()) resp_facet[v] = NOT_AN_ID;
        
			check_no_intersecting_faces(m);
		}



        Mesh m_bak;
        m_bak.copy(*m);

        for(;;) {
            split_edges_by_iso_uvs(m, uv_name, singular_name);
            facets_split(m, uv_name, singular_name);

            triangulate_surface_preserve_attributes(m);

            vector<index_t> fails;
            find_degenerate_facets(m, fails);

            vector<index_t> intersections;
            find_self_intersections(m, intersections);
            fails.insert(fails.end(), intersections.begin(), intersections.end()); // degenerate triangles or intersecting ones, all the same, I do not want them
            if (!fails.size()) break;

            plop("imprint fail, need to undo");
            {
                Attribute<index_t> singular(m_bak.facets.attributes(), singular_name);
                Attribute<index_t> resp_facet(m->vertices.attributes(), "resp_facet");

				FOR(i, fails.size()) {
					FOR(j, 3) {
						index_t f = resp_facet[m->facets.vertex(fails[i], j)];
						if (NOT_AN_ID!=f) singular[f] = true;
                    }
                }
            }
            m->copy(m_bak);
        }
    }

    void facets_split(Mesh* m, const char *uv_name, const char *singular_name) {
        Attribute<bool> added_vertices(m->vertices.attributes(), "added_vertices");
        Attribute<GEO::vec2> uv(m->facet_corners.attributes(), uv_name);
        Attribute<index_t> singular(m->facets.attributes(), singular_name);

        Attribute<index_t> orig_tri_fid(m->facets.attributes(), "orig_tri_fid");
        Attribute<index_t> resp_facet(m->vertices.attributes(), "resp_facet");

        vector<index_t> to_kill(m->facets.nb(), 0);
        FOR(f, m->facets.nb()) {
            if (singular[f]) continue;
            index_t nbc = m->facets.nb_corners(f);


            // STEP 1: find a couple (coordinate, iso value) to split the facet
            index_t coord = index_t(-1);
            double iso = 0;
            FOR(test_coord, 2) {
                double min_v =  1e20;
                double max_v = -1e20;
                FOR(fc, nbc) {
                    min_v = std::min(min_v, uv[m->facets.corner(f, fc)][test_coord]);
                    max_v = std::max(max_v, uv[m->facets.corner(f, fc)][test_coord]);
                }
                if (floor(min_v)+1 < max_v) {  // floor(min_v)+1 is the first integer value strictly superior to min_v
                    coord = test_coord;
                    iso = floor(min_v)+1;
                    break;
                }
            }
            if (coord == index_t(-1)) continue;

            // STEP 2: if STEP 1 succedeed, compute the extremities of the new edge
            index_t cut[2] = { NOT_AN_ID, NOT_AN_ID };

            FOR(fc, nbc) {
                if (uv[m->facets.corner(f, fc)][coord] != iso) continue;
                if (cut[0] == NOT_AN_ID) {
                    cut[0] = fc;
                } else if (fc != next_mod(cut[0], nbc) && next_mod(fc, nbc) != cut[0]) { // no biangles
                    cut[1] = fc;
                }
            }
            if (cut[1] == NOT_AN_ID) continue;

            { // let us check that the cut separates the facet
                bool inf1=true, sup1=true, inf2=true, sup2=true;
                for (index_t fc = cut[0]+1; fc<cut[1]; fc++) {
                    double tex = uv[m->facets.corner(f, fc)][coord];
                    inf1 = inf1 && (tex<iso);
                    sup1 = sup1 && (tex>iso);
                }
                for (index_t fc = cut[1]+1; fc<cut[0]+nbc; fc++) {
                    double tex = uv[m->facets.corner(f, fc%nbc)][coord];
                    inf2 = inf2 && (tex<iso);
                    sup2 = sup2 && (tex>iso);
                }
                if (!( (inf1 && sup2) || (sup1 && inf2) )) {
                    plop("WARNING: facet cannot be properly cut by the iso");
                    continue;
                }
            }


            to_kill[f] = true;

            // STEP 3: compute new vertices (iso-integer value of uv) on the new edge
            vector<vec3> nv_pts;
            vector<vec2> nv_uv;
            {
                vec3 lX[2];
                FOR(i, 2) lX[i] = m->vertices.point(m->facets.vertex(f, cut[i]));
                vec2 lU[2];
                FOR(i, 2) lU[i] = uv[m->facets.corner(f, cut[i])];
                vector<double> coeff;
                double v[2] = { lU[0][(coord+1)%2], lU[1][(coord+1)%2] }; // recall that coord is the cutting dimension

                for (double cur_iso = ceil(std::min(v[0], v[1])); cur_iso < std::max(v[0], v[1]); cur_iso += 1.0) {
                    double c = (cur_iso - v[0]) / (v[1] - v[0]);
		    if (!Numeric::is_nan(c) && c > 0 && c < 1)
                        coeff.push_back(c);
                }

                std::sort(coeff.begin(), coeff.end(), std::less<double>());
                FOR(i, coeff.size()) {
                    vec3 x = lX[0] + coeff[i] * (lX[1] - lX[0]); // it guarantees x==lX[0] when lX[0]==lX[1]
                    vec2 u = lU[0] + coeff[i] * (lU[1] - lU[0]); // no need to worry about coeff[i]==0 and coeff[i]==1 because of the parameterization pre-snapping
                    nv_pts.push_back(x);
                    nv_uv.push_back(u);
                }
                // new vertices must have only integer values of uv --- remove possible numerical imprecision
                FOR(i, nv_pts.size()) {
                    FOR(d, 2) {
                        nv_uv[i][d] = round(nv_uv[i][d]);
                    }
                }
            }

            // STEP 4: create new vertices and new faces
            index_t off = m->vertices.create_vertices(nv_pts.size());
            FOR(i, nv_pts.size()) {
                resp_facet[off+i] = orig_tri_fid[f];
                added_vertices[off+i] = true;
                X(m)[off + i] = nv_pts[i];
            }
            FOR(half, 2) {
                vector <index_t> lv;
                vector <vec2> luv;

                // add original vertices
                index_t cir = cut[half];
                do {
                    lv.push_back(m->facets.vertex(f, cir));
                    luv.push_back(uv[m->facets.corner(f, cir)]);
                    cir = next_mod(cir, nbc);
                } while (cir != cut[(half + 1) % 2]);
                lv.push_back(m->facets.vertex(f, cir));
                luv.push_back(uv[m->facets.corner(f, cir)]);

                // add new vertices
                FOR(i, nv_pts.size()) {
                    index_t ind = i;
                    if (half == 0) ind = nv_pts.size() - 1 - i;
                    lv.push_back(off + ind);
                    luv.push_back(nv_uv[ind]);
                }

                index_t fid = m->facets.create_polygon(lv);
                m->facets.attributes().copy_item(fid, f);
                FOR(fc, m->facets.nb_corners(fid)) {
                    uv[m->facets.corner(fid, fc)] = luv[fc];
                }
                to_kill.push_back(false);
            }
        }
        m->facets.delete_elements(to_kill);
   }

    /**
     * INPUT:        facets with uv coordinates
     * OUTPUT:       chart facet attribute s.t. the chart frontier is included in edges that are iso-integer value of 'uv'
     */

    inline index_t indir_root(index_t i, vector<index_t>& indir) {
        while (i != indir[i]) i = indir[i];
        return i;
    }

    // merges charts for adjacent facets under two conditions:
    // 1) both facets are not singular
    // 2) the shared edge is not integer iso in both facets
    void mark_charts(Mesh* m, const char *uv_name, const char *chart_name, const char *singular_name) { // 2-manifold surface is supposed
        Attribute<bool> isovalue(m->facet_corners.attributes(), "isovalue");
        Attribute<bool> quadelement(m->facets.attributes(), "quadelement");
        Attribute<bool> quadcorners(m->vertices.attributes(), "quadcorners");
        Attribute<bool> added_vertices(m->vertices.attributes(), "added_vertices");

        Attribute<GEO::vec2> uv(m->facet_corners.attributes(), uv_name);
        Attribute<index_t> singular(m->facets.attributes(), singular_name);
        Attribute<index_t> chart(m->facets.attributes(), chart_name);

        FacetsExtraConnectivity fec(m);

        vector<index_t> indir(m->facets.nb());
        { // fill isovalue edge attribute and merge quad candidate charts
            FOR(f, m->facets.nb()) {
                indir[f] = f;
            }

            FOR(h, m->facet_corners.nb()) {
                index_t hopp = fec.opposite(h);
                index_t f = fec.facet(h);
                index_t fopp = NOT_AN_ID==hopp ? NOT_AN_ID : fec.facet(hopp);

                if (singular[f] || (NOT_AN_ID!=hopp && hopp>h)) continue;

                bool cut = false;
                index_t test[2] = { h, hopp };
                FOR(lh, 2) {
                    if (lh && (NOT_AN_ID==hopp || singular[fopp])) break;
                    FOR(coord, 2) {
                        cut = cut || (uv[test[lh]][coord] == uv[fec.next(test[lh])][coord] && uv[test[lh]][coord] == round(uv[test[lh]][coord]));
                    }
                }

                if (cut) {
                    isovalue[h] = true;
                    if (NOT_AN_ID!=hopp) isovalue[hopp] = true;
                }

                if (!cut && NOT_AN_ID!=fopp && !singular[fopp]) {
                    indir[indir_root(fopp, indir)] = indir_root(f, indir);
                }
            }

            FOR(f, m->facets.nb()) {
                chart[f] = indir_root(f, indir);
            }
        }


        { // determine quad corner vertices: fill quadcorners attribute
            FOR (h, m->facet_corners.nb()) {
                int cnt = 0;
                index_t cir = h;
                do {
                    if (NOT_AN_ID==cir) break;
                    cnt += int(isovalue[cir]);
                    cir = fec.next_around_vertex(cir);
                } while (cir != h);

                if ((NOT_AN_ID==cir && cnt>=2) || cnt>2) {
                    quadcorners[fec.org(h)] = true;
                }
            }
        }


        { // fill quadelement attribute

            // this code verifies only if chart boundaries are marked as isovalues + if it has 4 quadcorners
            // normally it is also necessary to check if the "quad" is a 2d disk (one boundary + Euler characterisic (imagine a torus with a hole))

            vector<bool> seen(m->facets.nb(), false);
            FOR(fseed, m->facets.nb()) {
                if (fseed != chart[fseed]) continue;
                int nb_quad_corners = 0;
                std::deque<index_t> Q;
                Q.push_back(fseed);
                seen[fseed] = true;
                bool iso_only_at_boundaries = true;
                std::vector<index_t> C;
                while (Q.size()) {
                    index_t f = Q.front();
                    if (singular[f]) {
                        iso_only_at_boundaries = false;
                        break;
                    }
                    C.push_back(f);
                    Q.pop_front();
                    FOR(fc, m->facets.nb_corners(f)) {
                        index_t h = m->facets.corner(f, fc);
                        index_t hopp = fec.opposite(h);
                        index_t fopp = NOT_AN_ID==hopp ? NOT_AN_ID : fec.facet(hopp);
                        if (fopp==NOT_AN_ID) {
                            if (isovalue[h] && quadcorners[fec.org(h)]) nb_quad_corners++;
                        } else {
                            if (chart[fopp] != chart[fseed]) {
                                if (isovalue[h]) {
                                    if (quadcorners[fec.org(h)])
                                        nb_quad_corners++;
                                    continue;
                                }
                                Q.clear();
                                iso_only_at_boundaries = false;
                                break;
                            }
                        }
                        if (NOT_AN_ID!=fopp && !seen[fopp]) {
                            Q.push_back(fopp);
                            seen[fopp] = true;
                        }
                    }
                }
                if (iso_only_at_boundaries && nb_quad_corners == 4) {
                    FOR(i, C.size()) {
                        quadelement[C[i]] = true;
                    }
                }
            }
        }

        { // charts = orig triangles in non-quad regions
            FOR(f, m->facets.nb()) {
                if (!quadelement[f]) indir[f] = f;
            }

            Attribute<index_t> orig_tri_fid(m->facets.attributes(), "orig_tri_fid");
            FOR(h, m->facet_corners.nb()) {
                index_t hopp = fec.opposite(h);
                index_t f = fec.facet(h);
                index_t fopp = NOT_AN_ID==hopp ? NOT_AN_ID : fec.facet(hopp);
                if (NOT_AN_ID == fopp || quadelement[f] || quadelement[fopp] || orig_tri_fid[f]!=orig_tri_fid[fopp]) continue;
                indir[indir_root(fopp, indir)] = indir_root(f, indir);
            }

            FOR(f, m->facets.nb()) {
                chart[f] = indir_root(f, indir);
            }
        }

        vector<vector<index_t> > v2f = generate_v2f(m);
        { // mark the boundary between triangles and quads
            FOR(v, m->vertices.nb()) {
                TrFan fan = TrFan(v, m, v2f, chart);
                bool touches_a_quad = false;
                FOR(i, fan.nb_fan_triangles()) {
                    touches_a_quad = touches_a_quad || quadelement[fan[i].f];
                }
                if (quadcorners[v] || !touches_a_quad || fan.ncharts()<=2) continue;

                index_t first_triangle = NOT_AN_ID;
                FOR(i, fan.nb_fan_triangles()) {
                    if (quadelement[fan[i].f]) continue;
                    if (NOT_AN_ID==first_triangle) {
                        first_triangle = fan[i].f;
                    }
                    indir[indir_root(first_triangle, indir)] = indir_root(fan[i].f, indir);
                }
            }
            FOR(f, m->facets.nb()) {
                chart[f] = indir_root(f, indir);
            }
        }

        { // fill verts to remove attribute
            Attribute<bool> verts_to_remove(m->vertices.attributes(), "verts_to_remove");
            FOR(v, m->vertices.nb()) {
                TrFan fan = TrFan(v, m, v2f, chart);
                bool touches_a_quad = false;
                FOR(i, fan.nb_fan_triangles()) {
                    touches_a_quad = touches_a_quad || quadelement[fan[i].f];
                }
                if (quadcorners[v])                         continue;
                if ( fan.incomplete_ && fan.ncharts()!=1)   continue;
                if (!fan.incomplete_ && fan.ncharts() >2)   continue;
                if (!touches_a_quad  && !added_vertices[v]) continue;
                verts_to_remove[v] = true;
            }
        }
    }

    /****************************************************************************************************/
    static bool try_export_quadtri_from_charts(Mesh* m, vector<BBox>& locked_regions) {
        bool modified = false;
        Attribute<index_t> chart(m->facets.attributes(), "chart");      // TODO document this
        Attribute<bool> quadelement(m->facets.attributes(), "quadelement");

        vector<index_t> to_kill(m->facets.nb(), false);

        index_t max_chart_no = 0;
        FOR(f, m->facets.nb()) {
            max_chart_no = std::max(max_chart_no, chart[f] + 1);
        }
        vector<int> nbtri_in_chart(max_chart_no, 0);
        FOR(f, m->facets.nb()) {
            nbtri_in_chart[chart[f]]++;
        }

        FacetsExtraConnectivity fec(m);
        index_t nbf = m->facets.nb();
        FOR(f, nbf) {
            geo_assert(3 == m->facets.nb_corners(f));

            if (nbtri_in_chart[chart[f]] != 2 || !quadelement[f]) continue;
            FOR(ih, 3) {
                index_t h = m->facets.corner(f, ih);
                index_t hopp = fec.opposite(h);
                if (NOT_AN_ID==hopp) continue;
                index_t fopp = fec.facet(hopp);
                geo_assert(NOT_AN_ID != fopp);
                if (chart[fopp] != chart[f]) continue;
                if (f < fopp) break;

                vector<index_t> pts;
                pts.push_back(fec.dest(h));
                pts.push_back(fec.dest(fec.next(h)));
                pts.push_back(fec.org(h));
                pts.push_back(fec.dest(fec.next(hopp)));

                bool intersect_locked_region = false;
                BBox b;
                FOR(v, 4) {
                    b.add(X(m)[pts[v]]);
                }
                FOR(i, locked_regions.size()) {
                    intersect_locked_region = intersect_locked_region || locked_regions[i].intersect(b);
                }

                if (!intersect_locked_region) {
                    index_t nf = m->facets.create_polygon(pts);
                    modified = true;
                    m->facets.attributes().copy_item(nf ,f);
                    to_kill.push_back(false);
                    to_kill[f] = true;
                    to_kill[fopp] = true;
                }
            }
        }
        m->facets.delete_elements(to_kill, false);
        return modified;
    }
 
    


      static void sample_triangle(vec3 *ABC, double eps, vector<vec3>& samples) {
		double max_edge_length = 0;
		FOR(p, 3) max_equal(max_edge_length, (ABC[(p + 1) % 3] - ABC[p]).length());
		index_t nb_steps = 10;
		if (eps>0) min_equal(nb_steps, index_t(max_edge_length / eps + 2));

		FOR(i, nb_steps)FOR(j, nb_steps - i) {
			double u = double(i) / double(nb_steps - 1);
			double v = double(j) / double(nb_steps - 1);
			samples.push_back(ABC[0] + u*(ABC[1] - ABC[0]) + v*(ABC[2] - ABC[0]));
		}
	}
	//double upper_bound_min_dist2_to_triangles(vec3 P, vector<vec3>& triangles) {
	//	vec3 closest_point;
	//	double l0, l1, l2;
	//	double min_dist2 = 1e20;
	//	FOR(t, triangles.size() / 3)
	//		min_equal(min_dist2,
	//			Geom::point_triangle_squared_distance<vec3>(P,
	//				triangles[t * 3], triangles[t * 3 + 1], triangles[t * 3 + 2], closest_point, l0, l1, l2)
	//		);
	//	return min_dist2;
	//}

	static vector<index_t> facets_having_a_point_further_than_eps(Mesh* m,Mesh* ref,double epsilon) {
	
		vector<index_t> res;

		vector<BBox> inboxes = facets_bbox(ref);
		DynamicHBoxes hb;  hb.init(inboxes);

		FOR(f, m->facets.nb()) {
			bool fail = false;
			vector<vec3> samples;
			vec3 ABC[3];
			FOR(lv,3) ABC[lv] = X(m)[m->facets.vertex(f,lv)];
			sample_triangle(ABC, epsilon / 2., samples);
			if (m->facets.nb_vertices(f) == 4) {
				FOR(lv, 3) ABC[lv] = X(m)[m->facets.vertex(f, (lv+2)%3)];
				sample_triangle(ABC, epsilon / 2., samples);
			}


			FOR(p, samples.size()) {
				vec3 P = samples[p];

				double min_dist2 = 1e20;

				BBox bbox; bbox.add(P); bbox.dilate(epsilon/2.);
				vector<index_t> prim;
				hb.intersect(bbox, prim);
				FOR(fid, prim.size()) {
					index_t other_f = prim[fid];
					vec3 closest_point;
					double l0, l1, l2;
					min_equal(min_dist2, Geom::point_triangle_squared_distance<vec3>(P,
						X(ref)[ref->facets.vertex(other_f, 0)],
						X(ref)[ref->facets.vertex(other_f, 1)],
						X(ref)[ref->facets.vertex(other_f, 2)],
						closest_point, l0, l1, l2));

					if (ref->facets.nb_vertices(other_f) == 4) {
						min_equal(min_dist2, Geom::point_triangle_squared_distance<vec3>(P,
							X(ref)[ref->facets.vertex(other_f, 0)],
							X(ref)[ref->facets.vertex(other_f, 2)],
							X(ref)[ref->facets.vertex(other_f, 3)],
							closest_point, l0, l1, l2));
					}
				}
			
				if (::sqrt(min_dist2) > epsilon / 2.) {
					fail = true;
					break;
				}
			}
			if (fail) res.push_back(f);
		}
		return res;
	}

    // Attention, sub-functions of this function need to access to attributes "chart" and "singular"
    void simplify_quad_charts(Mesh* m) {
        std::string msg;
        if (!surface_is_manifold(m, msg)) plop(msg);

        Mesh m_bak;
        m_bak.copy(*m);
		double epsilon = 0;// .4*get_facet_average_edge_size(&m_bak);
		//epsilon = 0;
        vector<BBox> locked_regions;
//        int cnt = 0;
        for(;;) {
			vector<index_t> invalid_m ;
			vector<index_t> invalid_bak ;
			vector<index_t> intersections;
			{
				Attribute<index_t> chart(m->facets.attributes(), "chart");
				Attribute<index_t> undo(m->facets.attributes(), "undo");
                FOR(fid, m->facets.nb()) {
                    undo[fid] = NOT_AN_ID;
                }
				Attribute<bool> verts_to_remove(m->vertices.attributes(), "verts_to_remove");
				plop("try_simplify(m, chart, verts_to_remove, undo)");
				try_simplify(m, chart, verts_to_remove, undo);
				plop("try_export_quadtri_from_charts(m, locked_regions)");
				try_export_quadtri_from_charts(m, locked_regions);

				plop("check for intersections");
				plop(m->facets.nb());
				find_self_intersections(m, intersections);
                FOR(f, intersections.size()) {
                    if (3 == m->facets.nb_vertices(f)) {
                        if (undo[intersections[f]] != NOT_AN_ID) verts_to_remove[undo[intersections[f]]] = false;
//                        GEO::Logger::out("HexDom")  << intersections[f] << " " << undo[intersections[f]] <<  std::endl;
                    } else {
                        geo_assert(4 == m->facets.nb_vertices(f));
                        BBox inbox;
                        FOR(fv, 4) {
                            inbox.add(X(m)[m->facets.vertex(intersections[f], fv)]);
                        }
                        locked_regions.push_back(inbox);
                    }
//                  Attribute<bool> gna(m->facets.attributes(), "gna");
//                  gna[intersections[f]] = true;
                }
				//intersections.clear();// HACK (simulation de quadhex)

				if (epsilon > 0) {
					plop("check for Hausdorff distance --- dist to init mesh");
					invalid_m = facets_having_a_point_further_than_eps(m, &m_bak, epsilon);
					plop(invalid_m.size());
					FOR(i, invalid_m.size()) {
						if (3 == m->facets.nb_vertices(invalid_m[i])) {
							if (undo[invalid_m[i]] != NOT_AN_ID) verts_to_remove[undo[invalid_m[i]]] = false;
						}
						else {
							BBox inbox;
							FOR(fv, 4) {
								inbox.add(X(m)[m->facets.vertex(invalid_m[i], fv)]);
							}
							locked_regions.push_back(inbox);
						}
					}

					plop("check for Hausdorff distance --- dist to new mesh");
					invalid_bak = facets_having_a_point_further_than_eps(&m_bak, m, epsilon);
					plop(invalid_bak.size());

					FOR(i, invalid_bak.size()) FOR(lv, 3)
						verts_to_remove[m_bak.facets.vertex(invalid_bak[i], lv)] = false;
				}
			}


			if (intersections.empty() && invalid_m.empty() && invalid_bak.empty()) break;

			plop("conflict detected");
            {
                Attribute<bool> m_verts_to_remove(m->vertices.attributes(), "verts_to_remove");
                Attribute<bool> m_bak_verts_to_remove(m_bak.vertices.attributes(), "verts_to_remove");
                geo_assert(m->vertices.nb() == m_bak.vertices.nb());
                FOR(v, m->vertices.nb()) m_bak_verts_to_remove[v] = m_verts_to_remove[v];
            }

//          char filename[1024], filename2[1024];
//          sprintf(filename,  "/home/ssloy/tmp/hexdom_nightly/geogram/zdebug%i_bak.geogram", cnt);
//          sprintf(filename2, "/home/ssloy/tmp/hexdom_nightly/geogram/zdebug%i_simp.geogram", cnt);
//          cnt++;
//          mesh_save(m_bak, filename);
//          mesh_save(*m, filename2);

            m->copy(m_bak);
        }
        kill_isolated_vertices(m);
    }



    
}

