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

#ifndef H_GEOGRAM_GFX_GUI_CONSOLE_H
#define H_GEOGRAM_GFX_GUI_CONSOLE_H

#include <geogram_gfx/basic/common.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram/basic/logger.h>

/**
 * \file geogram_gfx/gui/console.h
 * \brief A console.
 */

namespace GEO {

    /**
     * \brief A console, that displays logger messages, and where the
     *  user can enter commands.
     * \details Inspired from ImGui AppLog example.
     */
    class GEOGRAM_GFX_API Console : public GEO::LoggerClient {
    
    public:

        /**
         * \brief Console constructor.
         * \param[in] visible_flag an optional pointer to application's
         *  variable that controls the visibility of this Console.
         */
        Console(bool* visible_flag = nullptr);
        
        /**
         * \copydoc GEO::LoggerClient::div()
         */
        virtual void div(const std::string& value);
        
        /**
         * \copydoc GEO::LoggerClient::out()
         */
        virtual void out(const std::string& value);
        
        /**
         * \copydoc GEO::LoggerClient::warn()
         */
        virtual void warn(const std::string& value);
        
        /**
         * \copydoc GEO::LoggerClient::err()
         */
        virtual void err(const std::string& value);
        
        /**
         * \copydoc GEO::LoggerClient::status()
         */
        virtual void status(const std::string& value);

        /**
         * \brief Clears the contents of the console.
         */
        void clear();

        /**
         * \brief Displays a formatted string to the console.
         */
        virtual void printf(const char* fmt, ...) /* IM_FMTARGS(1) */;

        /**
         * \brief Draws the console and handles the gui.
         * \param[in] visible an optional pointer to a visibility
         *  flag, controlled by a close button if different from nullptr.
	 * \param[in] with_window if true, then creates a new window
	 *  using imgui::Begin() / imgui::End(), else caller is responsible
	 *  for doing that.
         */
        virtual void draw(bool* visible=nullptr, bool with_window=true);

	int TextEditCallback(ImGuiInputTextCallbackData* data);

	void show() {
	    *visible_flag_ = true;
	}

	void hide() {
	    *visible_flag_ = false;
	}

	typedef void (*CompletionCallback)(
	    Console* console,
	    const std::string& line, index_t startw, index_t endw,
	    const std::string& cmpword, std::vector<std::string>& matches
	);

	void set_completion_callback(CompletionCallback CB) {
	    completion_callback_ = CB;
	}

	typedef void (*HistoryCallback)(
	    Console* console,
	    index_t index,
	    std::string& command
	);

	void set_history_callback(HistoryCallback CB) {
	    history_callback_ = CB;
	}

	void set_history_size(index_t n) {
	    if(n != max_history_index_) {
		history_index_  = n;
	    }
	    max_history_index_ = n;
	}

	void show_command_prompt() {
	    command_prompt_ = true;
	}

	void hide_command_prompt() {
	    command_prompt_ = false;
	}
	
      protected:
	/**
	 * \brief This function is called whenever an error is
	 *  displayed using err()
	 * \details Base implementation does nothing. This function
	 *  is meant to be overloaded in derived classes.
	 * \param[in] err the error message sent to err()
	 */
	virtual void notify_error(const std::string& err);

	virtual bool exec_command(const char* command);
	
	/**
	 * \brief Redraws the GUI.
	 */
	virtual void update();

	bool command_prompt_;
        ImGuiTextBuffer buf_;
        ImGuiTextFilter filter_;
        /** \brief Index to lines offset */
        ImVector<int>      line_offsets_;   
        index_t            scroll_to_bottom_;
        bool*              visible_flag_;
	char               input_buf_[geo_imgui_string_length];
	CompletionCallback completion_callback_;
	HistoryCallback    history_callback_;
	index_t            history_index_;
	index_t            max_history_index_;
    };

    typedef SmartPointer<Console> Console_var;
}

#endif
