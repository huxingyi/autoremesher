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

#ifndef H_GEOGRAM_GFX_GUI_APPLICATION_H
#define H_GEOGRAM_GFX_GUI_APPLICATION_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/gui/events.h>

#ifndef GEO_OS_ANDROID
#  define GEO_GLFW
#endif

/**
 * \file geogram_gfx/gui/application.h
 * \brief Base class for all applications.
 */

namespace GEO {

    class Image;
    class ApplicationData;
    
    /**
     * \brief Base class for all applications.
     * \details This class handles the cross-platform creation of a window,
     *  OpenGL context, and ImGui. Client code may use SimpleApplication
     *  instead.
     */
    class GEOGRAM_GFX_API Application {
    public:
    
        /**
         * \brief Application constructor.
	 * \param[in] name the name of the application
         */
         Application(const std::string& name);

        /**
         * \brief Application destructor.
         */
         virtual ~Application();

        /**
         * \brief Gets the instance.
         * \return a pointer to the instance.
         */
         static Application* instance() {
	     return instance_;
	 }

    
        /**
	 * \brief Gets the name of this application.
	 * \return the name.
	 */
         const std::string& name() const {
	     return name_;
	 }

    
        /**
         * \brief Starts the main event loop of the application.
	 * \param[in] argc , argv optional command line parameters. If specified
	 *  then they are used to initialize geogram, else geogram is supposed
	 *  to be already initialized by caller.
         */
         virtual void start(int argc=0, char** argv=nullptr);

        /**
         * \brief Stops the application.
         */
        virtual void stop();

	/**
	 * \brief Gets the style.
	 * \return a string with the current style;
	 */
	const std::string& get_style() const {
	    return style_;
	}
    
        /**
         * \brief Sets the style of the application.
	 * \param[in] value one of Dark, Light, DarkGray, LightGray
	 * \see get_styles()
         */
        virtual void set_style(const std::string& value);

        /**
	 * \brief Gets the possible styles.
	 * \return A semi-colon separated list of the possible
	 *  styles.
	 */
        static std::string get_styles();
    
        /**
         * \brief Sets the font size.
	 * \param[in] value the font size.
         */
        void set_font_size(index_t value);

        /**
	 * \brief Gets the font size.
	 * \return the font size.
	 */
        index_t get_font_size() const {
	    return font_size_;
	}

	/**
	 * \brief Indicates that the main window should be redrawn.
	 */
        virtual void update();

	/**
	 * \brief Draws a dockspace that fills the current
	 *  window.
	 */
	void draw_dock_space();
    
	/**
	 * \brief Lock updates.
	 * \details If this function is called, updates are ignored. 
	 *  It is useful when a RenderingContext operation is occuring, to
	 *  prevent the Console for triggering a drawing operation.
	 */
	void lock_updates() {
	    ++nb_update_locks_;
	}

	/**
	 * \brief Unlock updates.
	 */
	void unlock_updates() {
	    // Note: Under Windows, when the Graphite window is iconified,
	    // it can happen that nb_update_locks_ is already 0 when
	    // reaching this point.
	    if(nb_update_locks_ > 0) {
		--nb_update_locks_;
	    }
	}

	/**
	 * \brief Redraws the main window.
	 * \details This function is called by commands that animate 
	 *  objects during computation, by the progress bar and by
	 *  console output.
	 */
	virtual void draw();

	/**
	 * \brief Gets the global scaling to be applied to all GUI elements.
	 * \return 1.0 if the default font is used, more/less if a larger/
	 *  smaller font is used.
	 */
	double scaling() const;

	/**
	 * \brief Sets full-screen mode.
	 * \details All arguments to zero sets default mode.
	 * \param[in] w , h width and height in pixels
	 * \param[in] hz refresh rate in Hz
	 * \param[in] monitor the id of the monitor
	 */
	void set_full_screen_mode(
	    index_t w=0, index_t h=0, index_t hz=0,
	    index_t monitor=0
	);

	/**
	 * \brief Sets windowed mode.
	 * \param[in] w , h width and height in pixels. If
	 *  zero, use current dimensions.
	 */
	void set_windowed_mode(index_t w=0, index_t h=0);

	/**
	 * \brief Lists the video modes that can be used for
	 *  set_full_screen_mode()
	 * \details The video modes are listed in the terminal.
	 */
	void list_video_modes();

	/**
	 * \brief Iconifies this application.
	 */
	void iconify();

	/**
	 * \brief Restores this application.
	 */
	void restore();

        /**
         * \brief Sets the gui state.
         * \param[in] x a string that encodes
         *  the windows geometries and docking configuration
         *  obtained through get_gui_state()
         */
	void set_gui_state(std::string x);

        /**
         * \brief Gets the gui state.
         * \return a string that encodes
         *  the windows geometries and docking configuration
	 */
	std::string get_gui_state() const;
	
	/**
	 * \brief Sets full-screen mode.
	 * \param[in] x true if full-screen mode should be used,
	 *  false if windowed-mode should be used.
	 */
	void set_full_screen(bool x);

