/*
 *  Copyright (c) 2012-2014, Bruno Levy All rights reserved.
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

/*
 * GEOGRAM example program:
 * simple mesh raytracing using AABB tree.
 * GUI version of geogram/simple_raytrace
 * disclaimer: not the fastest, lighting model is ridiculous, there is no
 *  antialiasing etc... (just a demo program for the mesh AABB class).
 * mouse interaction is nearly unusable...
 * usage: geogram_demo_Raytrace
 *        geogram_demo_Raytrace meshfile.(obj|ply|mesh|...)
 */ 

#include <geogram_gfx/gui/simple_application.h>
#define RAYTRACE_GUI
#include "../../geogram/simple_raytrace/raytracing.h"
#include <geogram/basic/stopwatch.h>

namespace {
    using namespace GEO;

// Some phones (OpenGL ES) only support GL_RGBA as
// internal format and also *image* format.
#if defined(GEO_OS_ANDROID) || defined(GEO_OS_EMSCRIPTEN)
    const GLenum image_format = GL_RGBA;
#else
    const GLenum image_format = GL_RGB;
#endif

#if defined(GEO_OS_EMSCRIPTEN) && !defined(GEO_WEBGL2)
   const GLenum internal_image_format = GL_RGBA;
#else   
   const GLenum internal_image_format = GL_RGBA8;
#endif
    
    /**
     * \brief An application that demonstrates both
     *  GLUP primitives and glup_viewer application
     *  framework.
     */
    class DemoRaytraceApplication : public SimpleApplication {
    public:

        /**
         * \brief DemoRaytraceApplication constructor.
         */
        DemoRaytraceApplication() :
	    SimpleApplication("RayTrace"),
    	    camera_(
		get_width(), get_height(),
		(image_format == GL_RGBA) ? 4 : 3
	    )
	{
            texture_ = 0;
	    total_time_ = 0.0;
	    frames_ = 0;
	    
            // The tradition !	    
	    scene_.add_object(new HorizontalCheckerboardPlane(0.0))  
   	          ->rename("Checkerboard");
	    
	    scene_.add_object(                         // A sphere
		new Sphere(vec3(-0.7, -0.7, 1.0),0.7)
		)->set_reflection_coefficient(vec3(0.8, 0.8, 0.8))
		 ->set_diffuse_coefficient(vec3(0.2, 0.2, 0.2))
		 ->rename("Sphere 1");


	    scene_.add_object(                         // Another sphere
		new Sphere(vec3( 0.0, 0.0, 0.0),0.25)
	    )->set_diffuse_coefficient(vec3(0.0, 1.0, 0.5))
	     ->rename("Sphere 2");		

	    
	    // Let there be (two) lights !
	    scene_.add_object(new Light(
				 vec3(0.5, 1.3, 0.5), // Position
				 0.02,                // Radius
				 vec3(0.0, 0.5, 1.0)  // Color
				 )
	    )->rename("Light 1");
	    
	    scene_.add_object(new Light(
				 vec3(1.0, 0.2, 1.0), // Position
				 0.02,                // Radius
				 vec3(1.0, 1.0, 1.0)  // Color
			         )
	    )->rename("Light 2");
	    
	    scene_changed_ = true;
	    set_region_of_interest(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);

	    FOR(i,4) {
		my_viewport_[i] = 0.0;
	    }
        }

        /**
         * \brief DemoRaytraceApplication destructor.
         */
        ~DemoRaytraceApplication() override {
        }


	/**
	 * \copydoc SimpleApplication::GL_terminate()
	 */
	void GL_terminate() override {
            if(texture_ != 0) {
                glDeleteTextures(1,&texture_);
		texture_ = 0;
            }
	    SimpleApplication::GL_terminate();
	}

