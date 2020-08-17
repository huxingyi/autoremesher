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

#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>
#include <geogram_gfx/third_party/ImGui/imgui.h>
#include <geogram_gfx/third_party/ImGui/imgui_internal.h>
#include <geogram/basic/string.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/command_line.h>
#include <map>

namespace {
    using namespace GEO;
    
    bool initialized = false;
    bool tooltips_enabled = true;

    /**
     * \brief Manages the GUI of a color editor.
     * \details This creates a custom dialog with the color editor and
     *  a default palette, as in ImGUI example.
     * \param[in] label the label of the widget, passed to ImGUI
     * \param[in,out] color_in a pointer to an array of 3 floats if 
     *  with_alpha is false or 4 floats if with_alpha is true
     * \param[in] with_alpha true if transparency is edited, false otherwise
     * \retval true if the color was changed
     * \retval false otherwise
     */
    bool ColorEdit3or4WithPalette(
	const char* label, float* color_in, bool with_alpha
    ) {
	bool result = false;
	static bool saved_palette_initialized = false;
	static ImVec4 saved_palette[40];
	static ImVec4 backup_color;
	ImGui::PushID(label);
	int flags =
	    ImGuiColorEditFlags_PickerHueWheel |
	    ImGuiColorEditFlags_Float;

	if(!with_alpha) {
	    flags |= ImGuiColorEditFlags_NoAlpha ; 
	}
	
	ImVec4& color = *(ImVec4*)color_in;

	if (!saved_palette_initialized) {

	    for (int n = 0; n < 8; n++) {
		saved_palette[n].x = 0.0f;
		saved_palette[n].y = 0.0f;
		saved_palette[n].z = 0.0f;
	    }

	    saved_palette[0] = ImVec4(0.0f, 0.0f, 0.0f, 1.0f);
	    saved_palette[1] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
	    saved_palette[2] = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
	    saved_palette[3] = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
	    saved_palette[4] = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
	    saved_palette[5] = ImVec4(0.0f, 0.0f, 1.0f, 1.0f);
	    saved_palette[6] = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
	    saved_palette[7] = ImVec4(0.0f, 1.0f, 1.0f, 1.0f);	    
	    
	    for (int n = 0; n < 32; n++) {
		ImGui::ColorConvertHSVtoRGB(
		    float(n) / 31.0f, 0.8f, 0.8f,
		    saved_palette[n+8].x,
		    saved_palette[n+8].y,
		    saved_palette[n+8].z
		);
	    }
	    saved_palette_initialized = true;
	}
	
	bool open_popup = ImGui::ColorButton(label, color, flags);
	
	if(label[0] != '#') {
	    ImGui::SameLine();	    
	    ImGui::Text("%s",label);
	}
	if (open_popup) {
	    ImGui::OpenPopup("##PickerPopup");
	    backup_color = color;
	}
	if (ImGui::BeginPopup("##PickerPopup")) {
	    if(label[0] != '#') {
		ImGui::Text("%s",label);
		ImGui::Separator();
	    }
	    if(ImGui::ColorPicker4(
		"##picker", (float*)&color,
		flags | ImGuiColorEditFlags_NoSidePreview
		      | ImGuiColorEditFlags_NoSmallPreview
		   )
	    ) {
		result = true;
	    }
	    ImGui::SameLine();
	    ImGui::BeginGroup();
	    ImGui::Text("Current");
	    ImGui::ColorButton(
		"##current", color,
		ImGuiColorEditFlags_NoPicker |
		ImGuiColorEditFlags_AlphaPreviewHalf,
		ImVec2(60,40)
	    );
	    ImGui::Text("Previous");
	    if (ImGui::ColorButton(
		    "##previous", backup_color,
		    ImGuiColorEditFlags_NoPicker |
		    ImGuiColorEditFlags_AlphaPreviewHalf,
		    ImVec2(60,40))
		) {
		color = backup_color;
		result = true;
	    }
	    ImGui::Separator();
	    ImGui::Text("Palette");

#ifdef GEO_OS_ANDROID
	    int nb_btn_per_row = 4;
	    float btn_size = 35.0;
#else
	    int nb_btn_per_row = 8;
	    float btn_size = 20.0;
#endif
	    for (int n = 0; n < 40; n++) {
		ImGui::PushID(n);
		if ( (n % nb_btn_per_row) != 0 ) {
		    ImGui::SameLine(0.0f, ImGui::GetStyle().ItemSpacing.y);
		}
		if (ImGui::ColorButton(
			"##palette",
			saved_palette[n],
			ImGuiColorEditFlags_NoPicker |
			ImGuiColorEditFlags_NoTooltip,
			ImVec2(btn_size,btn_size))
		) {
		    color = ImVec4(
			saved_palette[n].x,
			saved_palette[n].y,
			saved_palette[n].z,
			color.w
		    ); // Preserve alpha!
		    result = true;
		}
		ImGui::PopID();
	    }
	    ImGui::Separator();
	    if(ImGui::Button(
		   "OK", ImVec2(-1,-1)
	       )
	    ) {
		ImGui::CloseCurrentPopup();
	    }
	    ImGui::EndGroup();
	    ImGui::EndPopup();
	}
	ImGui::PopID();
	return result;
    }

