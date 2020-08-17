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

#ifndef GEOGRAM_BASIC_LINE_STREAM
#define GEOGRAM_BASIC_LINE_STREAM

#include <geogram/basic/common.h>
#include <geogram/basic/assert.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/string.h>
#include <cstring>
#include <stdio.h>

/**
 * \file geogram/basic/line_stream.h
 * \brief Utility class to read a file line per line and parsing
 *  fields from each line
 */

namespace GEO {

    /**
     * \brief Reads an ASCII file line per line
     * \details LineInput reads an ASCII file line by line and splits
     * the line into a list of white space separated fields that can be
     * accessed individually or converted to numeric values.
     *
     * Functions field_as_int() and field_as_double() throw exceptions when
     * they cannot convert a field to a integer or floating point value, so
     * it is safe to wrap the LineNumber usage in a try / catch block as
     * follows:
     *
     * \code
     * try {
     *     LineInput in(filename);
     *     while( !in.eof() && in.get_line() ) {
     *         in.get_fields();
     *         double d = in.field_as_double(2);
     *     }
     * }
     * catch(const std::logic_error& ex) {
     *     std::cerr << "Got an error: " << ex.what << std::endl;
     * }
     * \endcode
     */
    class GEOGRAM_API LineInput {
    public:
        /**
         * \brief Creates a new line reader from a file
         * \details This open the file \p filename for reading and prepares to
         * read it line by line. If the file could not be opened, OK() will
         * return false;
         * \param[in] filename the name of the file to read
         */
        LineInput(const std::string& filename);

        /**
         * \brief Destroys the line reader
         * \details This closes the current input file.
         */
        ~LineInput();

        /**
         * \brief Checks if the line reader is ready to read.
         */
        bool OK() const {
            return ok_;
        }

        /**
         * \brief Checks if line reader has reached the end of the input stream
         * \retval true if the stream is at end
         * \retval false otherwise
         */
        bool eof() const {
            return feof(F_) ? true : false;
        }

        /**
         * \brief Reads a new line
         * \details Reads a new line from the input stream. Function
         * get_fields() must be called if you need to access to individual
         * fields in the line with field() and its typed variants.
         * \retval true if a line could be read
         * \retval false otherwise.
         */
        bool get_line();

        /**
         * \brief Gets the number of fields in the current line
         * \details Function get_fields() must be called once after get_line()
         * before calling this function, otherwise the result is undefined.
         * \return the number of fields in the current line
         */
        index_t nb_fields() const {
            return index_t(field_.size());
        }

        /**
         * \brief Returns the current line number
         * \details If no line has been read so far, line_number() returns 0.
         */
        size_t line_number() const {
            return line_num_;
        }

        /**
         * \brief Gets a line field as a modifiable string
         * \details The function returns the field at index \p i. Function
         * get_fields() must be called once after get_line() before calling
         * this function, otherwise the result is undefined.
         * \param[in] i the index of the field
         * \return the modifiable pointer to field string at index \p i
         */
        char* field(index_t i) {
            geo_assert(i < nb_fields());
            return field_[i];
        }

        /**
         * \brief Gets a line field as a non-modifiable string
         * \details The function returns the field at index \p i. Function
         * get_fields() must be called once after get_line() before calling
         * this function, otherwise the result is undefined.
         * \param[in] i the index of the field
         * \return the const pointer to field string at index \p i
         */
        const char* field(index_t i) const {
            geo_assert(i < nb_fields());
            return field_[i];
        }

        /**
         * \brief Gets a line field as an integer.
         * \details The function returns the field at index \p i converted to
         * an integer. Function get_fields() must be called once after
         * get_line() before calling this function, otherwise the result is
         * undefined.
         * \param[in] i the index of the field
         * \return the integer value of the field at index \p i
         * \exception std::logic_error if the field cannot be converted to an
         * integer value
         */
        signed_index_t field_as_int(index_t i) const {
            signed_index_t result = 0;
            if(!String::from_string(field(i), result)) {
                conversion_error(i, "integer");
            }
            return result;
        }

        /**
         * \brief Gets a line field as an unsigned integer.
         * \details The function returns the field at index \p i converted to
         * an unsigned integer. Function get_fields() must be called once after
         * get_line() before calling this function, otherwise the result is
         * undefined.
         * \param[in] i the index of the field
         * \return the unsigned integer value of the field at index \p i
         * \exception std::logic_error if the field cannot be converted to an
         * unsigned integer value
         */
        index_t field_as_uint(index_t i) const {
            index_t result = 0;
            if(!String::from_string(field(i), result)) {
                conversion_error(i, "unsigned integer");
            }
            return result;
        }

        /**
         * \brief Gets a line field as a double.
         * \details The function returns the field at index \p i converted to
         * a double. Function get_fields() must be called once after
         * get_line() before calling this function, otherwise the result is
         * undefined.
         * \param[in] i the index of the field
         * \return the floating point value of the field at index \p i
         * \exception std::logic_error if the field cannot be converted to a
         * floating point value
         */
        double field_as_double(index_t i) const {
            double result = 0;
            if(!String::from_string(field(i), result)) {
                conversion_error(i, "floating point");
            }
            return result;
        }

        /**
         * \brief Compares a field with a string.
         * \details The function compares the field at index \p i with string
         * \p s and returns \c true if they are equal. Function get_fields()
         * must be called once after get_line() before calling this function,
         * otherwise the result is undefined.
         * \param[in] i the index of the field
         * \param[in] s the string to compare the field to
         * \retval true if field at index \p i equals string \p s
         * \retval false otherwise
         */
        bool field_matches(index_t i, const char* s) const {
            return strcmp(field(i), s) == 0;
        }

        /**
         * \brief Splits the current line into fields.
         * \details The function uses \p separators to split the 
         * current line into individual fields that can be accessed 
         * by field() and its typed variants.
         * \param[in] separators a string that contains all
         *  the characters considered as separators.
         * \see field()
         */
        void get_fields(const char* separators = " \t\r\n");

        /**
         * \brief Gets the current line.
         * \details If get_fields() was called, then an end-of-string
         *  marker '\0' is present at the end of the first field.
         * \return a const pointer to the internal buffer that stores
         *  the current line
         */ 
        const char* current_line() const {
            return line_;
        }

    private:
        /**
         * \brief Throws a conversion error.
         * \details This function is called by field_as_int() and
         * field_as_double() when the field \p index cannot be converted to
         * the desired type \p type.
         * \param[in] index index of the erroneous field.
         * \param[in] type the expected type.
         */
        GEO_NORETURN_DECL void conversion_error(
            index_t index, const char* type
        ) const GEO_NORETURN ;

        /**
         * \brief Defines the maximum size of a line
         */
        static const index_t MAX_LINE_LEN = 65535;

        FILE* F_;
        std::string file_name_;
        size_t line_num_;
        char line_[MAX_LINE_LEN];
        std::vector<char*> field_;
        bool ok_;
    };
}

#endif

