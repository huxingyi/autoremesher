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
#include "uv.xpm"

namespace {

    using namespace GEO;

    /**
     * \brief An application that demonstrates both
     *  GLUP primitives and glup_viewer application
     *  framework.
     */
    class DemoGlupApplication : public SimpleApplication {
    public:

        /**
         * \brief DemoGlupApplication constructor.
         */
        DemoGlupApplication(): SimpleApplication("Demo GLUP") {
            mesh_ = true;
            colors_ = true;
            texturing_ = false;
            picking_ = false;
            n_ = 30;
            point_size_ = 10.0f;
            shrink_ = 0.0f;

            // Key shortcuts.
	    add_key_toggle("c", &colors_);
            add_key_toggle("m", &mesh_);            
            add_key_toggle("t", &texturing_);
            add_key_toggle("o", &picking_);
            add_key_func(
                " ", [this](void) { cycle_primitives(); }  
            );

            // Define the 3d region that we want to display
            // (xmin, ymin, zmin, xmax, ymax, zmax)
	    set_region_of_interest(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
	    
            texture_ = 0;
            primitive_ = GLUP_POINTS;

	    smooth_ = true;
        }

        /**
         * \copydoc SimpleApplication::GL_terminate()
         */
        void GL_terminate() override {
            if(texture_ != 0) {
                glDeleteTextures(1,&texture_);
            }
	    SimpleApplication::GL_terminate();
        }
        
        /**
         * \brief Displays and handles the GUI for object properties.
         * \details Overloads Application::draw_object_properties().
         */
        void draw_object_properties() override {
	    SimpleApplication::draw_object_properties();	    
            ImGui::Combo("prim.", &primitive_,
         "points\0lines\0triangles\0quads\0tets\0hexes\0prisms\0pyramids\0\0"
            );
            ImGui::SliderInt("n", (int*)&n_, 4, 300);
            ImGui::Text(
                "%s",
                ("nb prim.: " +  String::to_string(nb_primitives())).c_str()
            );
            ImGui::Separator();
            if(primitive_ != GLUP_POINTS && primitive_ != GLUP_LINES) {
                ImGui::Checkbox("mesh [m]", &mesh_);
            }
            ImGui::Checkbox("colors [c]", &colors_);
            ImGui::Checkbox("texturing [t]", &texturing_);
            ImGui::Checkbox("picking [o]", &picking_);
	    if(primitive_ == GLUP_TRIANGLES || primitive_ == GLUP_QUADS) {
		ImGui::Checkbox("smooth", &smooth_);
	    }
            ImGui::Separator();
            if(primitive_ == GLUP_POINTS) {
                ImGui::SliderFloat("PtSz.", &point_size_, 1.0f, 50.0f, "%.1f");
            }
            if(primitive_ != GLUP_POINTS && primitive_ != GLUP_LINES) {
                ImGui::SliderFloat("shrk.", &shrink_, 0.0f, 1.0f, "%.2f");
            }
        }


        /**
         * \brief Gets the number of displayed primitives.
         * \details The number of displayed primitives is computed from
         *  the primitive type and the resolution n_
         * \see draw_scene()
         */
        index_t nb_primitives() {
            switch(primitive_) {
            case GLUP_POINTS:
                return n_*n_*n_;
            case GLUP_LINES:
                return 3*(n_-1)*(n_-1)*(n_-1);
            case GLUP_TRIANGLES:
                return 2*(n_-1)*(n_-1);
            case GLUP_QUADS:
                return (n_-1)*(n_-1);                
            case GLUP_TETRAHEDRA:
            case GLUP_HEXAHEDRA:
            case GLUP_PRISMS:
            case GLUP_PYRAMIDS:
            case GLUP_CONNECTORS:
                return 3*(n_-1)*(n_-1)*(n_-1);
            }
            return 0;
        }
        
        /**
         * \brief Cycles the drawn primitives.
         * \details Triggered when the user pushes the space bar, routes to
         *  DemoGlupApplication::cycles_primitives().
         */
	void cycle_primitives() {
            ++primitive_;
            if(primitive_ > GLUP_PYRAMIDS) {
                primitive_ = 0;
            }
        }

        /**
         * \brief Draws the scene according to currently set primitive and
         *  drawing modes.
         */
        void draw_scene() override {
	    
            // GLUP can have different colors for frontfacing and
            // backfacing polygons.
            glupSetColor3f(GLUP_FRONT_COLOR, 1.0f, 1.0f, 0.0f);
            glupSetColor3f(GLUP_BACK_COLOR, 1.0f, 0.0f, 1.0f);

            // Take into account the toggles from the Object pane:
            
            // Enable/disable individual per-vertex colors.
            if(colors_) {
                glupEnable(GLUP_VERTEX_COLORS);
            } else {
                glupDisable(GLUP_VERTEX_COLORS);                
            }

            // There is a global light switch. Note: facet normals are
            // automatically computed by GLUP, no need to specify
            // them ! (but you cannot have per-vertex normals).
            if(lighting_) {
                glupEnable(GLUP_LIGHTING);
            } else {
                glupDisable(GLUP_LIGHTING);
            }

            // Each facet can have a black outline displayed.
            if(mesh_) {
                glupEnable(GLUP_DRAW_MESH);
            } else {
                glupDisable(GLUP_DRAW_MESH);                
            }

            // If picking mode is active, colors are replaced with encoded
            // primitive IDs. Then one can find the ID of the primitive under
            // the mouse pointer using glReadPixels() and decoding the ID.
            if(picking_) {
                glupEnable(GLUP_PICKING);
            } else {
                glupDisable(GLUP_PICKING);                
            }

            // Texture mapping.
            if(texturing_) {
                glupEnable(GLUP_TEXTURING);
                glActiveTexture(GL_TEXTURE0 + GLUP_TEXTURE_2D_UNIT);
                glBindTexture(GL_TEXTURE_2D, texture_);
                glupTextureType(GLUP_TEXTURE_2D);
                glupTextureMode(GLUP_TEXTURE_REPLACE);
            } else {
                glupDisable(GLUP_TEXTURING);
            }

            // Polygons and polyhedra can be shrunk, this is useful for meshing
            // applications, sometimes makes it easier to see something.
            glupSetCellsShrink(shrink_);

	    if(primitive_ == GLUP_TRIANGLES || primitive_ == GLUP_QUADS) {
		if(smooth_) {
		    glupEnable(GLUP_VERTEX_NORMALS);
		} else {
		    glupDisable(GLUP_VERTEX_NORMALS);		    
		}
	    }
	    
            switch(primitive_) {
                
            case GLUP_POINTS: {
                glupSetPointSize(point_size_);
                glupBegin(GLUP_POINTS);
                for(index_t i=0; i<n_; ++i) {
                    for(index_t j=0; j<n_; ++j) {
                        for(index_t k=0; k<n_; ++k) {
                            draw_vertex_grid(i,j,k);
                        }
                    }
                }
                glupEnd();
            } break;
                
            case GLUP_LINES: {
                glLineWidth(1);
                glupBegin(GLUP_LINES);
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        for(index_t k=0; k<n_-1; ++k) {
                            draw_vertex_grid(i,j,k);
                            draw_vertex_grid(i+1,j,k);

                            draw_vertex_grid(i,j,k);
                            draw_vertex_grid(i,j+1,k);                    
                            
                            draw_vertex_grid(i,j,k);
                            draw_vertex_grid(i,j,k+1);                    
                        }
                    }
                }
                glupEnd();
            } break;
                
            case GLUP_TRIANGLES: {
                glupBegin(GLUP_TRIANGLES);            
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        draw_vertex_sphere(i,j);
                        draw_vertex_sphere(i+1,j+1);                
                        draw_vertex_sphere(i,j+1);
                        
                        draw_vertex_sphere(i+1,j+1);                
                        draw_vertex_sphere(i,j);                
                        draw_vertex_sphere(i+1,j);                
                    }
                }
                glupEnd();            
            } break;
                
