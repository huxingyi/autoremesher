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

#include <geogram_gfx/gui/command.h>
#include <geogram_gfx/third_party/ImGui/imgui.h>
#include <geogram_gfx/ImGui_ext/imgui_ext.h>
#include <geogram_gfx/ImGui_ext/icon_font.h>

namespace {
    /**
     * \brief Removes the underscores from a string and
     *  replaces them with spaces.
     * \details Utility for the prototype parser used by
     *  Command.
     * \param[in] s a const reference to the input string
     * \return a copy of \p s with underscores replaced with
     *  spaces
     */
    std::string remove_underscores(const std::string& s) {
        std::string result = s;
        for(GEO::index_t i=0; i<result.size(); ++i) {
            if(result[i] == '_') {
                result[i] = ' ';
            }
        }
        return result;
    }



    /**
     * \brief Removes leading and trailing spaces from
     *  a string.
     * \details Utility for the prototype parser used by
     *  Command.
     * \param[in,out] line the line from which spaces should
     *  be trimmed.
     */
    void trim_spaces(std::string& line) {
        size_t i=0;
        for(i=0; i<line.length(); ++i) {
            if(line[i] != ' ') {
                break;
            }
        }
        size_t j=line.length();
        for(j=line.length(); j>0; --j) {
            if(line[j-1] != ' ') {
                break;
            }
        }
        if(j>i) {
            line = line.substr(i, j-i);
        } else {
            line = "";
        }
    }
}

namespace GEO {

    /*****************************************************************/
    
    CommandInvoker::CommandInvoker() {
    }

    CommandInvoker::~CommandInvoker() {
    }
    
    /*****************************************************************/

    void Command::flush_queue() {
        if(queued_ != nullptr) {
            // Steal the queued command to avoid
            // infinite recursion.
            SmartPointer<Command> queued = queued_;
            queued_ = nullptr;
            queued->apply();
	    *(queued->is_visible_ptr()) = false;
        }
    }
    
    Command::~Command() {
    }

    Command::Command(const std::string& prototype_in) : visible_(false) {

        //   If there is no brace, then protype only has function
        // name, and then the invoker will construct the arguments
        // with names arg1, arg2 ...
        auto_create_args_ =
            (prototype_in.find('(') == std::string::npos);

        if(auto_create_args_) {
            name_ = prototype_in;
            if(name_ == "") {
                name_ = "command";
            }
            help_ = "Hay You Programmer ! No prototype was specified, \n"
                "see Command::make_current() documentation\n"
                "in geogram_gfx/glup_viewer/command.h\n"
                "to specify parameter names (and tooltips)";
            return;
        }
        
        // Parsing the prototype...
        
        std::string prototype = prototype_in;

        // Transform carriage returns into spaces
        {
            for(index_t i=0; i<prototype.size(); ++i) {
                if(prototype[i] == '\n') {
                    prototype[i] = ' ';
                }
            }
        }
        
        // Separate function name from argument list
        size_t p1 = std::string::npos;
        size_t p2 = std::string::npos;
        {
            int level = 0;
            for(size_t i=0; i<prototype.length(); ++i) {
                switch(prototype[i]) {
                case '[':
                    ++level;
                    break;
                case ']':
                    --level;
                    break;
                case '(':
                    if(level == 0) {
                        p1 = i;
                    }
                    break;
                case ')':
                    if(level == 0) {
                        p2 = i;
                    }
                    break;
                }
            }
        }

        
        geo_assert(p1 != std::string::npos && p2 != std::string::npos);
        name_ = prototype.substr(0,p1);
        
        // Trim spaces, and remove return type if it was specified.
        {
            std::vector<std::string> name_parts;
            String::split_string(name_, ' ', name_parts);
            name_ = name_parts[name_parts.size()-1];
        }

        // Find help if any
        {
            size_t bq1 = prototype.find('[',p2);
            size_t bq2 = prototype.find(']',p2);
            if(bq1 != std::string::npos && bq2 != std::string::npos) {
                help_ = prototype.substr(bq1+1, bq2-bq1-1);
            }
        }
        
        std::string args_string = prototype.substr(p1+1,p2-p1-1);
        std::vector<std::string> args;
        String::split_string(args_string, ',', args);
        
        for(index_t i=0; i<args.size(); ++i) {
            std::string arg = args[i];
            std::string default_value;
            std::string help;
            
            // Find help if any
            {
                size_t bq1 = arg.find('[');
                size_t bq2 = arg.find(']');
                if(bq1 != std::string::npos && bq2 != std::string::npos) {
                    help = arg.substr(bq1+1, bq2-bq1-1);
                    arg = arg.substr(0, bq1);
                }
            }
            
            // Find default value if any (to the right of the '=' sign)
            {
                size_t eq = arg.find('=');
                if(eq != std::string::npos) {
                   default_value = arg.substr(eq+1, arg.length()-eq-1);
                   trim_spaces(default_value);
                   arg = arg.substr(0, eq);
                }
            }

            // Analyze argument type and name
            std::vector<std::string> arg_words;
            String::split_string(arg, ' ', arg_words);

            // Argument name is the last word
            const std::string& arg_name = arg_words[arg_words.size()-1];
            int type = -1;
            if(arg_words.size() > 1) {
                bool is_unsigned = false;
                for(index_t w=0; w<arg_words.size()-1; ++w) {
                    if(arg_words[w] == "unsigned") {
                        is_unsigned = true;
                    } else if(arg_words[w] == "bool") {
                        type = Arg::ARG_BOOL;
                    } else if(arg_words[w] == "int") {
                        type = (is_unsigned) ? Arg::ARG_UINT : Arg::ARG_INT;
                    } else if(
                        arg_words[w] == "index_t" ||
                        arg_words[w] == "GEO::index_t"                        
                    ) {
                        type = Arg::ARG_UINT;
                    } else if(
                        arg_words[w] == "float" ||
                        arg_words[w] == "double"                        
                    ) {
                        type = Arg::ARG_FLOAT;
                    } else if(
                        arg_words[w] == "string" ||
                        arg_words[w] == "std::string" ||
                        arg_words[w] == "string&" ||
                        arg_words[w] == "std::string&"
                    ) {
                        type = Arg::ARG_STRING;
                    }
                }
            }

            switch(type) {
            case Arg::ARG_BOOL: {
                bool val = false;
                if(default_value != "") {
                    String::from_string(default_value, val);
                }
                add_arg(arg_name, val, help);
            } break;
            case Arg::ARG_INT: {
                int val = 0;
                if(default_value != "") {
                    String::from_string(default_value, val);                    
                }
                add_arg(arg_name, val, help);
            } break;
            case Arg::ARG_UINT: {
                unsigned int val = 0;
                if(default_value != "") {
                    String::from_string(default_value, val);
                }
                add_arg(arg_name, val, help);
            } break;
            case Arg::ARG_FLOAT: {
                float val = 0.0f;
                if(default_value != "") {
                    String::from_string(default_value, val);
                }
                add_arg(arg_name, val, help);
            } break;
            case Arg::ARG_STRING: {
                if(default_value != "") {
                    // Remove quotes
                    default_value = default_value.substr(
                        1, default_value.length()-2
                    );
                    add_arg(arg_name, default_value, help);
                }
            } break;
            default: {
                geo_assert_not_reached;
            }
            }
        }
        name_ = remove_underscores(name_);
    }
    
