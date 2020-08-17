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

#ifndef GEOGRAM_BASIC_STRING
#define GEOGRAM_BASIC_STRING

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>

#include <string>
#include <sstream>
#include <stdexcept>
#include <iomanip>

#include <vector>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <stdio.h>
#include <limits.h>

/**
 * \file geogram/basic/string.h
 * \brief Functions for string manipulation
 */

namespace GEO {

    /*
     * \brief String manipulation utilities.
     */
    namespace String {

        /**
         * \brief Splits a string into parts
         * \details Splits the string \p in into a list of substrings \p out
         * wherever \p separator occurs.
         * \param[in] in the input string to split
         * \param[in] separator the separator character
         * \param[in] out the resulting list of substrings
         * \param[in] skip_empty_fields specifies whether empty parts should
         * be ignored and not stored in list \p out (this is true by default).
         * \see join_strings()
         */
        void GEOGRAM_API split_string(
            const std::string& in,
            char separator,
            std::vector<std::string>& out,
            bool skip_empty_fields = true
        );

        /**
         * \brief Splits a string into two parts.
         * \param[in] in the input string to split
         * \param[in] separator the separator character
         * \param[in] left the part of the input string on the left
         *   of the separator or the empty string if the separator
         *   did not appear in the input string
         * \param[in] right the right of the input string on the left
         *   of the separator or the empty string if the separator
         *   did not appear in the input string
         * \retval true if the separator was found in the input string
         * \retval false otherwise
         */
        bool GEOGRAM_API split_string(
            const std::string& in,
            char separator,
            std::string& left,
            std::string& right
        );
        
        /**
         * \brief Join multiple strings
         * \details Joins all the strings in list \p in into a single string
         * with each element separated by the given \p separator character.
         * \param[in] in the list of strings to join
         * \param[in] separator the separator character
         * \return the joined string
         * \see split_string()
         */
        std::string GEOGRAM_API join_strings(
            const std::vector<std::string>& in,
            char separator
        );

        /**
         * \brief Join multiple strings
         * \details Joins all the strings in list \p in into a single string
         * with each element separated by the given \p separator string.
         * \param[in] in the list of strings to join
         * \param[in] separator the separator string (can be an empty string)
         * \return the joined string
         * \see split_string()
         */
        std::string GEOGRAM_API join_strings(
            const std::vector<std::string>& in,
            const std::string& separator
        );

        /**
         * \brief Converts a string to lowercase
         * \details The conversion is done in place in the string \p s.
         * \param[in,out] s The string to convert
         * \see to_uppercase()
         */
        std::string GEOGRAM_API to_lowercase(const std::string& s);

        /**
         * \brief Converts a string to uppercase
         * \details The conversion is done in place in the string \p s.
         * \param[in,out] s The string to convert
         * \see to_lowercase()
         */
        std::string GEOGRAM_API to_uppercase(const std::string& s);

        /**
         * \brief Creates a one char string
         * \param[in] c the character to convert to a string
         * \return a string that contains characater \p c
         */
        inline std::string char_to_string(char c) {
            char s[2];
            s[0] = c;
            s[1] = '\0';
            return std::string(s);
        }

        /**
         * \brief Adds quotes to a string
         * \details Adds character \p quote at the beginning and the end of
         * string \p s and returns the resulting string.
         * \param[in] s the string to quote
         * \param[in] quotes the quoting char (default is '"')
         * \return the quoted string
         */
        std::string GEOGRAM_API quote(
            const std::string& s, char quotes = '\"'
        );

        /**
         * \brief Checks if a string starts with a substring
         * \param[in] haystack the input string
         * \param[in] needle the substring to check
         * \return \c true if \p haystack starts with \p needle, \c false
         * otherwise.
         */
        bool GEOGRAM_API string_starts_with(
            const std::string& haystack, const std::string& needle
        );

        /**
         * \brief Checks if a string ends with a substring
         * \param[in] haystack the input string
         * \param[in] needle the substring to check
         * \return \c true if \p haystack ends with \p needle, \c false
         * otherwise.
         */
        bool GEOGRAM_API string_ends_with(
            const std::string& haystack, const std::string& needle
        );

        /**
         * \brief Converts a typed value to a string
         * \param[in] value the typed value to convert
         * \return a string that contain the stringified form of the value
         */
        template <class T>
        inline std::string to_string(const T& value) {
            std::ostringstream out;
	    // Makes sure that double-precision number are displayed
	    // with a sufficient number of digits. This is important
	    // to avoid losing precision when using ASCII files.
	    out << std::setprecision(17);
            out << value;
            return out.str();
        }

        /**
         * \brief Converts a typed value to a string for display.
	 * \details Does not keep all significant digits for floating point
	 *   numbers.
         * \param[in] value the typed value to convert
         * \return a string that contain the stringified form of the value
         */
        template <class T>
        inline std::string to_display_string(const T& value) {
	    return to_string(value);
	}