	/**
	 * \brief Ray-traces a new frame.
	 * \details Only if scene or viewing parameters
	 *  changed.
	 */
	void raytrace() {
	    if(!scene_changed_) {
		return;
	    }
	    {
		double t0 = SystemStopwatch::now();
		parallel_for(
		    0, camera_.image_height(),
		    [this](index_t Y) {
			for(index_t X=0; X<camera_.image_width(); ++X) {
			    Ray R = primary_ray(X,Y);
			    vec3 K = scene_.raytrace(R);
			    camera_.set_pixel(X,Y,K);
			}
		    }
		);
		total_time_ += (SystemStopwatch::now() - t0);
		++frames_;
	    }
	    if(texture_ != 0) {
		GEO_CHECK_GL();
		glActiveTexture(GL_TEXTURE0);
		GEO_CHECK_GL();		
		glBindTexture(GL_TEXTURE_2D, texture_);
		GEO_CHECK_GL();				
		glTexImage2D(
		    GL_TEXTURE_2D,
		    0,
		    internal_image_format,
		    GLsizei(camera_.image_width()),
		    GLsizei(camera_.image_height()),
		    0,
		    image_format,
		    GL_UNSIGNED_BYTE,
		    camera_.image_data()
		);
		GEO_CHECK_GL();						
		glBindTexture(GL_TEXTURE_2D, 0);
		GEO_CHECK_GL();						
	    }
	    scene_changed_ = false;
	}

	
        /**
         * \brief Displays and handles the GUI for object properties.
         * \details Overloads Application::draw_object_properties().
         */
        void draw_object_properties() override {
	    SimpleApplication::draw_object_properties();	    
	    if(ImGui::Button(
		   (icon_UTF8("home") + " Home [H]").c_str(), ImVec2(-1.0, 0.0))
	    ) {
		home();
	    }
	    double fps = double(frames_) / total_time_;
	    int ifps = int(fps);
	    int ffps = int((fps - double(ifps))*100.0);
	    ImGui::Text(
		"%s", (
		    String::to_string(ifps) +"." +
		    String::to_string(ffps) + " FPS"
		).c_str()
	    );
	    if(scene_.draw_gui()) {
		scene_changed_ = true;
	    }
        }


	/**
	 * \brief Launches a primary ray.
	 * \param[in] X , Y the pixel coordinates,
	 *   in [0..width-1] x [0..height-1]
	 */
	Ray primary_ray(index_t X, index_t Y) {
	    double x = double(X);
	    double y = double(Y);
	    vec4 nnear(
		2.0*((x - my_viewport_[0]) / my_viewport_[2]-0.5),
		2.0*((y - my_viewport_[1]) / my_viewport_[3]-0.5),
		-1.0,
		1.0
	    );
	    vec4 ffar = nnear; // 'far' is reserved under Win32 !
	    ffar.z = 1.0;
	    nnear = mult(inv_project_modelview_,nnear);
	    ffar =  mult(inv_project_modelview_,ffar);
	    vec3 nearp = (1.0/nnear.w)*vec3(nnear.x, nnear.y, nnear.z);
	    vec3 farp  = (1.0/ffar.w)*vec3(ffar.x , ffar.y , ffar.z );
	    return Ray(nearp, farp-nearp);
	}

	/**
	 * \brief Gets the viewing parameters from OpenGL and GLUP
	 *  in a form suitable for software raytracing.
	 */
	void get_viewing_parameters() {
	    GLint viewport[4];
	    glGetIntegerv(GL_VIEWPORT, viewport);
	    FOR(i,4) {
		if(my_viewport_[i] != double(viewport[i])) {
		    scene_changed_ = true;
		}
		my_viewport_[i] = double(viewport[i]);
	    }
	
	    mat4 modelview;
	    glupGetMatrixdv(GLUP_MODELVIEW_MATRIX, modelview.data());
	    mat3 normalmatrix;
	    FOR(i,3) {
		FOR(j,3) {
		    normalmatrix(i,j) = modelview(i,j);
		}
	    }
	    modelview = modelview.transpose();
	    mat4 project;
	    glupGetMatrixdv(GLUP_PROJECTION_MATRIX, project.data());
	    project = project.transpose();
	    
	    mat4 inv_project_modelview = (project*modelview).inverse();
	    FOR(i,4) {
		FOR(j, 4) {
		    if(inv_project_modelview(i,j) !=
		       inv_project_modelview_(i,j))  {
			scene_changed_ = true;
		    }
		}
	    }
		 
	    inv_project_modelview_ = inv_project_modelview;
	    
	    float Lf[3];
	    glupGetLightVector3fv(Lf);
	    
	    vec3 L((double)Lf[0], (double)Lf[1], (double)Lf[2]);
	    L = normalize(mult(normalmatrix,L));
	    
	    if(L_.x != L.x || L_.y != L.y || L_.z != L.z) {
		scene_changed_ = true;
	    }
	    L_ = L;
	}
	
