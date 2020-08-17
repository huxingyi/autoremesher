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
#include <geogram_gfx/GLUP/GLUP_private.h>
#include <geogram/delaunay/periodic_delaunay_3d.h>


namespace {

    using namespace GEO;

    /**
     * \brief An application that demonstrates both
     *  GLUP primitives and glup_viewer application
     *  framework.
     */
    class DemoDelaunay3dApplication : public SimpleApplication {
    public:

        /**
         * \brief DemoDelaunay3dApplication constructor.
         */
        DemoDelaunay3dApplication() : SimpleApplication("Delaunay3d") {

	    periodic_ = false;
	    
            // Define the 3d region that we want to display
            // (xmin, ymin, zmin, xmax, ymax, zmax)
            set_region_of_interest(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);

	    draw_points_ = false;
	    draw_cells_ = true;
	    point_size_ = 10.0f;
	    cells_shrink_ = 0.1f;
	    nb_points_ = 100;
	    draw_box_ = false;
	    draw_period_ = false;
	    
	    start_animation();
        }

	void geogram_initialize(int argc, char** argv) override {
	    SimpleApplication::geogram_initialize(argc, argv);
	    delaunay_ = new PeriodicDelaunay3d(periodic_, 1.0);
	    // In non-periodic mode, we need to keep the infinite
	    // tetrahedra to be able to query the combinatorics of
	    // all cells (including the infinite ones).
	    if(!periodic_) {
		delaunay_->set_keeps_infinite(true);
	    }
	    init_random_points(nb_points_);
	}
	
        /**
         * \brief DemoDelaunay3dApplication destructor.
         */
	~DemoDelaunay3dApplication() override {
        }
        
        /**
         * \brief Displays and handles the GUI for object properties.
         * \details Overloads Application::draw_object_properties().
         */
	void draw_object_properties() override {
	    SimpleApplication::draw_object_properties();	    
	    ImGui::Checkbox("Animate", animate_ptr());
	    
	    if(ImGui::Button("One iteration", ImVec2(-1.0, 0.0))) {
		Lloyd_iteration();
	    }

	    if(
		ImGui::Button(
		    "+",
		    ImVec2(-ImGui::GetContentRegionAvail().x/2.0f,0.0f)
		) && nb_points_ < 10000
	    ) {
		++nb_points_;
		init_random_points(nb_points_);				
	    }
	    ImGui::SameLine();
	    if(
		ImGui::Button(
		    "-", ImVec2(-1.0f, 0.0f)
		) && nb_points_ > 4
	    ) {
		--nb_points_;
		init_random_points(nb_points_);				
	    }
	    
	    if(ImGui::SliderInt("N", &nb_points_, 4, 10000)) {
		init_random_points(nb_points_);		
	    }
	    
	    if(ImGui::Button("Reset", ImVec2(-1.0, 0.0))) {
		init_random_points(nb_points_);
	    }
	    
	    if(ImGui::Checkbox("Periodic", &periodic_)) {
		delaunay_ = new PeriodicDelaunay3d(periodic_, 1.0);
		// In non-periodic mode, we need to keep the infinite
		// tetrahedra to be able to query the combinatorics of
		// all cells (including the infinite ones).
		if(!periodic_) {
		    delaunay_->set_keeps_infinite(true);
		}
		delaunay_->set_vertices(points_.size()/3, points_.data());
		delaunay_->compute();
	    }

	    ImGui::Spacing();
	    ImGui::Separator();
	    ImGui::Spacing();
	    
	    ImGui::Checkbox("Box", &draw_box_);
	    if(ImGui::Checkbox("Period 3x3x3", &draw_period_)) {
		if(draw_period_) {
		    set_region_of_interest(
			-1.0, -1.0, -1.0, 2.0, 2.0, 2.0
		    );
		} else {
		    set_region_of_interest(
			0.0, 0.0, 0.0, 1.0, 1.0, 1.0
		    );
		}
	    }
	    ImGui::Checkbox("Points", &draw_points_);
	    ImGui::SameLine();
	    ImGui::SliderFloat("##PtSz.", &point_size_, 1.0f, 50.0f, "%.1f");
	    ImGui::Checkbox("Cells", &draw_cells_);
	    ImGui::SameLine();
	    ImGui::SliderFloat("##Shrk.", &cells_shrink_, 0.0f, 1.0f, "%.2f");
        }

