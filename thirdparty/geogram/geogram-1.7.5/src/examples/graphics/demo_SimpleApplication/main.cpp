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

//   Demo for the SimpleApplication class.
// SimpleApplication includes camera controls,
// viewer properties, place holder for object
// properties, console window and built-in text
// editor.
//   Note: there is also a lower-level Application
// class (see demo_Application).

namespace {
   using namespace GEO;

    // Point coordinates of the vertices of an icosahedron
    static double points[] = {
	0,          0.0,       1.175571,
	1.051462,   0.0,       0.5257311,
	0.3249197,  1.0,       0.5257311,
	-0.8506508, 0.618034,  0.5257311,
	-0.8506508, -0.618034, 0.5257311,
	0.3249197,  -1.0,      0.5257311,
	0.8506508,  0.618034,  -0.5257311,
	0.8506508,  -0.618034, -0.5257311,
	-0.3249197,  1.0,      -0.5257311,
	-1.051462,   0.0,      -0.5257311,
	-0.3249197, -1.0,      -0.5257311,
	0.0,         0.0,      -1.175571
    };

    // Facets of an icosahedron
    static index_t facets[] = {
	0,1,2,
	0,2,3,
	0,3,4,
	0,4,5,
	0,5,1,
	1,5,7,
	1,7,6,
	1,6,2,
	2,6,8,
	2,8,3,
	3,8,9,
	3,9,4,
	4,9,10,
	4,10,5,
	5,10,7,
	6,7,11,
	6,11,8,
	7,10,11,
	8,11,9,
	9,11,10,
    };
    
   class DemoApplication : public GEO::SimpleApplication {
    public:
	DemoApplication() : GEO::SimpleApplication(
	    "Geogram Demo App"
      )	{
	    demo_window_visible_ = false;
	    frame_ = 0;
	    set_region_of_interest(-1.0, -1.0, -1.0, 1.0, 1.0, 1.0);
	    surface_color_ =   vec4f(0.0f, 0.0f, 0.0f, 1.0f);
	    show_mesh_ = true;
	    mesh_color_ = vec4f(1.0f, 1.0f, 1.0f, 1.0f);
	    mesh_width_ = 0.2f;
	    add_key_toggle("m", &show_mesh_);
	    lighting_ = false;
	    use_text_editor_ = true; // Activate built-in text editor.
      }

    protected:
       
        // You can add elements in the GUI by overriding
        // the following functions.

       
       void draw_gui() override {
	   SimpleApplication::draw_gui();
	   if(demo_window_visible_) {
	       ImGui::ShowDemoWindow(&demo_window_visible_);
	   }
       }
       
       void draw_object_properties() override {
	   SimpleApplication::draw_object_properties();
	   ImGui::ColorEdit3WithPalette(
	       "Surface", surface_color_.data()
	   );
	   ImGui::Checkbox("##MeshOnOff", &show_mesh_);
	   ImGui::SameLine();
	   ImGui::ColorEdit3WithPalette("mesh [m]", mesh_color_.data());
	   if(show_mesh_) {
	       ImGui::SliderFloat(
		   "wid.", &mesh_width_, 0.1f, 2.0f, "%.1f"
	       );
	   }
       }
       
       void draw_windows_menu() override {
	   ImGui::MenuItem(
	       "ImGui Demo", nullptr, &demo_window_visible_
	   );
	   SimpleApplication::draw_windows_menu();
       }

