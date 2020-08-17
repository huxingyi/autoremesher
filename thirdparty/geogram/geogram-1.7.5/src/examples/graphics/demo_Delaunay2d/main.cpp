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

#include <geogram_gfx/gui/simple_application.h>
#include <geogram/delaunay/delaunay.h>
#include <geogram/numerics/predicates.h>

namespace {
    using namespace GEO;

    typedef vector<vec2> Polygon;

    /*******************************************************************/
    
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

    /*******************************************************************/

#define c1 0.35 
#define c2 0.5 
#define c3 1.0 

    double color_table[12][3] = {
	{c3, c2, c2},
	{c2, c3, c2},
	{c2, c2, c3},
	{c2, c3, c3},
	{c3, c2, c3},
	{c3, c3, c2},
	
	{c1, c2, c2},
	{c2, c1, c2},
	{c2, c2, c1},
	{c2, c1, c1},
	{c1, c2, c1},
	{c1, c1, c2}
    };

    int random_color_index_ = 0 ;


    float white[4] = {
	0.8f, 0.8f, 0.3f, 1.0f
    };
    
    void glup_random_color() {
	glupColor3d(
	    color_table[random_color_index_][0], 
	    color_table[random_color_index_][1], 
	    color_table[random_color_index_][2]
	    );
	random_color_index_ = (random_color_index_ + 1) % 12 ;
    }
    
    void glup_randomize_colors(int index) {
	random_color_index_ = (index % 12) ;
    }

    void glup_random_color_from_index(int index) {
	if(index >= 0) {
	    glup_randomize_colors(index) ;
	    glup_random_color() ;
	} else {
	    glupColor4fv(white) ;
	}
    }
    
    /*******************************************************************/
    
    class Delaunay2dApplication : public SimpleApplication {
    public:
	Delaunay2dApplication() : SimpleApplication("Delaunay2d") {
	    console_visible_ = false;
	    viewer_properties_visible_ = false;
	    delaunay_ = Delaunay::create(2,"BDEL2d");
	    border_shape_ = index_t(-1);
	    set_border_shape(0);
	    create_random_points(3);
	    point_size_ = 20;
	    edit_ = true;
	    show_Voronoi_cells_ = true;
	    show_Delaunay_triangles_ = true;
	    show_Voronoi_edges_ = true;
	    show_points_ = true;
	    show_border_ = true;
	    picked_point_ = index_t(-1);
	    last_button_ = index_t(-1);
	    add_key_toggle("F10", &edit_);
	    set_2d();
	    start_animation();
	}

    protected:
	/**
	 * \brief Updates the Delaunay triangulation with the current
	 *  vector of points.
	 */
	void update_Delaunay() {
	    delaunay_->set_vertices(points_.size(), &points_.data()->x);
	}

	/**
	 * \brief Creates random points.
	 * \details Points are created uniformly in the [0,1]x[0,1] 
	 *  square
	 * \param[in] nb the number of points to create.
	 */
	void create_random_points(index_t nb) {
	    for(index_t i=0; i<nb; ++i) {
		points_.push_back(
		    vec2(0.25, 0.25) + 
		    vec2(
			Numeric::random_float64()*0.5,
			Numeric::random_float64()*0.5
		    )
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
	    if(points_.size() == 0) {
		return index_t(-1);
	    }
	    double dist_so_far = Numeric::max_float64();
	    index_t nearest = index_t(-1);
	    for(index_t i=0; i<points_.size(); ++i) {
		double dist = distance2(p, points_[i]);
		if(dist < dist_so_far) {
		    nearest = i;
		    dist_so_far = dist;
		}
	    }
	    if(!get_nearest) {
		vec3 q(points_[nearest].x, points_[nearest].y, 0.0);
		vec3 p_scr = project(vec3(p.x, p.y, 0.0));
		vec3 q_scr = project(vec3(q.x, q.y, 0.0));
		if(distance2(p_scr,q_scr) > 2.0 * double(glupGetPointSize())) {
		    nearest = index_t(-1);
		}
	    }
	    return nearest;
	}
	
	void set_border_as_polygon(index_t nb_sides) {
	    border_.clear();
	    for(index_t i=0; i<nb_sides; ++i) {
		double alpha = double(i) * 2.0 * M_PI / double(nb_sides);
		double s = sin(alpha);
		double c = cos(alpha);
		border_.push_back( vec2(0.5*(c + 1.0), 0.5*(s + 1.0)) );
	    }
	}

	void set_border_shape(index_t shape) {
	    if(border_shape_ == shape) {
		return;
	    }
	    border_shape_ = shape;
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
		    border_.clear();            
		    border_.push_back(vec2(0.0, 0.0));
		    border_.push_back(vec2(1.0, 0.0));
		    border_.push_back(vec2(1.0, 1.0));
		    border_.push_back(vec2(0.0, 1.0));        
		    break;
	    }
	}

	
	/**
	 * \brief Draws the border of the domain.
	 */
	void draw_border() {
	    glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.0, 0.0, 0.0);
	    glupSetMeshWidth(4);
	    glupBegin(GLUP_LINES);
	    for(index_t i=0; i<border_.size(); ++i) {
		glupVertex(border_[i]);
		glupVertex(border_[(i+1)%border_.size()]);
	    }
	    glupEnd();
	}

