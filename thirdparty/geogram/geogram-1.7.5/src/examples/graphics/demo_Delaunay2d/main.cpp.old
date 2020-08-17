/*
 *  Copyright (c) 2012-2014, Bruno Levy
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
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram_gfx/basic/GLSL.h>
#include <geogram_gfx/basic/GL.h>
#include <geogram_gfx/GLUP/GLUP.h>
#include <geogram_gfx/glup_viewer/glup_viewer.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/logger.h>
#include <geogram/numerics/predicates.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/mesh_io.h>

namespace {

    using namespace GEO;

    vector<vec2> points;
    Delaunay_var delaunay;
    typedef vector<vec2> Polygon;
    Polygon border;

    void Lloyd_relaxation();
    
    void convex_clip_polygon(
        const Polygon& P, const Polygon& clip, Polygon& result
    );

    vec2 centroid(const Polygon& P);
    
    /**
     * \brief Updates the Delaunay triangulation with the current
     *  vector of points.
     */
    void update_Delaunay() {
        delaunay->set_vertices(points.size(), &points.data()->x);
    }

    /**
     * \brief Creates random points.
     * \details Points are created uniformly in the [0,1]x[0,1] 
     *  square
     * \param[in] nb the number of points to create.
     */
    void create_random_points(index_t nb) {
        for(index_t i=0; i<nb; ++i) {
            points.push_back(
                vec2(0.25, 0.25) + 
                vec2(Numeric::random_float64()*0.5, Numeric::random_float64()*0.5)
            );
        }
        update_Delaunay();
    }

    /**
     * \brief Gets the index of the point that matches a given
     *  point.
     * \details The current GLUP point size is taken into account
     *  to determine the tolerance.
     * \param[in] p a const reference coordinate to a world-space
     *  point.
     * \param[in] get_nearest if true, gets the nearest point, else
     *  only gets the point that matches p (up to current value of
     *  point size when reprojected on the screen)
     * \return the index of the point that corresponds to p if it
     *  exists, or index_t(-1) if no such point exists.
     */
    index_t get_picked_point(const vec2& p, bool get_nearest = false) {
        if(points.size() == 0) {
            return index_t(-1);
        }
        double dist_so_far = Numeric::max_float64();
        index_t nearest = index_t(-1);
        for(index_t i=0; i<points.size(); ++i) {
            double dist = distance2(p, points[i]);
            if(dist < dist_so_far) {
                nearest = i;
                dist_so_far = dist;
            }
        }
        if(!get_nearest) {
            vec3 q(points[nearest].x, points[nearest].y, 0.0);
            vec3 p_scr;
            vec3 q_scr;
            glup_viewer_project(p.x, p.y, 0, &p_scr.x, &p_scr.y, &p_scr.z);
            glup_viewer_project(q.x, q.y, 0, &q_scr.x, &q_scr.y, &q_scr.z);
            if(distance2(p_scr,q_scr) > 2.0 * double(glupGetPointSize())) {
                nearest = index_t(-1);
            }
        }
        return nearest;
    }

    /**
     * \brief The size of all the displayed points.
     */
    GLint point_size = 20;


    void set_border_as_polygon(index_t nb_sides) {
        border.clear();
        for(index_t i=0; i<nb_sides; ++i) {
            double alpha = double(i) * 2.0 * M_PI / double(nb_sides);
            double s = sin(alpha);
            double c = cos(alpha);
            border.push_back( vec2(0.5*(c + 1.0), 0.5*(s + 1.0)) );
        }
    }
    
    void set_border_shape(int shape) {
        static int current_shape=-1;
        if(current_shape != shape) {
            current_shape = shape;
        }
        switch(shape) {
        case 1:
            set_border_as_polygon(3);
            break;
        case 2:
            set_border_as_polygon(5);
            break;
        case 3:
            set_border_as_polygon(100);
            break;
        default:
            border.clear();            
            border.push_back(vec2(0.0, 0.0));
            border.push_back(vec2(1.0, 0.0));
            border.push_back(vec2(1.0, 1.0));
            border.push_back(vec2(0.0, 1.0));        
            break;
        }
    }
    
    /**
     * \brief Initializes OpenGL objects.
     * \details Specifed as glup_viewer_set_init_func() callback.
     */
    void init() {
        GEO::Graphics::initialize();
        
        glup_viewer_set_background_color(1.0, 1.0, 1.0);
        glup_viewer_add_toggle(
            'T', glup_viewer_is_enabled_ptr(GLUP_VIEWER_TWEAKBARS),
            "Toggle tweakbars"
        );

        glup_viewer_disable(GLUP_VIEWER_BACKGROUND);
        glup_viewer_disable(GLUP_VIEWER_3D);
        
	delaunay = Delaunay::create(2,"BDEL2d");
	create_random_points(3);
    }

    /**
     * \brief Displays the border of the domain.
     */
    void display_border() {
        glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.0, 0.0, 0.0);
        glupSetMeshWidth(4);
        glupBegin(GLUP_LINES);
        for(index_t i=0; i<border.size(); ++i) {
            glupVertex(border[i]);
            glupVertex(border[(i+1)%border.size()]);
        }
        glupEnd();
    }
    
    /**
     * \brief Displays the points.
     */
    void display_points() {
       glupEnable(GLUP_LIGHTING);
       glupSetPointSize(GLfloat(point_size));
       glupDisable(GLUP_VERTEX_COLORS);
       glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.0f, 1.0f, 1.0f);
       glupBegin(GLUP_POINTS);
       for(index_t i=0; i<points.size(); ++i) {
           glupVertex(points[i]);
       }
       glupEnd();
       glupDisable(GLUP_LIGHTING);       
    }

    /**
     * \brief Displays the Delaunay triangles.
     */
    void display_Delaunay_triangles() {
        glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.7f, 0.7f, 0.7f);
        glupSetMeshWidth(1);
        glupBegin(GLUP_LINES);
        for(index_t c=0; c<delaunay->nb_cells(); ++c) {
            const signed_index_t* cell = delaunay->cell_to_v() + 3*c;
            for(index_t e=0; e<3; ++e) {
                signed_index_t v1 = cell[e];
                signed_index_t v2 = cell[(e+1)%3];
                glupVertex2dv(delaunay->vertex_ptr(index_t(v1)));
                glupVertex2dv(delaunay->vertex_ptr(index_t(v2)));
            }
        }
        glupEnd();
    }

    /**
     * \brief Gets the circumcenter of a triangle.
     * \param[in] t the index of the triangle, in 0..delaunay->nb_cells()-1
     * \return the circumcenter of triangle \p t
     */
    vec2 circumcenter(index_t t) {
        signed_index_t v1 = delaunay->cell_to_v()[3*t];
        signed_index_t v2 = delaunay->cell_to_v()[3*t+1];
        signed_index_t v3 = delaunay->cell_to_v()[3*t+2];
        vec2 p1(delaunay->vertex_ptr(index_t(v1)));
        vec2 p2(delaunay->vertex_ptr(index_t(v2)));
        vec2 p3(delaunay->vertex_ptr(index_t(v3)));
        return Geom::triangle_circumcenter(p1,p2,p3);
    }

    /**
     * \brief Gets an infinite vertex in the direction normal to an
     *  edge on the boundary of the convex hull.
     * \param[in] t the index of the triangle, in 0..delaunay->nb_cells()-1
     * \param[in] e the local index of the edge, in {0,1,2}
     * \return a point located far away along the direction normal to the
     *  edge \p e of triangle \p t
     */
    vec2 infinite_vertex(index_t t, index_t e) {
        index_t lv1 = (e+1)%3;
        index_t lv2 = (e+2)%3;
        index_t v1 = index_t(delaunay->cell_to_v()[3*t+lv1]);
        index_t v2 = index_t(delaunay->cell_to_v()[3*t+lv2]);
        vec2 p1(delaunay->vertex_ptr(v1));
        vec2 p2(delaunay->vertex_ptr(v2));
        vec2 n = normalize(p2-p1);
        n = vec2(n.y, -n.x);
        return 0.5*(p1+p2)+100000.0*n;
    }

    /**
     * \brief Displays the Voronoi edges.
     */
    void display_Voronoi_edges() {
        glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.3f, 0.3f, 0.3f);
        glupSetMeshWidth(2);
        glupBegin(GLUP_LINES);
        for(index_t t=0; t<delaunay->nb_cells(); ++t) {
            vec2 cc = circumcenter(t);
            for(index_t e=0; e<3; ++e) {
                signed_index_t t2 = delaunay->cell_to_cell()[3*t+e];
                if(t2 == -1) {
                    glupVertex(cc);
                    glupVertex(infinite_vertex(t,e));
                } else if(t2 >signed_index_t(t)) {
                    glupVertex(cc);
                    glupVertex(circumcenter(index_t(t2)));
                }
            }
        }
        glupEnd();
    }

    /**
     * \brief Finds the local index of a vertex in a triangle.
     * \details Throws an assertion failure if the triangle \p t is
     *  not incident to vertex \p v
     * \param[in] t the triangle, in 0..delaunay->nb_cells()-1
     * \param[in] v the vertex, in 0..delaunay->nb_vertices()-1
     * \return the local index of v in t, in {0,1,2}
     */
    index_t find_vertex(index_t t, index_t v) {
        for(index_t lv=0; lv<3; ++lv) {
            if(index_t(delaunay->cell_to_v()[3*t+lv]) == v) {
                return lv;
            }
        }
        geo_assert_not_reached;
    }

    /**
     * \brief Gets a Voronoi cell of a vertex
     * \details The vertex is specified by a triangle and a local index in
     *  the triangle
     * \param[in] t0 the triangle
     * \param[in] lv the local index of the vertex in triangle \p t0
     * \param[out] cell a reference to the Voronoi cell
     */
    void get_Voronoi_cell(index_t t0, index_t lv, Polygon& cell) {
        cell.resize(0);
        index_t v = index_t(delaunay->cell_to_v()[3*t0+lv]);
        bool on_border = false;
        index_t t = t0;
        
        // First, we turn around the vertex v. To do that, we compute
        // lv, the local index of v in the current triangle. Following
        // the standard numerotation of a triangle, edge number lv is
        // not incident to vertex v. The two other edges (lv+1)%3 and
        // (lv+2)%3 of the triangle are indicent to vertex v. By always
        // traversing (lv+1)%3, we turn around vertex v.
        do {
            index_t e = (lv+1)%3;
            signed_index_t neigh_t = delaunay->cell_to_cell()[3*t+e];
            if(neigh_t == -1) {
                on_border = true;
                break;
            }
            cell.push_back(circumcenter(t));
            t = index_t(neigh_t);
            lv = find_vertex(t,v);
        } while(t != t0);

        
        // If one traversed edge is on the border of the convex hull, then
        // we empty the cell, and start turing around the vertex in the other
        // direction, i.e. by traversing this time edge (lv+2)%3 until we
        // reach the other edge on the border of the convex hull that is
        // incident to v.
        if(on_border) {
            cell.resize(0);
            cell.push_back(infinite_vertex(t,(lv + 1)%3));
            for(;;) {
                cell.push_back(circumcenter(t));                
                index_t e = (lv+2)%3;
                signed_index_t neigh_t = delaunay->cell_to_cell()[3*t+e];
                if(neigh_t == -1) {
                    cell.push_back(infinite_vertex(t, e));
                    break;
                }
                t = index_t(neigh_t);
                lv = find_vertex(t,v);
            }
        }

        Polygon clipped;
        convex_clip_polygon(cell, border, clipped);
        cell.swap(clipped);
    }


    /**
     * \brief Displays the Voronoi cells as filled polygons with
     *  random colors.
     */
    void display_Voronoi_cells() {
        std::vector<bool> v_visited(delaunay->nb_vertices());
        Polygon cell;
        glupEnable(GLUP_VERTEX_COLORS);
        glupBegin(GLUP_TRIANGLES);        
        for(index_t t=0; t<delaunay->nb_cells(); ++t) {
            for(index_t lv=0; lv<3; ++lv) {
                index_t v = index_t(delaunay->cell_to_v()[3*t+lv]);
                if(!v_visited[v]) {
                    glup_viewer_random_color_from_index(int(v));
                    v_visited[v] = true;
                    get_Voronoi_cell(t,lv,cell);
                    for(index_t i=1; i+1<cell.size(); ++i) {
                        glupVertex(cell[0]);
                        glupVertex(cell[i]);
                        glupVertex(cell[i+1]);
                    }
                }
            }
        }
        glupEnd();
        glupDisable(GLUP_VERTEX_COLORS);        
    }


    bool show_Voronoi_cells = true;
    bool show_Delaunay_triangles = true;
    bool show_Voronoi_edges = true;
    bool show_points = true;
    bool show_border = true;
    
    /**
     * \brief Draws all the elements of the Delaunay triangulation / 
     *  Voronoi diagram.
     * \details Specified as glup_viewer_set_display_func() callback.
     */
    void display() {
        if(show_Voronoi_cells) {
            display_Voronoi_cells();
        }
        if(show_Delaunay_triangles) {
            display_Delaunay_triangles();
        }
        if(show_Voronoi_edges) {
            display_Voronoi_edges();
        }
        if(show_points) {
            display_points();
        }
        if(show_border) {
            display_border();
        }
        if(glup_viewer_is_enabled(GLUP_VIEWER_IDLE_REDRAW)) {
            Lloyd_relaxation();
        }
    }

    /**
     * \brief Displays and manages the GUI.
     */
    void overlay() {
        static int border_shape = 0;
        
        ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_Once);
        ImGui::SetNextWindowSize(ImVec2(180, 320), ImGuiCond_Once);

        ImGui::Begin("Delaunay");

        ImGui::Text("Display options");
        ImGui::Checkbox("Voronoi cells", &show_Voronoi_cells);
        ImGui::Checkbox("Voronoi edges", &show_Voronoi_edges);
        ImGui::Checkbox("Delaunay triangles", &show_Delaunay_triangles);
        ImGui::Checkbox("points", &show_points);
        ImGui::Checkbox("border", &show_border);

        ImGui::Separator();        
        ImGui::Text("Lloyd");
        if(ImGui::Button("one iteration")) {
            Lloyd_relaxation();
        }
        ImGui::Checkbox(
            "animate",
            (bool*)glup_viewer_is_enabled_ptr(GLUP_VIEWER_IDLE_REDRAW)
        );

        ImGui::Separator();                
        ImGui::Text("Border");
        ImGui::Combo("", &border_shape, "square\0triangle\0pentagon\0circle\0\0");

        ImGui::Separator();
        if(ImGui::Button("reset points")) {
            points.clear();
            create_random_points(3);
            update_Delaunay();
        }
        
        set_border_shape(border_shape);
        
        ImGui::End();
    }

    
    /**
     * \brief The callback function called for each mouse event
     * \param[in] x , y the screen-space coordinates of the mouse pointer
     * \param[in] button the button that changed state
     * \param[in] event one of GLUP_VIEWER_UP, GLUP_VIEWER_DOWN, GLUP_VIEWER_MOVE
     */
    GLboolean mouse(float x, float y, int button, enum GlupViewerEvent event) {
        const index_t NO_POINT = index_t(-1);        
        static index_t picked_point = NO_POINT;
        static index_t last_button = index_t(-1);
        
        GEO::geo_argused(x);
        GEO::geo_argused(y);

        GLdouble xyz[3];
        GLboolean bkgnd;
        glup_viewer_get_picked_point(xyz, &bkgnd);
        vec2 p(xyz[0], xyz[1]);
        
        if(glup_viewer_is_enabled(GLUP_VIEWER_IDLE_REDRAW)) {
            switch(event) {
            case GLUP_VIEWER_DOWN:
                last_button = index_t(button);
                break;
            case GLUP_VIEWER_UP:
                last_button = index_t(-1);
                break;
            case GLUP_VIEWER_MOVE:
                break;
            }

            switch(last_button) {
            case 0: {
                points.push_back(p);
                picked_point = points.size() - 1;                    
            } break;
            case 1: {
                picked_point = get_picked_point(p,true);                    
                if(points.size() > 3) {
                    points.erase(points.begin() + int(picked_point));
                }
            } break;
            }
            update_Delaunay();
            return GL_TRUE;
            
        } else {
            
            if(event == GLUP_VIEWER_DOWN) {
                picked_point = get_picked_point(p);
                switch(button) {
                case 0: {
                    if(picked_point == NO_POINT) {
                        points.push_back(p);
                        picked_point = points.size() - 1;                    
                    }
                } break;
                case 1: {
                    if(points.size() > 3) {
                        if(picked_point != NO_POINT) {
                            points.erase(points.begin() + int(picked_point));
                        }
                    }
                    picked_point = NO_POINT;
                } break;
                }
                update_Delaunay();
                return GL_TRUE;
            }
            if(event == GLUP_VIEWER_MOVE && picked_point != NO_POINT) {
                points[picked_point] = p;
                update_Delaunay();
                return GL_TRUE;
            }
            return GL_FALSE;
        }
    }


    void Lloyd_relaxation() {
        std::vector<bool> v_visited(delaunay->nb_vertices());        
        vector<vec2> new_sites(points.size());
        Polygon cell;
        for(index_t t=0; t<delaunay->nb_cells(); ++t) {
            for(index_t lv=0; lv<3; ++lv) {
                index_t v = index_t(delaunay->cell_to_v()[3*t+lv]);
                if(!v_visited[v]) {
                    v_visited[v] = true;
                    get_Voronoi_cell(t,lv,cell);
                    if(cell.size() > 0) {
                        new_sites[v] = centroid(cell);
                    } else {
                        new_sites[v] = points[v];
                    }
                }
            }
        }
        for(index_t v=0; v<points.size(); ++v) {
            points[v] = new_sites[v];
        }
        update_Delaunay();
    }

    
}