    /**************************************************************************/
    
    /**
     * \brief Safer version of strncpy()
     * \param[in] dest a pointer to the destination string
     * \param[in] source a pointer to the source string
     * \param[in] max_dest_size number of characters available in
     *  destination string
     * \return the length of the destination string after copy. If
     *  the source string + null terminator was greater than max_dest_size,
     *  then it is cropped. On exit, dest is always null-terminated (in 
     *  contrast with strncpy()).
     */ 
    size_t safe_strncpy(
        char* dest, const char* source, size_t max_dest_size
    ) {
        strncpy(dest, source, max_dest_size-1);
        dest[max_dest_size-1] = '\0';
        return strlen(dest);
    }

   /**
    * \brief Converts a complete path to a file to a label
    *  displayed in the file browser.
    * \details Strips viewer_path from the input path.
    * \param[in] path the complete path, can be either a directory or
    *  a file
    * \return the label to be displayed in the menu
    */
    std::string path_to_label(
        const std::string& viewer_path, const std::string& path
    ) {
        std::string result = path;
        if(GEO::String::string_starts_with(result, viewer_path)) {
            result = result.substr(
                viewer_path.length(), result.length()-viewer_path.length()
            );
        }
        return result;
    }


    /**
     * \brief Converts an icon symbolic name and a label to a string.
     * \param[in] icon_sym the symbolic name of the icon.
     * \param[in] label the label to be displayed.
     * \param[in] no_label if true, do not display the label
     * \return a UTF8 string with the icon and label, or just the label
     *  if the icon font is not initialized.
     */
    std::string icon_label(
	const char* icon_sym,
	const char* label = nullptr,
	bool no_label = false
    ) {
	wchar_t str[2];
	str[0] = icon_wchar(icon_sym);
	if(str[0] == '\0') {
	    return std::string(label);
	}
	str[1] = '\0';
	if(label == nullptr) {
	    return GEO::String::wchar_to_UTF8(str);
	}
	if(no_label) {
	    return GEO::String::wchar_to_UTF8(str) + "##" + label;
	}
	if(label[0] == '#') {
	    return GEO::String::wchar_to_UTF8(str) + label;
	}
	return GEO::String::wchar_to_UTF8(str) + " " + label;
    }

    /**************************************************************************/

    /**
     * \brief The state for OpenFileDialog() and FileDialog()
     */
    class FileDialog {
    public:

        /**
         * \brief FileDialog constructor.
         * \param[in] save_mode if true, FileDialog is used to create files
         * \param[in] default_filename the default file name used if save_mode
         *  is set
         */
        FileDialog(
            bool save_mode=false,
            const std::string& default_filename="",
	    FileSystem::Node* root = nullptr
        ) : visible_(false),
	    root_(nullptr),
	    current_write_extension_index_(0),        
	    pinned_(false),
	    show_hidden_(false),
	    scroll_to_file_(false),
	    save_mode_(save_mode),
	    are_you_sure_(false)
	{
	    set_root(root);
	    set_default_filename(default_filename);
	    current_file_index_ = 0;
	    current_directory_index_ = 0;
	    current_write_extension_index_ = 0;
	    no_docking_ =
		!CmdLine::get_arg_bool("gui:expert") &&
		!CmdLine::get_arg_bool("gui:phone_screen");
	}


