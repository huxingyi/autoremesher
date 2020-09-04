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

#ifndef H_GEOGRAM_GFX_GUI_SIMPLE_APPLICATION_H
#define H_GEOGRAM_GFX_GUI_SIMPLE_APPLICATION_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/gui/application.h>
#include <geogram_gfx/gui/status_bar.h>
#include <geogram_gfx/gui/console.h>
#include <geogram_gfx/gui/text_editor.h>
#include <geogram_gfx/gui/command.h>
#include <geogram_gfx/gui/arc_ball.h>
#include <geogram_gfx/full_screen_effects/full_screen_effect.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>

#include <map>
#include <functional>

struct lua_State;

namespace GEO {

    class GEOGRAM_GFX_API SimpleApplication : public Application {
      public:

	/**
	 * \brief SimpleApplication constructor.
	 * \param[in] name the name of the application.
	 */
	SimpleApplication(const std::string& name);

	/**
	 * \brief SimpleApplication destructor.
	 */
	~SimpleApplication() override;
	
	/**
	 * \copydoc GEO::Application::draw_gui()
	 */
	void draw_gui() override;

	/**
	 * \copydoc GEO::Application::draw_graphics()
	 */
	void draw_graphics() override;

        /**
         * \brief Saves the current content to a file.
         * \details Baseclass implementation does nothing. Derived classes
         *  may overload this function.
         * \retval true if the file could be sucessfully saved.
         * \retval false otherwise
         */
        virtual bool save(const std::string& filename);
        
        /**
         * \brief Loads a file.
         * \details Baseclass implementation does nothing. Derived classes
         *  may overload this function.
         * \retval true if the file could be sucessfully loaded
         * \retval false otherwise
         */
        virtual bool load(const std::string& filename);

	/**
	 * \brief Gets the text editor.
	 * \return a reference to the text editor.
	 */
	TextEditor& text_editor() {
	    return text_editor_;
	}

	/**
	 * \brief Shows the text editor.
	 */
	void show_text_editor() {
	    text_editor_visible_ = true;
	}

	/**
	 * \brief Hides the text editor.
	 */
	void hide_text_editor() {
	    text_editor_visible_ = false;
	}

	/**
	 * \brief Shows the console.
	 */
	void show_console() {
	    console_visible_ = true;
	}

	/**
	 * \brief Hides the console.
	 */
	void hide_console() {
	    console_visible_ = false;
	}

	/**
	 * \brief Restores default viewing parameters.
	 */
	void home();

	/**
	 * \copydoc GEO::Application::set_style()
	 */
	void set_style(const std::string& style) override;

	/**
	 * \brief Sets the region of interest
	 * \details This defines the default target of the camera
	 * \param[in] xmin , ymin , zmin , xmax , ymax , zmax the
	 *   bounds of the region of interest.
	 */
	void set_region_of_interest(
	    double xmin, double ymin, double zmin,
	    double xmax, double ymax, double zmax
	);

	/**
	 * \brief Gets the region of interest
	 * \see set_region_of_interest()
	 * \param[out] xmin , ymin , zmin , xmax , ymax , zmax the
	 *   bounds of the region of interest.
	 */
	void get_region_of_interest(
	    double& xmin, double& ymin, double& zmin,
	    double& xmax, double& ymax, double& zmax
	) const;
	
	void zoom_in() {
	    zoom_ *= 1.1;
	}

	void zoom_out() {
	    zoom_ /= 1.1;
	}

	void set_clipping(bool x) {
	    clipping_ = x;
	}

	void set_lighting(bool x) {
	    lighting_ = x;
	}

	void set_background_color(const vec4f& color) {
	    background_color_ = color;
	}

	virtual bool exec_command(const char* command);
	
	static SimpleApplication* instance() {
	    return dynamic_cast<SimpleApplication*>(
		GEO::Application::instance()
	    );
	}

	/**
	 * \brief Projects a point from model space to window coordinates.
	 * \param[in] p the point in model space coordinates.
	 * \return the point in window coordinates, that is
	 *    [0,width-1] x [0,height-1]
	 */
	vec3 project(const vec3& p);

	/**
	 * \brief Unprojects a 3d point from window coordinates to model space.
	 * \param[in] p the 3d point in window coordinates, that is
	 *   [0,width-1] x [0,height-1]
	 * \return the point in model space coordinates.
	 */
	vec3 unproject(const vec3& p);

	/**
	 * \brief Unprojects a 2d point from window coordinates to model space.
	 * \param[in] p the 2d point in screen space coordinates.
	 * \return the 2d point in model space coordinates.
	 */
	vec2 unproject_2d(const vec2& p);


