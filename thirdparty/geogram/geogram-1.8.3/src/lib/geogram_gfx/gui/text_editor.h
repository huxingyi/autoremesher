/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
 */

#ifndef H_GEOGRAM_GFX_GUI_TEXT_EDITOR_H
#define H_GEOGRAM_GFX_GUI_TEXT_EDITOR_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/third_party/ImGuiColorTextEdit/TextEditor.h>

/**
 * \file geogram_gfx/gui/text_editor.h
 * \brief A simple text editor.
 */

namespace GEO {

    /**
     * \brief A minimalistic text editor
     * \details It is the text editor used by geocod. Note that Graphite uses
     *  a more elaborate text editor, not based on this class.
     */
    class GEOGRAM_GFX_API TextEditor {
    public:
	TextEditor(bool* visible);
	void draw();
	std::string text() const;
	void load(const std::string& filename);
	void save(const std::string& filename);
	void clear();
	void load_data(const char* data);
        void set_fixed_layout(bool x) {
	   fixed_layout_ = x;
	}
    
    private:
	::TextEditor impl_;
	bool* visible_;
        bool fixed_layout_;
    };
}

#endif
