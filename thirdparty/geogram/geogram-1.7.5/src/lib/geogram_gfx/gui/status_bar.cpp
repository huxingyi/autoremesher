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

#include <geogram_gfx/gui/status_bar.h>
#include <geogram_gfx/gui/application.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>
#include <geogram/basic/string.h>

namespace GEO {
    StatusBar::StatusBar() {
        step_ = 0;
        percent_ = 0;
        progress_ = false;
        canceled_ = false;
        nb_active_ = 0;
	height_ = 0.0f;
    }
    
    void StatusBar::begin() {
        progress_ = true;
        canceled_ = false;
        ++nb_active_;
    }

    void StatusBar::progress(GEO::index_t step, GEO::index_t percent) {
        step_ = step;
        percent_ = percent;
	update();
    }

    void StatusBar::end(bool canceled) {
        geo_argused(canceled);
        step_ = 0;
        percent_ = 0;
        progress_ = false;
        --nb_active_;
    }

    void StatusBar::draw() {
        ImGui::Begin(
            "##Status", nullptr,
            ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoScrollbar	    
        );
        if(progress_) {
// "Cancel" button does not work for now under Android
// (to be investigated...)
#ifndef GEO_OS_ANDROID	    
            if(ImGui::SimpleButton(icon_UTF8("window-close"))) {
                Progress::cancel();
            }
            ImGui::SameLine();
#endif	    
            ImGui::Text(
                "%s", Progress::current_task()->task_name().c_str()
            );
            ImGui::SameLine();
            std::string overlay =
                String::to_string(step_) + "/" +
                String::to_string(
                    Progress::current_task()->max_steps()
                ) + " (" + 
                String::to_string(percent_) +
                "%)";
            
            ImGui::ProgressBar(
                std::max(0.001f, float(percent_)/float(100.0)),
                ImVec2(-1,0.0),
                overlay.c_str()
            );
        }
	height_ = ImGui::GetFrameHeight();
        ImGui::End();
    }

    void StatusBar::update() {
	if(Application::instance() != nullptr) {
	    Application::instance()->draw();
	}
    }
}
