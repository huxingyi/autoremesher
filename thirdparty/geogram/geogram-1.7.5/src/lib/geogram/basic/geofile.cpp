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

#include <geogram/basic/geofile.h>
#include <geogram/basic/string.h>
#include <geogram/basic/logger.h>
#include <ctype.h>

/* Using portable printf modifier for 64 bit ints from pstdint.h */
#include <geogram/third_party/pstdint.h>  
#define INT64_T_FMT "%" PRINTF_INT64_MODIFIER "d"

namespace {
    
    void skip_comments(FILE* f) {
        while(char(fgetc(f)) != '\n');
    }


    std::string encode(const std::string& s) {
        std::string result;
        for(size_t i=0; i<s.size(); ++i) {
            switch(s[i]) {
            case '\\':
                result.push_back('\\');
                result.push_back('\\');                
                break;
            case '\"':
                result.push_back('\\');
                result.push_back('\"');
                break;
            default:
                result.push_back(s[i]);
            }
        }
        return result;
    }

    std::string decode(const std::string& s) {
        std::string result;
        size_t i=0;
        while(i < s.size()) {
            if(s[i] == '\\') {
                ++i;
                if(i >= s.size()) {
                    break;
                }
            }
            result.push_back(s[i]);                
            ++i;
        }
        return result;
    }
    
}

namespace GEO {
    
    /**************************************************************/

    GeoFileException::~GeoFileException() GEO_NOEXCEPT {
    }
    
    /**************************************************************/

    std::map<std::string, GeoFile::AsciiAttributeSerializer>
       GeoFile::ascii_attribute_read_;

    std::map<std::string, GeoFile::AsciiAttributeSerializer>
       GeoFile::ascii_attribute_write_;

    void GeoFile::register_ascii_attribute_serializer(
        const std::string& type_name,
        AsciiAttributeSerializer read,
        AsciiAttributeSerializer write
    ) {
        geo_assert(
            ascii_attribute_read_.find(type_name) ==
            ascii_attribute_read_.end()
        );
        ascii_attribute_read_[type_name] = read;
        geo_assert(
            ascii_attribute_write_.find(type_name) ==
            ascii_attribute_write_.end()
        );
        ascii_attribute_write_[type_name] = write;
    }

    
    
    
    /**************************************************************/
    
    GeoFile::GeoFile(const std::string& filename) :
        filename_(filename),
        file_(nullptr),
        ascii_(false),
        ascii_file_(nullptr),
        current_chunk_class_("0000"),
        current_chunk_size_(0),
        current_chunk_file_pos_(0) {
        ascii_ = String::string_ends_with(filename, "_ascii");
    }
    
    GeoFile::~GeoFile() {
        if(file_ != nullptr) {
            gzclose(file_);
        }
        if(ascii_file_ != nullptr) {
            fclose(ascii_file_);
        }
    }

    void GeoFile::check_chunk_size() {
        if(ascii_) {
            return;
        }
        long chunk_size = gztell(file_) - current_chunk_file_pos_;
        if(current_chunk_size_ != chunk_size) {
            throw GeoFileException(
                std::string("Chunk size mismatch: ") + 
                " expected " + String::to_string(current_chunk_size_) +
                "/ got " + String::to_string(chunk_size)
            );
        }
    }

    void GeoFile::check_zlib_version() {
        if(strcmp(ZLIB_VERSION, zlibVersion())) {
            Logger::warn("GeoFile") << "ZLib version mismatch !" << std::endl;
            Logger::warn("GeoFile") << "  from  header: " << ZLIB_VERSION
                                    << std::endl;
            Logger::warn("GeoFile") << "  from runtime: " << zlibVersion()
                                    << std::endl;
        }
    }

    void GeoFile::read_chunk_header() {
        current_chunk_class_ = read_chunk_class();
        if(ascii_) {
            if(feof(ascii_file_)) {
                fclose(ascii_file_);
                ascii_file_ = nullptr;
                current_chunk_size_ = 0;
                current_chunk_class_ = "EOFL";
                return;
            }
        } else {
            if(gzeof(file_)) {
                gzclose(file_);
                file_ = nullptr;
                current_chunk_size_ = 0;
                current_chunk_class_ = "EOFL";
                return;
            }
            current_chunk_size_ = ascii_ ? 0 : long(read_size());
            current_chunk_file_pos_ = ascii_ ? 0 : gztell(file_);
        }
    }

