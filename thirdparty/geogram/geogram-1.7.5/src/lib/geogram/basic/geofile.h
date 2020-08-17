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

#ifndef GEOGRAM_BASIC_GEOFILE
#define GEOGRAM_BASIC_GEOFILE

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/memory.h>
#include <geogram/basic/string.h>
#include <geogram/third_party/zlib/zlib.h>

#include <stdexcept>
#include <fstream>
#include <map>


/**
 * \file geogram/basic/geofile.h
 * \brief Functions to read and write structured files
 */

namespace GEO {

    /**
     * \brief GeoFile exception.
     * \details This exception is thrown by GeoFile functions 
     *  whenever a file cannot be written or read.
     */
    class GEOGRAM_API GeoFileException : public std::logic_error {
    public:
        /**
         * \brief GeoFileException constructor
         * \param[in] s a const reference to the message to be
         *  memorized in the exception.
         */
        GeoFileException(const std::string& s) : logic_error(s) {
        }

        /**
         * \brief GeoFileException copy constructor.
         * \param[in] rhs a const reference to the GeoFileException to be
         *  copied.
         */
        GeoFileException(const GeoFileException& rhs) : logic_error(rhs) {
        }
        
        /**
         * \brief GeoFileException destructor.
         */
        virtual ~GeoFileException() GEO_NOEXCEPT;
    };

    /**************************************************************/

    /**
     * \brief Reads an ASCII attribute from a file.
     * \param[in] file the input file, obtained through fopen()
     * \param[out] base_addr an array with sufficient space for
     *  storing nb_elements of type \p T
     * \param[in] nb_elements the number of elements to be read
     * \retval true on success
     * \retval false otherwise
     * \tparam T the type of the elements to be read
     */
    template <class T> inline bool read_ascii_attribute(
        FILE* file, Memory::pointer base_addr, index_t nb_elements
    ) {
        T* attrib = reinterpret_cast<T*>(base_addr);
        for(index_t i=0; i<nb_elements; ++i) {
            std::string buff;
            int res;
            while(char(res = fgetc(file)) != '\n') {
                if(res == EOF) {
                    return false;
                }
                buff.push_back(char(res));
            }
            if(!String::from_string(buff.c_str(),attrib[i])) {
                return false;
            }
        }
        return true;
    }

    /**
     * \brief Writes an ASCII attribute to a file.
     * \param[in] file the output file, obtained through fopen()
     * \param[in] base_addr an array with nb_elements of type \p T
     * \param[in] nb_elements the number of elements to be written
     * \retval true on success
     * \retval false otherwise
     * \tparam T the type of the elements to be written
     */
    template <class T> inline bool write_ascii_attribute(
        FILE* file, Memory::pointer base_addr, index_t nb_elements
    ) {
        T* attrib = reinterpret_cast<T*>(base_addr);
        for(index_t i=0; i<nb_elements; ++i) {
            if(
                fprintf(
                    file, "%s\n", String::to_string(attrib[i]).c_str()
                ) == 0
            ) {
                return false;
            }
        }
        return true;
    }

    /**
     * \brief Reads an ASCII attribute from a file.
     * \details Template specialization for char, needed because we
     *  want chars to appear as integers in ASCII files.
     * \param[in] file the input file, obtained through fopen()
     * \param[out] base_addr an array with sufficient space for
     *  storing nb_elements of type char
     * \param[in] nb_elements the number of elements to be read
     * \retval true on success
     * \retval false otherwise
     */
    template <> inline bool read_ascii_attribute<char>(
        FILE* file, Memory::pointer base_addr, index_t nb_elements            
    ) {
        char* attrib = reinterpret_cast<char*>(base_addr);            
        for(index_t i=0; i<nb_elements; ++i) {
            int val;
            if(fscanf(file, "%d", &val) == 0) {
                return false;
            }
            attrib[i] = char(val);
        }
        return true;
    }

    /**
     * \brief Writes an ASCII attribute to a file.
     * \details Template specialization for char, needed because we
     *  want chars to appear as integers in ASCII files.
     * \param[in] file the output file, obtained through fopen()
     * \param[in] base_addr an array with nb_elements of type char
     * \param[in] nb_elements the number of elements to be written
     * \retval true on success
     * \retval false otherwise
     */
    template <> inline bool write_ascii_attribute<char>(
        FILE* file, Memory::pointer base_addr, index_t nb_elements            
    ) {
        char* attrib = reinterpret_cast<char*>(base_addr);            
        for(index_t i=0; i<nb_elements; ++i) {
            if(fprintf(file, "%d\n", int(attrib[i])) == 0) {
                return false;
            }
        }
        return true;
    }

