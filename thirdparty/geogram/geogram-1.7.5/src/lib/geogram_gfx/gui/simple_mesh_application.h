/*
 *  Copyright (c) 2012-2016, Bruno Levy
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

#ifndef H_GEOGRAM_GFX_GUI_SIMPLE_MESH_APPLICATION_H
#define H_GEOGRAM_GFX_GUI_SIMPLE_MESH_APPLICATION_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/gui/simple_application.h>
#include <geogram_gfx/mesh/mesh_gfx.h>

namespace GEO {

    /**
     * \brief An Application that manipulates a single Mesh.
     */
    class GEOGRAM_GFX_API SimpleMeshApplication : public SimpleApplication {
    public:
        /**
         * \brief Application constructor.
         */
        SimpleMeshApplication(const std::string& name);

      protected:

	/**
	 * \copydoc SimpleApplication::geogram_initialize()
	 */
	void geogram_initialize(int argc, char** argv) override;
	
        /**
         * \copydoc SimpleApplication::supported_read_file_extensions()
         */
        std::string supported_read_file_extensions() override;

        /**
         * \copydoc SimpleApplication::supported_write_file_extensions()
         */
	std::string supported_write_file_extensions() override;

        /**
         * \copydoc SimpleApplication::draw_object_properties()
         */
	void draw_object_properties() override;

        /**
         * \copydoc SimpleApplication::draw_scene()
         */
	void draw_scene() override;

        /**
         * \copydoc SimpleApplication::GL_initialize()
         */
	void GL_initialize() override;

        /**
         * \copydoc SimpleApplication::load()
         */
	bool load(const std::string& filename) override;

        /**
         * \copydoc SimpleApplication::save()
         */
	bool save(const std::string& filename) override;

        /**
         * \brief Gets the instance.
         * \return a pointer to the current SimpleMeshApplication.
         */
        static SimpleMeshApplication* instance() {
            SimpleMeshApplication* result =
                dynamic_cast<SimpleMeshApplication*>(
		    SimpleApplication::instance()
		);
            geo_assert(result != nullptr);
            return result;
        }

    protected:

        /**
         * \brief Gets the bounding box of a mesh animation.
         * \details In animated mode, the mesh animation is stored as 
         *  a mesh with 6d coordinates, that correspond to the geometric 
         *  location at the vertices at time 0 and at time 1.
         * \param[in] M_in the mesh
         * \param[out] xyzmin a pointer to the three minimum coordinates
         * \param[out] xyzmax a pointer to the three maximum coordinates
         * \param[in] animate true if displaying a mesh animation
         */
        void get_bbox(
            const Mesh& M_in, double* xyzmin, double* xyzmax, bool animate
        );
        
        /**
         * \brief increments the animation time in the current instance.
         * \details Callback bound to the 't' key
         */
        static void increment_anim_time_callback();

        /**
         * \brief derements the animation time in the current instance.
         * \details Callback bound to the 'r' key
         */
        static void decrement_anim_time_callback();

        /**
         * \brief increments the cells shrinkage.
         * \details Callback bound to the 'w' key
         */
        static void increment_cells_shrink_callback();

        /**
         * \brief decrements the cells shrinkage.
         * \details Callback bound to the 'x' key
         */
        static void decrement_cells_shrink_callback();

    protected:

        /**
         * \brief Gets the mesh.
         * \return a pointer to the Mesh.
         */
        Mesh* mesh() {
            return &mesh_;
        }

        /**
         * \brief Gets the mesh graphics.
         * \return a pointer to the MeshGfx.
         */
        MeshGfx* mesh_gfx() {
            return &mesh_gfx_;
        }

        /**
         * \brief Makes the vertices visible.
         */
        void show_vertices() {
            show_vertices_ = true;
        }

        /**
         * \brief Makes the vertices invisible.
         */
        void hide_vertices() {
            show_vertices_ = false;
        }

        /**
         * \brief Makes the surface facets visible.
         */
        void show_surface() {
            show_surface_ = true;
        }

        /**
         * \brief Makes the surface facets invisible.
         */
        void hide_surface() {
            show_surface_ = false;            
        }

        /**
         * \brief Makes the volume cells visible.
         */
        void show_volume() {
            show_volume_ = true;
        }

        /**
         * \brief Makes the volume cells invisible.
         */
        void hide_volume() {
            show_volume_ = false;            
        }

        /**
         * \brief Makes the attributes visible.
         */
        void show_attributes() {
            show_attributes_ = true;
        }
        
        /**
         * \brief Makes the attributes invisible.
         */
        void hide_attributes() {
            show_attributes_ = false;
        }
        
        /**
         * \brief Adjusts the current minimum and maximum attribute value
         *  to the currently bound attribute if any.
         */
        void autorange();

        /**
         * \brief Gets the list of attribute names.
         * \return the ';'-separated list of attribute names.
         */
        std::string attribute_names();

        /**
         * \brief Sets the currently displayed attribute.
         * \param[in] attribute the name of the attribute
         *  to be displayed, prefixed by element type.
         */
        void set_attribute(const std::string& attribute);
        
    protected:
        Mesh mesh_;
        MeshGfx mesh_gfx_;
        std::string file_extensions_;

        float anim_speed_;
        float anim_time_;

        bool show_vertices_;
        bool show_vertices_selection_;
        float vertices_size_;
	vec4f vertices_color_;
	float vertices_transparency_;
	
        bool show_surface_;
        bool show_surface_sides_;        
        bool show_mesh_;
	float mesh_width_;
	vec4f mesh_color_;
	
        bool show_surface_borders_;
	vec4f surface_color_;
	vec4f surface_color_2_;

        bool show_volume_;
        float cells_shrink_;
	vec4f volume_color_;
        bool show_colored_cells_;
        bool show_hexes_;
	bool show_connectors_;

        bool show_attributes_;
        GLuint current_colormap_texture_;
        std::string       attribute_;
        MeshElementsFlags attribute_subelements_;
        std::string       attribute_name_;
        float             attribute_min_;
        float             attribute_max_;
    };

    /*****************************************************************/    
}

#endif