    void Command::apply() {
        if(invoker_ != nullptr) {
            invoker_->invoke();
        }
    }
    
    int Command::int_arg_by_index(index_t i) const {
        const Arg& arg = find_arg_by_index(i);
        geo_assert(
            arg.type == Arg::ARG_INT ||
            arg.type == Arg::ARG_UINT
        );
        int result = arg.val.int_val;
        if(
            arg.type == Arg::ARG_UINT &&
            result < 0
        ) {
            Logger::warn("Cmd")
                << "Argument " << arg.name
                << "Of type UNIT had a negative value"
                << std::endl;
            result = 0;
        }
        return result;
    }

    unsigned int Command::uint_arg_by_index(index_t i) const {
        const Arg& arg = find_arg_by_index(i);
        geo_assert(
            arg.type == Arg::ARG_INT ||
            arg.type == Arg::ARG_UINT
        );
        int result = arg.val.int_val;
        if(result < 0) {
            Logger::warn("Cmd")
                << "Argument " << arg.name
                << "queried as uint had a negative value"
                << std::endl;
            result = 0;
        }
        return (unsigned int)(result);
    }
    
    void Command::draw() {
	ImGui::Text("%s",name().c_str());
	if(ImGui::SimpleButton(icon_UTF8("window-close").c_str())) {
	    visible_ = false;
        }
	ImGui::Tooltip("close command");
        ImGui::SameLine();	
        if(ImGui::SimpleButton(
	       icon_UTF8("cog").c_str()
	)) {
            reset_factory_settings();
        }
	ImGui::Tooltip("reset factory settings");
        ImGui::SameLine();
	if(ImGui::Button(icon_UTF8("check").c_str(), ImVec2(-1.0, 0.0))) {
            queued_ = this;
        }
	if(help_ == "") {
	    ImGui::Tooltip("apply command");
	} else {
	    ImGui::Tooltip(help_);                
	}
        ImGui::Separator();            
        for(index_t i=0; i<args_.size(); ++i) {
            args_[i].draw();
        }
        ImGui::Separator();            
    }

    void Command::reset_factory_settings() {
        for(index_t i=0; i<args_.size(); ++i) {
            args_[i].val = args_[i].default_val;
        }
    }