        /**
         * \brief Converts a typed value to a string for display.
	 * \details Does not keep all significant digits for floating point
	 *   numbers.
         * \param[in] value the typed value to convert
         * \return a string that contain the stringified form of the value
         */
        template <>
        inline std::string to_display_string(const double& value) {
            std::ostringstream out;	    
            out << value;
            return out.str();
	}

        /**
         * \brief Converts a typed value to a string for display.
	 * \details Does not keep all significant digits for floating point
	 *   numbers.
         * \param[in] value the typed value to convert
         * \return a string that contain the stringified form of the value
         */
        template <>
        inline std::string to_display_string(const float& value) {
            std::ostringstream out;	    
            out << value;
            return out.str();
	}
	
        /**
         * \brief Converts a boolean value to a string
         * \param[in] value the boolean value to convert
         * \return string \c "true" if the boolean value is true
         * or \c "false" if the boolean value is false
         */
        template <>
        inline std::string to_string(const bool& value) {
            return value ? "true" : "false";
        }

        /**
         * \brief Conversion exception
         * \details This exception is thrown by the conversion functions
         * to_bool(), to_int() and to_double() when a string cannot be
         * converted to the desired type.
         */
        class GEOGRAM_API ConversionError : public std::logic_error {
        public:
            /**
             * \brief Constructs a conversion exception
             * \param[in] s the input string that could not be converted
             * \param[in] type the expected destination type
             */
            ConversionError(const std::string& s, const std::string& type);

            /**
             * \brief Gets the string identifying the exception
             */
            virtual const char* what() const GEO_NOEXCEPT;
        };

        /**
         * \brief Converts a C string to a typed value
         * \details This is a generic version that uses a std::istringstream
         * to extract the value from the string. This function is specialized
         * for integral types to reach the maximum efficiency.
         * \param[in] s the source string
         * \param[out] value the typed value
         * \retval true if the conversion was successful
         * \retval false otherwise
         */
        template <class T>
        inline bool from_string(const char* s, T& value) {
            std::istringstream in(s);
            return (in >> value) && (in.eof() || ((in >> std::ws) && in.eof()));
        }

        /**
         * \brief Converts a std::string to a typed value
         * \details This is a generic version that uses a std::istringstream
         * to extract the value from the string. This function is specialized
         * for integral types to reach the maximum efficiency.
         * \param[in] s the source string
         * \param[out] value the typed value
         * \retval true if the conversion was successful
         * \retval false otherwise
         */
        template <class T>
        inline bool from_string(const std::string& s, T& value) {
            return from_string(s.c_str(), value);
        }

        /**
         * \brief Converts a string to a double value
         * \param[in] s the source string
         * \param[out] value the double value
         * \retval true if the conversion was successful
         * \retval false otherwise
         */
        template <>
        inline bool from_string(const char* s, double& value) {
            errno = 0;
            char* end;
            value = strtod(s, &end);
            return end != s && *end == '\0' && errno == 0;
        }

        /**
         * \brief Converts a string to a signed integer value
         * \param[in] s the source string
         * \param[out] value the integer value
         * \retval true if the conversion was successful
         * \retval false otherwise
         */
        template <typename T>
        inline bool string_to_signed_integer(const char* s, T& value) {
            errno = 0;
            char* end;
#ifdef GEO_OS_WINDOWS
            Numeric::int64 v = _strtoi64(s, &end, 10);
#else
            Numeric::int64 v = strtoll(s, &end, 10);
#endif
            if(
                end != s && *end == '\0' && errno == 0 &&
                v >= std::numeric_limits<T>::min() &&
                v <= std::numeric_limits<T>::max()
            ) {
                value = static_cast<T>(v);
                return true;
            }

            return false;
        }

        /**
         * \brief Converts a string to a Numeric::int8 value
         * \see string_to_signed_integer()
         */
        template <>
        inline bool from_string(const char* s, Numeric::int8& value) {
            return string_to_signed_integer(s, value);
        }

        /**
         * \brief Converts a string to a Numeric::int16 value
         * \see string_to_signed_integer()
         */
        template <>
        inline bool from_string(const char* s, Numeric::int16& value) {
            return string_to_signed_integer(s, value);
        }

        /**
         * \brief Converts a string to a Numeric::int32 value
         * \see string_to_signed_integer()
         */
        template <>
        inline bool from_string(const char* s, Numeric::int32& value) {
            return string_to_signed_integer(s, value);
        }

        /**
         * \brief Converts a string to a Numeric::int64 value
         */
        template <>
        inline bool from_string(const char* s, Numeric::int64& value) {
            errno = 0;
            char* end;
#ifdef GEO_OS_WINDOWS
            value = _strtoi64(s, &end, 10);
#else
            value = strtoll(s, &end, 10);
#endif
            return end != s && *end == '\0' && errno == 0;
        }