/*********************************************************************/

namespace {
    using namespace GEO;

    // http://astronomy.swin.edu.au/~pbourke/geometry/polyarea/
    double signed_area(const Polygon& P) {
        double result = 0 ;
        for(unsigned int i=0; i<P.size(); i++) {
            unsigned int j = (i+1) % P.size() ;
            const vec2& t1 = P[i] ;
            const vec2& t2 = P[j] ;
            result += t1.x * t2.y - t2.x * t1.y ;
        }
        result /= 2.0 ;
        return result ;
    }

    // http://astronomy.swin.edu.au/~pbourke/geometry/polyarea/
    vec2 centroid(const Polygon& P) {
        geo_assert(P.size() > 0) ;

        double A = signed_area(P) ;

        if(::fabs(A) < 1e-30) {
            return P[0] ;
        }

        double x = 0.0 ;
        double y = 0.0 ;
        for(unsigned int i=0; i<P.size(); i++) {
            unsigned int j = (i+1) % P.size() ;
            const vec2& t1 = P[i] ;
            const vec2& t2 = P[j] ;
            double d = (t1.x * t2.y - t2.x * t1.y) ;
            x += (t1.x + t2.x) * d ;
            y += (t1.y + t2.y) * d ;
        }
        
        return vec2(
            x / (6.0 * A),
            y / (6.0 * A)
        ) ;
    }
    
