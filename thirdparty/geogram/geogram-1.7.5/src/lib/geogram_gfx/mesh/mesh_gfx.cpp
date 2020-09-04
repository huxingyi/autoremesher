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

#include <geogram_gfx/mesh/mesh_gfx.h>
#include <geogram_gfx/GLUP/GLUP_private.h>
#include <geogram_gfx/basic/GLSL.h>

#include <geogram/basic/logger.h>
#include <geogram/basic/command_line.h>


// TODO: implement attribute display for cell facets.
// TODO: use vertex arrays for attribute display for
//   vertex attributes whenever possible.

namespace {
    using namespace GEO;
}

namespace GEO {

    MeshGfx::MeshGfx() {
        show_mesh_ = true;
        mesh_width_ = 1;
        mesh_border_width_ = 2;
        shrink_ = 0.0;
        animate_ = false;
        time_ = 0.0;
        draw_cells_[MESH_TET] = true;
        draw_cells_[MESH_HEX] = true;
        draw_cells_[MESH_PRISM] = true;
        draw_cells_[MESH_PYRAMID] = true;
        draw_cells_[MESH_CONNECTOR] = true;
        points_size_ = 1.0f;
        set_points_color(0.0f, 1.0f, 0.0f);
        set_mesh_color(0.0f, 0.0f, 0.0f);
        set_surface_color(0.0f, 0.5f, 1.0f);
        set_backface_surface_color(1.0f, 0.0f, 0.0f);
        set_cells_color(0.9f, 0.9f, 0.9f);
        cells_colors_by_type_ = false;
        lighting_ = true;
        picking_mode_ = MESH_NONE;
        object_picking_id_ = index_t(-1);
        mesh_ = nullptr;
        triangles_and_quads_ = true;
        quads_ = true;

        buffer_objects_dirty_ = false;
        attributes_buffer_objects_dirty_ = false;
	long_vector_attribute_ = false;
        
        vertices_VAO_ = 0;
        edges_VAO_ = 0;
        facets_VAO_ = 0;
        cells_VAO_ = 0;
        
        vertices_VBO_  = 0;
        edge_indices_VBO_ = 0;
        facet_indices_VBO_ = 0;
        cell_indices_VBO_ = 0;
        vertices_attribute_VBO_ = 0;
        
        do_animation_ = false;

        attribute_subelements_ = MESH_NONE;
        attribute_min_ = 0.0;
        attribute_max_ = 0.0;
        attribute_texture_ = 0;
        attribute_repeat_ = 1;
	attribute_dim_ = 1;
	
        ES_profile_ = false;
    }

    MeshGfx::~MeshGfx() {
        if(vertices_VAO_ != 0) {
            glupDeleteVertexArrays(1,&vertices_VAO_);
            vertices_VAO_ = 0;
        }

        if(edges_VAO_ != 0) {
            glupDeleteVertexArrays(1,&edges_VAO_);
            edges_VAO_ = 0;
        }

        if(facets_VAO_ != 0) {
            glupDeleteVertexArrays(1,&facets_VAO_);
            facets_VAO_ = 0;
        }

        if(cells_VAO_ != 0) {
            glupDeleteVertexArrays(1,&cells_VAO_);
            cells_VAO_ = 0;
        }
        
        if(vertices_VBO_ != 0) {
            glDeleteBuffers(1,&vertices_VBO_);
            vertices_VBO_ = 0;
        }

        if(edge_indices_VBO_ != 0) {
            glDeleteBuffers(1,&edge_indices_VBO_);
            edge_indices_VBO_ = 0;
        }
        
        if(facet_indices_VBO_ != 0) {
            glDeleteBuffers(1,&facet_indices_VBO_);
            facet_indices_VBO_ = 0;
        }

        if(cell_indices_VBO_ != 0) {
            glDeleteBuffers(1,&cell_indices_VBO_);
            cell_indices_VBO_ = 0;
        }

        if(vertices_attribute_VBO_ != 0) {
            glDeleteBuffers(1,&vertices_attribute_VBO_);
            vertices_attribute_VBO_ = 0;
        }
    }

    bool MeshGfx::can_use_array_mode(GLUPprimitive prim) const {
        if(do_animation_) {
            return false;
        }

        // Special case: GLUPES2 can use array mode for triangles, but
        // not with mesh and not with facet shrink.
        if(
            prim == GLUP_TRIANGLES &&
            ES_profile_ &&
            (show_mesh_ || shrink_ != 0.0)
        ) {
            return false;
        }
        
        if(!glupPrimitiveSupportsArrayMode(prim)) {
            return false;
        }
        if(
            attribute_subelements_ != MESH_NONE &&
            attribute_subelements_ != MESH_VERTICES 
        ) {
            return false;
        }
#ifdef GEO_GL_NO_DOUBLES
	// If there is an attribute bound, then return false,
	// this will force switching to immediate mode.
        if(attribute_subelements_ != MESH_NONE) {
	    return false;
	}
#endif	
	if(long_vector_attribute_) {
	    return false;
	}

	// For now, texturing is only implemented in
	// immediate mode (TODO: implement tex coords
	// in vertex array objects).
	if(attribute_dim_ > 1) {
	    return false;
	}
	
        return true;
    }


    /*********************************** vertices ***************/

    void MeshGfx::draw_vertices_immediate_plain() {
        glupBegin(GLUP_POINTS);
        for(index_t v: mesh_->vertices) {
            draw_vertex(v);
        }
        glupEnd();
    }

    void MeshGfx::draw_vertices_immediate_attrib() {
        begin_attributes();
        glupBegin(GLUP_POINTS);
        for(index_t v: mesh_->vertices) {
            draw_vertex_with_attribute(v);
        }
        glupEnd();
        end_attributes();
    }

    void MeshGfx::draw_vertices_array() {
        glupBindVertexArray(vertices_VAO_);
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }
        glupDrawArrays(GLUP_POINTS, 0, GLUPsizei(mesh_->vertices.nb()));
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_vertices_selection() {
        if(picking_mode_ != MESH_NONE) {
            return;
        }
        Attribute<bool> v_selection;
        v_selection.bind_if_is_defined(
            mesh_->vertices.attributes(), vertices_selection_
        );
        if(!v_selection.is_bound()) {
            return;
        }
        glupBegin(GLUP_POINTS);            
        for(index_t v: mesh_->vertices) {
            if(v_selection[v]) {
                draw_vertex(v);
            }
        }
        glupEnd();
    }
    