    void Command::ArgVal::clear() {
        bool_val = false;
        int_val = 0;
        float_val = 0.0f;
        string_val[0] = '\0';
    }

    Command::ArgVal::ArgVal(const Command::ArgVal& rhs) {
        bool_val = rhs.bool_val;
        int_val = rhs.int_val;
        float_val = rhs.float_val;
        Memory::copy(string_val, rhs.string_val, 64);
    }

    Command::ArgVal& Command::ArgVal::operator=(const ArgVal& rhs) {
        if(&rhs != this) {
            bool_val = rhs.bool_val;
            int_val = rhs.int_val;
            float_val = rhs.float_val;
            Memory::copy(string_val, rhs.string_val, 64);
        }
        return *this;
    }

    Command::Arg::Arg() {
        name = "unnamed";
        type = ARG_BOOL;
        val.clear();
        default_val.clear();
        val.bool_val = false;
        default_val.bool_val = false;
    }
    
    Command::Arg::Arg(
        const std::string& name_in, bool x,
        const std::string& help_in
    ) {
        name = name_in;
        help = help_in;
        type = ARG_BOOL;
        val.clear();
        default_val.clear();
        val.bool_val = x;
        default_val.bool_val = x;
    }

    Command::Arg::Arg(
        const std::string& name_in, int x,
        const std::string& help_in
    ) {
        name = name_in;
        help = help_in;                
        type = ARG_INT;
        val.clear();
        default_val.clear();
        val.int_val = x;
        default_val.int_val = x;
    }

    Command::Arg::Arg(
        const std::string& name_in, unsigned int x,
        const std::string& help_in
    ) {
        name = name_in;
        help = help_in;                
        type = ARG_UINT;
        val.clear();
        default_val.clear();
        val.int_val = int(x);
        default_val.int_val = int(x);
    }

    Command::Arg::Arg(
        const std::string& name_in, float x,
        const std::string& help_in                
    ) {
        name = name_in;
        help = help_in;
        type = ARG_FLOAT;
        val.clear();
        default_val.clear();
        val.float_val = x;
        default_val.float_val = x;
    }

    Command::Arg::Arg(
        const std::string& name_in, double x,
        const std::string& help_in                                
    ) {
        name = name_in;
        help = help_in;                
        type = ARG_FLOAT;
        val.clear();
        default_val.clear();
        val.float_val = float(x);
        default_val.float_val = float(x);
    }

    Command::Arg::Arg(
        const std::string& name_in, const std::string& x,
        const std::string& help_in
    ) {
        name = name_in;
        help = help_in;                
        type = ARG_STRING;
        val.clear();
        default_val.clear();
        geo_assert(x.length() < 63);
        Memory::copy(default_val.string_val,x.c_str(), x.length());
        default_val.string_val[x.length()] = '\0';        
        Memory::copy(val.string_val,x.c_str(), x.length());        
        val.string_val[x.length()] = '\0';        
    }

    void Command::Arg::draw() {
        // Some widgets with labels are too wide,
        // therefore their label is displayed with
        // a separate ImGui::Text().
        //   Each ImGUI widget requires a unique Id,
        // that is normally generated from the label.
        //   If label starts with "##", then ImGui
        // makes it invisible (and uses what's after
        // "##" to generate the Id).
        switch(type) {
        case ARG_BOOL:
	    ImGui::SetNextItemWidth(-1.0f);
            ImGui::Checkbox(remove_underscores(name).c_str(), &val.bool_val);
	    ImGui::Tooltip(help);
            break;
        case ARG_INT:
            ImGui::Text("%s",remove_underscores(name).c_str());
	    ImGui::SetNextItemWidth(-1.0f);	    
	    ImGui::Tooltip(help);
            ImGui::InputInt(("##" + name).c_str(), &val.int_val);
            break;
        case ARG_UINT:
            ImGui::Text("%s",remove_underscores(name).c_str());
	    ImGui::SetNextItemWidth(-1.0f);	    
	    ImGui::Tooltip(help);
            ImGui::InputInt(("##" + name).c_str(), &val.int_val);
            break;
        case ARG_FLOAT:
            ImGui::Text("%s",remove_underscores(name).c_str());
	    ImGui::SetNextItemWidth(-1.0f);	    
	    ImGui::Tooltip(help);
            ImGui::InputFloat(("##" + name).c_str(), &val.float_val);
            break;
        case ARG_STRING:
            ImGui::Text("%s",remove_underscores(name).c_str());
	    ImGui::SetNextItemWidth(-1.0f);	    
	    ImGui::Tooltip(help);
            ImGui::InputText(("##" + name).c_str(), val.string_val, 64);
            break;
        }
    }

    SmartPointer<Command> Command::current_;
    SmartPointer<Command> Command::queued_;
}

