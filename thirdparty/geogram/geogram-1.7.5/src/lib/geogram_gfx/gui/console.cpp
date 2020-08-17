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

#include <geogram_gfx/gui/console.h>
#include <geogram_gfx/gui/application.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>
#include <geogram/basic/string.h>
#include <geogram/basic/command_line.h>

namespace GEO {

    Console::Console(bool* visible_flag) :
	command_prompt_(true),
        visible_flag_(visible_flag),
        completion_callback_(nullptr),
        history_callback_(nullptr),
        history_index_(0),
        max_history_index_(0)
    {
	input_buf_[0] = '\0';
	scroll_to_bottom_ = 0;
    }

    void Console::notify_error(const std::string& err) {
	geo_argused(err);
	return;
    }
    
    void Console::div(const std::string& value) {
        this->printf("========== %s", value.c_str());
    }
        
    void Console::out(const std::string& value) {
        this->printf("    %s", value.c_str());
    }
        
    void Console::warn(const std::string& value) {
        this->printf("[W] %s", value.c_str());
        if(visible_flag_ != nullptr) {
            *visible_flag_ = true;
        }
    }
        
    void Console::err(const std::string& value) {
        this->printf("[E] %s", value.c_str());
        if(visible_flag_ != nullptr) {
            *visible_flag_ = true;
        }
	notify_error(value);
    }

    void Console::status(const std::string& value) {
	// Do not display error messages twice.
	if(String::string_starts_with(value, "Error:")) {
	    return;
	}
	this->printf("[status] %s", value.c_str());
    }
    
    void Console::clear() {
        buf_.clear();
        line_offsets_.clear();
    }
    
    void Console::printf(const char* fmt, ...) {
        va_list args;
        va_start(args, fmt);
        int old_size = buf_.size();
        buf_.appendfv(fmt, args); 
        va_end(args);
        for (int new_size = buf_.size(); old_size < new_size; old_size++) {
            if (buf_[old_size] == '\n') {
                line_offsets_.push_back(old_size);
            }
        }
	// If I just use a boolean as in ImGuiDemo, then when printing to a
	// hidden console then showing it again does not scroll to bottom
	// (it seems that ImGui needs a couple of frames to 'figure out'
	// what happened).
        scroll_to_bottom_ = 10;
	update();
    }