    void MeshGfx::draw_vertices() {
        if(mesh_ == nullptr) {
            return;
        }
        
        set_GLUP_parameters();
        set_GLUP_picking(MESH_VERTICES);
        update_buffer_objects_if_needed();
        
        glupSetColor4fv(GLUP_FRONT_COLOR, points_color_);
        glupSetPointSize(points_size_ * 5.0f);

        if(vertices_selection_ == "") {
            if(
		can_use_array_mode(GLUP_POINTS) && vertices_VAO_ != 0
	    ) {
                draw_vertices_array();
            } else {
                if(attribute_subelements_ == MESH_VERTICES) {
                    draw_vertices_immediate_attrib();
                } else {
                    draw_vertices_immediate_plain();
                }
            }
        } else {
            draw_vertices_selection();
        }
        
        glupDisable(GLUP_PICKING);        
    }

    /*********************************** edges ***************/

    void MeshGfx::draw_edges_array() {
        glupBindVertexArray(edges_VAO_);
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }
        glupDrawElements(
            GLUP_LINES,
            GLUPsizei(mesh_->edges.nb()*2),
            GL_UNSIGNED_INT,
            nullptr
        );
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_edges_immediate_plain() {
        glupBegin(GLUP_LINES);
        for(index_t e: mesh_->edges) {
            index_t v1 = mesh_->edges.vertex(e,0);
            index_t v2 = mesh_->edges.vertex(e,1);
            draw_vertex(v1);
            draw_vertex(v2);
        }
        glupEnd();
    }

    void MeshGfx::draw_edges_immediate_attrib() {
        begin_attributes();
        if(attribute_subelements_ == MESH_VERTICES) {
            glupBegin(GLUP_LINES);
            for(index_t e: mesh_->edges) {
                index_t v1 = mesh_->edges.vertex(e,0);
                index_t v2 = mesh_->edges.vertex(e,1);
                draw_vertex_with_attribute(v1);
                draw_vertex_with_attribute(v2);
            }
            glupEnd();
        } else if(attribute_subelements_ == MESH_EDGES) {
            glupBegin(GLUP_LINES);
            for(index_t e: mesh_->edges) {
                index_t v1 = mesh_->edges.vertex(e,0);
                index_t v2 = mesh_->edges.vertex(e,1);
		draw_attribute_as_tex_coord(e);
                draw_vertex(v1);
                draw_vertex(v2);
            }
            glupEnd();
        }
        end_attributes();
    }
    
    void MeshGfx::draw_edges() {
        if(mesh_ == nullptr) {
            return;
        }
        
        set_GLUP_parameters();
        set_GLUP_picking(MESH_EDGES);
        update_buffer_objects_if_needed();
        
        glupSetColor4fv(GLUP_FRONT_COLOR, mesh_color_);
        glupSetMeshWidth(GLUPint(mesh_width_));
        if(can_use_array_mode(GLUP_LINES) && edges_VAO_ != 0) {
            draw_edges_array();
        } else {
            if(attribute_subelements_ == MESH_VERTICES ||
               attribute_subelements_ == MESH_EDGES) {
                draw_edges_immediate_attrib();
            } else {
                draw_edges_immediate_plain();
            }
        }
    }

    /******************************************************************/

    void MeshGfx::draw_triangles() {
        if(can_use_array_mode(GLUP_TRIANGLES) && facets_VAO_ != 0) {
            draw_triangles_array();
        } else {
            if(
                attribute_subelements_ == MESH_VERTICES ||
                attribute_subelements_ == MESH_FACETS   ||
                attribute_subelements_ == MESH_FACET_CORNERS
            ) {
                draw_triangles_immediate_attrib();
            } else {
                draw_triangles_immediate_plain();
            }
        }
    }

    void MeshGfx::draw_triangles_array() {
        glupBindVertexArray(facets_VAO_);
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }
        glupDrawElements(
            GLUP_TRIANGLES,
            GLUPsizei(mesh_->facets.nb()*3),
            GL_UNSIGNED_INT,
            nullptr
        );
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_triangles_immediate_plain() {
        glupBegin(GLUP_TRIANGLES);
	// Optimized code for triangle surface with no attribute, single
	// and double precision. Writes mesh data directly in GLUP buffers.
	if(!do_animation_ && mesh_->vertices.dimension() >= 3) {
	    GLUP::Context* context = (GLUP::Context*)(glupCurrentContext());
	    GLUP::ImmediateState& state = context->immediate_state();
	    GLUP::ImmediateBuffer& buffer =
		state.buffer[GLUP::GLUP_VERTEX_ATTRIBUTE];
	    if(mesh_->vertices.single_precision()) {
		index_t t1 = 0;
		while(t1 < mesh_->facets.nb()) {
		    index_t t2 = t1 + (state.max_current_vertex()/3);
		    t2 = std::min(t2, mesh_->facets.nb());
		    GLfloat* current_vertex = buffer.data();
		    for(index_t t=t1; t<t2; ++t) {
			for(index_t lv=0; lv<3; ++lv) {
			    index_t v = mesh_->facets.vertex(t,lv);
			    const float* p = mesh_->vertices.
				single_precision_point_ptr(v);
			    current_vertex[0] = p[0];
			    current_vertex[1] = p[1];
			    current_vertex[2] = p[2];
			    current_vertex[3] = 1.0f;
			    current_vertex += 4;
			}
		    }
		    state.set_current_vertex(3*(t2-t1));
		    context->flush_immediate_buffers();
		    t1 = t2;
		}
	    } else {
		index_t t1 = 0;
		while(t1 < mesh_->facets.nb()) {
		    index_t t2 = t1 + (state.max_current_vertex()/3);
		    t2 = std::min(t2, mesh_->facets.nb());
		    GLfloat* current_vertex = buffer.data();
		    for(index_t t=t1; t<t2; ++t) {
			for(index_t lv=0; lv<3; ++lv) {
			    index_t v = mesh_->facets.vertex(t,lv);
			    const double* p = mesh_->vertices.point_ptr(v);
			    current_vertex[0] = float(p[0]);
			    current_vertex[1] = float(p[1]);
			    current_vertex[2] = float(p[2]);
			    current_vertex[3] = 1.0f;
			    current_vertex += 4;
			}
		    }
		    state.set_current_vertex(3*(t2-t1));
		    context->flush_immediate_buffers();
		    t1 = t2;
		}
	    }
	} else {
	    for(index_t t: mesh_->facets) {
		draw_vertex(mesh_->facets.vertex(t,0));
		draw_vertex(mesh_->facets.vertex(t,1));
		draw_vertex(mesh_->facets.vertex(t,2));                        
	    }
	}
        glupEnd();
    }

