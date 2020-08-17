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
#include <geogram_gfx/basic/GLSL.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/stopwatch.h>
#include <algorithm>
#include <map>

extern void register_embedded_glsl_files(GEO::FileSystem::MemoryNode* n);

namespace {

    using namespace GEO;

    /**
     * \brief A super-simple shader authoring application.
     */
    class GeoShadeApplication : public SimpleApplication {
    public:

        /**
         * \brief GeoShadeApplication constructor.
         */
        GeoShadeApplication() : SimpleApplication("GeoShade") {
	    builtin_files_ = new FileSystem::MemoryNode();
	    set_default_filename("hello.glsl");
	    use_text_editor_ = true;
            set_region_of_interest(0.0, 0.0, 0.0, 1.0, 1.0, 1.0);
	    register_embedded_glsl_files(builtin_files_);
	    glsl_program_ = 0;	    
	    three_D_ = false;
	    glsl_frame_ = 0;
	    glsl_start_time_ = SystemStopwatch::now();
	    object_properties_visible_ = false;
	    new_file();
        }

        /**
         * \brief GeoShadeApplication destructor.
         */
	~GeoShadeApplication() override {
	    if(glsl_program_ != 0) {
		glDeleteProgram(glsl_program_);
		glsl_program_ = 0;
	    }
        }


	/**
	 * \copydoc SimpleApplication::geogram_initialize()
	 */
	void geogram_initialize(int argc, char** argv) override {
	    SimpleApplication::geogram_initialize(argc, argv);
	    if(phone_screen_) {
		viewer_properties_visible_ = false;
	    }
	}

	
        /**
         * \brief Displays and handles the GUI for object properties.
         * \details Overloads Application::draw_object_properties().
         */
	void draw_object_properties() override {
	    // TODO
        }

        /**
         * \brief Draws the scene according to currently set primitive and
         *  drawing modes.
         */
	void draw_scene() override {
	    
	    if(glsl_program_ != 0) {
		if(animate()) {
		    
		    glUseProgram(glsl_program_);

		    // If shader has an iTime uniform (e.g. a ShaderToy shader),
		    // then update it, and indicate that graphics should be
		    // updated again and again (call update() at each frame).
		    GLint iTime_loc = glGetUniformLocation(
			glsl_program_, "iTime"
		    );
		    if(iTime_loc != -1) {
			glUniform1f(
			    iTime_loc,
			    float(SystemStopwatch::now() - glsl_start_time_)
			);
		    }

		    GLint iFrame_loc = glGetUniformLocation(
			glsl_program_, "iFrame"
		    );
		    if(iFrame_loc != -1) {
			glUniform1f(
			    iFrame_loc, float(glsl_frame_)
			);
		    }

		    GLint iDate_loc = glGetUniformLocation(
			glsl_program_, "iDate"
		    );

		    if(iDate_loc != -1) {
			time_t t = time(nullptr);
			struct tm* tm = localtime(&t);
			float datex = float(tm->tm_year + 1900);
			float datey = float(tm->tm_mon);
			float datez = float(tm->tm_mday);
			float datew = float(tm->tm_sec) +
			    60.0f * float(tm->tm_min) +
			    3600.0f * float(tm->tm_hour);
			glUniform4f(iDate_loc, datex, datey, datez, datew);
		    }

		    glUseProgram(0);

		    ++glsl_frame_;

		    // Of course, there is a much faster way of drawing a
		    // quad, but here we do not care about this (tiny) loss
		    // of performance.
		    
		    glupDisable(GLUP_VERTEX_COLORS);
		    glupEnable(GLUP_TEXTURING);		    		    
		    glupUseProgram(glsl_program_);
		    glupBegin(GLUP_TRIANGLES);
		    
		    glupTexCoord2d(0.0, 0.0);
		    glupVertex2d(  0.0, 0.0);

		    glupTexCoord2d(1.0, 1.0);
		    glupVertex2d(  1.0, 1.0);

		    glupTexCoord2d(0.0, 1.0);
		    glupVertex2d(  0.0, 1.0);
		    
		    
		    glupTexCoord2d(1.0, 1.0);
		    glupVertex2d(  1.0, 1.0);
		    
		    glupTexCoord2d(0.0, 0.0);
		    glupVertex2d(  0.0, 0.0);

		    glupTexCoord2d(1.0, 0.0);
		    glupVertex2d(  1.0, 0.0);		    

		    glupEnd();
		    glupUseProgram(0);
		    glupDisable(GLUP_TEXTURING);		    
		}
	    }
        }

