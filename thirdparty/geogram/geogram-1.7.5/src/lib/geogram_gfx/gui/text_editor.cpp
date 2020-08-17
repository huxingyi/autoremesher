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

#include <geogram_gfx/gui/text_editor.h>
#include <geogram_gfx/gui/application.h>
#include <geogram/basic/string.h>
#include <fstream>

namespace GEO {

    TextEditor::TextEditor(bool* visible) : visible_(visible) {
	impl_.SetText("\n");
	impl_.SetCursorPosition(
	    ::TextEditor::Coordinates(0,0)
	);
	impl_.SetLanguageDefinition(
	    ::TextEditor::LanguageDefinition::Lua()
	);
	impl_.SetPalette(::TextEditor::GetDarkPalette());
	fixed_layout_ = true;
    }

    std::string TextEditor::text() const {
	return impl_.GetText();
    }
    
    void TextEditor::draw() {
	ImGui::Begin(
	    "Text Editor", visible_,
	    fixed_layout_ ? (
		ImGuiWindowFlags_NoResize |
		ImGuiWindowFlags_NoMove |
		ImGuiWindowFlags_NoCollapse
	    ) : 0
	);

	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);	    

	if(Application::instance() != nullptr) {
	    if(
		String::string_starts_with(
		    Application::instance()->get_style(),
		    "Light"
	        )
	    ) {
		impl_.SetPalette(::TextEditor::GetLightPalette());	    
	    } else {
		impl_.SetPalette(::TextEditor::GetDarkPalette());
	    }
	}
	
	impl_.Render("##source");
	
	ImGui::PopFont();
	
	ImGui::End();
    }

    void TextEditor::load(const std::string& filename) {
	std::ifstream in(filename.c_str());
	std::string text;
	std::string line;
	while(std::getline(in,line)) {
	    text += line;
	    text += "\n";
	}
	impl_.SetText(text);
	impl_.SetCursorPosition(
	    ::TextEditor::Coordinates(0,0)
	);
    }

    void TextEditor::save(const std::string& filename) {
	std::ofstream out(filename.c_str());
	out << impl_.GetText();
    }

    void TextEditor::clear() {
	impl_.SetText("\n");
	impl_.SetCursorPosition(
	    ::TextEditor::Coordinates(0,0)
	);
    }

    void TextEditor::load_data(const char* data) {
	impl_.SetText(data);
	impl_.SetCursorPosition(
	    ::TextEditor::Coordinates(0,0)
	);
    }
    
}