	/**
	 * \brief Sets the root node to be used with the FileDialog.
	 * \param[in] root a pointer to the root node.
	 */
	void set_root(FileSystem::Node* root) {
	    FileSystem::Node* prev_root = root_;
	    root_ = root;
	    if(root_ == nullptr) {
		FileSystem::get_root(root_);
	    }
	    if(prev_root != root_) {
#if defined(GEO_OS_WINDOWS) || defined(GEO_OS_ANDROID)
		directory_ = root_->documents_directory();
#else	
		directory_ = root_->get_current_working_directory();
#endif
		if(
		    directory_ == "" ||
		    directory_[directory_.length()-1] != '/'
		) {
		    directory_ += "/";
		}
	    }
	}
	
	/** 
	 * \brief Sets the default file.
	 * \details Only valid if save_mode is set.
         * \param[in] default_filename the default file name.
	 */	
	void set_default_filename(const std::string& default_filename) {
	    safe_strncpy(
		current_file_, default_filename.c_str(), sizeof(current_file_)
	    );
	}
	
        /**
         * \brief Makes this FileDialog visible.
         */
        void show() {
            update_files();
            visible_ = true;
        }

        /**
         * \brief Makes this FileDialog invisibile.
         */
        void hide() {
            visible_ = false;
        }

        /**
         * \brief Draws the console and handles the gui.
         */
        void draw() {
	    if(!visible_) {
		return;
	    }

	    bool phone_screen = CmdLine::get_arg_bool("gui:phone_screen");

	    if(!phone_screen) {
		ImGui::SetNextWindowSize(
		    ImVec2(ImGui::scaling()*400.0f, ImGui::scaling()*415.0f),
		    ImGuiCond_Once
		);
	    }

	    std::string label = std::string(
		save_mode_ ? "Save as...##" : "Load...##"		
	    );
	    
	    if(!phone_screen) {
		label += String::to_string(this);
	    }
	    
	    ImGui::Begin(
		label.c_str(),
		&visible_,
		ImGuiWindowFlags_NoCollapse | (
		    (no_docking_ && !pinned_) ? ImGuiWindowFlags_NoDocking : 0
		)
	    );

	    float s = ImGui::CalcTextSize(icon_UTF8("thumbtack").c_str()).x;
	    float spacing = 0.15f*s;

	    bool compact = (ImGui::GetContentRegionAvail().x < s*10.0f);
	    
	    if(phone_screen) {
		if(ImGui::SimpleButton(icon_label(
		   "window-close","##file_dialog_close", compact
		))) {
		    visible_ = false;
		}
		ImGui::SameLine();
		ImGui::Dummy(ImVec2(spacing,1.0f));
		ImGui::SameLine();
	    }
	    
	    if(ImGui::SimpleButton(icon_label(
		"arrow-circle-up","parent", compact
	    ))) {
		set_directory("../");
	    }
	    ImGui::SameLine();
	    ImGui::Dummy(ImVec2(spacing,1.0f));
	    ImGui::SameLine();
	    if(ImGui::SimpleButton(icon_label("home","home",compact))) {
		set_directory(root_->documents_directory());
		update_files();
	    }
	    ImGui::SameLine();
	    ImGui::Dummy(ImVec2(spacing,1.0f));
	    ImGui::SameLine();
	    if(ImGui::SimpleButton(icon_label("sync-alt","refresh",compact))) {
		update_files();
	    }

	    if(!save_mode_ && !phone_screen) {
		ImGui::SameLine();        		
		ImGui::Dummy(
		    ImVec2(
			ImGui::GetContentRegionAvail().x - s*1.1f,1.0f
		    )
		);
		ImGui::SameLine();
		if(pinned_) {
		    if(ImGui::SimpleButton(
			   icon_UTF8("dot-circle") + "##pin"
		    )) {
			pinned_ = !pinned_;
		    }
		} else {
		    if(ImGui::SimpleButton(
			   icon_UTF8("thumbtack") + "##pin"
		    )) {
			pinned_ = !pinned_;
		    }
		}
		if(ImGui::IsItemHovered()) {
		    ImGui::SetTooltip("Keeps this dialog open.");
		}
	    }

	    draw_disk_drives();
	    ImGui::Separator();
	    
	    {
		std::vector<std::string> path;
		String::split_string(directory_, '/', path);
		for(index_t i=0; i<path.size(); ++i) {
		    if(i != 0) {
			ImGui::SameLine();
			if(
			    ImGui::GetContentRegionAvail().x <
			    ImGui::CalcTextSize(path[i].c_str()).x +
			    10.0f*ImGui::scaling()
			    ) {
			    ImGui::NewLine();
			}
		    }
		    // We need to generate a unique id, else there is an id
		    // clash with the "home" button right before !!
		    if(ImGui::SimpleButton(
			   (path[i] + "##path" + String::to_string(i))
		    )) {
			std::string new_dir;
			if(path[0].length() >= 2 && path[0][1] == ':') {
			    new_dir = path[0];
			} else {
			    new_dir += "/";
			    new_dir += path[0];
			}
			for(index_t j=1; j<=i; ++j) {
			    new_dir += "/";
			    new_dir += path[j];
			}
			set_directory(new_dir);
		    }
		    ImGui::SameLine();
		    ImGui::Text("/");
		}
	    }

	    const float footer_size =
		phone_screen ? 0.0f : 35.0f*ImGui::scaling();
	    
	    if(phone_screen) {
		draw_footer();
	    }
	    {
		ImGui::BeginChild(
		    "##directories",
		    ImVec2(
			ImGui::GetWindowWidth()*0.5f-10.0f*ImGui::scaling(),
			-footer_size
		    ),
		    true
		);
		for(index_t i=0; i<directories_.size(); ++i) {
		    if(ImGui::Selectable(
			   directories_[i].c_str(),
			   (i == current_directory_index_)
		    )) {
			current_directory_index_ = i;
			set_directory(directories_[current_directory_index_]);
		    }
		}
		ImGui::EndChild();
	    }
	    ImGui::SameLine();
	    {
		ImGui::BeginChild(
		    "##files",
		    ImVec2(
			ImGui::GetWindowWidth()*0.5f-10.0f*ImGui::scaling(),
			-footer_size
		    ),
		    true
		);
		for(index_t i=0; i<files_.size(); ++i) {
		    if(ImGui::Selectable(
			   files_[i].c_str(),
			   (i == current_file_index_)
		    )) {
			safe_strncpy(
			    current_file_,files_[i].c_str(),
			    sizeof(current_file_)
			);
			if(current_file_index_ == i) {
			    file_selected();
			} else {
			    current_file_index_ = i;
			}
		    }
		    if(scroll_to_file_ && i == current_file_index_) {
			ImGui::SetScrollHereY();
			scroll_to_file_ = false;
		    }
		}
		ImGui::EndChild();
	    }
	    if(!phone_screen) {
		draw_footer();
	    }
	    ImGui::End();
	    draw_are_you_sure();
	}