    /**
     * \brief Reads an ASCII attribute from a file.
     * \details Template specialization for bool.
     * \param[in] file the input file, obtained through fopen()
     * \param[out] base_addr an array with sufficient space for
     *  storing nb_elements of type bool (1 byte per element)
     * \param[in] nb_elements the number of elements to be read
     * \retval true on success
     * \retval false otherwise
     */
    template <> inline bool read_ascii_attribute<bool>(
        FILE* file, Memory::pointer base_addr, index_t nb_elements            
    ) {
        char* attrib = reinterpret_cast<char*>(base_addr);            
        for(index_t i=0; i<nb_elements; ++i) {
            int val;
            if(fscanf(file, "%d", &val) == 0) {
                return false;
            }
            attrib[i] = char(val);
        }
        return true;
    }

    /**
     * \brief Writes an ASCII attribute to a file.
     * \details Template specialization for bool.
     * \param[in] file the output file, obtained through fopen()
     * \param[in] base_addr an array with nb_elements of type bool 
     *  (1 byte per element)
     * \param[in] nb_elements the number of elements to be written
     * \retval true on success
     * \retval false otherwise
     */
    template <> inline bool write_ascii_attribute<bool>(
        FILE* file, Memory::pointer base_addr, index_t nb_elements            
    ) {
        char* attrib = reinterpret_cast<char*>(base_addr);            
        for(index_t i=0; i<nb_elements; ++i) {
            if(fprintf(file, "%d\n", int(attrib[i])) == 0) {
                return false;
            }
        }
        return true;
    }

    /**************************************************************/
    
    /**
     * \brief Base class for reading or writing Geogram structured binary
     *  files.
     * \details Geogram structured binary files are organized into "chunks",
     *  in a way inspired by the Interchange File Format (IFF), with
     *  several differences (GeoFile uses little endian and does not use
     *  the standard IFF chunks). Like in IFF, each chunk starts with a 
     *  four characters code (FourCC) and its size in bytes, stored in 
     *  a 4 bytes unsigned integer. This makes it possible to easily skip 
     *  the chunks that are not needed / not understood by the software. 
     *  In addition, structured binary files are (optionally)
     *  compressed, using ZLib. Structured files can also be saved/loaded in
     *  ASCII, human-readable form. Natively, GeoFile uses the following chunks:
     *   - CMNT (Comment): contains a string
     *   - CMDL (Command Line): contains a vector of string 
     *   - EOFL (End of file): an end of file marker. Can be used to
     *    indicate the boundaries of multiple objects stored in the
     *    same file
     *   - HEAD (Geofile header): contains the string GEOGRAM and a
     *    version string
     *   - PSET (Property Set): a set of properties. For instance, in
     *    a mesh, each mesh element type (vertices, edges, facets...) 
     *    corresponds to a property set.
     *   - PROP (Property): a property attached to Property Set.
     *   - SPTR (Separator): marks the boundaries between multiple objects
     *    stored in the same GeoFile
     */
    class GEOGRAM_API GeoFile {
    public:

        /**
         * \brief The function pointer type for reading and writing attributes
         *  in ASCII files.
         */
        typedef bool (*AsciiAttributeSerializer)(
            FILE* file, Memory::pointer base_address, index_t nb_elements
        );

        /**
         * \brief Declares a new attribute type that can be read from 
         *  and written to ascii files.
         * \param[in] type_name the C++ type name of the attribute
         * \param[in] read the function pointer for reading an attribute
         * \param[in] write the function pointer for writing an attribute
         */
        static void register_ascii_attribute_serializer(
            const std::string& type_name,
            AsciiAttributeSerializer read,
            AsciiAttributeSerializer write
        );

        /**
         * \brief GeoFile constructor.
         * \param[in] filename a const reference to the file name.
         */
        GeoFile(const std::string& filename);

        /**
         * \brief GeoFile destructor.
         */
        ~GeoFile();

        /**
         * \brief Tests whether this GeoFile is ascii.
         * \details GeoFile can be ascii or binary. If file name
         *  ends with "_ascii", then GeoFile is ascii.
         * \retval true if this GeoFile is ascii
         * \retval false otherwise
         */
        bool is_ascii() const {
            return ascii_;
        }