        /**
         * \brief Draws the scene according to currently set primitive and
         *  drawing modes.
         */
         void draw_scene() override {
	    get_viewing_parameters();
	    
	    // OpenGL does not like textures dimensions that
	    // are not multiples of 4
	    index_t w = get_width() & index_t(~3);
	    index_t h = get_height() & index_t(~3);
	    
	    if(camera_.image_width() != w ||
	       camera_.image_height() != h) {
		camera_.resize(w,h);
		scene_changed_ = true;
	    }
	    raytrace();
	    glViewport(
		0, 0,
		GLsizei(camera_.image_width()),
		GLsizei(camera_.image_height())
	    );
	    glDisable(GL_DEPTH_TEST);
	    GEO_CHECK_GL();
	    glActiveTexture(GL_TEXTURE0); 
	    glBindTexture(GL_TEXTURE_2D, texture_);
	    GEO_CHECK_GL();
	    draw_unit_textured_quad();
	    glBindTexture(GL_TEXTURE_2D, 0);
	    GEO_CHECK_GL();
        }

        /**
         * \brief Creates the texture.
         * \details This function overloads Application::init_graphics(). It
         *  is called as soon as the OpenGL context is ready for rendering. It
         *  is meant to initialize the graphic objects used by the application.
         */
        void GL_initialize() override {
            SimpleApplication::GL_initialize();
            glGenTextures(1, &texture_);
	    GEO_CHECK_GL();
            glActiveTexture(GL_TEXTURE0); 
            glBindTexture(GL_TEXTURE_2D, texture_);
	    GEO_CHECK_GL();
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	    GEO_CHECK_GL();
        }

	bool load(const std::string& filename) override {
	    mesh_load(filename, mesh_);
	    normalize_mesh(mesh_);
	    scene_.add_object(new MeshObject(mesh_));
	    scene_changed_ = true;
	    return true;
	}

        void draw_application_menus() override {
            if(ImGui::BeginMenu("New...")) {
		if(ImGui::MenuItem("Sphere")) {
		    scene_.add_object(                                       
			new Sphere(vec3(0.5, 0.5, 0.5),0.25)
		    )->set_diffuse_coefficient(random_color());
		    scene_changed_ = true;
		}
		if(ImGui::MenuItem("Light")) {
		    scene_.add_object(                                       
			new Light(vec3(0.5, 0.5, 1.5),0.15,random_color())
		    );
		    scene_changed_ = true;
		}
		if(ImGui::MenuItem("Checkerboard")) {
		    scene_.add_object(new HorizontalCheckerboardPlane(0.0));
		    scene_changed_ = true;		    
		}
		ImGui::EndMenu();
	    }
	}

	void geogram_initialize(int argc, char** argv) override {
	    SimpleApplication::geogram_initialize(argc, argv);
	    viewer_properties_visible_ = false;
	    object_properties_visible_ = true;
	}
	
    private:
	Camera camera_;
	Scene scene_;
	Mesh mesh_;
        GLuint texture_;
	bool scene_changed_;
	double total_time_;
	index_t frames_;

	double my_viewport_[4];
	mat4 inv_project_modelview_;
	vec3 L_; /**< light vector in object space. */
    };
      
}

int main(int argc, char** argv) {
    DemoRaytraceApplication app;
    app.start(argc, argv);
    return 0;
}