    void GeoFile::write_chunk_header(
        const std::string& chunk_class, size_t size
    ) {
        write_chunk_class(chunk_class);
        if(!ascii_) {
            write_size(size);
        }
        current_chunk_file_pos_ = ascii_ ? 0 : gztell(file_);
        current_chunk_class_ = chunk_class;
        current_chunk_size_ = long(size);
    }
    
    index_t GeoFile::read_int() {
        Numeric::uint32 result=0;
        if(ascii_) {
            if(fscanf(ascii_file_, "%u", &result) == 0) {
                throw GeoFileException("Could not read integer from file");
            }
            skip_comments(ascii_file_);
            return result;
        }
        int check = gzread(file_, &result, sizeof(Numeric::uint32));
        if(check == 0 && gzeof(file_)) {
            result = Numeric::uint32(-1);
        } else {
            if(size_t(check) != sizeof(Numeric::uint32)) {
                throw GeoFileException("Could not read integer from file");
            }
        }
        return result;
    }
    
    void GeoFile::write_int(index_t x_in, const char* comment) {
        Numeric::uint32 x = Numeric::uint32(x_in);
        if(ascii_) {
            if(comment == nullptr) {
                if(fprintf(ascii_file_,"%u\n",x) ==0) {
                    throw GeoFileException("Could not write integer to file");
                }
            } else {
                if(fprintf(ascii_file_,"%u # this is %s\n",x,comment) ==0) {
                    throw GeoFileException("Could not write integer to file");
                }
            }
            return;
        }
        int check = gzwrite(file_, &x, sizeof(Numeric::uint32));
        if(size_t(check) != sizeof(Numeric::uint32)) {
            throw GeoFileException("Could not write integer to file");
        }
    }

    std::string GeoFile::read_string() {
        std::string result;        
        if(ascii_) {
            int cur;
            while(char(cur = fgetc(ascii_file_)) != '\"') {
                if(cur == EOF) {
                    throw GeoFileException(
                        "Could not read string data from file"
                    );
                }
            }
            while(char(cur = fgetc(ascii_file_)) != '\"') {
                if(cur == EOF) {
                    throw GeoFileException(
                        "Could not read string data from file"
                    );
                }
                result.push_back(char(cur));
            }
            skip_comments(ascii_file_);
            result = decode(result);
            return result;
        }

        index_t len=read_int();
        result.resize(len);
        if(len != 0) {
            int check = gzread(file_, &result[0], (unsigned int)(len));
            if(index_t(check) != len) {
                throw GeoFileException("Could not read string data from file");
            }
        }
        return result;
    }
    
    void GeoFile::write_string(const std::string& str, const char* comment) {
        if(ascii_) {
            if(comment == nullptr) {
                if(fprintf(ascii_file_, "\"%s\"\n", encode(str).c_str()) == 0) {
                    throw GeoFileException("Could not write string data to file");
                }
            } else {
                if(fprintf(ascii_file_, "\"%s\" # this is %s\n", encode(str).c_str(), comment) == 0) {
                    throw GeoFileException("Could not write string data to file");
                }
            }
            return;
        }
        index_t len = index_t(str.length());
        write_int(len);
        if(len != 0) {
            int check = gzwrite(file_, &str[0], (unsigned int)(len));
            if(index_t(check) != len) {
                throw GeoFileException("Could not write string data to file");
            }
        }
    }

    size_t GeoFile::read_size() {
        if(ascii_) {
            int64_t x = 0;
            if(fscanf(ascii_file_, INT64_T_FMT "\n", &x) == 0) {
                throw GeoFileException("Could not write size to file");                
            }
            return size_t(x);
        }
        Numeric::uint64 result=0;
        int check = gzread(file_, &result, sizeof(Numeric::uint64));
        if(check == 0 && gzeof(file_)) {
            result = size_t(-1);
        } else {
            if(size_t(check) != sizeof(Numeric::uint64)) {
                throw GeoFileException("Could not read size from file");
            }
        }
        return size_t(result);
    }
    