        /**
         * \brief Gets the current chunk class.
         * \return the current chunk class
         */
        const std::string& current_chunk_class() const {
            return current_chunk_class_;
        }

        /**
         * \brief Gets the size of the current chunk.
         * \return the size of the current chunk, in bytes
         */
        long current_chunk_size() const {
            return current_chunk_size_;
        }
        
        /**
         * \brief Internal representation of attributes.
         */
        struct AttributeInfo {

            /**
             * \brief AttributeInfo constructor.
             */
            AttributeInfo() : element_size(0), dimension(0) {
            }

            /**
             * \brief AttributeInfo constructor.
             * \param[in] name_in name of the attribute
             * \param[in] element_type_in C++ type of the elements, as a string
             * \param[in] element_size_in size in bytes of an element
             * \param[in] dimension_in number of elements per item
             */
            AttributeInfo(
                const std::string& name_in,
                const std::string& element_type_in,
                size_t element_size_in,
                index_t dimension_in
             ) :
                name(name_in),
                element_type(element_type_in),
                element_size(element_size_in),
                dimension(dimension_in) {
            }

            
            /**
             * \brief Name of the attribute.
             */
            std::string name;

            /**
             * \brief A string with the name fo the C++ type
             *  of the elements.
             */
            std::string element_type;

            /**
             * \brief The size in bytes of each element.
             */
            size_t element_size;

            /**
             * \brief The number of elements per item.
             */
            index_t dimension;
        };

        /**
         * \brief Internal representation of an attribute set.
         */
        struct AttributeSetInfo {

            /**
             * \brief AttributeSetInfo constructor.
             */
            AttributeSetInfo() : nb_items(0), skip(false) {
            }

            /**
             * \brief AttributeSetInfo constructor.
             * \param[in] name_in name of the attribute set
             * \param[in] nb_items_in number of items in each attribute of the
             *  set
             */
            AttributeSetInfo(
                const std::string& name_in,
                index_t nb_items_in
            ) :
                name(name_in),
                nb_items(nb_items_in),
                skip(false) {
            }

            /**
             * \brief Finds an AttributeInfo by name.
             * \param[in] name_in a const reference to the name of the 
             *  attribute
             * \return a const pointer to the AttributeInfo or nullptr if there 
             *  is no such attribute.
             */
            const AttributeInfo* find_attribute(
                const std::string& name_in
            ) const {
                for(index_t i=0; i<attributes.size(); ++i) {
                    if(attributes[i].name == name_in) {
                        return &(attributes[i]);
                    }
                }
                return nullptr;
            }

            /**
             * \brief Finds an AttributeInfo by name.
             * \param[in] name_in a const reference to the name of the 
             *  attribute
             * \return a pointer to the AttributeInfo or nullptr if there 
             *  is no such attribute.
             */
             AttributeInfo* find_attribute(const std::string& name_in) {
                for(index_t i=0; i<attributes.size(); ++i) {
                    if(attributes[i].name == name_in) {
                        return &(attributes[i]);
                    }
                }
                return nullptr;
            }
            
            /**
             * \brief name of the attribute set.
             */
            std::string name;
            
            /**
             * \brief number of items in each attribute of the set.
             */
            index_t nb_items;

            /**
             * \brief the attributes of the set.
             */
            vector<AttributeInfo> attributes;

            /**
             * \brief if set, all attributes in the set are 
             *  skipped when reading the file.
             */
            bool skip;
        };

        /**
         * \brief Finds an attribute set by name.
         * \param[in] name a const reference to the name of the attribute set
         * \return a pointer to the AttributeSetInfo or nullptr if there is
         *  no such attribute set.
         */
        AttributeSetInfo* find_attribute_set(const std::string& name) {
	    auto it = attribute_sets_.find(name);
            if(it == attribute_sets_.end()) {
                return nullptr;
            }
            return &(it->second);
        }

        /**
         * \brief Finds an attribute set by name.
         * \param[in] name a const reference to the name of the attribute set
         * \return a const pointer to the AttributeSetInfo or nullptr if there is
         *  no such attribute set.
         */
        const AttributeSetInfo* find_attribute_set(
            const std::string& name
        ) const {
	    auto it = attribute_sets_.find(name);
            if(it == attribute_sets_.end()) {
                return nullptr;
            }
            return &(it->second);
        }

        /**
         * \brief Reads an unsigned 32 bits integer from the file.
         * \details Checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \return the read integer
         */
        index_t read_int();

