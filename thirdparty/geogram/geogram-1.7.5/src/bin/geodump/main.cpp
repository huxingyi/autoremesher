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

#include <geogram/basic/command_line.h>
#include <geogram/basic/command_line_args.h>
#include <geogram/basic/geofile.h>
#include <geogram/basic/logger.h>
#include <iomanip>

int main(int argc, char** argv) {
    GEO::initialize();
    GEO::Logger::instance()->set_quiet(false);
    GEO::CmdLine::import_arg_group("standard");
    std::vector<std::string> filenames;
    if(!GEO::CmdLine::parse(argc, argv, filenames, "filename")) {
        return 1;
    }
    try {
        GEO::InputGeoFile file(filenames[0]);
        for(
            std::string chunk_class = file.next_chunk();
            chunk_class != "EOFL";
            chunk_class = file.next_chunk()) {
            GEO::Logger::out("GeoFile")
                << "Chunk "
                << chunk_class
                << " size= "
                << file.current_chunk_size()
                << std::endl;
            if(chunk_class == "SPTR") {
                GEO::Logger::out("GeoFile")
                    << "=================================================="
                    << std::endl;
            } else if(
                chunk_class == "CMDL" ||
                chunk_class == "HIST"
            ) {
                GEO::index_t nb_lines = file.read_int();
                for(GEO::index_t i=0; i<nb_lines; ++i) {
                    GEO::Logger::out("GeoFile")
                        << "    "
                        << std::setw(3)
                        << i
                        << " : "
                        << file.read_string()
                        << std::endl;
                }
            } else if(chunk_class == "CMNT") {
                GEO::Logger::out("GeoFile")
                    << "comment: "
                    << file.current_comment()
                    << std::endl;
            } else if(
                chunk_class == "GROB" ||
                chunk_class == "SHDR" ||
                chunk_class == "SCNG"
            ) {
                GEO::index_t nb = file.read_int();
                for(GEO::index_t i=0; i<nb; ++i) {
                    std::string name = file.read_string();
                    std::string value = file.read_string();
                    GEO::Logger::out("GeoFile")
                        << "    "
                        << name << "=" << value
                        << std::endl;
                }
            } else if(chunk_class == "ATTS") {
                const GEO::GeoFile::AttributeSetInfo& atts =
                    file.current_attribute_set();
                GEO::Logger::out("GeoFile")
                    << "   name=" << atts.name << std::endl;
                GEO::Logger::out("GeoFile")
                    << "   nb_items=" << atts.nb_items << std::endl;
            } else if(chunk_class == "ATTR") {
                const GEO::GeoFile::AttributeSetInfo& atts =
                    file.current_attribute_set();
                const GEO::GeoFile::AttributeInfo& attr =
                    file.current_attribute();
                GEO::Logger::out("GeoFile")
                    << "   name=" << attr.name << std::endl;
                GEO::Logger::out("GeoFile")
                    << "   attr. set=" << atts.name << std::endl;
                GEO::Logger::out("GeoFile")
                    << "   type=" << attr.element_type << std::endl;
                GEO::Logger::out("GeoFile")
                    << "   dim=" << attr.dimension << std::endl;
            }
        }

    } catch(const std::logic_error& e) {
        GEO::Logger::err("I/O") << "Caught exception " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