    void MeshGfx::draw_triangles_immediate_attrib() {
        begin_attributes();
        glupBegin(GLUP_TRIANGLES);
        for(index_t f: mesh_->facets) {
            for(index_t c: mesh_->facets.corners(f)) {
                index_t v=mesh_->facet_corners.vertex(c);
                draw_surface_vertex_with_attribute(v,f,c);
            }
        }
        glupEnd();
        end_attributes();        
    }
    
    void MeshGfx::draw_quads() {
        if(can_use_array_mode(GLUP_QUADS) && facets_VAO_ != 0) {
            draw_quads_array();
        } else {
            if(
                attribute_subelements_ == MESH_VERTICES ||
                attribute_subelements_ == MESH_FACETS   ||
                attribute_subelements_ == MESH_FACET_CORNERS
            ) {
                draw_quads_immediate_attrib();
            } else {
                draw_quads_immediate_plain();
            }
        }
    }

    void MeshGfx::draw_quads_array() {
        glupBindVertexArray(facets_VAO_);
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }
        glupDrawElements(
            GLUP_QUADS,
            GLUPsizei(mesh_->facets.nb()*4),
            GL_UNSIGNED_INT,
            nullptr
        );
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_quads_immediate_plain() {
        glupBegin(GLUP_QUADS);
        for(index_t q: mesh_->facets) {
            draw_vertex(mesh_->facets.vertex(q,0));
            draw_vertex(mesh_->facets.vertex(q,1));
            draw_vertex(mesh_->facets.vertex(q,2));
            draw_vertex(mesh_->facets.vertex(q,3));            
        }
        glupEnd();
    }

    void MeshGfx::draw_quads_immediate_attrib() {
        begin_attributes();
        glupBegin(GLUP_QUADS);
        for(index_t q: mesh_->facets) {
            for(index_t c: mesh_->facets.corners(q)) {
                index_t v=mesh_->facet_corners.vertex(c);
                draw_surface_vertex_with_attribute(v,q,c);
            }
        }
        glupEnd();
        end_attributes();        
    }

    void MeshGfx::draw_triangles_and_quads() {
        
        if(picking_mode_ != MESH_NONE) {
            draw_polygons_plain();
            return;
        }
        
        if(
            can_use_array_mode(GLUP_TRIANGLES) &&
            can_use_array_mode(GLUP_QUADS) &&            
            facets_VAO_ != 0
        ) {
            draw_triangles_and_quads_array();
        } else {
            if(
                attribute_subelements_ == MESH_VERTICES ||
                attribute_subelements_ == MESH_FACETS   ||
                attribute_subelements_ == MESH_FACET_CORNERS
            ) {
                draw_triangles_and_quads_immediate_attrib();
            } else {
                draw_triangles_and_quads_immediate_plain();
            }
        }
    }

    void MeshGfx::draw_triangles_and_quads_array() {
        
        glupBindVertexArray(facets_VAO_);
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }

        index_t b = 0;
        for(;;) {
            while(
                b != mesh_->facets.nb() && mesh_->facets.nb_vertices(b) != 3) {
                ++b;
            }
            if(b == mesh_->facets.nb()) {
                break;
            }
            index_t e=b;
            while(
                e != mesh_->facets.nb() && mesh_->facets.nb_vertices(e) == 3) {
                ++e;
            }

            glupDrawElements(
                GLUP_TRIANGLES,
                GLUPsizei((e-b)*3),
                GL_UNSIGNED_INT,
                (GLUPvoid*)(mesh_->facets.corners_begin(b) * sizeof(index_t))
            );
            
            b = e;
        } 


        b = 0;
        for(;;) {
            while(
                b != mesh_->facets.nb() && mesh_->facets.nb_vertices(b) != 4) {
                ++b;
            }
            if(b == mesh_->facets.nb()) {
                break;
            }
            index_t e=b;
            while(
                e != mesh_->facets.nb() && mesh_->facets.nb_vertices(e) == 4) {
                ++e;
            }

            glupDrawElements(
                GLUP_QUADS,
                GLUPsizei((e-b)*4),
                GL_UNSIGNED_INT,
                (GLUPvoid*)(mesh_->facets.corners_begin(b) * sizeof(index_t))
            );
            
            b = e;
        } 
        
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_triangles_and_quads_immediate_plain() {
        glupBegin(GLUP_TRIANGLES);
        for(index_t t: mesh_->facets) {
            if(mesh_->facets.nb_vertices(t) == 3) {
                draw_vertex(mesh_->facets.vertex(t,0));
                draw_vertex(mesh_->facets.vertex(t,1));
                draw_vertex(mesh_->facets.vertex(t,2));
            }
        }
        glupEnd();
        glupBegin(GLUP_QUADS);
        for(index_t q: mesh_->facets) {
            if(mesh_->facets.nb_vertices(q) == 4) {
                draw_vertex(mesh_->facets.vertex(q,0));
                draw_vertex(mesh_->facets.vertex(q,1));
                draw_vertex(mesh_->facets.vertex(q,2));
                draw_vertex(mesh_->facets.vertex(q,3));                
            }
        }
        glupEnd();
    }

    void MeshGfx::draw_triangles_and_quads_immediate_attrib() {
        begin_attributes();
        glupBegin(GLUP_TRIANGLES);
        for(index_t f: mesh_->facets) {
            if(mesh_->facets.nb_vertices(f) == 3) {            
                for(index_t c: mesh_->facets.corners(f)) {
                    index_t v=mesh_->facet_corners.vertex(c);
                    draw_surface_vertex_with_attribute(v,f,c);
                }
            }
        }
        glupEnd();
        glupBegin(GLUP_QUADS);
        for(index_t f: mesh_->facets) {
            if(mesh_->facets.nb_vertices(f) == 4) {            
                for(index_t c: mesh_->facets.corners(f)) {
                    index_t v=mesh_->facet_corners.vertex(c);
                    draw_surface_vertex_with_attribute(v,f,c);
                }
            }
        }
        glupEnd();
        end_attributes();        
    }

    void MeshGfx::draw_polygons() {
        if(
            picking_mode_ == MESH_NONE && (
                attribute_subelements_ == MESH_VERTICES ||
                attribute_subelements_ == MESH_FACETS   ||
                attribute_subelements_ == MESH_FACET_CORNERS
            )
        ) {
            draw_polygons_attrib();
        } else {
            draw_polygons_plain();
        }
    }
    