        /**
         * \brief Writes an unsigned 32 bits integer into the file.
         * \details Checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \param[in] x the integer
         * \param[in] comment an optional comment string, written to
         *  ASCII geofiles
         */
        void write_int(index_t x, const char* comment = nullptr);

        /**
         * \brief Reads a string from the file.
         * \details Checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \return the read string
         */
        std::string read_string();

        /**
         * \brief Writes a string into the file.
         * \details Checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \param[in] s a const reference to the string
         * \param[in] comment an optional comment string, written to
         *  ASCII geofiles
         */
        void write_string(const std::string& s, const char* comment = nullptr);

        /**
         * \brief Reads an unsigned 64 bits integer from the file.
         * \details Checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \return the read integer
         */
        size_t read_size();

        /**
         * \brief Writes an unsigned 64 bits integer into the file.
         * \details Checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \param[in] x the integer
         */
        void write_size(size_t x);

        /**
         * \brief Reads a chunk class from the file.
         * \details A chunk class is a 4 characters string.
         *  The function checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \return A 4 characters string with the chunk class.
         */
        std::string read_chunk_class();

        /**
         * \brief Writes a chunk class into the file.
         * \details A chunk class is a 4 characters string.
         *  The function checks that I/O was completed and throws a
         *  GeoFileException if the file is truncated.
         * \param[in] chunk_class A 4 characters string with the chunk class.
         * \pre chunk_class.length() == 4
         */
        void write_chunk_class(const std::string& chunk_class);

        /**
         * \brief Writes a string array into the file.
         * \param[in] strings the string array, as a const reference to
         *  a vector of strings.
         */
        void write_string_array(const std::vector<std::string>& strings);

        /**
         * \brief Reads a string array from the file.
         * \param[out] strings the read string array, as a reference to
         *  a vector of strings.
         */
        void read_string_array(std::vector<std::string>& strings);

        
        /**
         * \brief Gets the size in bytes used by a given string in 
         *  the file.
         * \details The file stored the length of the string in a 32
         *  bits integer plus all the characters of the string
         * \return the size in bytes used to store the string in the
         *  file.
         */
        size_t string_size(const std::string& s) const {
            return sizeof(index_t) + s.length();
        }

        /**
         * \brief Gets the size in bytes used by a given string array in 
         *  the file.
         * \return the size in bytes used to store the string array in the
         *  file.
         */
        size_t string_array_size(
            const std::vector<std::string>& strings
        ) const ;
        
        /**
         * \brief Reads a chunk header from the file.
         */
        void read_chunk_header();
        
        /**
         * \brief Writes a chunk header into the file.
         * \param[in] chunk_class the chunk class
         * \param[in] size the size in bytes of the data 
         *  attached to the chunk.
         * \details When reading the file, to skip the chunk, 
         *  one calls fseek(file_, size, SEEK_CUR)
         */
        void write_chunk_header(
            const std::string& chunk_class, size_t size
        );

        /**
         * \brief Checks that the actual chunk size corresponds
         *  to the specified chunk size.
         */
        void check_chunk_size();

        /**
         * \brief Compares the zlib version declared in the header
         *  file with the zlib version obtained from the runtime,
         *  and outputs an error message if they differ.
         */
        void check_zlib_version();

        /**
         * \brief Clears all memorized information about attributes
         *  and attribute sets.
         * \details This function is called whenever a separator is read.
         */
        void clear_attribute_maps();
        
    protected:
        std::string filename_;
        gzFile file_;
        bool ascii_;
        FILE* ascii_file_;
        std::string current_chunk_class_;
        long current_chunk_size_;
        long current_chunk_file_pos_;
        std::map<std::string, AttributeSetInfo> attribute_sets_;

        static std::map<std::string, AsciiAttributeSerializer>
            ascii_attribute_read_;

        static std::map<std::string, AsciiAttributeSerializer>
            ascii_attribute_write_;
    };

    /**************************************************************/
    
    /**
     * \brief Used to read a structured binary file.
     */
    class GEOGRAM_API InputGeoFile : public GeoFile {
    public:
        /**
         * \brief InputGeoFile constructor.
         * \param[in] filename a const reference to the file name.
         */
        InputGeoFile(const std::string& filename);

        /**
         * \brief Advances to the next chunk.
         * \return The read chunk class.
         */
        const std::string& next_chunk();

        /**
         * \brief Reads the latest attribute.
         * \details This function can be only called right after next_chunk(),
         *  if it returned ATTRIBUTE.
         */
        void read_attribute(void* addr);