    static inline Sign point_is_in_half_plane(
        const vec2& p, const vec2& q1, const vec2& q2
    ) {
        return PCK::orient_2d(q1, q2, p);
    }

    static inline bool intersect_segments(
        const vec2& p1, const vec2& p2,
        const vec2& q1, const vec2& q2,
        vec2& result
    ) {

        vec2 Vp = p2 - p1;
        vec2 Vq = q2 - q1;
        vec2 pq = q1 - p1;
        
        double a =  Vp.x;
        double b = -Vq.x;
        double c =  Vp.y;
        double d = -Vq.y;
        
        double delta = a*d-b*c;
        if(delta == 0.0) {
            return false ;
        }
            
        double tp = (d * pq.x -b * pq.y) / delta;
        
        result = vec2(
            (1.0 - tp) * p1.x + tp * p2.x,
            (1.0 - tp) * p1.y + tp * p2.y
        );
        
        return true;
    }
    
    void clip_polygon_by_half_plane(
        const Polygon& P, 
        const vec2& q1,
        const vec2& q2,
        Polygon& result
    ) {
        result.clear() ;
        
        if(P.size() == 0) {
            return ;
        }

        if(P.size() == 1) {
            if(point_is_in_half_plane(P[0], q1, q2)) {
                result.push_back(P[0]) ;
            }
            return ;
        }

        vec2 prev_p = P[P.size() - 1] ;
        Sign prev_status = point_is_in_half_plane(
            prev_p, q1, q2
        );
        
        for(unsigned int i=0; i<P.size(); i++) {
            vec2 p = P[i] ;
            Sign status = point_is_in_half_plane(
                p, q1, q2
            );
            if(
                status != prev_status &&
                status != ZERO &&
                prev_status != ZERO
            ) {
                vec2 intersect ;
                if(intersect_segments(prev_p, p, q1, q2, intersect)) {
                    result.push_back(intersect) ;
                }
            }

            switch(status) {
            case NEGATIVE:
                break ;
            case ZERO:
                result.push_back(p) ;
                break ;
            case POSITIVE:
                result.push_back(p) ;
                break ;
            }
            
            prev_p = p ;
            prev_status = status ;
        }
    }
    