	/**
	 * \brief Gets a Voronoi cell.
	 * \details In non-periodic mode, the cell is clipped by the domain.
	 * \param[in] v the index of the vertex
	 * \param[out] C the cell
	 */
	void get_cell(index_t v, ConvexCell& C) {
	    delaunay_->copy_Laguerre_cell_from_Delaunay(v, C, W_);
	    if(!periodic_) {
		C.clip_by_plane(vec4( 1.0, 0.0, 0.0, 0.0));
		C.clip_by_plane(vec4(-1.0, 0.0, 0.0, 1.0));
		C.clip_by_plane(vec4( 0.0, 1.0, 0.0, 0.0));
		C.clip_by_plane(vec4( 0.0,-1.0, 0.0, 1.0));	
		C.clip_by_plane(vec4( 0.0, 0.0, 1.0, 0.0));
		C.clip_by_plane(vec4( 0.0, 0.0,-1.0, 1.0));
	    }
	    C.compute_geometry();
	}

	/**
	 * \brief Draws a cell.
	 * \details Needs to be called between glupBegin(GLUP_TRIANGLES) 
	 *  and glupEnd().
	 */
	void draw_cell(ConvexCell& C, index_t instance = 0) {
	    double s = double(cells_shrink_);
	    double Tx = double(Periodic::translation[instance][0]);
	    double Ty = double(Periodic::translation[instance][1]);
	    double Tz = double(Periodic::translation[instance][2]);
	    
	    vec3 g;
	    if(cells_shrink_ != 0.0f) {
		g = C.barycenter();
	    }

	    // For each vertex of the Voronoi cell
	    // Start at 1 (vertex 0 is point at infinity)
	    for(index_t v=1; v<C.nb_v(); ++v) {
		index_t t = C.vertex_triangle(v);

		//  Happens if a clipping plane did not
		// clip anything.
		if(t == VBW::END_OF_LIST) {
		    continue;
		}

		//   Now iterate on the Voronoi vertices of the
		// Voronoi facet. In dual form this means iterating
		// on the triangles incident to vertex v
		
		vec3 P[3];
		index_t n=0;
		do {

		    //   Triangulate the Voronoi facet and send the
		    // triangles to OpenGL/GLUP.
		    if(n == 0) {
			P[0] = C.triangle_point(VBW::ushort(t));
		    } else if(n == 1) {
			P[1] = C.triangle_point(VBW::ushort(t));
		    } else {
			P[2] = C.triangle_point(VBW::ushort(t));
			if(s == 0.0) {
			    for(index_t i=0; i<3; ++i) {
				glupPrivateVertex3d(
				    P[i].x + Tx, P[i].y + Ty, P[i].z + Tz
				);
			    }
			} else {
			    for(index_t i=0; i<3; ++i) {  
				glupPrivateVertex3d(
				    s*g.x + (1.0-s)*P[i].x + Tx,
				    s*g.y + (1.0-s)*P[i].y + Ty,
				    s*g.z + (1.0-s)*P[i].z + Tz
				);
			    }
			}
			P[1] = P[2];
		    }

		    //   Move to the next triangle incident to
		    // vertex v.
		    index_t lv = C.triangle_find_vertex(t,v);		   
		    t = C.triangle_adjacent(t, (lv + 1)%3);
		    
		    ++n;
		} while(t != C.vertex_triangle(v));
	    }
	}
	
        /**
         * \brief Draws the scene according to currently set primitive and
         *  drawing modes.
         */
        void draw_scene() override {
	    // Avoid re-entry, for instance when a message is sent to
	    // the logger, this triggers a graphic redisplay.
	    static bool locked = false;
	    if(locked) {
		return;
	    }
	    locked = true;
	    if(animate()) {
		Lloyd_iteration();
	    }
            glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 0.5f, 0.5f, 0.5f);
	    glupEnable(GLUP_LIGHTING);

	    index_t max_instance = (draw_period_ ? 27 : 1);


	    
	    if(draw_points_) {
		double R = double(point_size_) / 250.0;

		for(index_t i=0; i<max_instance; ++i) {
		    glupTranslated(
			double(Periodic::translation[i][0]),
			double(Periodic::translation[i][1]),
			double(Periodic::translation[i][2])
	    	    );
		
		    glupBegin(GLUP_SPHERES);
		    for(index_t v=0; v<points_.size()/3; ++v) {
			glupPrivateVertex4d(
			    points_[3*v],
			    points_[3*v+1],
			    points_[3*v+2],
			    R
			);
		    }
		    glupEnd();

		    glupTranslated(
			-double(Periodic::translation[i][0]),
			-double(Periodic::translation[i][1]),
			-double(Periodic::translation[i][2])
	    	    );
		}
		
	    }

	    if(draw_cells_) {
		ConvexCell C;
		glupBegin(GLUP_TRIANGLES);
		for(index_t v=0; v<points_.size()/3; ++v) {
		    get_cell(v, C);
		    if(draw_period_) {
			for(index_t i=0; i<27; ++i) {
			    draw_cell(C,i);
			}
		    } else {
			draw_cell(C);
		    }
		}
		glupEnd();
	    }

