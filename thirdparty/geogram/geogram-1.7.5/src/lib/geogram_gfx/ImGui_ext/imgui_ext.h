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

#ifndef GEOGRAM_GFX_IMGUI_EXT
#define GEOGRAM_GFX_IMGUI_EXT

#include <geogram_gfx/basic/common.h>
#include <string>

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunknown-warning-option"
#pragma GCC diagnostic ignored "-Wzero-as-null-pointer-constant"
#endif

#include <geogram_gfx/third_party/ImGui/imgui.h>

#ifdef GEO_COMPILER_CLANG
#pragma GCC diagnostic pop
#endif


/**
 * \file geogram_gfx/ImGui_ext/imgui_ext.h
 * \brief Extension functions for ImGui.
 */

namespace GEO {
    /**
     * \brief Maximum string length for ImGUI.
     * TODO replace with ImGui functions to handle dynamic buffer with
     *  InputText().
     */
    enum { geo_imgui_string_length = 4096 };

    namespace FileSystem {
	class Node;
    }
}

typedef int ImGuiExtFileDialogFlags;


enum ImGuiExtFileDialogFlags_ {
    ImGuiExtFileDialogFlags_Load  = 1,
    ImGuiExtFileDialogFlags_Save  = 2
};


namespace ImGui {

    /**
     * \brief Gets the global application scaling.
     * \details The global application scaling can be changed
     *   for high DPI displays. Some GUI elements need to
     *   be scaled accordingly.
     * \return The global application scaling.
     */
    float GEOGRAM_GFX_API scaling();

    /**
     * \brief Sets the global application scaling.
     * \details The global application scaling can be changed
     *   for high DPI displays. Some GUI elements need to
     *   be scaled accordingly.
     * \param x The global application scaling.
     */
    void GEOGRAM_GFX_API set_scaling(float x);
    
    /**
     * \brief Manages the GUI of a color editor.
     * \details This creates a custom dialog with the color editor and
     *  a default palette, as in ImGUI example.
     * \param[in] label the label of the widget, passed to ImGUI
     * \param[in,out] color a pointer to an array of 3 floats
     * \retval true if the color was changed
     * \retval false otherwise
     */
    bool GEOGRAM_GFX_API ColorEdit3WithPalette(
	const char* label, float* color
    );

    /**
     * \brief Manages the GUI of a color editor.
     * \details This creates a custom dialog with the color editor and
     *  a default palette, as in ImGUI example.
     * \param[in] label the label of the widget, passed to ImGUI
     * \param[in,out] color a pointer to an array of 4 floats
     * \retval true if the color was changed
     * \retval false otherwise
     */
    bool GEOGRAM_GFX_API ColorEdit4WithPalette(
	const char* label, float* color
    );

    /**
     * \brief Opens a file dialog.
     * \param[in] label the window label of the file dialog
     * \param[in] extensions semi-colon-separated list of extensions, without
     *  the dot
     * \param[in] filename initial filename or empty string
     * \param[in] root an optional root Node. If null or unspecified, then
     *  the default root is used.
     * \details The file dialog is drawn and handled after, by calling
     *  FileDialog()
     */
    void GEOGRAM_GFX_API OpenFileDialog(
	const char* label,
	const char* extensions,
	const char* filename,
	ImGuiExtFileDialogFlags flags,
	GEO::FileSystem::Node* root = nullptr
    );

    /**
     * \brief Draws a FileDialog.
     * \details If OpenFileDialog() was called before, then the dialog is drawn,
     *  otherwise it is ignored.
     * \param[in] label the window label of the file dialog
     * \param[in,out] filename the file to be read
     * \param[in] filename_buff_len the size of the buffer pointed by filename
     * \retval true if a file was selected
     * \retval false otherwise
     */
    bool GEOGRAM_GFX_API FileDialog(
	const char* label,
	char* filename, size_t filename_buff_len
    );

    /**
     * \brief Adapter for ImGui::MenuItem() for std::string.
     */
    inline bool MenuItem(
	const std::string& name, const char* shortcut,
	bool* p_selected = nullptr, bool enabled = true
    ) {
	return ImGui::MenuItem(name.c_str(), shortcut, p_selected, enabled);
    }

    /**
     * \brief Adapter for ImGui::MenuItem() for std::string.
     */
    inline bool MenuItem(
	const std::string& name, const char* shortcut = nullptr,
	bool selected = false, bool enabled = true
    ) {
	return ImGui::MenuItem(name.c_str(), shortcut, selected, enabled);
    }
    
    /**
     * \brief Adapter for ImGui::BeginMenu() for std::string.
     */
    inline bool BeginMenu(const std::string& name) {
	return ImGui::BeginMenu(name.c_str());
    }

    /**
     * \brief Adapter for ImGui::Button() for std::string.
     */
    inline bool Button(const std::string& name) {
	return ImGui::Button(name.c_str());
    }

    /**
     * \brief Displays a tooltip.
     * \details The tooltip is displayed if the previous item is hovered,
     *  \p str is non-null and tooltips are enabled.
     * \param[in] str the tooltip to be displayed.
     * \see EnableTooltips(), DisableToolTips()
     */
    void GEOGRAM_GFX_API Tooltip(const char* str);

    /**
     * \brief Displays a tooltip.
     * \details The tooltip is displayed if the previous item is hovered,
     *  \p str is non-null and tooltips are enabled.
     * \param[in] s the tooltip to be displayed.
     * \see EnableTooltips(), DisableToolTips()
     */
    inline void Tooltip(const std::string& s) {
	Tooltip(s.c_str());
    }
    
    /**
     * \brief Enables tooltips.
     * \see ToolTip()
     */
    void GEOGRAM_GFX_API EnableTooltips();

    /**
     * \brief Disables tooltips.
     * \see ToolTip()
     */
    void GEOGRAM_GFX_API DisableTooltips();


    /**
     * \brief Button without border.
     */
    bool GEOGRAM_GFX_API SimpleButton(const char* label);

    /**
     * \brief Button without border.
     */
    bool GEOGRAM_GFX_API SimpleButton(const char* label, const ImVec2& size);

    /**
     * \brief Wrapper for std::string around SimpleButton()
     */
    inline bool SimpleButton(const std::string& label) {
	return SimpleButton(label.c_str());
    }

    /**
     * \brief Draws a text label centered in the current window.
     * \param[in] text the text to be drawn.
     */
    void GEOGRAM_GFX_API CenteredText(const char* text);
}

#endif