	/**
	 * \copydoc Application::drop_callback()
	 */
	void drop_callback(int nb, const char** f) override;
	
      protected:

	/**
	 * \brief Declares a function to be triggered when a key is pressed.
	 * \param[in] key the key ("a" for a, "F1" for F1)
	 * \param[in] cb the function to be called
	 * \param[in] help an optional help string
	 */
	void add_key_func(
	    const std::string& key, std::function<void()> cb,
	    const char* help = nullptr
	);

	/**
	 * \brief Declares a boolean to be toggled when a key is pressed.
	 * \param[in] key the key ("a" for a, "F1" for F1)
	 * \param[in] p_val a pointer to the boolean
	 * \param[in] help an optional help string
	 */
	void add_key_toggle(
	    const std::string& key, bool* p_val,
	    const char* help = nullptr
	);

	/**
	 * \copydoc GEO::Application::char_callback()
	 */
	void char_callback(unsigned int c) override;
	
	/**
	 * \copydoc GEO::Application::key_callback()
	 */
        void key_callback(int key, int scancode, int action, int mods) override;

        /**
	 * \copydoc GEO::Application::mouse_button_callback()
	 */
	void mouse_button_callback(
	    int button, int action, int mods, int source
	) override;

        /**
	 * \copydoc GEO::Application::cursor_pos_callback()
	 */
	void cursor_pos_callback(double x, double y, int source) override;

        /**
	 * \copydoc GEO::Application::scroll_callback()
	 */
	void scroll_callback(double xoffset, double yoffset) override;
	
	/**
	 * \brief Setups OpenGL for scene drawing.
	 */
	virtual void draw_scene_begin();

	/**
	 * \brief Draws the scene.
	 */
	virtual void draw_scene();

	/**
	 * \brief Cleanups OpenGL after scene drawing.
	 */
	virtual void draw_scene_end();
	
        /**
         * \brief Draws the viewer properties window frame and contents.
         */
        virtual void draw_viewer_properties_window();
        
        /**
         * \brief Draws the contents of viewer properties window.
         */
        virtual void draw_viewer_properties();

        /**
         * \brief Draw the object properties window frame and contents.
         */
        virtual void draw_object_properties_window();
        
        /**
         * \brief Draws the contents of the object properties window.
         */
        virtual void draw_object_properties();


        /**
         * \brief Draws the active command window if any.
         */
        virtual void draw_command_window();
	
        /**
         * \brief Draws the console.
         */
        virtual void draw_console();

	
	/**
         * \brief Draws the menu bar.
         */
        virtual void draw_menu_bar();


        /**
         * \brief Draws the load menu and browser.
         */
        virtual void draw_load_menu();

        /**
         * \brief Draws the save menu.
         */
        virtual void draw_save_menu();

	/**
	 * \brief Draws other file operation menu.
	 * \details Default implementation does nothing.
	 *  It can be overloaded to add other menu
	 *  items in the file menu.
	 */
	virtual void draw_fileops_menu();
	
        /**
         * \brief Draws the about box in the file menu.
         */
        virtual void draw_about();

        /**
         * \brief Draws help info (accelarators)
         */
        virtual void draw_help();
	
        /**
         * \brief Draws the windows menu.
         */
        virtual void draw_windows_menu();

        /**
         * \brief Draws the application menus.
         * \details Meant to be overloaded by derived classes.
         */
        virtual void draw_application_menus();

        /**
         * \brief Draws the application icons on the menubar.
         * \details Meant to be overloaded by derived classes.
         */
	virtual void draw_application_icons();
	
	/**
	 * \copydoc Application::post_draw()
	 */
        void post_draw() override;
	
        /**
         * \brief Tests whether a file can be loaded.
         * \details This function can be used to filter the files displayed
         *  in the "Load..." menu. Baseclass implementation always return true.
         *  Derived classes may overload it and return false for files with
         *  unknown extensions.
         */  
        virtual bool can_load(const std::string& filename);
	
        /**
         * \brief Gets the list of supported file extensions for reading.
         * \details This function may be olverloaded by derived class. Base
         *  class implementation returns "". If this function returns "", then
         *  no "Load..." option is displayed in the "File" menu. 
         * \return The semi-colon separated list of supported file extensions,
         *  or "*" if all file extensions are supported.
         */
        virtual std::string supported_read_file_extensions(); 