    void MeshGfx::draw_polygons_plain() {
        glupDisable(GLUP_DRAW_MESH);

        // Using vertex colors to do the picking.
        if(picking_mode_ != MESH_NONE) {
            glupDisable(GLUP_PICKING);
            glupDisable(GLUP_LIGHTING);
            glupEnable(GLUP_VERTEX_COLORS);
        }
            
        glupBegin(GLUP_TRIANGLES);
        bool picking_vertex_colors = false;
        if(picking_mode_ != MESH_NONE) {
            picking_vertex_colors = (
                (picking_mode_ & MESH_FACETS) != 0 &&
                object_picking_id_ == index_t(-1)
            );
            set_GLUP_vertex_color_from_picking_id(object_picking_id_);
        }
        for(index_t f: mesh_->facets) {
            if(picking_vertex_colors) {
                set_GLUP_vertex_color_from_picking_id(f);      
            }
            index_t v1 = mesh_->facets.vertex(f,0);
            for(index_t lv=1; lv+1<mesh_->facets.nb_vertices(f); ++lv) {
                index_t v2 = mesh_->facets.vertex(f,lv);
                index_t v3 = mesh_->facets.vertex(f,lv+1);
                draw_vertex(v1);
                draw_vertex(v2);
                draw_vertex(v3);
            }
        }
        glupEnd();
        glupDisable(GLUP_VERTEX_COLORS);        
        if(show_mesh_ && (picking_mode_ == MESH_NONE)) {
            draw_surface_mesh_with_lines();
        }
    }

    void MeshGfx::draw_polygons_attrib() {
        begin_attributes();
        glupDisable(GLUP_DRAW_MESH);
        glupBegin(GLUP_TRIANGLES);
        for(index_t f: mesh_->facets) {
            index_t c1 = mesh_->facets.corners_begin(f);
            index_t v1 = mesh_->facet_corners.vertex(c1);
            for(
                index_t c2 = c1+1;
                c2+1<mesh_->facets.corners_end(f); ++c2
             ) {
                index_t c3=c2+1;
                index_t v2=mesh_->facet_corners.vertex(c2);
                index_t v3=mesh_->facet_corners.vertex(c3);
                draw_surface_vertex_with_attribute(v1,f,c1);
                draw_surface_vertex_with_attribute(v2,f,c2);
                draw_surface_vertex_with_attribute(v3,f,c3);
            }
        }
        glupEnd();
        end_attributes();
        if(show_mesh_ && (picking_mode_ == MESH_NONE)) {
            glupDisable(GLUP_VERTEX_COLORS);                            
            draw_surface_mesh_with_lines();
        }
    }
    
    void MeshGfx::draw_surface() {
        if(mesh_ == nullptr) {
            return;
        }
        set_GLUP_parameters();
        set_GLUP_picking(MESH_FACETS);
        update_buffer_objects_if_needed();

        glupSetCellsShrink(0.0f);

        if(
	    attribute_subelements_ != MESH_NONE &&
	    !glupIsEnabled(GLUP_NORMAL_MAPPING) 
	) {
            glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 1.0f, 1.0f, 1.0f);
        } else {
            glupSetColor4fv(GLUP_FRONT_COLOR, surface_color_);
            glupSetColor4fv(GLUP_BACK_COLOR, backface_surface_color_);
        }