        /**
         * \brief Converts a string to a unsigned integer value
         * \param[in] s the source string
         * \param[out] value the integer value
         * \retval true if the conversion was successful
         * \retval false otherwise
         */
        template <typename T>
        inline bool string_to_unsigned_integer(const char* s, T& value) {
            errno = 0;
            char* end;
#ifdef GEO_OS_WINDOWS
            Numeric::uint64 v = _strtoui64(s, &end, 10);
#else
            Numeric::uint64 v = strtoull(s, &end, 10);
#endif
            if(
                end != s && *end == '\0' && errno == 0 &&
                v <= std::numeric_limits<T>::max()
            ) {
                value = static_cast<T>(v);
                return true;
            }

            return false;
        }

        /**
         * \brief Converts a string to a Numeric::uint8 value
         * \see string_to_unsigned_integer()
         */
        template <>
        inline bool from_string(const char* s, Numeric::uint8& value) {
            return string_to_unsigned_integer(s, value);
        }

        /**
         * \brief Converts a string to a Numeric::uint16 value
         * \see string_to_unsigned_integer()
         */
        template <>
        inline bool from_string(const char* s, Numeric::uint16& value) {
            return string_to_unsigned_integer(s, value);
        }

        /**
         * \brief Converts a string to a Numeric::uint32 value
         * \see string_to_unsigned_integer()
         */
        template <>
        inline bool from_string(const char* s, Numeric::uint32& value) {
            return string_to_unsigned_integer(s, value);
        }

        /**
         * \brief Converts a string to a Numeric::uint64 value
         */
        template <>
        inline bool from_string(const char* s, Numeric::uint64& value) {
            errno = 0;
            char* end;
#ifdef GEO_OS_WINDOWS
            value = _strtoui64(s, &end, 10);
#else
            value = strtoull(s, &end, 10);
#endif
            return end != s && *end == '\0' && errno == 0;
        }

        /**
         * \brief Converts a string to a boolean value
         * \details
         * Legal values for the true boolean value are "true","True" and "1".
         * Legal values for the false boolean value are "false","False" and "0".
         * \param[in] s the source string
         * \param[out] value the boolean value
         * \retval true if the conversion was successful
         * \retval false otherwise
         */
        template <>
        inline bool from_string(const char* s, bool& value) {
            if(strcmp(s, "true") == 0 ||
                strcmp(s, "True") == 0 ||
                strcmp(s, "1") == 0
            ) {
                value = true;
                return true;
            }
            if(strcmp(s, "false") == 0 ||
                strcmp(s, "False") == 0 ||
                strcmp(s, "0") == 0
            ) {
                value = false;
                return true;
            }
            return false;
        }

        /**
         * \brief Converts a string to an int
         * \details If the entire string cannot be
         * converted to an int, the function
         * throws an exception ConversionError.
         * \param[in] s the source string
         * \return the extracted integer value
         * \see ConversionError
         */
        inline int to_int(const std::string& s) {
            int value;
            if(!from_string(s, value)) {
                throw ConversionError(s, "integer");
            }
            return value;
        }

        /**
         * \brief Converts a string to an unsigned int
         * \details If the entire string cannot be
         * converted to an unsigned int, the function
         * throws an exception ConversionError.
         * \param[in] s the source string
         * \return the extracted integer value
         * \see ConversionError
         */
        inline unsigned int to_uint(const std::string& s) {
            unsigned int value;
            if(!from_string(s, value)) {
                throw ConversionError(s, "integer");
            }
            return value;
        }

        /**
         * \brief Converts a string to a double
         * \details If the entire string cannot be
         * converted to a double, the function
         * throws an exception ConversionError.
         * \param[in] s the source string
         * \return the extracted double value
         * \see ConversionError
         */
        inline double to_double(const std::string& s) {
            double value;
            if(!from_string(s, value)) {
                throw ConversionError(s, "double");
            }
            return value;
        }

        /**
         * \brief Converts a string to a boolean
         * \details If the entire string cannot be
         * converted to a boolean, the function
         * throws an exception ConversionError.
         * \param[in] s the source string
         * \return the extracted boolean value
         * \see ConversionError
         */
        inline bool to_bool(const std::string& s) {
            bool value;
            if(!from_string(s, value)) {
                throw ConversionError(s, "boolean");
            }
            return value;
        }

	/**
	 * \brief Converts a wide char string into an UTF8 string.
	 * \param[in] in the input null-terminated wide-char string.
	 * \return the UTF8-encoded string in a std::string.
	 */
	std::string GEOGRAM_API wchar_to_UTF8(const wchar_t* in);
    }
}

#endif