    void GeoFile::write_size(size_t x_in) {
        Numeric::uint64 x = Numeric::uint64(x_in);
        if(ascii_) {
            if(fprintf(ascii_file_, INT64_T_FMT "\n", int64_t(x_in)) == 0) {
                throw GeoFileException("Could not write size to file");                
            }
            return;
        }
        int check = gzwrite(file_, &x, sizeof(Numeric::uint64));
        if(size_t(check) != sizeof(Numeric::uint64)) {
            throw GeoFileException("Could not write size to file");
        }
    }

    std::string GeoFile::read_chunk_class() {
        std::string result;
        if(ascii_) {
            int cur;
            while(char(cur = fgetc(ascii_file_)) != '[') {
                if(cur == EOF) {
                    result = "EOFL";
                    return result;
                }
            }
            for(index_t i=0; i<4; ++i) {
                cur = fgetc(ascii_file_);
                if(cur == EOF) {
                    throw GeoFileException(
                        "Could not read chunk class from file"
                    );                        
                }
                result.push_back(char(cur));
            }
            cur = fgetc(ascii_file_);
            if(char(cur) != ']') {
                throw GeoFileException(
                    "Could not read chunk class from file"
                );                        
            }
            return result;
        }
        result.resize(4,'\0');
        int check = gzread(file_, &result[0], 4);
        if(check == 0 && gzeof(file_)) {
            result = "EOFL";
        } else {
            if(check != 4) {
                throw GeoFileException("Could not read chunk class from file");
            }
        }
        return result;
    }

    void GeoFile::write_chunk_class(const std::string& chunk_class) {
        geo_assert(chunk_class.length() == 4);
        if(ascii_) {
            if(fprintf(ascii_file_, "[%s]\n", chunk_class.c_str()) == 0) {
                throw GeoFileException("Could not write chunk class to file");
            }
            return;
        }
        int check = gzwrite(file_, &chunk_class[0], 4);
        if(check != 4) {
            throw GeoFileException("Could not write chunk class to file");
        }
    }

    void GeoFile::write_string_array(const std::vector<std::string>& strings) {
        write_int(index_t(strings.size()),"the number of strings");
        for(index_t i=0; i<strings.size(); ++i) {
            write_string(strings[i]);
        }
    }
    
    void GeoFile::read_string_array(std::vector<std::string>& strings) {
        index_t nb_strings = read_int();
        strings.resize(nb_strings);
        for(index_t i=0; i<nb_strings; ++i) {
            strings[i] = read_string();
        }
    }

    size_t GeoFile::string_array_size(
        const std::vector<std::string>& strings
    ) const {
        size_t result = sizeof(index_t);
        for(index_t i=0; i<strings.size(); ++i) {
            result += string_size(strings[i]);
        }
        return result;
    }

    void GeoFile::clear_attribute_maps() {
        attribute_sets_.clear();
    }
    
    /**********************************************************************/
    
    InputGeoFile::InputGeoFile(
        const std::string& filename
    ) : GeoFile(filename),
        current_attribute_set_(nullptr),
        current_attribute_(nullptr)
    {
        if(ascii_) {
            ascii_file_ = fopen(filename.c_str(), "rb");
            if(ascii_file_ == nullptr) {
                throw GeoFileException("Could not open file: " + filename);
            }
        } else {
            check_zlib_version();
            file_ = gzopen(filename.c_str(), "rb");
            if(file_ == nullptr) {
                throw GeoFileException("Could not open file: " + filename);
            }
        }

        read_chunk_header();
        if(current_chunk_class_ != "HEAD") {
            throw GeoFileException(
                filename + " Does not start with HEAD chunk"
            );
        }
        
        std::string magic = read_string();
        if(magic != "GEOGRAM") {
            throw GeoFileException(
                filename + " is not a GEOGRAM file"
            );
        }
        std::string version = read_string();
        Logger::out("I/O") << "GeoFile version: " << version << std::endl;
        check_chunk_size();
    }

