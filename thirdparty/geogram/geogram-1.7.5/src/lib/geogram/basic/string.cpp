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

#include <geogram/basic/string.h>
#include <ctype.h>

namespace GEO {

    /**
     * \brief Builds the conversion error message
     * \param[in] s the input string that could not be converted
     * \param[in] type the expected destination type
     * \return a string that contains the error message
     */
    static std::string conversion_error(
        const std::string& s, const std::string& type
    ) {
        std::ostringstream out;
        out << "Conversion error: cannot convert string '"
            << s << "' to " << type;
        return out.str();
    }
}

namespace GEO {

    namespace String {

        void split_string(
            const std::string& in,
            char separator,
            std::vector<std::string>& out,
            bool skip_empty_fields
        ) {
            size_t length = in.length();
            size_t start = 0;
            while(start < length) {
                size_t end = in.find(separator, start);
                if(end == std::string::npos) {
                    end = length;
                }
                if(!skip_empty_fields || (end - start > 0)) {
                    out.push_back(in.substr(start, end - start));
                }
                start = end + 1;
            }
        }

        bool GEOGRAM_API split_string(
            const std::string& in,
            char separator,
            std::string& left,
            std::string& right
        ) {
            size_t p = in.find(separator);
            if(p == std::string::npos) {
                left = "";
                right = "";
                return false;
            }
            left = in.substr(0,p);
            right = in.substr(p+1,in.length()-p);
            return true;
        }
        
        std::string join_strings(
            const std::vector<std::string>& in,
            char separator
        ) {
            std::string result;
            for(unsigned int i = 0; i < in.size(); i++) {
                if(result.length() != 0) {
                    result += separator;
                }
                result += in[i];
            }
            return result;
        }

        std::string join_strings(
            const std::vector<std::string>& in,
            const std::string& separator
        ) {
            std::string result;
            for(unsigned int i = 0; i < in.size(); i++) {
                if(result.length() != 0) {
                    result += separator;
                }
                result += in[i];
            }
            return result;
        }

        std::string to_lowercase(const std::string& in) {
            std::string s = in;
            for(unsigned int i = 0; i < s.length(); i++) {
                s[i] = char(tolower(s[i]));
            }
            return s;
        }

        std::string to_uppercase(const std::string& in) {
            std::string s = in;
            for(unsigned int i = 0; i < s.length(); i++) {
                s[i] = char(toupper(s[i]));
            }
            return s;
        }

        std::string quote(const std::string& s, char quotes) {
            return char_to_string(quotes) + s + char_to_string(quotes);
        }

        bool string_starts_with(
            const std::string& haystack, const std::string& needle
        ) {
            return haystack.compare(0, needle.length(), needle) == 0;
        }

        bool string_ends_with(
            const std::string& haystack, const std::string& needle
        ) {
            size_t l1 = haystack.length();
            size_t l2 = needle.length();
            return l1 > l2 && haystack.compare(l1 - l2, l1, needle) == 0;
        }

	// Reference: https://stackoverflow.com/questions/148403/
	//     utf8-to-from-wide-char-conversion-in-stl
	
	std::string wchar_to_UTF8(const wchar_t* in) {
	    std::string out;
	    unsigned int codepoint = 0;
	    for (; *in != 0;  ++in) {
		if (*in >= 0xd800 && *in <= 0xdbff) {
		    codepoint = (unsigned int)(
			((*in - 0xd800) << 10) + 0x10000
		    );
		} else {
		    if (*in >= 0xdc00 && *in <= 0xdfff) {
			codepoint |= (unsigned int)(*in - 0xdc00);
		    } else {
			codepoint = (unsigned int)(*in);
		    }
		
		    if (codepoint <= 0x7f) {
			out.append(1, char(codepoint));
		    } else if (codepoint <= 0x7ff) {
			out.append(1, char(0xc0 | ((codepoint >> 6) & 0x1f)));
			out.append(1, char(0x80 | (codepoint & 0x3f)));
		    } else if (codepoint <= 0xffff) {
			out.append(1, char(0xe0 | ((codepoint >> 12) & 0x0f)));
			out.append(1, char(0x80 | ((codepoint >> 6) & 0x3f)));
			out.append(1, char(0x80 | (codepoint & 0x3f)));
		    } else {
			out.append(1, char(0xf0 | ((codepoint >> 18) & 0x07)));
			out.append(1, char(0x80 | ((codepoint >> 12) & 0x3f)));
			out.append(1, char(0x80 | ((codepoint >> 6) & 0x3f)));
			out.append(1, char(0x80 | (codepoint & 0x3f)));
		    }
		    codepoint = 0;
		}
	    }
	    return out;
	}
	
        /********************************************************************/

        ConversionError::ConversionError(
            const std::string& s, const std::string& type
        ) :
            std::logic_error(conversion_error(s, type)) {
        }

        const char* ConversionError::what() const GEO_NOEXCEPT {
            return std::logic_error::what();
        }
    }
}

