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

#include <geogram_gfx/gui/application.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram_gfx/GLUP/GLUP.h>

// Demo for the Application class.
// Note: Application is the lowest level access to
//  geogram's application framework. It initializes
//  a window, OpenGL/GLUP and ImGui contexts.
// You can override draw_gui() and draw_graphics() to
//  draw your own stuff.
// Take a look at SimpleApplication / demo_SimpleApplication
//  for an easier-to-use baseclass.

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

    
   class DemoApplication : public GEO::Application {
    public:
	DemoApplication() : GEO::Application(
	    "Geogram Demo App"
      )	{
	    demo_window_visible_ = false;
	    frame_ = 0;
	    // Call 'start_animation()' if you want draw_graphics() to be
	    // called continuously. Without it, draw_graphics() is only called
	    // when there is a gui event.
	    start_animation(); 
      }

    protected:
	void draw_gui() override {
	    // Draw the GUI of your application here, using ImGui.
	    if(demo_window_visible_) {
		ImGui::ShowDemoWindow(&demo_window_visible_);
	    }
	    ImGui::Begin("My window");
	    ImGui::Text("%s","Hello, world");

	    // To learn how to program a GUI element in ImGui, find
	    // it in the demo window, then look it up in the sources
	    // of ImGui (geogram_gfx/third_party/ImGui/imgui_demo.cpp)
	    ImGui::Checkbox("ImGui demo window", &demo_window_visible_);
	    ImGui::Checkbox("Animate", animate_ptr()); // Toggle animation
	    ImGui::End();
	}

	void draw_graphics() override {
	    // Draw the OpenGL graphic part of your application here.
	    Application::draw_graphics();
	    GLsizei L = GLsizei(std::min(get_width(), get_height()));
	    glViewport(0, 0, L, L);
	    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
	    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	    glEnable(GL_DEPTH_TEST);


	    // GLUP has a re-implementation of the old fixed functionality
	    // pipeline (on top of modern OpenGL). It can be used to
	    // create simple geometry. For larger objects, use glupDrawArrays()
	    // and glupDrawElements()
	    glupMatrixMode(GLUP_MODELVIEW_MATRIX);
	    glupLoadIdentity();
	    glupScaled(0.7, 0.7, 0.7);
	    glupRotatef(float(frame_) * 0.3f, 1.0f, 1.0f, 1.0f);	    
	    glupSetColor3d(GLUP_FRONT_AND_BACK_COLOR, 0.0, 0.0, 0.0);
	    glupSetColor3d(GLUP_MESH_COLOR, 1.0, 1.0, 1.0);	    
	    glupEnable(GLUP_DRAW_MESH);
	    glupSetMeshWidth(3);
	    glupDisable(GLUP_LIGHTING);
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

       bool demo_window_visible_;
       index_t frame_;
   };
}

int main(int argc, char** argv) {
    DemoApplication app;
    app.start(argc, argv);
    return 0;
}
