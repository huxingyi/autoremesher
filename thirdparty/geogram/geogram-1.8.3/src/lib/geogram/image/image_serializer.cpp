/*
 *  Copyright (c) 2000-2022 Inria
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
 *  Contact: Bruno Levy
 *
 *     https://www.inria.fr/fr/bruno-levy
 *
 *     Inria,
 *     Domaine de Voluceau,
 *     78150 Le Chesnay - Rocquencourt
 *     FRANCE
 *
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