	/**
	 * \brief Sets whether this file dialog is for 
	 *  saving file.
	 * \details If this file dialog is for saving file,
	 *  then the user can enter the name of a non-existing
	 *  file, else he can only select existing files.
	 * \param[in] x true if this file dialog is for
	 *  saving file.
	 */
	void set_save_mode(bool x) {
	    save_mode_ = x;
	}

	/**
	 * \brief Gets the selected file if any and resets it
	 *  to the empty string.
	 * \return the selected file if there is any or the 
	 *  empty string otherwise.
	 */
	std::string get_and_reset_selected_file() {
	    std::string result;
	    std::swap(result,selected_file_);
	    return result;
	}

	/**
	 * \brief Defines the file extensions managed by this 
	 *  FileDialog.
	 * \param[in] extensions a ';'-separated list of extensions
	 */
	void set_extensions(const std::string& extensions) {
	    extensions_.clear();
	    GEO::String::split_string(extensions, ';', extensions_);
	}
	
    protected:

	void draw_footer() { 
	    if(ImGui::Button(
		   save_mode_ ?
		   icon_label("save","Save as").c_str() :
		   icon_label("folder-open","Load").c_str()
 	    )) {
		file_selected();
	    }
	    ImGui::SameLine();
	    ImGui::PushItemWidth(
		save_mode_ ?
		-80.0f*ImGui::scaling() : -5.0f*ImGui::scaling()
		);
	    if(ImGui::InputText(
		   "##filename",
		   current_file_, geo_imgui_string_length,
		   ImGuiInputTextFlags_EnterReturnsTrue    |
		   ImGuiInputTextFlags_CallbackHistory     |
		   ImGuiInputTextFlags_CallbackCompletion ,
		   text_input_callback,
		   this 
		   )
		) {
		scroll_to_file_ = true;
		std::string file = current_file_;
		for(index_t i=0; i<files_.size(); ++i) {
		    if(files_[i] == file) {
			current_file_index_ = i;
		    }
		}
		file_selected();
	    }
	    ImGui::PopItemWidth();
	    // Keep auto focus on the input box
	    if (ImGui::IsItemHovered()) {
		// Auto focus previous widget                
		ImGui::SetKeyboardFocusHere(-1); 
	    }

	    if(save_mode_) {
		ImGui::SameLine();
		ImGui::PushItemWidth(-5.0f*ImGui::scaling());
		
		std::vector<const char*> write_extensions;
		for(index_t i=0; i<extensions_.size(); ++i) {
		    write_extensions.push_back(&extensions_[i][0]);
		}
		if(ImGui::Combo(
		       "##extension",
		       (int*)(&current_write_extension_index_),
		       &write_extensions[0],
		       int(write_extensions.size())
		       )
		    ) {
		    std::string file = current_file_;
		    file = root_->base_name(file) + "." +
			extensions_[current_write_extension_index_];
		    safe_strncpy(
			current_file_, file.c_str(),
			sizeof(current_file_)
		    );
		}
		ImGui::PopItemWidth();
	    }
	}
	