	    if(draw_box_) {
		static GLfloat cube[8][3] = {
		    {0.0f, 0.0f, 0.0f},
		    {0.0f, 0.0f, 1.0f},
		    {0.0f, 1.0f, 0.0f},
		    {0.0f, 1.0f, 1.0f},
		    {1.0f, 0.0f, 0.0f},
		    {1.0f, 0.0f, 1.0f},
		    {1.0f, 1.0f, 0.0f},
		    {1.0f, 1.0f, 1.0f}
		};
		
		
		glupEnable(GLUP_DRAW_MESH);
		glupEnable(GLUP_ALPHA_DISCARD);
		glupSetColor4f(
		    GLUP_FRONT_AND_BACK_COLOR, 1.0f, 1.0f, 1.0f, 0.0f
		);		
		glupSetMeshWidth(10);
		glupDisable(GLUP_LIGHTING);

		for(index_t i=0; i<max_instance; ++i) {
		    glupTranslated(
			double(Periodic::translation[i][0]),
			double(Periodic::translation[i][1]),
			double(Periodic::translation[i][2])
	    	    );
		    
		    glupBegin(GLUP_QUADS);
		    
		    glupVertex3fv(cube[0]);
		    glupVertex3fv(cube[1]);
		    glupVertex3fv(cube[3]);
		    glupVertex3fv(cube[2]);
		    
		    glupVertex3fv(cube[4]);
		    glupVertex3fv(cube[5]);
		    glupVertex3fv(cube[7]);
		    glupVertex3fv(cube[6]);
		    
		    glupVertex3fv(cube[0]);
		    glupVertex3fv(cube[1]);
		    glupVertex3fv(cube[5]);
		    glupVertex3fv(cube[4]);
		    
		    glupVertex3fv(cube[1]);
		    glupVertex3fv(cube[3]);
		    glupVertex3fv(cube[7]);
		    glupVertex3fv(cube[5]);
		    
		    glupVertex3fv(cube[3]);
		    glupVertex3fv(cube[2]);
		    glupVertex3fv(cube[6]);
		    glupVertex3fv(cube[7]);
		    
		    glupVertex3fv(cube[2]);
		    glupVertex3fv(cube[0]);
		    glupVertex3fv(cube[4]);
		    glupVertex3fv(cube[6]);
		
		    glupEnd();

		    glupTranslated(
			-double(Periodic::translation[i][0]),
			-double(Periodic::translation[i][1]),
			-double(Periodic::translation[i][2])
	    	    );
		}


		
		glupDisable(GLUP_DRAW_MESH);
		glupDisable(GLUP_ALPHA_DISCARD);
	    }
	    
	    locked = false;
        }

	/**
	 * \brief Initializes the Delaunay triangulation with 
	 *  random points.
	 * \param[in] nb_points_in number of points
	 */
	void init_random_points(int nb_points_in) {
	    index_t nb_points = index_t(nb_points_in);
	    bool bkp = draw_cells_;
	    draw_cells_ = false;
	    points_.resize(3*nb_points);
	    for(index_t i=0; i<3*nb_points; ++i) {
		points_[i] = Numeric::random_float64();
	    }
	    delaunay_->set_vertices(nb_points, points_.data());
	    delaunay_->compute();
	    draw_cells_ = bkp;
	}

	/**
	 * \brief Does one iteration of Lloyd relaxation.
	 * \details Moves each point to the centroid of its
	 *  Voronoi cell. In non-periodic mode, the Voronoi
	 *  cell is clipped by the domain.
	 */
	void Lloyd_iteration() {
	    vector<double> new_points(points_.size());
	    ConvexCell C;
	    for(index_t v=0; v<points_.size()/3; ++v) {
		get_cell(v, C);
		vec3 g = C.barycenter();
		new_points[3*v]   = g.x;
		new_points[3*v+1] = g.y;
		new_points[3*v+2] = g.z;		
	    }
	    // In periodic mode, points may escape out of
	    // the domain. Relocate them in [0,1]^3
	    for(index_t i=0; i<new_points.size(); ++i) {
		if(new_points[i] < 0.0) {
		    new_points[i] += 1.0;
		}
		if(new_points[i] > 1.0) {
		    new_points[i] -= 1.0;
		}
	    }
	    points_.swap(new_points);
	    delaunay_->set_vertices(points_.size()/3, points_.data());
	    delaunay_->compute();
	}
	
    private:
	SmartPointer<PeriodicDelaunay3d> delaunay_;
	bool periodic_;
	bool draw_points_;
        float point_size_;
	bool draw_cells_;
	bool draw_box_;
	bool draw_period_;
	float cells_shrink_;
	vector<double> points_;
	int nb_points_;
	PeriodicDelaunay3d::IncidentTetrahedra W_;
    };
      
}

int main(int argc, char** argv) {
    DemoDelaunay3dApplication app;
    app.start(argc, argv);
    return 0;
}