            case GLUP_QUADS: {
                glupBegin(GLUP_QUADS);
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        draw_vertex_sphere(i,j);
                        draw_vertex_sphere(i+1,j);
                        draw_vertex_sphere(i+1,j+1);                
                        draw_vertex_sphere(i,j+1);                    
                    }
                }
                glupEnd();            
            } break;
                
            case GLUP_TETRAHEDRA: {
                glupBegin(GLUP_TETRAHEDRA);
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        for(index_t k=0; k<n_-1; ++k) {
                            draw_vertex_grid(i,j,k);
                            draw_vertex_grid(i,j,k+1);
                            draw_vertex_grid(i,j+1,k);              
                            draw_vertex_grid(i+1,j,k);
                        }
                    }
                }
                glupEnd();            
            } break;
                
            case GLUP_HEXAHEDRA: {
                glupBegin(GLUP_HEXAHEDRA);
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        for(index_t k=0; k<n_-1; ++k) {
                            draw_vertex_grid(i,  j,    k);
                            draw_vertex_grid(i  ,j+1,  k);
                            draw_vertex_grid(i+1,j,    k);
                            draw_vertex_grid(i+1,j+1,  k);
                            draw_vertex_grid(i,  j,    k+1);
                            draw_vertex_grid(i  ,j+1,  k+1);
                            draw_vertex_grid(i+1,j,    k+1);
                            draw_vertex_grid(i+1,j+1,  k+1);
                        }
                    }
                }
                glupEnd();            
            } break;
                
            case GLUP_PRISMS: {
                glupBegin(GLUP_PRISMS);
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        for(index_t k=0; k<n_-1; ++k) {
                            draw_vertex_grid(i,  j,    k);
                            draw_vertex_grid(i  ,j+1,  k);
                            draw_vertex_grid(i+1,j  ,  k);
                            draw_vertex_grid(i,  j,    k+1);
                            draw_vertex_grid(i  ,j+1,  k+1);
                            draw_vertex_grid(i+1,j  ,  k+1);              
                        }
                    }
                }
                glupEnd();            
            } break;
                
            case GLUP_PYRAMIDS: {
                glupBegin(GLUP_PYRAMIDS);
                for(index_t i=0; i<n_-1; ++i) {
                    for(index_t j=0; j<n_-1; ++j) {
                        for(index_t k=0; k<n_-1; ++k) {
                            draw_vertex_grid(i,  j,    k);
                            draw_vertex_grid(i  ,j+1,  k);
                            draw_vertex_grid(i+1,j+1,  k);
                            draw_vertex_grid(i+1,j  ,  k);
                            draw_vertex_grid(i,  j,    k+1);
                        }
                    }
                }
                glupEnd();
                
            } break;
            default:
                break;
            }

	    glupDisable(GLUP_VERTEX_NORMALS);		    	    
        }

        /**
         * \brief Creates the texture.
         * \details This function overloads Application::init_graphics(). It
         *  is called as soon as the OpenGL context is ready for rendering. It
         *  is meant to initialize the graphic objects used by the application.
         */
	void GL_initialize() override {
            SimpleApplication::GL_initialize();
	    
            // Create the texture and initialize its texturing modes
            glGenTextures(1, &texture_);
            glActiveTexture(GL_TEXTURE0 + GLUP_TEXTURE_2D_UNIT);
            glBindTexture(GL_TEXTURE_2D, texture_);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
            glTexImage2Dxpm(uv);
            glupTextureType(GLUP_TEXTURE_2D);
            glupTextureMode(GLUP_TEXTURE_REPLACE);
        }
        
    protected:

        /**
         * \brief Draws a vertex on a grid, with colors and texture
         *  coordinates.
         * \param[in] i , j , k the grid coordinates of the vertex
         */
        inline void draw_vertex_grid(
            GEO::index_t i, GEO::index_t j, GEO::index_t k
        ) {
            glupColor3f(
                float(i)/float(n_ - 1),
                float(j)/float(n_ - 1),
                float(k)/float(n_ - 1)                        
            );
            glupTexCoord3f(
                float(i)/float(n_ - 1),
                float(j)/float(n_ - 1),
                float(k)/float(n_ - 1)                        
            );
            glupVertex3f(
                float(i)/float(n_ - 1),
                float(j)/float(n_ - 1),
                float(k)/float(n_ - 1)                        
            );
        }

        inline void draw_vertex_sphere(GEO::index_t i, GEO::index_t j) {
            double theta = double(i) * 2.0 * M_PI / double(n_-1);
            double phi = -M_PI/2.0 + double(j) * M_PI / double(n_-1);
            
            double x = (cos(theta)*cos(phi) + 1.0)/2.0;
            double y = (sin(theta)*cos(phi) + 1.0)/2.0;
            double z = (sin(phi) + 1.0) / 2.0;
            
            glupColor3d(x,y,z);
            glupTexCoord3d(x,y,z);
	    glupNormal3d(x-0.5,y-0.5,z-0.5);
            glupVertex3d(x,y,z);
        }

        
    private:
        int primitive_;
        bool mesh_;
        bool colors_;
        bool texturing_;
        bool picking_;
        float point_size_;
        float shrink_;
        index_t n_;
        GLuint texture_;
	bool smooth_;
    };
      
}

int main(int argc, char** argv) {
    DemoGlupApplication app;
    app.start(argc, argv);
    return 0;
}