	/**
	 * \brief Tests whether a file can be read.
	 * \param[in] filename the file name to be tested.
	 * \retval true if this file can be read.
	 * \retval false otherwise.
	 */
	bool can_load(const std::string& filename) {
	    if(!root_->is_file(filename)) {
		return false;
	    }
	    std::string ext = root_->extension(filename);
	    for(size_t i=0; i<extensions_.size(); ++i) {
		if(extensions_[i] == ext || extensions_[i] == "*") {
		    return true;
		}
	    }
	    return false;
	}
	
        /**
         * \brief Updates the list of files and directories
         *  displayed by this FileDialog.
         */
        void update_files() {
	    directories_.clear();
	    files_.clear();

	    directories_.push_back("../");
        
	    std::vector<std::string> entries;
	    root_->get_directory_entries(directory_, entries);
	    std::sort(entries.begin(), entries.end());
	    for(index_t i=0; i<entries.size(); ++i) {
		if(can_load(entries[i])) {
		    files_.push_back(path_to_label(directory_,entries[i]));
		} else if(root_->is_directory(entries[i])) {
		    std::string subdir =
			path_to_label(directory_,entries[i]) + "/";
		    if(show_hidden_ || subdir[0] != '.') {
			directories_.push_back(subdir);
		    }
		}
	    }
	    if(current_directory_index_ >= directories_.size()) {
		current_directory_index_ = 0;
	    }
	    if(current_file_index_ >= files_.size()) {
		current_file_index_ = 0;
	    }
	    if(!save_mode_) {
		if(current_file_index_ >= files_.size()) {
		    current_file_[0] = '\0';
		} else {
		    safe_strncpy(
			current_file_,
			files_[current_file_index_].c_str(),
			sizeof(current_file_)
		    );
		}
	    }
	}

        /**
         * \brief Changes the current directory.
         * \param[in] directory either the path relative to the
         *  current directory or an absolute path
         */
        void set_directory(const std::string& directory) {
	    current_directory_index_ = 0;
	    current_file_index_ = 0;
	    if(directory[0] == '/' || directory[1] == ':') {
		directory_ = directory;
	    } else {
		directory_ = root_->normalized_path(
		    directory_ + "/" +
		    directory 
		);
	    }
	    if(directory_[directory_.length()-1] != '/') {
		directory_ += "/";
	    }
	    update_files();
	}

        /**
         * \brief The callback for handling the text input.
         * \param[in,out] data a pointer to the callback data
         */
        static int text_input_callback(ImGuiInputTextCallbackData* data) {
	    FileDialog* dlg = static_cast<FileDialog*>(data->UserData);
	    if(
		(data->EventFlag &
		 ImGuiInputTextFlags_CallbackCompletion) != 0
	    ) {
		dlg->tab_callback(data);
	    } else if(
		(data->EventFlag & ImGuiInputTextFlags_CallbackHistory) != 0
	      ) {
		if(data->EventKey == ImGuiKey_UpArrow) {
		    dlg->updown_callback(data,-1);
		} else if(data->EventKey == ImGuiKey_DownArrow) {
		    dlg->updown_callback(data,1);                
		}
	    } 
	    return 0;
	}

