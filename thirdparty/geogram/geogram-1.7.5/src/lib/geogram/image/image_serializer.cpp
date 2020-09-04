/*
 *  OGF/Graphite: Geometry and Graphics Programming Library + Utilities
 *  Copyright (C) 2000 Bruno Levy
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     levy@loria.fr
 *
 *     ISA Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 *  Note that the GNU General Public License does not permit incorporating
 *  the Software into proprietary programs. 
 */
 
 
#include <geogram/image/image_serializer.h>
#include <geogram/basic/logger.h>

#include <fstream>

namespace GEO {

//_________________________________________________________


    Image* ImageSerializer::serialize_read(const std::string& file_name) {

        std::fstream::openmode mode = binary() ?
            (std::fstream::in | std::fstream::binary) :
             std::fstream::in ;

        std::ifstream input(file_name.c_str(),mode) ;
        if(!input) {
            Logger::err("ImageSerializer") 
                << "could not open file\'" 
                << file_name << "\'" << std::endl ;
            return nullptr ;
        }
        return serialize_read(input) ;
    }

    bool ImageSerializer::serialize_write(
        const std::string& file_name, const Image* image
    ) {
        std::fstream::openmode mode = binary() ?
            (std::fstream::out | std::fstream::trunc | std::fstream::binary) :
            (std::fstream::out | std::fstream::trunc) ;

        std::ofstream output(file_name.c_str(), mode) ;

        if(!output) {
            Logger::err("ImageSerializer") 
                << "could not open file\'" 
                << file_name << "\'" << std::endl ;
            return false ;
        }

        return serialize_write(output, image) ;
    }

    Image* ImageSerializer::serialize_read(std::istream& stream) {
        geo_argused(stream);
        bool implemented = false ;
        geo_assert(implemented) ;
        return nullptr ;
    }

    bool ImageSerializer::serialize_write(
        std::ostream& stream, const Image* image
    ) {
        geo_argused(stream);
        geo_argused(image);
        bool implemented = false ;
        geo_assert(implemented) ;
        return false ;
    }
    

    bool ImageSerializer::binary() const {
        return true ;
    }

    bool ImageSerializer::streams_supported() const {
        return true ;
    }

    bool ImageSerializer::read_supported() const {
        return false ;
    }

    bool ImageSerializer::write_supported() const {
        return false ;
    }

//_________________________________________________________

}