    void Console::update() {
	if(Application::instance() != nullptr) {
	    Application::instance()->draw();
	}
    }

    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
        Console* console = (Console*)data->UserData;
	return console->TextEditCallback(data);
    }


    int Console::TextEditCallback(ImGuiInputTextCallbackData* data)  {
        switch (data->EventFlag) {
        case ImGuiInputTextFlags_CallbackCompletion: {
	    if(completion_callback_ != nullptr) {
		static const char*
		    completer_word_break_characters = " .(){},+-*/=";
		// Locate beginning of current word
		const char* word_end = data->Buf + data->CursorPos;
		const char* word_start = word_end;
		while (word_start > data->Buf)
		{
		    const char c = word_start[-1];
		    if(strchr(completer_word_break_characters,c) != nullptr) {
			break;
		    }
		    word_start--;
		}  
		index_t startw = index_t(word_start - data->Buf);
		index_t endw   = index_t(word_end - data->Buf);
		std::string cmpword(word_start, size_t(word_end - word_start));
		std::vector<std::string> matches;
		completion_callback_(
		    this, std::string(data->Buf),
		    startw, endw, cmpword, matches
		);
		if(matches.size() == 0) {
		    this->printf("Completions: no match\n");
		} else if(matches.size() == 1) {
		    // Single match. Delete the beginning of the word and
		    // replace it entirely so we've got nice casing
                    data->DeleteChars(
			(int)(word_start-data->Buf),
			(int)(word_end-word_start)
		    );
                    data->InsertChars(data->CursorPos, matches[0].c_str());
		} else {
		    // Several matches, find longest common prefix
		    std::string longest_prefix;
		    size_t cur_char = 0;
		    bool finished = false;
		    while(!finished) {
			char c = '\0';
			for(size_t i=0; i<matches.size(); ++i) {
			    if(
				cur_char >= matches[i].length() ||
				(i != 0 && matches[i][cur_char] != c)
			    ) {
				finished = true;
				break;
			    }
			    c = matches[i][cur_char];
			}
			if(!finished) {
			    longest_prefix.push_back(c);
			}
			++cur_char;
		    }
		    // Replace edited text with longest prefix
		    if(longest_prefix.length() != 0) {
			data->DeleteChars(
			    (int)(word_start-data->Buf),
			    (int)(word_end-word_start)
			);
			data->InsertChars(
			    data->CursorPos, longest_prefix.c_str()
			);
		    }
		    this->printf("Completions:\n");
		    for(size_t i=0; i<matches.size(); ++i) {
			this->printf(
			    "[%d] ... %s\n", int(i), matches[i].c_str()
			);
		    }
		}
	    }
	} break;
        case ImGuiInputTextFlags_CallbackHistory: {
	    if(history_callback_ != nullptr) {
		std::string history_command;
		//   Call the callback first, to give it the opportunity to
		// declare the history size.
		history_callback_(this, history_index_, history_command);
		if(max_history_index_ > 0) {
		    int h = int(history_index_);
		    if(data->EventKey == ImGuiKey_UpArrow) {
			--h;
			if(h < 0) {
			    h = int(max_history_index_);
			}
		    } else if(data->EventKey == ImGuiKey_DownArrow) {
			++h;
			if(h > int(max_history_index_)) {
			    h = 0;
			}
		    }
		    {
			history_index_ = index_t(h);
			if(history_index_ == max_history_index_) {
			    history_command = "";
			} else {
			    history_callback_(
				this, history_index_, history_command
			    );
			}
			int newpos = std::min(
			    data->BufSize-1, int(history_command.length())
			);
			strncpy(
			    data->Buf, history_command.c_str(), size_t(newpos)
			);
			data->Buf[newpos] = '\0';
			data->CursorPos = newpos;
			data->SelectionStart = newpos;
			data->SelectionEnd = newpos;
			data->BufTextLen = newpos;
			data->BufDirty = true;   
		    }
		}
	    }
        } break;
	}
        return 0;
    }

    bool Console::exec_command(const char* command) {
	geo_argused(command);
	// Note: history_index_ and max_history_index_ are
	// not managed here. They are managed by the callback.
	return false;
    }
    
    void Console::draw(bool* visible, bool with_window) {
	if(!*visible) {
	    return;
	}
	if(with_window) {
	    ImGui::Begin("Console", visible);
	}

	bool phone_screen = CmdLine::get_arg_bool("gui:phone_screen");
	
        // Add a close button under Android 	
	if(phone_screen) {
	    if (ImGui::SimpleButton(icon_UTF8("window-close").c_str())) {
		*visible = false;
	    }
	    ImGui::SameLine();
	}
        if (ImGui::SimpleButton(icon_UTF8("eraser").c_str())) {
            clear();
        }
	ImGui::Tooltip("clear");
        ImGui::SameLine();
        bool copy = ImGui::SimpleButton(icon_UTF8("copy").c_str());
	ImGui::Tooltip("copy");	
        ImGui::SameLine();
        filter_.Draw((icon_UTF8("filter")+" Filter").c_str(), -200.0f);
        ImGui::Separator();
	
	if(phone_screen) {
	    // Use smaller font if using phone in vertical mode.
	    if(
		Application::instance() != nullptr &&
		Application::instance()->get_height() >
		Application::instance()->get_width()
	    ) {
		ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[3]);
	    } else {
	    	ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]); 
	    }
	} else {
	    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);	    	    
	}
	
	float scaling = ImGui::GetIO().FontDefault->FontSize / 16.0f;
	
        ImGui::BeginChild(
	    "scrolling",
	    (command_prompt_?ImVec2(0.0f,-20.0f*scaling):ImVec2(0.0f, 0.0f)),
	    false,
            ImGuiWindowFlags_HorizontalScrollbar
        );
	
        if (copy) {
            ImGui::LogToClipboard();
        }
            
        {
            const char* buf_begin = buf_.begin();
            const char* line = buf_begin;
            for (int line_no = 0; line != nullptr; line_no++) {
                const char* line_end =
                    (line_no < line_offsets_.Size) ?
                    buf_begin + line_offsets_[line_no] : nullptr;
                
                if (!filter_.IsActive() ||
		    filter_.PassFilter(line, line_end)
		) {
		    bool is_error = ((line_end - line) >= 3 &&
			line[0] == '[' &&
			line[2] == ']' &&
			(line[1] == 'E' || line[1] == 'W')
		    );
		    if(is_error) {
			ImGui::PushStyleColor(
			    ImGuiCol_Text,ImVec4(1.0f,0.0f,0.0f,1.0f)
			);
		    }
                    ImGui::TextUnformatted(line, line_end);
		    if(is_error) {
			ImGui::PopStyleColor();
		    }
                }
                line = line_end && line_end[1] ? line_end + 1 : nullptr;
            }
        }
	// If I just use a boolean as in ImGuiDemo, then when printing to a
	// hidden console then showing it again does not scroll to bottom
	// (it seems that ImGui needs a couple of frames to 'figure out'
	// what happened).
        if (scroll_to_bottom_ > 0) {
	    ImGui::SetScrollHereY(1.0);
	    --scroll_to_bottom_;
        }
        ImGui::EndChild();
	ImGui::PopFont();

	if(command_prompt_) {
	    ImGui::PushFont(ImGui::GetIO().Fonts->Fonts[1]);	    	    
	    ImGui::Text(">>>");
	    ImGui::SameLine();
	    ImGui::PushItemWidth(-20);
	    if(ImGui::InputText(
		   "##CommandInput", input_buf_, geo_imgui_string_length,
		   ImGuiInputTextFlags_EnterReturnsTrue |
		   ImGuiInputTextFlags_CallbackCompletion |
		   ImGuiInputTextFlags_CallbackHistory,
		   &TextEditCallbackStub, (void*)this)
	    ) {
		char* input_end = input_buf_+strlen(input_buf_);
		while (input_end > input_buf_ && input_end[-1] == ' ') {
		    input_end--;
		}
		*input_end = 0;
		if (input_buf_[0]) {
		    exec_command(input_buf_);
		}
		strcpy(input_buf_, "");
	    }
	    ImGui::PopItemWidth();
	    // Keeping auto focus on the input box
	    if (ImGui::IsItemHovered()) {
		ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget
	    }
	    ImGui::PopFont();
	}

	if(with_window) {
	    ImGui::End();
	}
    }
}
