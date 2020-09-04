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

#include <geogram/basic/line_stream.h>
#include <geogram/basic/logger.h>
#include <ctype.h>

namespace GEO {

    LineInput::LineInput(const std::string& filename) :
        file_name_(filename),
        line_num_(0)
    {
        F_ = fopen(filename.c_str(), "r");
        ok_ = (F_ != nullptr);
        line_[0] = '\0';
    }

    LineInput::~LineInput() {
        if(F_ != nullptr) {
            fclose(F_);
            F_ = nullptr;
        }
    }

    bool LineInput::get_line() {
        if(F_ == nullptr) {
            return false;
        }
        line_[0] = '\0';
        // Skip the empty lines
        while(!isprint(line_[0])) {
            ++line_num_;
            if(fgets(line_, MAX_LINE_LEN, F_) == nullptr) {
                return false;
            }
        }
        // If the line ends with a backslash, append
        // the next line to the current line.
        bool check_multiline = true;
        Numeric::int64 total_length = MAX_LINE_LEN;
        char* ptr = line_;
        while(check_multiline) {
            size_t L = strlen(ptr);
            total_length -= Numeric::int64(L);
            ptr = ptr + L - 2;
            if(*ptr == '\\' && total_length > 0) {
                *ptr = ' ';
                ptr++;
                if(fgets(ptr, int(total_length), F_) == nullptr) {
                    return false;
                }
                ++line_num_;
            } else {
                check_multiline = false;
            }
        }
        if(total_length < 0) {
            Logger::err("LineInput")
                << "MultiLine longer than "
                << MAX_LINE_LEN << " bytes" << std::endl;
        }
        return true;
    }

#if 1
#ifdef GEO_OS_WINDOWS
#define safe_strtok strtok_s
#else
#define safe_strtok strtok_r
#endif

    void LineInput::get_fields(const char* separators) {
        field_.resize(0);
        char* context = nullptr;
        char* tok = safe_strtok(line_, separators, &context);
        while(tok != nullptr) {
            field_.push_back(tok);
            tok = safe_strtok(nullptr, separators, &context);
        }
    }

#else
    void LineInput::get_fields(const char* separators) {
        field_.resize(0);
        char* tok = strtok(line_, separators);
        while(tok != nullptr) {
            field_.push_back(tok);
            tok = strtok(nullptr, separators);
        }
    }

#endif

    void LineInput::conversion_error(index_t index, const char* type) const {
        std::ostringstream out;
        out << "Line " << line_num_
            << ": field #" << index
            << " is not a valid " << type << " value: " << field(index);
        throw std::logic_error(out.str());
    }
}