    const std::string& InputGeoFile::next_chunk() {

        // If the file pointer did not advance as expected
        // between two consecutive calls of next_chunk, it
        // means that the client code does not want to
        // read the current chunk, then it needs to be
        // skipped.
        if(ascii_) {
            // TODO: skip chunk mechanism for ASCII
        } else {
            if(gztell(file_) != current_chunk_file_pos_ + current_chunk_size_) {
                skip_chunk();
            }
        }

        read_chunk_header();
        
        if(current_chunk_class_ == "ATTS") {
            std::string attribute_set_name = read_string();
            index_t nb_items = read_int();
            check_chunk_size();
            
            if(find_attribute_set(attribute_set_name) != nullptr) {
                throw GeoFileException(
                    "Duplicate attribute set " + attribute_set_name
                );
            }
            attribute_sets_[attribute_set_name] =
                AttributeSetInfo(attribute_set_name, nb_items);
            current_attribute_set_ = find_attribute_set(attribute_set_name);
            geo_assert(current_attribute_set_ != nullptr);
            current_attribute_ = nullptr;
            current_comment_ = "";
        } else if(current_chunk_class_ == "ATTR") {
            std::string attribute_set_name = read_string();
            std::string attribute_name = read_string();
            std::string element_type = read_string();
            index_t element_size = read_int();
            index_t dimension = read_int();
            current_attribute_set_ = find_attribute_set(attribute_set_name);
            if(current_attribute_set_->find_attribute(attribute_name) != nullptr) {
                throw GeoFileException(
                    "Duplicate attribute " + attribute_name +
                    " in attribute set " + attribute_set_name
                );
            }
            current_attribute_set_->attributes.push_back(
                AttributeInfo(
                    attribute_name,
                    element_type,
                    element_size,
                    dimension
                )
            );
            current_attribute_ =
                current_attribute_set_->find_attribute(attribute_name);
            geo_assert(current_attribute_ != nullptr);
            current_comment_ = "";
            
            if(current_attribute_set_->skip) {
                skip_chunk();
                return next_chunk();
            }
        } else if(current_chunk_class_ == "CMNT") {
            current_attribute_ = nullptr;
            current_attribute_set_ = nullptr;
            current_comment_ = read_string();
            check_chunk_size();
        } else if(current_chunk_class_ == "SPTR") {
            clear_attribute_maps();
        }
        return current_chunk_class_;
    }

    void InputGeoFile::read_attribute(void* addr) {
        geo_assert(current_chunk_class_ == "ATTR");
        if(ascii_) {
            AsciiAttributeSerializer read_attribute_func =
                ascii_attribute_read_[current_attribute_->element_type];
            if(read_attribute_func == nullptr) {
                throw GeoFileException(
                    "No ASCII serializer for type:" +
                    current_attribute_->element_type
                );                
            }
            bool result = (*read_attribute_func)(
                ascii_file_,
                Memory::pointer(addr),
                index_t(
                    current_attribute_set_->nb_items*
                    current_attribute_->dimension
                )
            );
            if(!result) {
                throw GeoFileException(
                    "Could not read attribute " + current_attribute_->name +
                    " in set " + current_attribute_set_->name
                );
            }
            return;
        }
        size_t size =
            size_t(current_attribute_->element_size) *
            size_t(current_attribute_->dimension) *
            size_t(current_attribute_set_->nb_items);
        int check = gzread(file_, addr, (unsigned int)(size));
        if(size_t(check) != size) {
            throw GeoFileException(
                "Could not read attribute " + current_attribute_->name +
                " in set " + current_attribute_set_->name +
                " (" + String::to_string(check) + "/"
                + String::to_string(size) + " bytes read)"
            );
        }
        check_chunk_size();
    }

    void InputGeoFile::skip_chunk() {
        if(ascii_) {
            // TODO
            return;
        }
        gzseek(
            file_,
            current_chunk_size_ + current_chunk_file_pos_,
            SEEK_SET
        );
    }

    void InputGeoFile::skip_attribute_set() {
        geo_assert(current_chunk_class_ == "ATTS");
        current_attribute_set_->skip = true;
    }

    void InputGeoFile::read_command_line(
        std::vector<std::string>& args
    ) {
        geo_assert(current_chunk_class() == "CMDL");
        read_string_array(args);
        check_chunk_size();
    }
    
    /**************************************************************/