        /**
         * \brief Called whenever the up or down arrows are pressed.
         * \param[in,out] data a pointer to the callback data
         * \param[in] direction -1 if the up arrow was pressed, 1 if the
         *  down arrow was pressed
         */
        void updown_callback(ImGuiInputTextCallbackData* data, int direction) {
	    int next = int(current_file_index_) + direction;
	    if(next < 0) {
		if(files_.size() == 0) {
		    current_file_index_ = 0;
		} else {
		    current_file_index_ = index_t(files_.size()-1);
		}
	    } else if(next >= int(files_.size())) {
		current_file_index_ = 0;
	    } else {
		current_file_index_ = index_t(next);
	    }

	    if(files_.size() == 0) {
		current_file_[0] = '\0';
	    } else {
		safe_strncpy(
		    current_file_,
		    files_[current_file_index_].c_str(),
		    sizeof(current_file_)
		);
	    }
	    update_text_edit_callback_data(data);
	    scroll_to_file_ = true;        
	}

        /**
         * \brief Called whenever the tab key is pressed.
         * \param[in,out] data a pointer to the callback data
         */
        void tab_callback(ImGuiInputTextCallbackData* data) {
	    std::string file(current_file_);
	    bool found = false;
	    for(index_t i=0; i<files_.size(); ++i) {
		if(String::string_starts_with(files_[i],file)) {
		    current_file_index_ = i;
		    found = true;
		    break;
		}
	    }
	    if(found) {
		safe_strncpy(
		    current_file_,
		    files_[current_file_index_].c_str(),
		    sizeof(current_file_)
		);
		update_text_edit_callback_data(data);
		scroll_to_file_ = true;
	    }
	}

        /**
         * \brief Copies the currently selected file into the 
         *  string currently manipulated by InputText.
         * \param[out] data a pointer to the callback data
         */
        void update_text_edit_callback_data(
            ImGuiInputTextCallbackData* data
	) {
	    data->BufTextLen = int(
		safe_strncpy(
		    data->Buf, current_file_, (size_t)data->BufSize
		)
	    );
	    data->CursorPos = data->BufTextLen;
	    data->SelectionStart = data->BufTextLen;
	    data->SelectionEnd = data->BufTextLen;
	    data->BufDirty = true;
	}
        
        /**
         * \brief Called whenever a file is selected.
         * \param[in] force in save_mode, if set, 
         *  overwrites the file even if it already 
         *  exists.
         */
        void file_selected(bool force=false) {
	    std::string file =
		root_->normalized_path(directory_+"/"+current_file_);
        
	    if(save_mode_) {
		if(!force && root_->is_file(file)) {
		    are_you_sure_ = true;
		    return;
		} else {
		    selected_file_ = file;
		}
	    } else {
		selected_file_ = file;
	    }
        
	    if(!pinned_) {
		hide();
	    }
	}

	/**
	 * \brief Handles the "are you sure ?" dialog
	 *  when a file is about to be overwritten.
	 */
        void draw_are_you_sure() {
	    if(are_you_sure_) {
		ImGui::OpenPopup("File exists");
	    }
	    if(
		ImGui::BeginPopupModal(
		    "File exists", nullptr, ImGuiWindowFlags_AlwaysAutoResize
		)
	    ) {
		ImGui::Text(
		    "%s",
		    (std::string("File ") + current_file_ +
		     " already exists\nDo you want to overwrite it ?"
		    ).c_str()
		);
		ImGui::Separator();
		if (ImGui::Button(
			"Overwrite",
			ImVec2(-ImGui::GetContentRegionAvail().x/2.0f,0.0f))
		) {
		    are_you_sure_ = false;
		    ImGui::CloseCurrentPopup();
		    file_selected(true);
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel", ImVec2(-1.0f, 0.0f))) { 
		    are_you_sure_ = false;                
		    ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	    }
	}

	/**
	 * \brief Under Windows, add buttons to change
	 *  disk drive.
	 */
	void draw_disk_drives() {
#ifdef GEO_OS_WINDOWS	
	    DWORD drives = GetLogicalDrives();
	    for(DWORD b=0; b<16; ++b) {
		if((drives & (1u << b)) != 0) {
		    std::string drive;
		    drive += char('A' + char(b));
		    drive += ":";
		    if(ImGui::Button(drive)) {
			set_directory(drive);
		    }
		    ImGui::SameLine();
		    if(
			ImGui::GetContentRegionAvail().x <
			ImGui::CalcTextSize("X:").x + 10.0f*ImGui::scaling()
		    ) {
			ImGui::NewLine();
		    }
		}
	    }
#endif	
	}
	
    private:
        bool visible_;
	FileSystem::Node* root_;
        std::string directory_;
        index_t current_directory_index_;
        index_t current_file_index_;
        std::vector<std::string> directories_;
        std::vector<std::string> files_;
        std::vector<std::string> extensions_;
        index_t current_write_extension_index_;
        char current_file_[geo_imgui_string_length];
        bool pinned_;
        bool show_hidden_;
        bool scroll_to_file_;
        bool save_mode_;
        bool are_you_sure_;
	std::string selected_file_;
	bool no_docking_;
    };