        /**
         * \brief Indicates that all the attributes attached to the 
         *  latest attribute set should be skipped.
         * \details This function can be only called right after next_chunk(),
         *  if it returned ATTRIBUTE_SET.
         */
        void skip_attribute_set();

        
        /**
         * \brief Gets the current attribute set.
         * \return a const reference to the AttributeSetInfo that
         *  represents the current attribute set
         * \pre current chunk class is either "ATTR" (ATTRIBUTE) or 
         *   ATTS (ATTRIBUTE_SET)
         */
        const AttributeSetInfo& current_attribute_set() const {
            geo_assert(current_attribute_set_ != nullptr);
            return *current_attribute_set_;
        }

        /**
         * \brief Gets the current attribute.
         * \return a const reference to the AttributeInfo that
         *  represents the current attribute
         * \pre current chunk class is "ATTR" (ATTRIBUTE)
         */
        const AttributeInfo& current_attribute() const {
            geo_assert(current_attribute_ != nullptr);
            return *current_attribute_;
        }

        /*
         * \brief Gets the current user comment.
         * \return a const reference to the current user comment
         * \pre current chunk class is "CMNT" (COMMENT)
         */
        const std::string& current_comment() const {
            geo_assert(current_chunk_class_ == "CMNT");
            return current_comment_;
        }


        /**
         * \brief Reads the command line from the file.
         * \details It is useful to save the command line arguments in
         *  each file, so that one can retrieve the parameters of each
         *  experiments when doing algorithm test.
         * \param[out] args the command line, as a vector of strings
         * \pre current_chunk_class() == "CMDL"
         */
        void read_command_line(std::vector<std::string>& args);
        
    protected:
        /**
         * \brief Skips the latest chunk.
         * \details This function can only be called right
         *  after next_chunk().
         */
        void skip_chunk();

        AttributeSetInfo* current_attribute_set_;
        AttributeInfo* current_attribute_;
        std::string current_comment_;

    private:        
        /**
         * \brief Forbids copy.
         */
        InputGeoFile(const InputGeoFile& rhs);

        /**
         * \brief Forbids copy.
         */
        InputGeoFile& operator=(const InputGeoFile& rhs);
    };

    /**************************************************************/
    
    /**
     * \brief Used to write a structured binary file.
     */
    class GEOGRAM_API OutputGeoFile : public GeoFile {
    public:
        /**
         * \brief OutputGeoFile constructor.
         * \param[in] filename a const reference to the file name.
         * \param[in] compression_level optional compression level, use
         *   0 for uncompressed and 6 for maximum compression.
         */
        OutputGeoFile(const std::string& filename, index_t compression_level=3);

        /**
         * \brief Writes a new attribute set to the file.
         * \param[in] name a const reference to the name of 
         *  the attribute set
         * \param[in] nb_items number of items in the attribute set
         */
        void write_attribute_set(
            const std::string& name, index_t nb_items
        );
        
        /**
         * \brief Writes a new attribute to the file.
         * \param[in] attribute_set_name a const reference to the name of
         *  an attribute set 
         * \param[in] attribute_name a const reference to the name of the
         *  attribute
         * \param[in] element_type a const reference to the C++ name of the
         *  element type
         * \param[in] element_size size in bytes of an element
         * \param[in] dimension number of elements per item
         * \param[in] data a const pointer to the data of the attribute, as
         *  a contiguous array of bytes in memory.
         */
        void write_attribute(
            const std::string& attribute_set_name,
            const std::string& attribute_name,
            const std::string& element_type,
            size_t element_size,            
            index_t dimension,
            const void* data
        );


        /**
         * \brief Writes a new comment to the file.
         * \param[in] comment a const reference to the comment.
         */
        void write_comment(const std::string& comment);

        /**
         * \brief Writes the command line to the file.
         * \details It is useful to save the command line arguments in
         *  each file, so that one can retrieve the parameters of each
         *  experiments when doing algorithm test.
         * \param[in] args the command line, as a vector of strings
         */
        void write_command_line(const std::vector<std::string>& args);


        /**
         * \brief Writes a separator into the file.
         * \details Separators are used to mark the boundaries between
         *  multiple objects saved in the same GeoFile.
         */
        void write_separator();
        
    private:
        /**
         * \brief Forbids copy.
         */
        OutputGeoFile(const InputGeoFile& rhs);

        /**
         * \brief Forbids copy.
         */
        OutputGeoFile& operator=(const InputGeoFile& rhs);
    };

    /**************************************************************/    
}

#endif