        if(mesh_->facets.are_simplices()) {
            draw_triangles(); 
        } else if(quads_) {
            draw_quads();
        } else if(triangles_and_quads_) {
            draw_triangles_and_quads(); 
        } else {
            draw_polygons();
        }
    }

    void MeshGfx::draw_surface_mesh_with_lines() {
        glupSetMeshWidth(GLUPint(mesh_width_));        
        glupSetColor4fv(GLUP_FRONT_AND_BACK_COLOR, mesh_color_);
        glupBegin(GLUP_LINES);
        for(index_t f: mesh_->facets) {
            for(index_t c1: mesh_->facets.corners(f)) {
                index_t c2 =
                    mesh_->facets.next_corner_around_facet(f,c1);
                index_t v1 = mesh_->facet_corners.vertex(c1);
                index_t v2 = mesh_->facet_corners.vertex(c2);
                draw_vertex(v1);
                draw_vertex(v2);
            }
        }
        glupEnd();
    }
    
    void MeshGfx::draw_surface_borders() {
        if(picking_mode_ != MESH_NONE) {
            return;
        }
        set_GLUP_parameters();
        glupSetColor4fv(GLUP_FRONT_COLOR, mesh_color_);
        glupSetMeshWidth(GLUPint(mesh_border_width_));
        glupBegin(GLUP_LINES);
        for(index_t f: mesh_->facets) {
            for(index_t c1: mesh_->facets.corners(f)) {
                if(mesh_->facet_corners.adjacent_facet(c1) == NO_FACET) {
                    index_t v1 = mesh_->facet_corners.vertex(c1);
                    index_t c2 = mesh_->facets.next_corner_around_facet(f,c1);
                    index_t v2 = mesh_->facet_corners.vertex(c2);
                    draw_vertex(v1);
                    draw_vertex(v2);                    
                }
            }
        }
        glupEnd();
    }

    /***********************************************************************/
    
    void MeshGfx::draw_tets() {
        if(!draw_cells_[MESH_TET]) {
            return;
        }
        glupSetColor4fv(GLUP_FRONT_AND_BACK_COLOR, cells_color_[MESH_TET]);
        if(can_use_array_mode(GLUP_TETRAHEDRA) && cells_VAO_ != 0) {
            draw_tets_array();
        } else {
            if(
                attribute_subelements_ == MESH_VERTICES ||
                attribute_subelements_ == MESH_CELLS ||
                attribute_subelements_ == MESH_CELL_FACETS   ||
                attribute_subelements_ == MESH_CELL_CORNERS
            ) {
                draw_tets_immediate_attrib();
            } else {
                draw_tets_immediate_plain();
            }
        }
    }

    void MeshGfx::draw_tets_array() {
        glupBindVertexArray(cells_VAO_);
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }
        glupDrawElements(
            GLUP_TETRAHEDRA,
            GLUPsizei(mesh_->cells.nb()*4),
            GL_UNSIGNED_INT,
            nullptr
        );
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_tets_immediate_plain() {
        glupBegin(GLUP_TETRAHEDRA);
	// Optimized code for tet mesh with no attribute, single
	// and double precision. Writes mesh data directly in GLUP buffers.
	if(!do_animation_ && mesh_->vertices.dimension() >= 3) {
	    GLUP::Context* context = (GLUP::Context*)(glupCurrentContext());
	    GLUP::ImmediateState& state = context->immediate_state();
	    GLUP::ImmediateBuffer& buffer =
		state.buffer[GLUP::GLUP_VERTEX_ATTRIBUTE];
	    if(mesh_->vertices.single_precision()) {
		index_t t1 = 0;
		while(t1 < mesh_->cells.nb()) {
		    index_t t2 = t1 + (state.max_current_vertex()/4);
		    t2 = std::min(t2, mesh_->cells.nb());
		    GLfloat* current_vertex = buffer.data();
		    for(index_t t=t1; t<t2; ++t) {
			for(index_t lv=0; lv<4; ++lv) {
			    index_t v = mesh_->cells.vertex(t,lv);
			    const float* p = mesh_->vertices.
				single_precision_point_ptr(v);
			    current_vertex[0] = p[0];
			    current_vertex[1] = p[1];
			    current_vertex[2] = p[2];
			    current_vertex[3] = 1.0f;
			    current_vertex += 4;
			}
		    }
		    state.set_current_vertex(4*(t2-t1));
		    context->flush_immediate_buffers();
		    t1 = t2;
		}
	    } else {
		index_t t1 = 0;
		while(t1 < mesh_->cells.nb()) {
		    index_t t2 = t1 + (state.max_current_vertex()/4);
		    t2 = std::min(t2, mesh_->cells.nb());
		    GLfloat* current_vertex = buffer.data();
		    for(index_t t=t1; t<t2; ++t) {
			for(index_t lv=0; lv<4; ++lv) {
			    index_t v = mesh_->cells.vertex(t,lv);
			    const double* p = mesh_->vertices.point_ptr(v);
			    current_vertex[0] = float(p[0]);
			    current_vertex[1] = float(p[1]);
			    current_vertex[2] = float(p[2]);
			    current_vertex[3] = 1.0f;
			    current_vertex += 4;
			}
		    }
		    state.set_current_vertex(4*(t2-t1));
		    context->flush_immediate_buffers();
		    t1 = t2;
		}
	    }
	} else {	
	    for(index_t t: mesh_->cells) {
		draw_vertex(mesh_->cells.vertex(t,0));
		draw_vertex(mesh_->cells.vertex(t,1));
		draw_vertex(mesh_->cells.vertex(t,2));
		draw_vertex(mesh_->cells.vertex(t,3));            
	    }
	}
        glupEnd();
    }

    void MeshGfx::draw_tets_immediate_attrib() {
        begin_attributes();
        glupBegin(GLUP_TETRAHEDRA);
        for(index_t t: mesh_->cells) {
            index_t v0 = mesh_->cells.vertex(t,0);
            index_t v1 = mesh_->cells.vertex(t,1);
            index_t v2 = mesh_->cells.vertex(t,2);
            index_t v3 = mesh_->cells.vertex(t,3);
            index_t c0 = 4*t;
            draw_volume_vertex_with_attribute(v0, t, c0);
            draw_volume_vertex_with_attribute(v1, t, c0+1);
            draw_volume_vertex_with_attribute(v2, t, c0+2);
            draw_volume_vertex_with_attribute(v3, t, c0+3);
        }
        glupEnd();
        end_attributes();
    }

    static GLUPprimitive geogram_cell_to_glup[MESH_NB_CELL_TYPES] = {
        GLUP_TETRAHEDRA,
        GLUP_HEXAHEDRA,
        GLUP_PRISMS,
        GLUP_PYRAMIDS,
        GLUP_CONNECTORS
    };

    void MeshGfx::draw_hybrid() {
        if(
            cells_VAO_ != 0 &&
            can_use_array_mode(GLUP_TETRAHEDRA) &&
            can_use_array_mode(GLUP_HEXAHEDRA) &&
            can_use_array_mode(GLUP_PRISMS) &&
            can_use_array_mode(GLUP_PYRAMIDS) &&
            can_use_array_mode(GLUP_CONNECTORS)
        ) {
            draw_hybrid_array();
        } else {
            if(
                (
                    picking_mode_ == MESH_NONE) && (
                    attribute_subelements_ == MESH_VERTICES ||
                    attribute_subelements_ == MESH_CELLS ||
                    attribute_subelements_ == MESH_CELL_FACETS ||
                    attribute_subelements_ == MESH_CELL_CORNERS
                )
            ) {
                draw_hybrid_immediate_attrib();
            } else {
                draw_hybrid_immediate_plain();
            }
        }
    }

    void MeshGfx::draw_hybrid_array() {
        
        glupBindVertexArray(cells_VAO_);
        
        if(attribute_subelements_ == MESH_VERTICES) {
            begin_attributes();
        }

        bool has_cells[MESH_NB_CELL_TYPES];
        for(index_t type=0; type<MESH_NB_CELL_TYPES; ++type) {
            has_cells[type] = false;
        }
        for(index_t cell: mesh_->cells) {
            has_cells[mesh_->cells.type(cell)] = true;
        }


        for(index_t type=MESH_TET; type < MESH_NB_CELL_TYPES; ++type) {
            if(!draw_cells_[type] || !has_cells[type]) {
                continue;
            }
            if(attribute_subelements_ != MESH_VERTICES) {            
                glupSetColor4fv(GLUP_FRONT_AND_BACK_COLOR, cells_color_[type]);
            }

            GLUPprimitive glup_prim = geogram_cell_to_glup[type];
            index_t nb_vertices =
                mesh_->cells.cell_type_to_cell_descriptor(
                    MeshCellType(type)
                ).nb_vertices;
            
            index_t b = 0;
            for(;;) {
                while(
                    b != mesh_->cells.nb() &&
                    index_t(mesh_->cells.type(b)) != type
                ) {
                    ++b;
                }
                if(b == mesh_->cells.nb()) {
                    break;
                }
                index_t e=b;
                while(
                    e != mesh_->cells.nb() &&
                    index_t(mesh_->cells.type(e)) == type
                ) {
                    ++e;
                }
                glupDrawElements(
                    glup_prim,
                    GLUPsizei((e-b)*nb_vertices),
                    GL_UNSIGNED_INT,
                    (GLUPvoid*)(
                        mesh_->cells.corners_begin(b) * sizeof(index_t)
                    )
                );
                b = e;
            } 
        }
        
        if(attribute_subelements_ == MESH_VERTICES) {
            end_attributes();
        }
        
        glupBindVertexArray(0);
    }

    void MeshGfx::draw_hybrid_immediate_plain() {
        bool has_cells[MESH_NB_CELL_TYPES];
        for(index_t type=0; type<MESH_NB_CELL_TYPES; ++type) {
            has_cells[type] = false;
        }
        for(index_t cell: mesh_->cells) {
            has_cells[mesh_->cells.type(cell)] = true;
        }
        for(index_t type=MESH_TET; type < MESH_NB_CELL_TYPES; ++type) {
            if(!draw_cells_[type] || !has_cells[type]) {
                continue;
            }
            glupSetColor4fv(GLUP_FRONT_AND_BACK_COLOR, cells_color_[type]);
            glupBegin(geogram_cell_to_glup[type]);
            for(index_t cell: mesh_->cells) {
                index_t this_cell_type = index_t(mesh_->cells.type(cell));
                if(this_cell_type != type) {
                    continue;
                }
                for(index_t lv=0; lv<mesh_->cells.nb_vertices(cell); ++lv) {
                    draw_vertex(mesh_->cells.vertex(cell,lv));
                }
            }
            glupEnd();
        }
    }

    void MeshGfx::draw_hybrid_immediate_attrib() {
        bool has_cells[MESH_NB_CELL_TYPES];
        for(index_t type=0; type<MESH_NB_CELL_TYPES; ++type) {
            has_cells[type] = false;
        }
        for(index_t cell: mesh_->cells) {
            has_cells[mesh_->cells.type(cell)] = true;
        }
        begin_attributes();
        for(index_t type=MESH_TET; type<MESH_NB_CELL_TYPES; ++type) {
            if(!draw_cells_[type] || !has_cells[type]) {
                continue;
            }
            glupBegin(geogram_cell_to_glup[type]);
            for(index_t cell: mesh_->cells) {
                index_t this_cell_type = index_t(mesh_->cells.type(cell));
                if(this_cell_type != type) {
                    continue;
                }
                index_t c0 = mesh_->cells.corners_begin(cell);
                for(index_t lv=0;
                    lv<mesh_->cells.nb_vertices(cell); ++lv
                ) {
                    draw_volume_vertex_with_attribute(
                        mesh_->cells.vertex(cell,lv),
                        cell,
                        c0+lv
                    );
                }
            }
            glupEnd();
        }
        end_attributes();                        
    }
    
    void MeshGfx::draw_volume() {
        if(mesh_ == nullptr) {
            return;
        }
        if(mesh_->cells.nb() == 0) {
            return;
        }
        set_GLUP_parameters();
        set_GLUP_picking(MESH_VERTICES);
        update_buffer_objects_if_needed();
        glupSetCellsShrink(GLUPfloat(shrink_));

        if(mesh_->cells.are_simplices()) {
            draw_tets();
        } else {
            draw_hybrid();
        }
	glupSetCellsShrink(0.0f);
    }

    void MeshGfx::set_mesh(const Mesh* mesh) {
        mesh_ = mesh;
        triangles_and_quads_ = true;
        quads_ = true;
        if(mesh_ != nullptr) {
            for(index_t f: mesh_->facets) {
                index_t nb = mesh_->facets.nb_vertices(f);
                if(nb != 3 && nb != 4) {
                    triangles_and_quads_ = false;
                }
                if(nb != 4) {
                    quads_ = false;
                }
            }
        }
        buffer_objects_dirty_ = true;
        attributes_buffer_objects_dirty_ = true;
    }
    
    void MeshGfx::set_GLUP_parameters() {
        if(show_mesh_) {
            glupEnable(GLUP_DRAW_MESH);
        } else {
            glupDisable(GLUP_DRAW_MESH);
        }
        glupSetColor4fv(GLUP_MESH_COLOR, mesh_color_);
        glupSetMeshWidth(GLUPint(mesh_width_));
        glupSetPointSize(points_size_);
        if(lighting_) {
            glupEnable(GLUP_LIGHTING);
        } else {
            glupDisable(GLUP_LIGHTING);
        }

        do_animation_ =
            (animate_ && mesh_->vertices.dimension() >= 6);

        ES_profile_ = !strcmp(glupCurrentProfileName(), "GLUPES2");
    }

    void MeshGfx::set_GLUP_picking(MeshElementsFlags what) {
        if(picking_mode_ == MESH_NONE && object_picking_id_ == index_t(-1)) {
            glupDisable(GLUP_PICKING);
        } else {
            glupEnable(GLUP_PICKING);
            if(
                (object_picking_id_ == index_t(-1)) &&
                ((picking_mode_ & what) != 0)
            ) {
                glupPickingMode(GLUP_PICK_PRIMITIVE);                    
            } else {
                glupPickingMode(GLUP_PICK_CONSTANT);
                glupPickingId(object_picking_id_);
            }
        }
    }

    void MeshGfx::set_GLUP_vertex_color_from_picking_id(index_t id) {
        GLubyte r = GLubyte( id        & 255);
        GLubyte g = GLubyte((id >> 8)  & 255);
        GLubyte b = GLubyte((id >> 16) & 255);
        GLubyte a = GLubyte((id >> 24) & 255);
        glupColor4f(
            GLfloat(r)/255.0f,
            GLfloat(g)/255.0f,
            GLfloat(b)/255.0f,
            GLfloat(a)/255.0f
        );
    }

    void MeshGfx::bind_vertices_VBO() {
        glBindBuffer(GL_ARRAY_BUFFER, vertices_VBO_);
        glEnableVertexAttribArray(0);

        GLint dim = GLint(std::min(3u, mesh_->vertices.dimension()));
        
        if(mesh_->vertices.single_precision()) {
            GLsizei stride = GLsizei(
                mesh_->vertices.dimension() * sizeof(float)
            );
            glVertexAttribPointer(
                0,        // Attribute 0
                dim,      // nb coordinates per vertex
                GL_FLOAT, // input coordinates representation
                GL_FALSE, // do not normalize
                stride,   // offset between two consecutive vertices
                nullptr   // addr. relative to bound VBO 
            );
        } else {
#ifdef GEO_GL_NO_DOUBLES
	    // Do nothing, because if a double attribute is bound,
	    // then can_use_array_mode() returns 0, then we switch
	    // to immediate mode.
#else            
            GLsizei stride = GLsizei(
                mesh_->vertices.dimension() * sizeof(double)
            );
            glVertexAttribPointer(
                0,         // Attribute 0
                dim,       // nb coordinates per vertex
                GL_DOUBLE, // input coordinates representation
                GL_FALSE,  // do not normalize
                stride,    // offset between two consecutive vertices
                nullptr    // addr. relative to bound VBO 
            );
#endif                    
        }
    }

    
    void MeshGfx::update_buffer_objects_if_needed() {
        if(mesh_->vertices.nb() == 0) {
            return;
        }

        if(!buffer_objects_dirty_) {
            update_attribute_buffer_objects_if_needed();
            return;
        }

        if(!strcmp(glupCurrentProfileName(),"VanillaGL")) {
            return;
        }
        
        if(mesh_->vertices.single_precision()) {
            size_t size = mesh_->vertices.nb() *
                mesh_->vertices.dimension() * sizeof(float);
            update_or_check_buffer_object(
                vertices_VBO_, GL_ARRAY_BUFFER,
                size, mesh_->vertices.single_precision_point_ptr(0),
                buffer_objects_dirty_
            );
        } else {
            size_t size = mesh_->vertices.nb() *
                mesh_->vertices.dimension() * sizeof(double);
            
            update_or_check_buffer_object(
                vertices_VBO_, GL_ARRAY_BUFFER,
                size, mesh_->vertices.point_ptr(0),
                buffer_objects_dirty_
            );
        }

        if(vertices_VAO_ == 0) {
            glupGenVertexArrays(1, &vertices_VAO_);
        }
        glupBindVertexArray(vertices_VAO_);
        bind_vertices_VBO();
        glupBindVertexArray(0);

        if(mesh_->edges.nb()) {
            update_or_check_buffer_object(
                edge_indices_VBO_, GL_ELEMENT_ARRAY_BUFFER,
                mesh_->edges.nb() * 2 * sizeof(int),
                mesh_->edges.vertex_index_ptr(0),
                buffer_objects_dirty_
            );
            if(edges_VAO_ == 0) {
                glupGenVertexArrays(1, &edges_VAO_);
            }
            glupBindVertexArray(edges_VAO_);
            bind_vertices_VBO();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, edge_indices_VBO_);
            glupBindVertexArray(0);
        }
        
        if(
            mesh_->facets.nb() != 0 &&
            (mesh_->facets.are_simplices() || triangles_and_quads_)
        ) {
            update_or_check_buffer_object(
                facet_indices_VBO_, GL_ELEMENT_ARRAY_BUFFER,
                mesh_->facet_corners.nb() * sizeof(int),
                mesh_->facet_corners.vertex_index_ptr(0),
                buffer_objects_dirty_
            );
            if(facets_VAO_ == 0) {
                glupGenVertexArrays(1, &facets_VAO_);
            }
            glupBindVertexArray(facets_VAO_);
            bind_vertices_VBO();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facet_indices_VBO_);
            glupBindVertexArray(0);
        }
        
        if(mesh_->cells.nb() != 0) {
            update_or_check_buffer_object(
                cell_indices_VBO_, GL_ELEMENT_ARRAY_BUFFER,
                mesh_->cell_corners.nb() * sizeof(int),
                mesh_->cell_corners.vertex_index_ptr(0),
                buffer_objects_dirty_
            );
            if(cells_VAO_ == 0) {
                glupGenVertexArrays(1, &cells_VAO_);
            }
            glupBindVertexArray(cells_VAO_);
            bind_vertices_VBO();
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, cell_indices_VBO_);
            glupBindVertexArray(0);
        }
        
        buffer_objects_dirty_ = false;
        update_attribute_buffer_objects_if_needed();
    }

    void MeshGfx::set_scalar_attribute(
        MeshElementsFlags subelements,
        const std::string& name,
        double attr_min, double attr_max,
        GLuint colormap_texture,
        index_t repeat
    ) {
        if(
            subelements != attribute_subelements_ ||
            attribute_name_ != name
        ) {
            attributes_buffer_objects_dirty_ = true;
        }
        attribute_subelements_ = subelements;
        attribute_name_ = name;
        attribute_min_ = attr_min;
        attribute_max_ = attr_max;
        attribute_repeat_ = repeat;
        attribute_texture_ = colormap_texture;
	attribute_dim_ = 1;
	attribute_texture_dim_ = 1;
	
        const MeshSubElementsStore& mesh_subelements =
            mesh_->get_subelements_by_type(attribute_subelements_);

        if(!ReadOnlyScalarAttributeAdapter::is_defined(
               mesh_subelements.attributes(), attribute_name_
           )
        ) {
            attribute_subelements_ = MESH_NONE;
        }
        
    }

    void MeshGfx::set_texturing(
	MeshElementsFlags subelements,
	const std::string& name,
	GLuint texture,
	index_t texture_dim,
	index_t repeat
    ) {
        if(
            subelements != attribute_subelements_ ||
            attribute_name_ != name
        ) {
            attributes_buffer_objects_dirty_ = true;
        }
        attribute_subelements_ = subelements;
        attribute_name_ = name;
        attribute_min_ = 0.0;
        attribute_max_ = 1.0;
        attribute_repeat_ = repeat;
        attribute_texture_ = texture;
	attribute_texture_dim_ = texture_dim;
	
        const MeshSubElementsStore& mesh_subelements =
            mesh_->get_subelements_by_type(attribute_subelements_);

	attribute_dim_ = 0;
	FOR(i,3) {
	    tex_coord_attribute_[i].bind_if_is_defined(
		mesh_subelements.attributes(),
		attribute_name_ + "[" + String::to_string(i) + "]"
	    );
	    if(tex_coord_attribute_[i].is_bound()) {
		attribute_dim_ = i+1;
		tex_coord_attribute_[i].unbind();
	    }
	}
	if(attribute_dim_ == 0) {
	    attribute_subelements_ = MESH_NONE;
	}
    }
    
    void MeshGfx::update_attribute_buffer_objects_if_needed() {
        if(mesh_->vertices.nb() == 0) {
            return;
        }

        if(!attributes_buffer_objects_dirty_) {
            return;
        }

        if(!strcmp(glupCurrentProfileName(),"VanillaGL")) {
            return;
        }

	long_vector_attribute_ = false;

        if(attribute_subelements_ == MESH_VERTICES) {
            scalar_attribute_.bind_if_is_defined(
                mesh_->vertices.attributes(), attribute_name_
            );
            if(scalar_attribute_.attribute_store()->dimension() > 4) {
                scalar_attribute_.unbind();
		long_vector_attribute_ = true;
            }
        }
        
        if(scalar_attribute_.is_bound()) {
            size_t element_size =
		scalar_attribute_.attribute_store()->element_size();
            GLint dimension =
		GLint(scalar_attribute_.attribute_store()->dimension());
	    // nb_items should be scalar_attribute_.size(), using capacity()
	    // instead seemingly fixes a display bug (zero attribute on last
	    // vertex). To be further investigated...
            index_t nb_items = scalar_attribute_.size();
	        // ... or ... scalar_attribute_.attribute_store()->capacity();
            const void* data = scalar_attribute_.attribute_store()->data();

            update_or_check_buffer_object(
                vertices_attribute_VBO_, GL_ARRAY_BUFFER,
                element_size*index_t(dimension)*nb_items,
                data,
                attributes_buffer_objects_dirty_
            );

            bind_attribute_buffer_object(vertices_VAO_);
            bind_attribute_buffer_object(edges_VAO_);
            bind_attribute_buffer_object(facets_VAO_);
            bind_attribute_buffer_object(cells_VAO_);
            
            scalar_attribute_.unbind();            
        } else {
            unbind_attribute_buffer_object(vertices_VAO_);
            unbind_attribute_buffer_object(edges_VAO_);
            unbind_attribute_buffer_object(facets_VAO_);
            unbind_attribute_buffer_object(cells_VAO_);
        }
	
        attributes_buffer_objects_dirty_ = false;
    }

    void MeshGfx::bind_attribute_buffer_object(GLuint VAO) {
        if(VAO == 0) {
            return;
        }
        size_t element_size =
	    scalar_attribute_.attribute_store()->element_size();
        GLint dimension =
	    GLint(scalar_attribute_.attribute_store()->dimension());

        if(
            scalar_attribute_.element_type() ==
            ReadOnlyScalarAttributeAdapter::ET_VEC2) {
            dimension *= 2;
            element_size /= 2;
        } else if(scalar_attribute_.element_type() ==
            ReadOnlyScalarAttributeAdapter::ET_VEC3) {
            dimension *= 3;
            element_size /= 3;
        } 
        
        GLsizei stride = GLsizei(element_size) * dimension;

        const GLvoid* offset = (const GLvoid*)(
            element_size * index_t(scalar_attribute_.element_index())
        );
        
        glupBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, vertices_attribute_VBO_);
        glEnableVertexAttribArray(2); // 2 = tex coords

        switch(scalar_attribute_.element_type()) {
        case ReadOnlyScalarAttributeAdapter::ET_UINT8:
            glVertexAttribPointer(
                2, dimension, GL_UNSIGNED_BYTE, GL_FALSE, stride, offset
            );
            break;
        case ReadOnlyScalarAttributeAdapter::ET_INT8:
            glVertexAttribPointer(
                2, dimension, GL_BYTE, GL_FALSE, stride, offset
            );
            break;
        case ReadOnlyScalarAttributeAdapter::ET_UINT32:
            glVertexAttribPointer(
                2, dimension, GL_UNSIGNED_INT, GL_FALSE, stride, offset
            );
            break;
        case ReadOnlyScalarAttributeAdapter::ET_INT32:
            glVertexAttribPointer(
                2, dimension, GL_INT, GL_FALSE, stride, offset
            );
            break;
        case ReadOnlyScalarAttributeAdapter::ET_FLOAT32:
            glVertexAttribPointer(
                2, dimension, GL_FLOAT, GL_FALSE, stride, offset
            );
            break;
        case ReadOnlyScalarAttributeAdapter::ET_FLOAT64:
        case ReadOnlyScalarAttributeAdapter::ET_VEC2:
        case ReadOnlyScalarAttributeAdapter::ET_VEC3:                        
#ifdef GEO_GL_NO_DOUBLES
	    // Do nothing, because if a double attribute is bound,
	    // then can_use_array_mode() returns 0, then we switch
	    // to immediate mode.
#else            
            glVertexAttribPointer(
                2, dimension, GL_DOUBLE, GL_FALSE, stride, offset
            );
#endif            
            break;
        case ReadOnlyScalarAttributeAdapter::ET_NONE:
            geo_assert_not_reached;
        }
        glupBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    void MeshGfx::unbind_attribute_buffer_object(GLuint VAO) {
        if(VAO == 0) {
            return;
        }
        glupBindVertexArray(VAO);
        glDisableVertexAttribArray(2); // 2 = tex coords
        glupBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    
    void MeshGfx::begin_attributes() {
        if(picking_mode_ != MESH_NONE) {
            return;
        }
        if(attribute_subelements_ == MESH_NONE) {
            return;
        }
        const MeshSubElementsStore& subelements =
            mesh_->get_subelements_by_type(attribute_subelements_);

	if(attribute_dim_ == 1) {
	    scalar_attribute_.bind_if_is_defined(
		subelements.attributes(), attribute_name_
	    );
	    if(!scalar_attribute_.is_bound()) {
		return;
	    }
	} else {
	    FOR(i,3) {
		tex_coord_attribute_[i].bind_if_is_defined(
		    subelements.attributes(),
		    attribute_name_+"["+String::to_string(i)+"]"
		);
	    }
	    if(!tex_coord_attribute_[0].is_bound()) {
		return;
	    }
	}

        glupEnable(GLUP_TEXTURING);
        glupTextureMode(GLUP_TEXTURE_REPLACE);

	switch(attribute_texture_dim_) {
	    case 1:
		glupTextureType(GLUP_TEXTURE_1D);		
		glActiveTexture(GL_TEXTURE0 + GLUP_TEXTURE_1D_UNIT);
		glBindTexture(
		    GLUP_TEXTURE_1D_TARGET, attribute_texture_
		);
		break;
	    case 2:
		glupTextureType(GLUP_TEXTURE_2D);				
		glActiveTexture(GL_TEXTURE0 + GLUP_TEXTURE_2D_UNIT);
		glBindTexture(
		    GLUP_TEXTURE_2D_TARGET, attribute_texture_
		);
		break;
	    case 3:
		glupTextureType(GLUP_TEXTURE_3D);
		glActiveTexture(GL_TEXTURE0 + GLUP_TEXTURE_3D_UNIT);
		glBindTexture(
		    GLUP_TEXTURE_3D_TARGET, attribute_texture_
		);
		break;
	}

	if(attribute_dim_ == 1) {
	    // Setup a texture matrix that rescales attribute range
	    // from [attribute_min_,attribute_max_] to [0,1]
	    glupMapTexCoords1d(
		attribute_min_, attribute_max_, attribute_repeat_
	    );
	} else {
	    glupMatrixMode(GLUP_TEXTURE_MATRIX);
	    glupLoadIdentity();
	    if(attribute_repeat_ != 0) {
		glupScalef(
		    float(attribute_repeat_),
		    float(attribute_repeat_),
		    float(attribute_repeat_)
		);
	    }
	    glupMatrixMode(GLUP_MODELVIEW_MATRIX);
	}

	if(!glupIsEnabled(GLUP_NORMAL_MAPPING)) {
	    glupSetColor3f(GLUP_FRONT_AND_BACK_COLOR, 1.0f, 1.0f, 1.0f);
	}
    }

    void MeshGfx::end_attributes() {
        if(scalar_attribute_.is_bound()) {
            glupDisable(GLUP_TEXTURING);
            scalar_attribute_.unbind();
        }
	FOR(i,3) {
	    if(tex_coord_attribute_[i].is_bound()) {
		if(i==0) {
		    glupDisable(GLUP_TEXTURING);
		}
		tex_coord_attribute_[i].unbind();
	    }
	}
	glupMatrixMode(GLUP_TEXTURE_MATRIX);
	glupLoadIdentity();
	glupMatrixMode(GLUP_MODELVIEW_MATRIX);
    }

    
}