       /**
	* \brief Draws the application menus.
	* \details This function overloads 
	*  Application::draw_application_menus(). It can be used to create
	*  additional menus in the main menu bar.
	*/
       void draw_application_menus() override {
	   if(ImGui::BeginMenu("Commands")) {
	       if(ImGui::MenuItem("say hello")) {
		   // Command::set_current() creates the dialog box and
		   // invokes a function when the "apply" button of the
		   // dialog box is pushed.
		   //   It needs the prototype of the function as a string,
		   // to have argument names and default values.
		   //   If prototype is not specified, then default values are
		   // set to zero and argument names are arg0,arg1...argn.
		   //   The two other arguments are a pointer to an object and
		   // the pointer to the member function to be invoked (it is
		   // also possible to give a pointer to a global static
		   // function).
		   Command::set_current(
		       "say_hello(index_t nb_times=1)", 
		       this,
		       &DemoApplication::say_hello
		   );
	       }

	       if(ImGui::MenuItem("compute")) {
		   // Another example of Command::set_current() with more
		   // information in the function prototype string: each
		   // argument and the function can have a tooltip text,
		   // specified between square brackets.
		   Command::set_current(
		       "compute("
		       "   index_t nb_iter=300 [number of iterations]"
		       ") [pretends to compute something]",
		       this,
		       &DemoApplication::compute
		   );
	       }
	       ImGui::EndMenu();
	   }
       }

       /**
	* \brief An example function invoked from a menu.
	* \see draw_application_menus()
	*/
       void say_hello(index_t nb_times) {
	   show_console();
	   for(index_t i=1; i<=nb_times; ++i) {
	       Logger::out("MyApp") << i << ": Hello, world" << std::endl;
	   }
       }

       /**
	* \brief An example function invoked from a menu.
	* \see draw_application_menus()
	*/
       void compute(index_t nb_iter) {
	   // Create a progress bar
	   ProgressTask progress("Computing", nb_iter);
	   try {
	       for(index_t i=0; i<nb_iter; ++i) {
		   // Insert code here to do the actual computation
		   // Update the progress bar.
		   progress.next();
	       }
	   } catch(TaskCanceled& ) {
	       // This block is executed if the user pushes the "cancel"
	       // button.
	       show_console();
	       Logger::out("Compute") << "Task was canceled by the user"
				      << std::endl;
	   }
       }
        
       
       // Draw the OpenGL graphic part of your application here.       
       void draw_scene() override {
	   // GLUP has a re-implementation of the old fixed functionality
	   // pipeline (on top of modern OpenGL). It can be used to
	   // create simple geometry. For larger objects, use glupDrawArrays()
	   // and glupDrawElements()
	    
	   glupRotatef(float(frame_) * 0.3f, 1.0f, 1.0f, 1.0f);	    
	   glupSetColor3fv(GLUP_FRONT_AND_BACK_COLOR, surface_color_.data());
	   if(show_mesh_) {
	       glupSetColor3fv(GLUP_MESH_COLOR, mesh_color_.data());	    
	       glupEnable(GLUP_DRAW_MESH);
	       glupSetMeshWidth(GLUPint(10.0f*mesh_width_));
	   } else {
	       glupDisable(GLUP_DRAW_MESH);		
	   }
	   glupBegin(GLUP_TRIANGLES);
	   for(index_t i=0; i<sizeof(facets) / sizeof(index_t); ++i) {
	       index_t j = facets[i];
	       glupVertex3dv(&points[3*j]);
	   }
	   glupEnd();
	   if(animate()) {
	       ++frame_;
	   }
       }

       // Indicate here the list of file extensions that can
       // be loaded. Set it to "" to disable the load menu entry
       // and dialog.
       std::string supported_read_file_extensions() override {
	   return "txt;lua;glsl";
       }

       // Indicate here the list of file extensions that can
       // be saved. Set it to "" to disable the load menu entry
       // and dialog.
       std::string supported_write_file_extensions() override {
	   return "txt;lua;glsl";
       }

       // Override this function for loading a file.
       // This example: loads the file in the text editor
       // and shows the text editor.
       bool load(const std::string& filename) override {
	   text_editor().load(filename);
	   show_text_editor();
	   return true;
       }

       // Override this function for saving a file.
       bool save(const std::string& filename) override {
	   text_editor().save(filename);
	   return true;
       }
       
   private:
       bool demo_window_visible_;
       index_t frame_;
       vec4f surface_color_;
       bool show_mesh_;
       vec4f mesh_color_;
       float mesh_width_;
   };
}

int main(int argc, char** argv) {
    DemoApplication app;
    app.start(argc, argv);
    return 0;
}