	/**
	 * \brief Tests whether this application is in full-screen mode.
	 * \retval true if full-screen mode is used.
	 * \retval false if windowed mode is used.
	 */
	bool get_full_screen() const;
	
	/**
	 * \brief Gets the width of the window.
	 * \return the width of the window in pixels.
	 */
	index_t get_width() const {
	    return width_;
	}

	/**
	 * \brief Gets the height of the window.
	 * \return the height of the window in pixels.
	 */
	index_t get_height() const {
	    return height_;
	}

	/**
	 * \brief Gets the width of the window.
	 * \return the width of the frame buffer in pixels.
	 */
	index_t get_frame_buffer_width() const {
	    return frame_buffer_width_;
	}

	/**
	 * \brief Gets the height of the window.
	 * \return the height of the frame buffer in pixels.
	 */
	index_t get_frame_buffer_height() const {
	    return frame_buffer_height_;
	}
    
        /**
         * \brief Sets whether drag and drop events should be
         *  taken into account.
         * \param[in] value true if drag and drop events should be taken into
         *  account, false otherwise
         */
	void set_accept_drops(bool value) {
	    accept_drops_ = value;
	}

        /**
         * \brief Tests whether drag and drop events are taken into
         *  account.
         * \retval true if drag and drop events are taken into account
         * \retval false otherwise
         */
	bool get_accept_drops() const {
	    return accept_drops_;
	}

        /**
	 * \brief Sets the icon of the window.
	 * \param[in] image a pointer to the image to be used as the icon.
	 */
        void set_window_icon(Image* image);

        /**
	 * \brief Callback called whenenver a mouse button changed.
	 * \param[in] button the button
	 * \param[in] action the action (one of 
	 *  EVENT_ACTION_UP, EVENT_ACTION_DOWN, EVENT_ACTION_DRAG)
	 * \param[in] mods the current key modifiers (not implemented yet)
	 * \param[in] source the event source (one of EVENT_SOURCE_MOUSE,
	 *   EVENT_SOURCE_FINGER, EVENT_SOURCE_STYLUS)
	 */
        virtual void mouse_button_callback(
	    int button, int action, int mods=0, int source=EVENT_SOURCE_MOUSE
	);

        /**
	 * \brief Callback called whenenver the mouse wheel is moved.
	 * \param[in] xoffset , yoffset wheel displacement
	 */
        virtual void scroll_callback(double xoffset, double yoffset);

        /**
	 * \brief Callback called whenever the mouse cursor is moved.
	 * \param[in] x , y the new position of the mouse cursor.
	 * \param[in] source the event source (one of EVENT_SOURCE_MOUSE,
	 *   EVENT_SOURCE_FINGER, EVENT_SOURCE_STYLUS)
	 */
        virtual void cursor_pos_callback(
	    double x, double y, int source=EVENT_SOURCE_MOUSE
	);

        /**
	 * \brief Callback called whenever files are dropped in the window.
	 * \param[in] nb number of files.
	 * \param[in] f the array of file names.
	 */
        virtual void drop_callback(int nb, const char** f);

        /**
	 * \brief Callback called whenever a key is pushed (high level version)
	 * \param[in] c the ASCII code of the character that corresponds to the
	 *  pushed key.
	 */
        virtual void char_callback(unsigned int c);

        /**
	 * \brief Callback called whenever a key is pushed (low level version)
	 * \param[in] key key code (window system specific)
	 * \param[in] scancode scan code (window system specific)
	 * \param[in] action push or release (window system specific)
	 * \param[in] mods current key modifieds (window system specific)
	 */
        virtual void key_callback(int key, int scancode, int action, int mods);

        /**
         * \brief Restarts the gui.
	 * \details A flag is set and the gui is restarted at the next frame.
	 */
        void restart_gui() {
	    ImGui_restart_ = true;
	}
    
        /**
	 * \brief Gets a pointer to the implementation-specific data.
	 * \details For internal use only.
	 * \return a pointer to the implementation-specific data.
	 */
        ApplicationData* impl_data() {
	    return data_;
	}

        /**
	 * \brief Gets a pointer to the implementation-specific window.
	 * \details For internal use only.
	 * \return a pointer to the implementation-specific window.
	 */
        void* impl_window();

        /**
	 * \brief MacOS non-sense
	 * \return a scaling factor between real pixels and logical
	 *  pixels or something, well I do not understand. Sometimes
	 *  you need to multiply by it, sometimes to divide, and
	 *  sometimes you need to use pixel_ratio() instead.
	 */
        double hidpi_scaling() const {
	    return hidpi_scaling_;
	}

        /**
	 * \brief More MacOS non-sense
	 * \return something like hidpi_scaling(), that is a scaling 
	 *  factor between real pixels and logical
	 *  pixels or something, well I do not understand.
	 *  Sometimes you need to multiply by it, sometimes to divide, 
	 *  and sometimes you need to use hidpi_scaling() instead.
	 */
        double pixel_ratio() const {
	    return pixel_ratio_;
	}

        /**
	 * \brief Used internally.
	 */
        void reset_soft_keyboard_flag() {
	    soft_keyboard_visible_ = false;
	}
    
    protected:

        /**
         * \brief Converts a key to a symbolic string with the name of the key.
         * \param[in] key the key.
         * \return a string with the symbolic name of the key.
         */
        const char* key_to_string(int key);
    
	/**
	 * \brief This function is called when the GUI should be redisplayed.
	 * \details This function is meant to be overloaded by subclasses. 
	 *   default implementation does nothing.
	 */
        virtual void draw_gui();

	/**
	 * \brief This function is called when the 3d content should be 
	 *  redisplayed.
	 * \details This function is meant to be overloaded by subclasses. 
	 *   default implementation does nothing.
	 */
        virtual void draw_graphics();
    
        /**
	 * \brief This function is called before starting drawing operations.
	 * \details Some implementations use it to initialize / restore graphic
	 *  objects.
	 */
        virtual void pre_draw();

        /**
	 * \brief This function is called after all drawing operations.
	 * \details It can be used to execute queued commands.
	 */
        virtual void post_draw();
    
	/**
	 * \brief Tests whether the window needs to be redrawn.
	 * \retval true if the window needs to be redrawn.
	 * \retval false if the window is up to date.
	 */
	virtual bool needs_to_redraw() const;

	/**
	 * \brief Creates the window using GLFW.
	 */
	virtual void create_window();

	/**
	 * \brief Deletes the window created by GLFW.
	 */
	virtual void delete_window();

	/**
	 * \brief Called whenever window size changes
	 * \param[in] w , h the new window width and height in pixels.
	 * \param[in] fb_w , fb_h the new framebuffer width and height in
	 *   pixels.
	 * \details Called whenenver the size of the window does not
	 *  match the current size.
	 */
         virtual void resize(index_t w, index_t h, index_t fb_w, index_t fb_h);

	/**
	 * \brief Draws one frame.
	 * \details This triggers a GUI and/or a scene update as needed.
	 */
	virtual void one_frame();

	/**
	 * \brief Enters the main application loop.
	 * \details create_window() needs to be called before.
	 *  This initializes OpenGL and ImGui before the first frame is
	 *  displayed.
	 */
	virtual void main_loop();

	/**
	 * \brief Initializes OpenGL and GLUP objects.
	 */
	virtual void GL_initialize();

	/**
	 * \brief Deallocates OpenGL and GLUP objects.
	 */
	virtual void GL_terminate();

	/**
	 * \brief Initializes the ImGui library.
	 */
	virtual void ImGui_initialize();

	/**
	 * \brief Loads the fonts in ImGui.
	 */
	virtual void ImGui_load_fonts();
	
	/**
	 * \brief Deallocates objects used by the ImGui library.
	 */
	virtual void ImGui_terminate();

	/**
	 * \brief Notifies ImGui that a new frame has just started.
	 */
	virtual void ImGui_new_frame();

        /*
	 * \param[in] argc , argv command line parameters, used
	 *  to initialize geogram.
	 */
        virtual void geogram_initialize(int argc, char** argv);
    
        /**
         * \brief Initializes the callbacks if not already initialized.
         */
        void callbacks_initialize();

        /**
	 * \brief Gets all the filenames specified on the command line.
	 * \return a const reference to a vector of strings with the filenames.
	 */
        const std::vector<std::string>& filenames() const  {
	    return filenames_;
	}

        bool animate() const {
	    return animate_;
	}

        bool* animate_ptr() {
	    return &animate_;
	}
    
        void start_animation() {
	    animate_ = true;
	}

        void stop_animation() {
	    animate_ = false;
	}

      private:
        static Application* instance_; /**< a pointer to the instance */
        ApplicationData* data_;        /**< implementation dependent */
        index_t width_;                /**< window width */
        index_t height_;               /**< window height */
        index_t frame_buffer_width_;   /**< frame buffer width (glViewport) */
        index_t frame_buffer_height_;  /**< frame buffer height (glViewport) */
        bool in_main_loop_;            /**< main loop is running */
        bool accept_drops_;            /**< app. accepts dropping files */
        double scaling_;               /**< global scaling for to all sizes */
        index_t nb_update_locks_;      /**< lock graphic updates */
        std::string style_;            /**< ImGui style (Dark, Light, ...) */
        bool ImGui_restart_;           /**< ImGui needs to be restarted */
        bool ImGui_reload_font_;       /**< font size has changed */
        bool ImGui_initialized_;       /**< ImGui was initialized */
        index_t font_size_;            /**< current font size */
        index_t nb_frames_update_;     /**< if 0, take a small sleep */
        double hidpi_scaling_;         /**< for retina displays */ 
        double pixel_ratio_;           /**< for retina displays */
        std::string name_;             /**< application name */
        bool currently_drawing_gui_;   /**< currently drawing ImGui elements */
        std::vector<std::string> filenames_; /**< from the command line */
        bool animate_;                 /**< true if drawing always */

    
      protected:
        bool ImGui_firsttime_init_;  /**< true if ImGui was once initialized */
        bool menubar_visible_;
        bool phone_screen_;          /**< true if running on a phone */
        bool soft_keyboard_visible_;

#ifdef GEO_OS_EMSCRIPTEN
       friend void emscripten_one_frame();
#endif
    };

}

#endif

