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

// A dummy GLUP application

#include <geogram_gfx/glup_viewer/glup_viewer_gui.h>

namespace {
    using namespace GEO;

    class DemoGlupApplication : public GEO::Application {
    public:
        DemoGlupApplication(int argc, char** argv) :
            Application(argc, argv, "") {
        }

        /**
         * \brief Draws the application menus.
         * \details This function overloads 
         *  Application::draw_application_menus(). It can be used to create
         *  additional menus in the main menu bar.
         */
        virtual void draw_application_menus() {
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
                        &DemoGlupApplication::say_hello
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
                        &DemoGlupApplication::compute
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
        
    };
}

int main(int argc, char** argv) {
    DemoGlupApplication app(argc, argv);
    app.start();
    return 0;
}