	/**
	 * \brief Draws the points.
	 */
	void draw_points() {
	    glupEnable(GLUP_LIGHTING);
	    glupSetPointSize(GLfloat(point_size_));
	    glupDisable(GLUP_VERTEX_COLORS);
	    glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.0f, 1.0f, 1.0f);
	    glupBegin(GLUP_POINTS);
	    for(index_t i=0; i<points_.size(); ++i) {
		glupVertex(points_[i]);
	    }
	    glupEnd();
	    glupDisable(GLUP_LIGHTING);       
	}
	
	/**
	 * \brief Draws the Delaunay triangles.
	 */
	void draw_Delaunay_triangles() {
	    glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.7f, 0.7f, 0.7f);
	    glupSetMeshWidth(1);
	    glupBegin(GLUP_LINES);
	    for(index_t c=0; c<delaunay_->nb_cells(); ++c) {
		const signed_index_t* cell = delaunay_->cell_to_v() + 3*c;
		for(index_t e=0; e<3; ++e) {
		    signed_index_t v1 = cell[e];
		    signed_index_t v2 = cell[(e+1)%3];
		    glupVertex2dv(delaunay_->vertex_ptr(index_t(v1)));
		    glupVertex2dv(delaunay_->vertex_ptr(index_t(v2)));
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
	    signed_index_t v1 = delaunay_->cell_to_v()[3*t];
	    signed_index_t v2 = delaunay_->cell_to_v()[3*t+1];
	    signed_index_t v3 = delaunay_->cell_to_v()[3*t+2];
	    vec2 p1(delaunay_->vertex_ptr(index_t(v1)));
	    vec2 p2(delaunay_->vertex_ptr(index_t(v2)));
	    vec2 p3(delaunay_->vertex_ptr(index_t(v3)));
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
	    index_t v1 = index_t(delaunay_->cell_to_v()[3*t+lv1]);
	    index_t v2 = index_t(delaunay_->cell_to_v()[3*t+lv2]);
	    vec2 p1(delaunay_->vertex_ptr(v1));
	    vec2 p2(delaunay_->vertex_ptr(v2));
	    vec2 n = normalize(p2-p1);
	    n = vec2(n.y, -n.x);
	    return 0.5*(p1+p2)+100000.0*n;
	}

	/**
	 * \brief Draw the Voronoi edges.
	 */
	void draw_Voronoi_edges() {
	    glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.3f, 0.3f, 0.3f);
	    glupSetMeshWidth(2);
	    glupBegin(GLUP_LINES);
	    for(index_t t=0; t<delaunay_->nb_cells(); ++t) {
		vec2 cc = circumcenter(t);
		for(index_t e=0; e<3; ++e) {
		    signed_index_t t2 = delaunay_->cell_to_cell()[3*t+e];
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
		if(index_t(delaunay_->cell_to_v()[3*t+lv]) == v) {
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
	    index_t v = index_t(delaunay_->cell_to_v()[3*t0+lv]);
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
		signed_index_t neigh_t = delaunay_->cell_to_cell()[3*t+e];
		if(neigh_t == -1) {
		    on_border = true;
		    break;
		}
		cell.push_back(circumcenter(t));
		t = index_t(neigh_t);
		lv = find_vertex(t,v);
	    } while(t != t0);
	    
        
	    // If one traversed edge is on the border of the convex hull, then
	    // we empty the cell, and start turing around the vertex in
	    // the other direction, i.e. by traversing this time
	    // edge (lv+2)%3 until we reach the other edge on the border of
	    // the convex hull that is incident to v.
	    if(on_border) {
		cell.resize(0);
		cell.push_back(infinite_vertex(t,(lv + 1)%3));
		for(;;) {
		    cell.push_back(circumcenter(t));                
		    index_t e = (lv+2)%3;
		    signed_index_t neigh_t = delaunay_->cell_to_cell()[3*t+e];
		    if(neigh_t == -1) {
			cell.push_back(infinite_vertex(t, e));
			break;
		    }
		    t = index_t(neigh_t);
		    lv = find_vertex(t,v);
		}
	    }
	    
	    Polygon clipped;
	    convex_clip_polygon(cell, border_, clipped);
	    cell.swap(clipped);
	}

	/**
	 * \brief Draws the Voronoi cells as filled polygons with
	 *  random colors.
	 */
	void draw_Voronoi_cells() {
	    v_visited_.assign(points_.size(), false);
	    Polygon cell;
	    glupEnable(GLUP_VERTEX_COLORS);
	    glupBegin(GLUP_TRIANGLES);        
	    for(index_t t=0; t<delaunay_->nb_cells(); ++t) {
		for(index_t lv=0; lv<3; ++lv) {
		    index_t v = index_t(delaunay_->cell_to_v()[3*t+lv]);
		    if(!v_visited_[v]) {
			glup_random_color_from_index(int(v));
			v_visited_[v] = true;
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
	
	/**
	 * \brief Draws all the elements of the Delaunay triangulation / 
	 *  Voronoi diagram.
	 * \details Specified as glup_viewer_set_display_func() callback.
	 */
	void draw_scene() override {
	    glupDisable(GLUP_LIGHTING);
	    if(show_Voronoi_cells_) {
		draw_Voronoi_cells();
	    }
	    if(show_Delaunay_triangles_) {
		draw_Delaunay_triangles();
	    }
	    if(show_Voronoi_edges_) {
		draw_Voronoi_edges();
	    }
	    if(show_border_) {
		draw_border();
	    }
	    glupEnable(GLUP_LIGHTING);
	    if(show_points_) {
		draw_points();
	    }
	    if(animate()) {
		Lloyd_relaxation();
	    }
	}
	
	void Lloyd_relaxation() {
	    v_visited_.assign(delaunay_->nb_vertices(),false);
	    new_points_.resize(points_.size());
	    Polygon cell;
	    for(index_t t=0; t<delaunay_->nb_cells(); ++t) {
		for(index_t lv=0; lv<3; ++lv) {
		    index_t v = index_t(delaunay_->cell_to_v()[3*t+lv]);
		    if(!v_visited_[v]) {
			v_visited_[v] = true;
			get_Voronoi_cell(t,lv,cell);
			if(cell.size() > 0) {
			    new_points_[v] = centroid(cell);
			} else {
			    new_points_[v] = points_[v];
			}
		    }
		}
	    }
	    std::swap(points_, new_points_);
	    update_Delaunay();
	}

	void draw_object_properties() override {
	    SimpleApplication::draw_object_properties();	    
	    if(ImGui::Button(
		   (icon_UTF8("home") + " Home [H]").c_str(), ImVec2(-1.0, 0.0))
	    ) {
		home();
	    }
	    ImGui::Checkbox("Edit", &edit_);

	    ImGui::Separator();        
	    if(ImGui::Button("Relax.")) {
		Lloyd_relaxation();
	    }
	    ImGui::SameLine();
	    ImGui::Checkbox("animate",animate_ptr());
	    if(ImGui::Button("reset points", ImVec2(-1.0f,0.0f))) {
		points_.clear();
		create_random_points(3);
		update_Delaunay();
	    }
	    ImGui::Text("Bndry");
	    ImGui::SameLine();
	    int new_border_shape = int(border_shape_);
	    ImGui::Combo(
		"", &new_border_shape,
		"square\0triangle\0pentagon\0circle\0\0"
	    );
	    set_border_shape(index_t(new_border_shape));

	    ImGui::Separator();
	    ImGui::Checkbox("cells", &show_Voronoi_cells_);
	    ImGui::SameLine();
	    ImGui::Checkbox("edges", &show_Voronoi_edges_);
	    
	    ImGui::Checkbox("trgls", &show_Delaunay_triangles_);
	    ImGui::SameLine();	    
	    ImGui::Checkbox("points", &show_points_);
	}

	void mouse_button_callback(
	    int button, int action, int mods, int source
	) override {
	    geo_argused(source);

	    // Hide object and viewer properties if in phone
	    // mode and user clicked elsewhere.
	    if(phone_screen_ &&
	       !ImGui::GetIO().WantCaptureMouse &&
	       get_height() >= get_width()
	    ) {
		if(!props_pinned_) {
		    object_properties_visible_ = false;
		    viewer_properties_visible_ = false;
		}
	    }

	    if(!edit_) {
		SimpleApplication::mouse_button_callback(
		    button, action, mods, source
		);
		return;
	    }
	    
	    if(action != EVENT_ACTION_DOWN) {
		last_button_ = index_t(-1);
		return;
	    }

	    last_button_ = index_t(button);

	    if(animate()) {
		switch(button) {
		    case 0: {
			points_.push_back(mouse_point_);
			picked_point_ = points_.size() - 1;                    
		    } break;
		    case 1: {
			picked_point_ = get_picked_point(mouse_point_,true); 
			if(points_.size() > 3) {
			    points_.erase(points_.begin() + int(picked_point_));
			}
		    } break;
		}
	    } else {
		picked_point_ = get_picked_point(mouse_point_);
		switch(button) {
		    case 0: {
			if(picked_point_ == index_t(-1)) {
			    points_.push_back(mouse_point_);
			    picked_point_ = points_.size() - 1;
			}
		    } break;
		    case 1: {
			if(points_.size() > 3) {
			    if(picked_point_ != index_t(-1)) {
				points_.erase(
				    points_.begin() + int(picked_point_)
				);
			    }
			}
			picked_point_ = index_t(-1);
		    } break;
		}
	    }
	    update_Delaunay();
	}

	void cursor_pos_callback(double x, double y, int source) override {
	    geo_argused(source);
	    if(!edit_) {
		SimpleApplication::cursor_pos_callback(x,y,source);
		return;
	    }

	    // For retina displays: x and y are in 'window pixels',
	    // and GLUP project / unproject expect 'framebuffer pixels'.
	    double sx =
		double(get_frame_buffer_width()) / double(get_width());
	    
	    double sy =
		double(get_frame_buffer_height()) / double(get_height());
	    
	    mouse_point_ = unproject_2d(
		vec2(sx*x, sy*(double(get_height()) - y))
	    );
	    if(animate()) {
		if(last_button_ == 0) {
		    points_.push_back(mouse_point_);
		    picked_point_ = points_.size() - 1;
		    update_Delaunay();
		} else if(last_button_ == 1) {
		    picked_point_ = get_picked_point(mouse_point_,true); 
		    if(points_.size() > 3) {
			points_.erase(points_.begin() + int(picked_point_));
		    }
		    update_Delaunay();		    
		}
	    } else {
		if(picked_point_ != index_t(-1) && (last_button_ == 0)) {
		    points_[picked_point_] = mouse_point_;
		    update_Delaunay();
		}
	    }
	}

    private:
	vector<vec2> points_;
	vector<vec2> new_points_;
	vector<bool> v_visited_;
	Delaunay_var delaunay_;
	Polygon border_;
	index_t border_shape_;
	GLint point_size_; /**< The size of all displayed points. */

	bool edit_;
	bool show_Voronoi_cells_;
	bool show_Delaunay_triangles_;
	bool show_Voronoi_edges_;
	bool show_points_;
	bool show_border_;

	index_t picked_point_;
	index_t last_button_;
	vec2 mouse_point_;
    };
    
}

int main(int argc, char** argv) {
    Delaunay2dApplication app;
    app.start(argc, argv);
    return 0;
}