    std::map<std::string, FileDialog*> file_dialogs;
    
    void terminate_imgui_ext() {
	for(auto& it : file_dialogs) {
	    delete it.second;
	}
    }
    
    void initialize_imgui_ext() {
	if(!initialized) {
	    initialized = true;
	    atexit(terminate_imgui_ext);
	}
    }
}

namespace ImGui {

    float scaling() {
	ImGuiContext& g = *GImGui;
	float s = 1.0;
	if(g.Font->FontSize > 40.0f) {
	    s = g.Font->FontSize / 30.0f;
	} else {
	    s = g.Font->FontSize / 20.0f;	    
	}
	return s * ImGui::GetIO().FontGlobalScale;
    }

    void set_scaling(float x) {
	ImGui::GetIO().FontGlobalScale = x;
    }
    
    /*******************************************************************/
    
    bool ColorEdit3WithPalette(const char* label, float* color_in) {
	return ColorEdit3or4WithPalette(label, color_in, false);
    }

    bool ColorEdit4WithPalette(const char* label, float* color_in) {
	return ColorEdit3or4WithPalette(label, color_in, true);	
    }

    /*******************************************************************/
    
    void OpenFileDialog(
	const char* label,
	const char* extensions,
	const char* filename,
	ImGuiExtFileDialogFlags flags,
	FileSystem::Node* root
    ) {
	initialize_imgui_ext();	
	::FileDialog* dlg = nullptr;
	if(file_dialogs.find(label) == file_dialogs.end()) {
	    file_dialogs[label] = new ::FileDialog();
	}
	dlg = file_dialogs[label];
	dlg->set_extensions(extensions); 
	if(flags == ImGuiExtFileDialogFlags_Save) {
	    dlg->set_save_mode(true);
	    dlg->set_default_filename(filename);
	} else {
	    dlg->set_save_mode(false);
	}
	dlg->set_root(root);
	dlg->show();
    }

    bool FileDialog(
	const char* label, char* filename, size_t filename_buff_len
    ) {
	if(file_dialogs.find(label) == file_dialogs.end()) {
	    filename[0] = '\0';
	    return false;
	}
	::FileDialog* dlg = file_dialogs[label];
	dlg->draw();
	
	std::string result = dlg->get_and_reset_selected_file();
	if(result != "") {
	    if(result.length() + 1 >= filename_buff_len) {
		GEO::Logger::err("ImGui_ext") << "filename_buff_len exceeded"
					      << std::endl;
		return false;
	    } else {
		strcpy(filename, result.c_str());
		return true;
	    }
	} else {
	    return false;
	}
    }

    /****************************************************************/

    void Tooltip(const char* str) {
	if(
	    tooltips_enabled && (str != nullptr) && (*str != '\0') &&
	    IsItemHovered()
	) {
	    SetTooltip("%s",str);
	}
    }
    
    void EnableTooltips() {
	tooltips_enabled = true;
    }
    
    void DisableTooltips() {
	tooltips_enabled = false;	
    }

    /****************************************************************/

    bool SimpleButton(const char* label) {
	std::string str(label);
	size_t off = str.find("##");
	if(off != std::string::npos) {
	    str = str.substr(0, off);
	}
	ImVec2 label_size = ImGui::CalcTextSize(str.c_str(), NULL, true);
	return ImGui::Selectable(label, false, 0, label_size);
    }

    void CenteredText(const char* text) {
	ImVec2 text_size = ImGui::CalcTextSize(text, NULL, true);
	ImVec2 avail_size = ImGui::GetContentRegionAvail();
	float w = 0.95f*(avail_size.x - text_size.x) / 2.0f;
	if(w > 0.0f) {
	    ImGui::Dummy(ImVec2(w, text_size.y));
	}
	ImGui::SameLine();
	ImGui::Text("%s",text);
    }
}