    OutputGeoFile::OutputGeoFile(
        const std::string& filename, index_t compression_level
    ) : GeoFile(filename) {

        if(ascii_) {
            ascii_file_ = fopen(filename.c_str(), "wb");
            if(ascii_file_ == nullptr) {
                throw GeoFileException("Could not create file: " + filename);
            }
        } else {
            check_zlib_version();        
            if(compression_level == 0) {
                file_ = gzopen(filename.c_str(), "wb");
            } else {
                file_ = gzopen(
                    filename.c_str(),
                    ("wb" + String::to_string(compression_level)).c_str()
                );
            }
            if(file_ == nullptr) {
                throw GeoFileException("Could not create file: " + filename);
            }
        }
        
        std::string magic = "GEOGRAM";
        std::string version = "1.0";
        write_chunk_header("HEAD", string_size(magic) + string_size(version));
        write_string(magic);
        write_string(version);
        check_chunk_size();
        write_comment(
            "geogram version=" + Environment::instance()->get_value("version")
        );
        write_comment(
            "geogram release date=" +
            Environment::instance()->get_value("release_date")
            );
        write_comment(
            "geogram SVN revision=" +
            Environment::instance()->get_value("SVN revision")
        );
    }

    void OutputGeoFile::write_attribute_set(
        const std::string& attribute_set_name, index_t nb_items
    ) {
        geo_assert(find_attribute_set(attribute_set_name) == nullptr);

        attribute_sets_[attribute_set_name] =
            AttributeSetInfo(attribute_set_name, nb_items);

        write_chunk_header(
            "ATTS",
            string_size(attribute_set_name) +
            sizeof(index_t)
        );
        
        write_string(attribute_set_name, "the name of this attribute set");
        write_int(nb_items, "the number of items in this attribute set");

        check_chunk_size();
    }

    void OutputGeoFile::write_attribute(
        const std::string& attribute_set_name,
        const std::string& attribute_name,
        const std::string& element_type,
        size_t element_size,            
        index_t dimension,
        const void* data
    ) {
        AttributeSetInfo* attribute_set_info = find_attribute_set(
            attribute_set_name
        );
        geo_assert(attribute_set_info != nullptr);
        geo_assert(attribute_set_info->find_attribute(attribute_name) == nullptr);

        size_t data_size =
            element_size * dimension *
            attribute_sets_[attribute_set_name].nb_items;

        write_chunk_header(
            "ATTR",
            string_size(attribute_set_name) +
            string_size(attribute_name) +
            string_size(element_type) +
            sizeof(index_t) +
            sizeof(index_t) +
            data_size
        );
        
        write_string(
            attribute_set_name,
            "the name of the attribute set this attribute belongs to"
        );
        write_string(attribute_name, "the name of this attribute");
        write_string(element_type, "the type of the elements in this attribute");
        write_int(index_t(element_size), "the size of an element (in bytes)");
        write_int(dimension, "the number of elements per item");

        if(ascii_) {
            AsciiAttributeSerializer write_attribute_func =
                ascii_attribute_write_[element_type];
            if(write_attribute_func == nullptr) {
                throw GeoFileException("No ASCII serializer for type:"+element_type);                
            }
            bool result = (*write_attribute_func)(
                ascii_file_, Memory::pointer(data), index_t(data_size/element_size)
            );
            if(!result) {
                throw GeoFileException("Could not write attribute data");                
            }
        } else {
            int check = gzwrite(file_, data, (unsigned int)(data_size));
            if(size_t(check) != data_size) {
                throw GeoFileException("Could not write attribute data");
            }
        }

        check_chunk_size();
        
        attribute_set_info->attributes.push_back(
            AttributeInfo(attribute_name, element_type, element_size, dimension)
        );
    }

    void OutputGeoFile::write_comment(const std::string& comment) {
        write_chunk_header(
            "CMNT",
            string_size(comment)
        );
        write_string(comment);
        check_chunk_size();
    }

    void OutputGeoFile::write_command_line(
        const std::vector<std::string>& args
    ) {
        write_chunk_header("CMDL", string_array_size(args));
	if(ascii_) {
	    std::vector<std::string> new_args;
	    for(const std::string& arg : args) {
		bool serializable = true;
		for(index_t i=0; i<arg.size(); ++i) {
		    if(!isprint(arg[i]) || arg[i] == '\"') {
			serializable = false;
			break;
		    }
		}
		if(serializable) {
		    new_args.push_back(arg);
		} else {
		    Logger::warn("GeoFile") << "Skipping arg: " << arg << std::endl;
		}
	    }
	    write_string_array(new_args);	    
	} else {
	    write_string_array(args);
	}
        check_chunk_size();
    }

    void OutputGeoFile::write_separator() {
        clear_attribute_maps();        
        write_chunk_header("SPTR", 4);
        write_chunk_class("____");
        check_chunk_size();
    }
    
    /**************************************************************/    
}