    void convex_clip_polygon(
        const Polygon& P, const Polygon& clip, Polygon& result
    ) {
        Polygon tmp1 = P ;
        Polygon tmp2 ;
        Polygon* src = &tmp1 ;
        Polygon* dst = &tmp2 ;
        for(unsigned int i=0; i<clip.size(); i++) {
            unsigned int j = ((i+1) % clip.size()) ;
            const vec2& p1 = clip[i] ;
            const vec2& p2 = clip[j] ;
            clip_polygon_by_half_plane(*src, p1, p2, *dst);
	    std::swap(src, dst) ;
        }
        result = *src ;
    }
}

/*********************************************************************/

int main(int argc, char** argv) {

    GEO::initialize();
    GEO::Logger::instance()->set_quiet(false);

    GEO::CmdLine::import_arg_group("standard");
    GEO::CmdLine::import_arg_group("algo");
    GEO::CmdLine::import_arg_group("gfx");    

    GEO::CmdLine::set_arg("sys:assert","abort");
    
    std::vector<std::string> filenames;
    if(!GEO::CmdLine::parse(argc, argv, filenames, "")) {
        return 1;
    }

    glup_viewer_set_region_of_interest(
        0.0f, 0.0f, 0.0f,
        1.0f, 1.0f, 1.0f
    );

    glup_viewer_set_window_title(
        "Geogram Delaunay2d test"
    );

    glup_viewer_set_screen_size(1024,800);
    
    glup_viewer_set_init_func(init);
    glup_viewer_set_display_func(display);
    glup_viewer_set_overlay_func(overlay);
    glup_viewer_set_mouse_func(mouse);
    glup_viewer_add_key_func(
        'k', Lloyd_relaxation, "One iteration of Lloyd relaxation"
    );
    glup_viewer_add_toggle(
        'a', glup_viewer_is_enabled_ptr(GLUP_VIEWER_IDLE_REDRAW), "Animation"
    );
    
    if(GEO::CmdLine::get_arg_bool("gfx:full_screen")) {
       glup_viewer_enable(GLUP_VIEWER_FULL_SCREEN);
    }

    glup_viewer_main_loop(argc, argv);

    return 0;
}