	void new_file() {
	    text_editor_.clear();
	    text_editor_.load_data(
		"void mainImage(\n"
		"   out vec4 col,in vec2 coord\n"
		") {\n"
		"   col.r = coord.x /\n"
		"               iResolution.x;\n"
		"   col.g = coord.y /\n"
		"               iResolution.y;\n"
		"   col.b = 0.5*(col.x+col.y);\n"
		"}\n"
	    );
	    text_editor_visible_ = true;	    
	}
	
	virtual void draw_fileops_menu() override {
	    if(ImGui::MenuItem(
		   (icon_UTF8("play-circle") + " Run program").c_str(),
		   phone_screen_ ? nullptr : "[F5]"
	    )) {
		run_program();
	    }
	    if(ImGui::MenuItem(
		   (icon_UTF8("stop-circle") + "Stop program").c_str()
	    )) {
		stop_program();
	    }
	    ImGui::Separator();
	    if(ImGui::MenuItem(icon_UTF8("file")+" New...")) {
		new_file();
		current_file_ = "";
	    }
	    if(phone_screen_) {
		draw_load_menu();
		draw_save_menu();
	    }
	    ImGui::Separator();
	    if(ImGui::MenuItem(icon_UTF8("folder-open")+"Load example...")) {
		ImGui::OpenFileDialog(
		    "##load_dlg",
		    supported_read_file_extensions().c_str(),
		    filename_,
		    ImGuiExtFileDialogFlags_Load,
		    builtin_files_
		);
	    } 
	}

	void run_program() {
	    if(glsl_program_ != 0) {
		glDeleteProgram(glsl_program_);
		glsl_program_ = 0;
	    }
	    glsl_frame_ = 0;
	    glsl_start_time_ = SystemStopwatch::now();
	    if(text_editor_.text().find("<GLUP/ShaderToy.h>") != std::string::npos) {
		glsl_program_ = glupCompileProgram(text_editor_.text().c_str());		
	    } else {
		std::string source = (
		    "//stage GL_FRAGMENT_SHADER\n"
		    "//import <GLUP/ShaderToy.h>\n"
		    "#line 1\n"
		) + text_editor_.text();		    
		glsl_program_ = glupCompileProgram(source.c_str());
	    }
	    text_editor_visible_ = (glsl_program_ == 0);
	    start_animation();
	}

	void stop_program() {
	    stop_animation();
	    text_editor_visible_ = true;
	}

	/**
	 * \copydoc SimpleApplication::draw_application_icons()
	 */
	void draw_application_icons() override {
	    if(phone_screen_) {
		if(ImGui::SimpleButton(icon_UTF8("home"))) {
		    home();
		}
		if(ImGui::SimpleButton(icon_UTF8("play-circle"))) {
		    run_program();
		}
		if(ImGui::SimpleButton(icon_UTF8("stop-circle"))) {
		    stop_program();
		}
	    }
	}

	
	/**
	 * \copydoc Application::load()
	 */
	bool load(const std::string& filename) override {
	    if(FileSystem::is_file(filename)) {
		text_editor_.load(filename);
		current_file_ = filename;
		text_editor_visible_ = true;
		return true;
	    } else if(builtin_files_->is_file(filename)) {
		const char* data = builtin_files_->get_file_contents(filename);
		text_editor_.load_data(data);
		current_file_ = "";
		text_editor_visible_ = true;
		return true;
	    }
	    return false;
	}

	/**
	 * \copydoc Application::save()
	 */
	bool save(const std::string& filename) override {
	    text_editor_.save(filename);
	    current_file_ = filename;
	    return true;
	}

	/**
	 * \copydoc Application::supported_read_file_extensions()
	 */
	std::string supported_read_file_extensions() override {
	    return "glsl";
	}

	/**
	 * \copydoc Application::supported_write_file_extensions()
	 */
	std::string supported_write_file_extensions() override {
	    return "glsl";
	}

    protected:
	void draw_viewer_properties() override {
	    if(ImGui::Button(
		   (icon_UTF8("play-circle")+" run").c_str(),
		   ImVec2(-ImGui::GetContentRegionAvail().x/1.8f,0.0f))
	    ) {
		run_program();
	    }
	    ImGui::SameLine();
	    if(ImGui::Button(
		   (icon_UTF8("stop-circle")+" stop").c_str(),
		   ImVec2(-1.0f, 0.0f))
	    ) {
		stop_program();
	    }
            ImGui::Separator();
	    if(ImGui::Button(
		   (icon_UTF8("home") + " Home").c_str(), ImVec2(-1.0, 0.0))
	    ) {
		home();
	    }
	}

    private:
	GLuint glsl_program_;
	int glsl_frame_;
	double glsl_start_time_;
	SmartPointer<FileSystem::MemoryNode> builtin_files_;
    };
      
}

int main(int argc, char** argv) {
    GeoShadeApplication app;
    app.start(argc, argv);
    return 0;
}