        /**
         * \brief Gets the list of supported file extensions for writing.
         * \details This function may be olverloaded by derived class. Base
         *  class implementation returns "". If this function returns "", then
         *  no "Save..." option is displayed in the "File" menu.  
         *   If it returns a colon-separated list of extensions, then the
         *  "Save..." option displays a list of possible file names for each
         *  supported extension.
         * \return The semi-colon separated list of supported file extensions,
         *  or "*" if all file extensions are supported.
         */
        virtual std::string supported_write_file_extensions(); 

        /**
         * \brief Converts an OpenGL texture ID into an ImGUI texture ID.
         * \param[in] gl_texture_id_in the OpenGL texture ID
         * \return the corresponding ImGUI texture ID
         */
        ImTextureID convert_to_ImTextureID(GLuint gl_texture_id_in);

	/**
	 * \copydoc GEO::Application::GL_initialize()
	 */
	void GL_initialize() override;

	/**
	 * \copydoc GEO::Application::GL_terminate()
	 */
	void GL_terminate() override;

        /**
         * \brief Recursively browses a directory and generates
         *  menu items.
         * \param[in] path the path to be browsed
         * \param[in] subdirs if true, browse subdirectories as well
         */
        void browse(const std::string& path, bool subdirs=false);

	/**
	 * \copydoc GEO::Application::geogram_initialize()
	 */
	void geogram_initialize(int argc, char** argv) override;

	/**
	 * \brief Sets the default filename used to save
	 *  the current file.
	 * \param[in] filename the default filename.
	 */
	void set_default_filename(const std::string& filename) {
	    strcpy(filename_, filename.c_str());
	}

        /**
         * \brief Initializes a new colormap from name and xpm data.
         * \details This function can be called only once the OpenGL
         *  context is ready, for instance in the init_graphics() function.
         * \param[in] name the name of the colormap
         * \param[in] xpm_data the image data of the colormap.
         */
        void init_colormap(const std::string& name, const char** xpm_data);

        /**
         * \brief Initializes all the default colormaps.
         * \details This function can be called only once the OpenGL
         *  context is ready, for instance in the init_graphics() function.
         */
        void init_colormaps();

	/**
	 * \copydoc Application::ImGui_initialize()
	 */
	void ImGui_initialize() override;

	void set_2d() {
	    three_D_ = false;
	}

	void set_3d() {
	    three_D_ = true;
	}

	void set_default_layout();

        void resize(index_t w, index_t h, index_t fb_w, index_t fb_h) override;

	virtual const char* default_layout() const;
	virtual const char* default_layout_android_vertical() const;
	virtual const char* default_layout_android_horizontal() const;		
	
      protected:
	bool lighting_;
	bool edit_light_;
	bool clipping_;
        GLUPclipMode clip_mode_;
	bool edit_clip_;
	bool fixed_clip_;
	GLenum effect_;
	vec4f background_color_;
	
        bool viewer_properties_visible_;
        bool object_properties_visible_;
        bool console_visible_;
	bool text_editor_visible_;
	bool use_text_editor_;

	Box roi_;
	double roi_radius_;
	vec3    object_translation_;
	ArcBall object_rotation_;
	ArcBall light_rotation_;
	ArcBall clip_rotation_;
	vec3    clip_translation_;
	bool    three_D_;
	double  zoom_;
	double  zoom_down_; /**< Zoom when mouse down. */

	bool props_pinned_;
	
	enum MouseOp {
	    MOUSE_NOOP, MOUSE_ROTATE, MOUSE_TRANSLATE, MOUSE_ZOOM
	} mouse_op_;
	
	enum MouseTarget {
	    MOUSE_NOTARGET, MOUSE_OBJECT, MOUSE_LIGHT, MOUSE_CLIP
	} mouse_target_;

	vec2 mouse_down_xy_; // in [-1,1] x [-1,1]
	vec2 mouse_xy_;      // in [-1,1] x [-1,1]

	// Current transform, for picking
	GLint viewport_[4];
	mat4 modelview_transpose_;
	mat4 project_transpose_;
	
        std::string path_;	
	std::string current_file_;
        char filename_[geo_imgui_string_length]; // Buffer for file dialog.
        GLuint geogram_logo_texture_;

        Console_var console_;
        StatusBar_var status_bar_;
	TextEditor text_editor_;

	std::map< std::string, std::function<void()> > key_funcs_;
	std::map< std::string, std::string > key_funcs_help_;

        struct ColormapInfo {
            ColormapInfo() : texture(0) {
            }
            GLuint texture;
            std::string name;
        };

        vector<ColormapInfo> colormaps_;
	FullScreenEffectImpl_var full_screen_effect_;

	lua_State* lua_state_;
	bool lua_error_occured_;
    };

}

#endif
