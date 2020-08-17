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

#include <geogram/mesh/mesh_io.h>
#include <geogram/mesh/mesh.h>
#include <geogram/mesh/index.h>
#include <geogram/points/colocate.h>
#include <geogram/basic/line_stream.h>
#include <geogram/basic/b_stream.h>
#include <geogram/basic/geofile.h>
#include <geogram/basic/file_system.h>
#include <geogram/basic/command_line.h>
#include <geogram/basic/argused.h>
#include <geogram/basic/logger.h>
#include <geogram/basic/geometry.h>
#include <geogram/bibliography/bibliography.h>

#include <fstream>

extern "C" {
#include <geogram/third_party/LM7/libmeshb7.h>
}

#include <geogram/third_party/rply/rply.h>

// TODO: take into account selected mesh elements
// in loaders and exporters.

// We got some IOHandler classes declared locally that
// have no out-of-line virtual functions. It is not a
// problem since they are only visible from this translation
// unit, but clang will complain.
#ifdef __clang__
#pragma GCC diagnostic ignored "-Wweak-vtables"
#endif

namespace GEO {

    inline void set_mesh_point(
        Mesh& M, index_t v, const double* coords, index_t dim
    ) {
        geo_debug_assert(M.vertices.dimension() >= dim);
        if(M.vertices.single_precision()) {
            float* p = M.vertices.single_precision_point_ptr(v);
            for(index_t c=0; c<dim; ++c) {
                p[c] = float(coords[c]);
            }
        } else {
            double* p = M.vertices.point_ptr(v);
            for(index_t c=0; c<dim; ++c) {
                p[c] = coords[c];
            }
        }
    }

    inline void set_mesh_point(
        Mesh& M, index_t v, const float* coords, index_t dim
    ) {
        geo_debug_assert(M.vertices.dimension() >= dim);
        if(M.vertices.single_precision()) {
            float* p = M.vertices.single_precision_point_ptr(v);
            for(index_t c=0; c<dim; ++c) {
                p[c] = coords[c];
            }
        } else {
            double* p = M.vertices.point_ptr(v);
            for(index_t c=0; c<dim; ++c) {
                p[c] = double(coords[c]);
            }
        }
    }

    
    inline void get_mesh_point(
        const Mesh& M, index_t v, double* coords, index_t dim
    ) {
        geo_debug_assert(M.vertices.dimension() >= dim);
        if(M.vertices.single_precision()) {
            const float* p = M.vertices.single_precision_point_ptr(v);
            for(index_t c=0; c<dim; ++c) {
                coords[c] = double(p[c]);
            }
        } else {
            const double* p = M.vertices.point_ptr(v);
            for(index_t c=0; c<dim; ++c) {
                coords[c] = p[c];
            }
        }
    }

    /************************************************************************/

    /**
     * \brief IO handler for AliasWavefront OBJ format.
     * \see http://en.wikipedia.org/wiki/Wavefront_.obj_file
     */
    class GEOGRAM_API OBJIOHandler : public MeshIOHandler {
    public:
        /**
         * \brief Creates a OBJ IO handler.
         * \param[in] dimension dimension of the vertices 
         *  (3 for regular 3d mesh)
         */
        OBJIOHandler(coord_index_t dimension = 3) :
            dimension_(dimension) {
        }

	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
	    bool ignore_tex_coords = false; 
	        //!ioflags.has_attribute(MESH_VERTEX_TEX_COORD);
	    
	    vector<vec2> tex_vertices;
	    Attribute<double> tex_coord;
            vector<double> P(dimension_);
            if(M.vertices.dimension() != dimension_) {
                M.vertices.set_dimension(dimension_);
            }
            
            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }

            bind_attributes(M, ioflags, true);
            vector<index_t> facet_vertices;
	    vector<index_t> facet_tex_vertices;

            bool first_facet_attribute = true;
            bool read_facet_regions = false;
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                if(in.nb_fields() >= 1) {
                    if(in.field_matches(0, "v")) {
                        for(coord_index_t c = 0; c < dimension_; c++) {
                            if(index_t(c + 1) < in.nb_fields()) {
                                P[c] = in.field_as_double(index_t(c + 1));
                            } else {
                                P[c] = 0.0;
                            }
                        }
                        index_t v = M.vertices.create_vertex();
                        set_mesh_point(M, v, P.data(), dimension_);
                    } else if(
			!ignore_tex_coords &&
			in.field_matches(0, "vt")
		    ) {
			if(!tex_coord_.is_bound()) {
			    tex_coord_.bind_if_is_defined(
				M.facet_corners.attributes(), "tex_coord"
			    );
			    if(tex_coord.is_bound()) {
				if(tex_coord_.dimension() != 2) {
				    tex_coord_.unbind();
				    ignore_tex_coords = true;
				}
			    } else {
				tex_coord_.create_vector_attribute(
				    M.facet_corners.attributes(), "tex_coord", 2
				);
			    }
			}
			
			if(!ignore_tex_coords) {
			    if(
				in.nb_fields() != 3 &&
				in.nb_fields() != 4  // TODO: read 3D UVs ?
			    ) {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << " malformed texture vertex"
                                    << std::endl;
                                unbind_attributes();
                                return false;
			    }
			    tex_vertices.push_back(
				vec2(
				    in.field_as_double(1),
				    in.field_as_double(2))
				);
			}
		    } else if(
                        ioflags.has_element(MESH_FACETS) &&
                        in.field_matches(0, "f")
                    ) {
                        if(in.nb_fields() < 3) {
                            Logger::err("I/O")
                                << "Line " << in.line_number()
                                << ": facet only has " << in.nb_fields()
                                << " corners (at least 3 required)"
                                << std::endl;
                            unbind_attributes();
                            return false;
                        }

                        facet_vertices.resize(0);
			facet_tex_vertices.resize(0);
                        
                        for(index_t i = 1; i < in.nb_fields(); i++) {
			    char* tex_vertex_str = nullptr;
                            for(char* ptr = in.field(i); *ptr != '\0'; ptr++) {
                                if(*ptr == '/') {
				    if(!ignore_tex_coords &&
				       tex_vertex_str == nullptr) {
					tex_vertex_str = ptr+1;
				    }
                                    *ptr = '\0';
                                    break;
                                }
                            }
                            
                            // In .obj files, 
                            // negative vertex index means
                            // nb_vertices - vertex index
			    GEO::signed_index_t
				s_vertex_index = in.field_as_int(i);
                            index_t vertex_index = 0;
                            if(s_vertex_index < 0) {
                                vertex_index = index_t(
                                    1+int(M.vertices.nb()) + s_vertex_index
                                );
                            } else {
                                vertex_index = index_t(s_vertex_index);
                            }
                            if(
                                (vertex_index < 1) ||
                                (vertex_index > M.vertices.nb())
                            ) {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << ": facet corner #" << i
                                    << " references an invalid vertex: "
                                    << vertex_index
                                    << std::endl;
                                unbind_attributes();
                                return false;
                            }
                            facet_vertices.push_back(vertex_index-1);

			    if(tex_vertex_str != nullptr &&
			       tex_vertex_str[0] != '\0' &&
			       tex_vertex_str[0] != '/' 
			    ) {
				int s_tex_vertex_index = atoi(tex_vertex_str);
				index_t tex_vertex_index = 0;
				if(s_tex_vertex_index < 0) {
				    tex_vertex_index = index_t(
					1+int(tex_vertices.size()) +
					s_tex_vertex_index
				    );
				} else {
				    tex_vertex_index = index_t(
					s_tex_vertex_index
				    );
				}
				if(
				    (tex_vertex_index < 1) ||
				    (tex_vertex_index > tex_vertices.size())
				) {
				    Logger::err("I/O")
					<< "Line " << in.line_number()
					<< ": facet corner #" << i
					<< " references an invalid tex vertex: "
					<< tex_vertex_index
					<< std::endl;
				    unbind_attributes();
				    return false;
				}
				if(!ignore_tex_coords) {
				    facet_tex_vertices.push_back(
					tex_vertex_index-1
				    );
				}
			    }
			}
			    
			if(
			    facet_tex_vertices.size() != 0 &&
			    facet_tex_vertices.size() != facet_vertices.size()
			) {
			    Logger::err("I/O")
			    << "Line " << in.line_number()
			    << ": some facet vertices do not have tex vertices"
			    << std::endl;
			    unbind_attributes();
			    return false;
			}
			
                        index_t f = M.facets.create_polygon(
                            facet_vertices.size()
                        );
                        for(index_t lv=0; lv<facet_vertices.size(); ++lv) {
                            M.facets.set_vertex(f,lv,facet_vertices[lv]);
                        }
			if(facet_tex_vertices.size() != 0) {
			    for(index_t lv=0; lv<facet_vertices.size(); ++lv) {
				index_t c = M.facets.corners_begin(f) + lv;
				const vec2 vt =
				    tex_vertices[facet_tex_vertices[lv]];
				tex_coord_[2*c] = vt.x;
				tex_coord_[2*c+1] = vt.y;
			    }
			}
                    } else if(
                        ioflags.has_element(MESH_FACETS) &&
                        facet_region_.is_bound() &&
                        in.field_matches(0, "#")
                    ) {
                        if(
                            in.nb_fields() >= 5 &&
                            in.field_matches(1, "attribute") &&
                            in.field_matches(3, "facet")
                        ) {
                            if(
                                first_facet_attribute &&
                                in.field_matches(2, "chart") &&
                                in.field_matches(4, "integer")
                            ) {
                                read_facet_regions = true;
                            } else {
                                first_facet_attribute = false;
                            }
                        } else if(
                            read_facet_regions &&
                            in.nb_fields() >= 5 &&
                            in.field_matches(1, "attrs") &&
                            in.field_matches(2, "f")
                        ) {
                            index_t facet_index = in.field_as_uint(3);
                            signed_index_t facet_region = in.field_as_int(4);
                            if(
                                (facet_index < 1) ||
                                (facet_index > M.facets.nb())
                            ) {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << ": facet attributes "
                                    << "reference an invalid facet: "
                                    << facet_index
                                    << std::endl;
                                unbind_attributes();
                                return false;
                            }

                            facet_region_[facet_index - 1] =
                                index_t(facet_region);
                        }
                    }
                }
            }
            unbind_attributes();
            return true;
        }

	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {

	    std::string mtl_filename;
	    std::string mtl_filename_fullpath;
	    
	    if(ioflags.get_texture_filename().length() != 0) {
		mtl_filename =
		    FileSystem::base_name(filename) + ".mtl";
		mtl_filename_fullpath =
		    FileSystem::dir_name(filename) + "/" +
		    mtl_filename;
		std::ofstream mtl_out(mtl_filename_fullpath.c_str());
		if(!mtl_out) {
		    Logger::err("I/O") << "Could not create mtl file "
				       << mtl_filename_fullpath
				       << std::endl;
		} else {
		    Logger::out("I/O") << "Saving file "
				       << mtl_filename_fullpath
				       << std::endl;
		    mtl_out << "newmtl Material_0" << std::endl;
		    mtl_out << "map_Kd "
			    << FileSystem::base_name(ioflags.get_texture_filename())
			    << "."
			    << FileSystem::extension(ioflags.get_texture_filename())
			    << std::endl;
		}
	    }
	    
            geo_assert(M.vertices.dimension() >= dimension_);
            std::ofstream out(filename.c_str());
            if(!out) {
                Logger::err("I/O")
                    << "Could not create file \'" 
                    << filename << "\'" << std::endl;
                return false;
            }

            bind_attributes(M, ioflags, false);
            
            std::vector<std::string> args;
            CmdLine::get_args(args);
            for(index_t i = 0; i < args.size(); ++i) {
                out << "# vorpaline " << args[i] << std::endl;
            }

	    if(mtl_filename.length() != 0) {
		out << "mtllib " << mtl_filename << std::endl;
	    }
	    
            vector<double> P(dimension_);
            for(index_t v = 0; v < M.vertices.nb(); ++v) {
                get_mesh_point(M, v, P.data(), dimension_);
                out << "v ";
                for(index_t c = 0; c < dimension_; ++c) {
                    out << P[c] << ' ';
                }
                out << std::endl;
            }

	    // If mesh has facet corner tex coords, then "compress" tex coords
	    // by generating a single "texture vertex" (vt) for each group of
	    // corners with the same texture coordinates (makes the .obj file
	    // smaller).
	    vector<index_t> vt_old2new;
	    vector<index_t> vt_index;
	    if(tex_coord_.is_bound()) {
		index_t nb_vt = Geom::colocate_by_lexico_sort(
		    &tex_coord_[0], 2, M.facet_corners.nb(), vt_old2new, 2
		);
		vt_index.assign(M.facet_corners.nb(), index_t(-1));
		index_t cur_vt=0;
		for(index_t c=0; c<M.facet_corners.nb(); ++c) {
		    if(vt_old2new[c] == c) {
			out << "vt " << tex_coord_[2*c] << " "
			    << tex_coord_[2*c+1] << std::endl;
			vt_index[c] = cur_vt;
			++cur_vt;
		    }
		}
		geo_assert(cur_vt == nb_vt);
	    } else if(vertex_tex_coord_.is_bound()) {
		for(index_t v=0; v<M.vertices.nb(); ++v) {
		    out << "vt " << vertex_tex_coord_[2*v] << " "
			<< vertex_tex_coord_[2*v+1] << std::endl;
		}
	    }

	    out << "usemtl Material_0" << std::endl;
            if(ioflags.has_element(MESH_FACETS)) {
                for(index_t f = 0; f < M.facets.nb(); ++f) {
                    out << "f ";
                    for(index_t c = M.facets.corners_begin(f);
                        c < M.facets.corners_end(f); ++c
                    ) {
                        out << M.facet_corners.vertex(c) + 1;
			if(tex_coord_.is_bound()) {
			    out << "/" << vt_index[ vt_old2new[c] ] + 1;
			} else if(vertex_tex_coord_.is_bound()) {
			    out << "/" << M.facet_corners.vertex(c) + 1;
			}
			out << " ";
                    }
                    out << std::endl;
                }
                if(
                    facet_region_.is_bound()
                ) {
                    out << "# attribute chart facet integer" << std::endl;
                    for(index_t f = 0; f < M.facets.nb(); ++f) {
                        out << "# attrs f "
                            << f + 1 << " "
                            << facet_region_[f] << std::endl;
                    }
                }
            }

            unbind_attributes();
            
            return true;
        }

    protected:
	~OBJIOHandler() override {
        }

        void bind_attributes(
            const Mesh& M, const MeshIOFlags& flags, bool create
	) override {
	    MeshIOHandler::bind_attributes(M, flags, create);
	    
	    tex_coord_.bind_if_is_defined(
		M.facet_corners.attributes(), "tex_coord"
	    );
	    if(tex_coord_.is_bound() && tex_coord_.dimension() != 2) {
		tex_coord_.unbind();
	    }

	    vertex_tex_coord_.bind_if_is_defined(
		M.vertices.attributes(), "tex_coord"
	    );
	    if(
		vertex_tex_coord_.is_bound() &&
		vertex_tex_coord_.dimension() != 2
	    ) {
		vertex_tex_coord_.unbind();
	    }
	    
	}

	void unbind_attributes() override {
	    if(tex_coord_.is_bound()) {
		tex_coord_.unbind();
	    }
	    if(vertex_tex_coord_.is_bound()) {
		vertex_tex_coord_.unbind();
	    }
	    MeshIOHandler::unbind_attributes();
	}
	
    private:
        coord_index_t dimension_;
	Attribute<double> tex_coord_;
	Attribute<double> vertex_tex_coord_;
    };
    
    
    /************************************************************************/

    /**
     * \brief IO handler for the OBJ6 file format
     * \see OBJIOHandler
     */
    class GEOGRAM_API OBJ6IOHandler : public OBJIOHandler {
    public:
        OBJ6IOHandler() :
            OBJIOHandler(6) {
        }
    };
    
    /************************************************************************/

    
    /**
     * \brief IO handler for LM5/LM6/Gamma mesh file format
     * \see http://www-roc.inria.fr/gamma/gamma/Membres/CIPD/Loic.Marechal/Research/LM5.html
     */
    class GEOGRAM_API LMIOHandler : public MeshIOHandler {
    public:

        LMIOHandler() {
	    
	    geo_cite("WEB:libMeshb");
	    
            keyword2name_[GmfTriangles] = "triangle";
            keyword2name_[GmfQuadrilaterals] = "quad";
            keyword2name_[GmfTetrahedra] = "tet";
            keyword2name_[GmfHexahedra] = "hex";
            keyword2name_[GmfPrisms] = "prism";
            keyword2name_[GmfPyramids] = "pyramid";
            keyword2name_[GmfEdges] = "edge";
            keyword2nbv_[GmfTriangles] = 3;
            keyword2nbv_[GmfQuadrilaterals] = 4;
            keyword2nbv_[GmfTetrahedra] = 4;
            keyword2nbv_[GmfHexahedra] = 8;
            keyword2nbv_[GmfPrisms] = 6;
            keyword2nbv_[GmfPyramids] = 5;
            keyword2nbv_[GmfEdges] = 2;            
        }
        
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {

            int ver, dim;
            int64_t mesh_file_handle = GmfOpenMesh(
                const_cast<char*>(filename.c_str()), GmfRead, &ver, &dim
            );
            if(!mesh_file_handle) {
                Logger::err("I/O") << "Could not open file: "
                    << filename << std::endl;
                return false;
            }

            if(ver != GmfFloat && ver != GmfDouble) {
                Logger::err("I/O") << "Invalid version: " << ver << std::endl;
                GmfCloseMesh(mesh_file_handle);
                return false;
            }

            bool use_doubles = (ver == GmfDouble);

            if(dim != 3) {
                Logger::err("I/O") << "Invalid dimension: " << dim << std::endl;
                GmfCloseMesh(mesh_file_handle);
                return false;
            }

            bind_attributes(M, ioflags, true);
            
            index_t nb_vertices =
                index_t(GmfStatKwd(mesh_file_handle, GmfVertices));

            index_t nb_edges =
                index_t(GmfStatKwd(mesh_file_handle, GmfEdges));
            
            index_t nb_tris =
                index_t(GmfStatKwd(mesh_file_handle, GmfTriangles));
            index_t nb_quads =
                index_t(GmfStatKwd(mesh_file_handle, GmfQuadrilaterals));
            
            index_t nb_tets =
                index_t(GmfStatKwd(mesh_file_handle, GmfTetrahedra));
            index_t nb_hexes =
                index_t(GmfStatKwd(mesh_file_handle, GmfHexahedra));
            index_t nb_prisms =
                index_t(GmfStatKwd(mesh_file_handle, GmfPrisms));
            index_t nb_pyramids =
                index_t(GmfStatKwd(mesh_file_handle, GmfPyramids));
            
            // Read vertices
            if(!goto_elements(mesh_file_handle, GmfVertices)) {
                return false;
            }
            M.vertices.create_vertices(nb_vertices);
            if(use_doubles) {
                for(index_t v = 0; v < index_t(nb_vertices); ++v) {
                    double xyz[3];
                    int ref;
                    if(!GmfGetLin(
                           mesh_file_handle, GmfVertices,
                           &xyz[0], &xyz[1], &xyz[2], &ref
                    )) {
                        Logger::err("I/O") << "Failed to read vertex #" << v
                            << std::endl;
                        GmfCloseMesh(mesh_file_handle);
                        unbind_attributes();
                        return false;
                    }
                    set_mesh_point(M,v,xyz,3);
                    if(vertex_region_.is_bound()) {
                        vertex_region_[v] = index_t(ref);
                    }
                }
            } else {
                for(index_t v = 0; v < index_t(nb_vertices); ++v) {
                    float x,y,z;
                    double xyz[3];
                    int ref;
                    if(!GmfGetLin(
                           mesh_file_handle, GmfVertices, &x, &y, &z, &ref)
                    ) {
                        Logger::err("I/O") << "Failed to read vertex #" << v
                            << std::endl;
                        GmfCloseMesh(mesh_file_handle);
                        return false;
                    }
                    xyz[0] = double(x);
                    xyz[1] = double(y);
                    xyz[2] = double(z);                    
                    set_mesh_point(M,v,xyz,3);
                    if(vertex_region_.is_bound()) {
                        vertex_region_[v] = index_t(ref);
                    }
                }
            }

            if(ioflags.has_element(MESH_EDGES)) {
                if(nb_edges > 0) {
                    if(!goto_elements(mesh_file_handle, GmfEdges)) {
                        return false;
                    }
                    index_t first_edge = M.edges.create_edges(nb_edges);
                    int v[8];
                    int ref;
                    for(index_t e=0; e<nb_edges; ++e) {
                        if(!read_element(
                               mesh_file_handle, GmfEdges, v, ref, M, e
                        )) {
                            return false;
                        }
                        for(index_t lv=0; lv<2; ++lv) {
                            M.edges.set_vertex(
                                first_edge+e, lv, index_t(v[lv]-1)
                            );
                        }
                    }                    
                }
            }
            
            if(ioflags.has_element(MESH_FACETS)) {
                // Read triangles
                if(nb_tris > 0) {
                    if(!goto_elements(mesh_file_handle, GmfTriangles)) {
                        return false;
                    }
                    index_t first_tri = M.facets.create_triangles(nb_tris);
                    int v[8];
                    int ref;
                    for(index_t t=0; t<nb_tris; ++t) {
                        if(!read_element(
                               mesh_file_handle, GmfTriangles, v, ref, M, t
                        )) {
                            return false;
                        }
                        for(index_t lv=0; lv<3; ++lv) {
                            M.facets.set_vertex(
                                first_tri+t, lv, index_t(v[lv]-1)
                            );
                        }
                        if(facet_region_.is_bound()) {
                            facet_region_[first_tri+t] = index_t(ref);
                        }
                    }
                }
                
                // Read quads
                if(nb_quads > 0) {
                    if(!goto_elements(mesh_file_handle, GmfQuadrilaterals)) {
                        return false;
                    }
                    index_t first_quad = M.facets.create_quads(nb_quads);
                    int v[8];
                    int ref;
                    for(index_t q=0; q<nb_quads; ++q) {
                        if(!read_element(
                               mesh_file_handle, GmfQuadrilaterals,
                               v, ref, M, q
                        )) {
                            return false;
                        }
                        for(index_t lv=0; lv<4; ++lv) {
                            M.facets.set_vertex(
                                first_quad+q, lv, index_t(v[lv]-1)
                            );
                        }
                        if(facet_region_.is_bound()) {
                            facet_region_[first_quad+q] = index_t(ref);
                        }
                    }
                }
            }

            if(ioflags.has_element(MESH_CELLS)) {

                // Read tets
                if(nb_tets > 0) {
                    if(!goto_elements(mesh_file_handle, GmfTetrahedra)) {
                        return false;
                    }
                    index_t first_tet = M.cells.create_tets(nb_tets);
                    int v[8];
                    int ref;
                    for(index_t t=0; t<nb_tets; ++t) {
                        if(!read_element(
                               mesh_file_handle, GmfTetrahedra, v, ref, M, t
                        )) {
                            return false;
                        }
                        for(index_t lv=0; lv<4; ++lv) {
                            M.cells.set_vertex(
                                first_tet+t, lv, index_t(v[lv]-1)
                            );
                        }
                        if(cell_region_.is_bound()) {
                            cell_region_[first_tet+t] = index_t(ref);
                        }
                    }
                }

                // Read hexes
                if(nb_hexes > 0) {
                    if(!goto_elements(mesh_file_handle, GmfHexahedra)) {
                        return false;
                    }
                    index_t first_hex = M.cells.create_hexes(nb_hexes);
                    
                    int v[8];
                    int ref;
                    for(index_t h=0; h<nb_hexes; ++h) {
                        if(!read_element(
                               mesh_file_handle, GmfHexahedra, v, ref, M, h
                        )) {
                            return false;
                        }

                        // Swapping vertices 1<->0 and 4<->5 to
                        // account for differences in the indexing
                        // convetions in .mesh/.meshb files w.r.t.
                        // geogram internal conventions.
                        std::swap(v[0], v[1]);
                        std::swap(v[4], v[5]);
                        
                        for(index_t lv=0; lv<8; ++lv) {
                            M.cells.set_vertex(
                                first_hex+h, lv, index_t(v[lv]-1)
                            );
                        }
                        if(cell_region_.is_bound()) {
                            cell_region_[first_hex+h] = index_t(ref);
                        }
                    }
                }

                // Read prisms
                if(nb_prisms > 0) {
                    if(!goto_elements(mesh_file_handle, GmfPrisms)) {
                        return false;
                    }
                    index_t first_prism = M.cells.create_prisms(nb_prisms);
                    int v[8];
                    int ref;
                    for(index_t p=0; p<nb_prisms; ++p) {
                        if(!read_element(
                               mesh_file_handle, GmfPrisms, v, ref, M, p
                        )) {
                            return false;
                        }
                        for(index_t lv=0; lv<6; ++lv) {
                            M.cells.set_vertex(
                                first_prism+p, lv, index_t(v[lv]-1)
                            );
                        }
                        if(cell_region_.is_bound()) {
                            cell_region_[first_prism+p] = index_t(ref);
                        }
                    }
                }

                // Read pyramids
                if(nb_pyramids > 0) {
                    if(!goto_elements(mesh_file_handle, GmfPyramids)) {
                        return false;
                    }
                    index_t first_pyramid =
                        M.cells.create_pyramids(nb_pyramids);
                    int v[8];
                    int ref;
                    for(index_t p=0; p<nb_pyramids; ++p) {
                        if(!read_element(
                               mesh_file_handle, GmfPyramids, v, ref, M, p
                        )) {
                            return false;
                        }
                        for(index_t lv=0; lv<5; ++lv) {
                            M.cells.set_vertex(
                                first_pyramid+p, lv, index_t(v[lv]-1)
                            );
                        }
                        if(cell_region_.is_bound()) {
                            cell_region_[first_pyramid+p] = index_t(ref);
                        }
                    }
                }
            }
            
            GmfCloseMesh(mesh_file_handle);
            unbind_attributes();            
            return true;
        }

	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
            bool use_doubles = CmdLine::get_arg_bool("sys:use_doubles");
            int64_t mesh_file_handle = GmfOpenMesh(
                const_cast<char*>(filename.c_str()), GmfWrite,
                (use_doubles ? GmfDouble : GmfFloat), 3
            );

            if(mesh_file_handle == 0) {
                Logger::err("I/O")
                    << "Could not create file \'" << filename << "\'"
                    << std::endl;
                return false;
            }
            bind_attributes(M, ioflags, false);

            // Save vertices
            GmfSetKwd(mesh_file_handle, GmfVertices, M.vertices.nb());
            for(index_t v = 0; v < M.vertices.nb(); ++v) {
                double xyz[3];
                index_t ref =
                    vertex_region_.is_bound() ? vertex_region_[v] : 0;
                get_mesh_point(M, v, xyz, 3);
                GmfSetLin(
                    mesh_file_handle, GmfVertices, xyz[0], xyz[1], xyz[2], ref
                );
            }

            if(ioflags.has_element(MESH_FACETS)) {
                
                index_t nb_tris = 0;
                index_t nb_quads = 0;
                index_t nb_other = 0;
                
                for(index_t f = 0; f < M.facets.nb(); ++f) {
                    switch(M.facets.nb_vertices(f)) {
                    case 3:
                        nb_tris++;
                        break;
                    case 4:
                        nb_quads++;
                        break;
                    default:
                        nb_other++;
                        break;
                    }
                }

                if(nb_tris > 0) {
                    GmfSetKwd(mesh_file_handle, GmfTriangles, nb_tris);
                    for(index_t f = 0; f < M.facets.nb(); ++f) {
                        if(M.facets.nb_vertices(f) == 3) {
                            index_t ref =
                                facet_region_.is_bound() ?
                                facet_region_[f] : 0 ;
                            GmfSetLin(
                                mesh_file_handle, GmfTriangles,
                                int(M.facets.vertex(f,0)+1),
                                int(M.facets.vertex(f,1)+1),
                                int(M.facets.vertex(f,2)+1),
                                int(ref)
                            );
                        }
                    }
                }

                if(nb_quads > 0) {
                    GmfSetKwd(mesh_file_handle, GmfQuadrilaterals, nb_quads);
                    for(index_t f = 0; f < M.facets.nb(); ++f) {
                        if(M.facets.nb_vertices(f) == 4) {
                            index_t ref =
                                facet_region_.is_bound() ?
                                facet_region_[f] : 0 ;
                            GmfSetLin(
                                mesh_file_handle, GmfQuadrilaterals,
                                int(M.facets.vertex(f,0)+1),
                                int(M.facets.vertex(f,1)+1),
                                int(M.facets.vertex(f,2)+1),
                                int(M.facets.vertex(f,3)+1),            
                                int(ref)
                            );
                        }
                    }
                }

                if(nb_other > 0) {
                    Logger::warn("I/O")
                        << "Encountered " << nb_other 
                        << " non-tri / non-quad facets"
                        << " (not saved)"
                        << std::endl;
                }
            }

            if(ioflags.has_element(MESH_EDGES)) {
                GmfSetKwd(mesh_file_handle, GmfEdges, M.edges.nb());
                for(index_t e=0; e<M.edges.nb(); ++e) {
                    index_t ref = 0;
                    GmfSetLin(
                        mesh_file_handle, GmfEdges,
                        int(M.edges.vertex(e,0) + 1),
                        int(M.edges.vertex(e,1) + 1),
                        ref
                    );
                }
            }
            
            if(ioflags.has_element(MESH_CELLS)) {
                index_t nb_tets=0;
                index_t nb_hexes=0;
                index_t nb_prisms=0;
                index_t nb_pyramids=0;
                for(index_t c=0; c<M.cells.nb(); ++c) {
                    switch(M.cells.type(c)) {
                    case MESH_TET:
                        ++nb_tets;
                        break;
                    case MESH_HEX:
                        ++nb_hexes;
                        break;
                    case MESH_PRISM:
                        ++nb_prisms;
                        break;
                    case MESH_PYRAMID:
                        ++nb_pyramids;
                        break;
                    case MESH_CONNECTOR:
                    case MESH_NB_CELL_TYPES:
                        break;
                    }
                }

                if(nb_tets > 0) {
                    GmfSetKwd(mesh_file_handle, GmfTetrahedra, nb_tets);
                    for(index_t c=0; c<M.cells.nb(); ++c) {
                        if(M.cells.type(c) == MESH_TET) {
                            index_t ref =
                                cell_region_.is_bound() ? cell_region_[c] : 0;
                            GmfSetLin(
                                mesh_file_handle, GmfTetrahedra,
                                int(M.cells.vertex(c,0) + 1),
                                int(M.cells.vertex(c,1) + 1),
                                int(M.cells.vertex(c,2) + 1),
                                int(M.cells.vertex(c,3) + 1),
                                ref
                            );
                        }
                    }
                }

                if(nb_hexes > 0) {
                    GmfSetKwd(mesh_file_handle, GmfHexahedra, nb_hexes);
                    for(index_t c=0; c<M.cells.nb(); ++c) {
                        if(M.cells.type(c) == MESH_HEX) {
                            index_t ref =
                                cell_region_.is_bound() ? cell_region_[c] : 0;

                            // Swapping vertices 1<->0 and 4<->5 to
                            // account for differences in the indexing
                            // convetions in .mesh/.meshb files w.r.t.
                            // geogram internal conventions.
                            
                            GmfSetLin(
                                mesh_file_handle, GmfHexahedra,
                                int(M.cells.vertex(c,1) + 1),
                                int(M.cells.vertex(c,0) + 1),
                                int(M.cells.vertex(c,2) + 1),
                                int(M.cells.vertex(c,3) + 1),
                                int(M.cells.vertex(c,5) + 1),
                                int(M.cells.vertex(c,4) + 1),
                                int(M.cells.vertex(c,6) + 1),
                                int(M.cells.vertex(c,7) + 1),
                                ref
                            );
                        }
                    }
                }

                if(nb_prisms > 0) {
                    GmfSetKwd(mesh_file_handle, GmfPrisms, nb_prisms);
                    for(index_t c=0; c<M.cells.nb(); ++c) {
                        if(M.cells.type(c) == MESH_PRISM) {
                            index_t ref =
                                cell_region_.is_bound() ? cell_region_[c] : 0;
                            GmfSetLin(
                                mesh_file_handle, GmfPrisms,
                                int(M.cells.vertex(c,0) + 1),
                                int(M.cells.vertex(c,1) + 1),
                                int(M.cells.vertex(c,2) + 1),
                                int(M.cells.vertex(c,3) + 1),
                                int(M.cells.vertex(c,4) + 1),
                                int(M.cells.vertex(c,5) + 1),
                                ref
                            );
                        }
                    }
                }

                if(nb_pyramids > 0) {
                    GmfSetKwd(mesh_file_handle, GmfPyramids, nb_pyramids);
                    for(index_t c=0; c<M.cells.nb(); ++c) {
                        if(M.cells.type(c) == MESH_PYRAMID) {
                            index_t ref =
                                cell_region_.is_bound() ? cell_region_[c] : 0;
                            GmfSetLin(
                                mesh_file_handle, GmfPyramids,
                                int(M.cells.vertex(c,0) + 1),
                                int(M.cells.vertex(c,1) + 1),
                                int(M.cells.vertex(c,2) + 1),
                                int(M.cells.vertex(c,3) + 1),
                                int(M.cells.vertex(c,4) + 1),
                                ref
                            );
                        }
                    }
                }
                
            }
            
            unbind_attributes();
            GmfCloseMesh(mesh_file_handle);

            // If file is in ASCII, append parameters as comments
            // at the end of the file.
            if(FileSystem::extension(filename) == "mesh") {
                FILE* f = fopen(filename.c_str(), "a");
                std::vector<std::string> args;
                CmdLine::get_args(args);
                for(index_t i = 0; i < args.size(); i++) {
                    fprintf(f, "# vorpaline %s\n", args[i].c_str());
                }
                fclose(f);
            }
            
            return true;
        }

    protected:
        bool goto_elements(int64_t mesh_file_handle, int keyword) {
            if(!GmfGotoKwd(mesh_file_handle, keyword)) {
                Logger::err("I/O") << "Failed to access "
                                   << keyword2name_[keyword]
                                   << " section"
                                   << std::endl;
                GmfCloseMesh(mesh_file_handle);
                unbind_attributes();
                return false;
            }
            return true;
        }
        
        bool read_element(
            int64_t mesh_file_handle, 
            int keyword, int *v, int& ref,
            Mesh& M, index_t element_id
        ) {
            index_t nbv = keyword2nbv_[keyword];
            int res = 0;
            switch(nbv) {
            case 2:
                res = GmfGetLin(
                    mesh_file_handle, keyword,
                    &v[0], &v[1], &ref
                );
                break;
            case 3:
                res = GmfGetLin(
                    mesh_file_handle, keyword,
                    &v[0], &v[1], &v[2], &ref
                );
                break;
            case 4:
                res = GmfGetLin(
                    mesh_file_handle, keyword,
                    &v[0], &v[1], &v[2], &v[3], &ref
                );
                break;
            case 5:
                res = GmfGetLin(
                    mesh_file_handle, keyword,
                    &v[0], &v[1], &v[2], &v[3], &v[4],
                    &ref
                );
                break;
            case 6:
                res = GmfGetLin(
                    mesh_file_handle, keyword,
                    &v[0], &v[1], &v[2], 
                    &v[3], &v[4], &v[5], 
                    &ref
                );
                break;
            case 8:
                res = GmfGetLin(
                    mesh_file_handle, keyword,
                    &v[0], &v[1], &v[2], &v[3],
                    &v[4], &v[5], &v[6], &v[7],                    
                    &ref
                );
                break;
            default:
                geo_assert_not_reached;
            }

            if(!res) {
                Logger::err("I/O")
                    << "Failed to read "
                    << keyword2name_[keyword]                    
                    << " #" << element_id
                    << std::endl;
                GmfCloseMesh(mesh_file_handle);
                unbind_attributes();
                return false;
            }

            for(index_t lv=0; lv < nbv; ++lv) {
                if(
                    v[lv] < 1 ||
                    index_t(v[lv]) > M.vertices.nb()
                ) {
                    Logger::err("I/O")
                        << "Error: " << keyword2name_[keyword]
                        <<" # " << element_id
                        << " references an invalid vertex: " << v[lv]
                        << std::endl;
                    GmfCloseMesh(mesh_file_handle);
                    unbind_attributes();                    
                    return false;
                }
            }
            
            return true;
        }
        
    protected:
        std::string keyword2name_[GmfLastKeyword+1];
        index_t keyword2nbv_[GmfLastKeyword+1];
    };

    /************************************************************************/

    /**
     * \brief IO handler for the PLY file format
     * \details ASCII and binary, single and double precision are supported
     * \see http://en.wikipedia.org/wiki/PLY_(file_format)
     */
    class GEOGRAM_API PLYIOHandler : public MeshIOHandler {
    public:

	/**
	 * \brief PLYIOHandler constructor.
	 */
	PLYIOHandler() {
	    geo_cite("WEB:rply");
	}
	
        /**
         * \brief Helper class to read files in PLY format
         */
        class PlyLoader {
        public:

            /**
             * \brief Construts a new PlyLoader.
             * \param[in] filename name of the file to be loaded
             * \param[out] M the loaded mesh
             * \param[in] flags flags that determine which elements
             *  and attributes should be read
             */
            PlyLoader(
                const std::string& filename, Mesh& M, const MeshIOFlags& flags
            ) :
                mesh_(M),
                filename_(filename),
                flags_(flags),
                current_vertex_(max_index_t()),
                has_colors_(false),
                color_mult_(1.0),
                current_color_(max_index_t()),
                tristrip_index_(0),
                load_colors_(true) {
            }

            /**
             * \brief Specifies whether vertices colors should be loaded.
             */
            void set_load_colors(bool x) {
                load_colors_ = x;
            }

            /**
             * \brief Loads the file.
             * \return true on success, false otherwise
             */
            bool load() {
                p_ply ply = ply_open(filename_.c_str(), nullptr, 0, nullptr);

                if(ply == nullptr) {
                    Logger::err("I/O")
                        << "Could not open file: " << filename_
                        << std::endl;
                    return false;
                }

                if(!ply_read_header(ply)) {
                    Logger::err("I/O")
                        << "Invalid PLY header"
                        << std::endl;
                    ply_close(ply);
                    return false;
                }

                current_vertex_ = 0;
                current_color_ = 0;
                if(load_colors_) {
                    check_for_colors(ply);
                } else {
                    has_colors_ = false;
                }

                long nvertices = ply_set_read_cb(
                    ply, "vertex", "x", PlyLoader::vertex_cb, this, 0
                );
                ply_set_read_cb(
                    ply, "vertex", "y", PlyLoader::vertex_cb, this, 1
                );
                ply_set_read_cb(
                    ply, "vertex", "z", PlyLoader::vertex_cb, this, 2
                );

                long nfaces = 0;
                long ntstrips = 0;
                if(flags_.has_element(MESH_FACETS)) {
                    nfaces += ply_set_read_cb(
                        ply, "face", "vertex_indices",
                        PlyLoader::face_cb, this, 0
                    );
                    nfaces += ply_set_read_cb(
                        ply, "face", "vertex_index",
                        PlyLoader::face_cb, this, 0
                    );
                    ntstrips += ply_set_read_cb(
                        ply, "tristrips", "vertex_indices",
                        PlyLoader::tristrip_cb, this, 0
                    );
                    ntstrips += ply_set_read_cb(
                        ply, "tristrips", "vertex_index",
                        PlyLoader::tristrip_cb, this, 0
                    );
                }
                if(nvertices == 0) {
                    Logger::err("I/O") 
                        << "File contains no vertices" << std::endl;
                    ply_close(ply);
                    return false;
                }

                mesh_.vertices.create_vertices(index_t(nvertices));

		geo_argused(nfaces);
		geo_argused(ntstrips);
                // TODO: here we could create / reserve facets
                
                if(!ply_read(ply)) {
                    Logger::err("I/O")
                        << "Problem occurred while parsing PLY file"
                        << std::endl;
                    ply_close(ply);
                    return false;
                }

                ply_close(ply);

                if(current_vertex_ != mesh_.vertices.nb()) {
                    Logger::err("I/O")
                        << "File does not contain enough vertex data"
                        << std::endl;
                    return false;
                }

                if(
                    has_colors_ &&
                    current_color_ != mesh_.vertices.nb()
                ) {
                    Logger::err("I/O")
                        << "File does not contain enough color data"
                        << std::endl;
                    return false;
                }

                return true;
            }

        protected:
            /**
             * \brief Detects whether the input file has colors
             * \param[in] ply the p_ply handle to the file
             */
            void check_for_colors(p_ply ply) {
                p_ply_element element = nullptr;

                bool has_r = false;
                bool has_g = false;
                bool has_b = false;

                bool has_red = false;
                bool has_green = false;
                bool has_blue = false;

                for(;;) {
                    element = ply_get_next_element(ply, element);
                    if(element == nullptr) {
                        break;
                    }
                    const char* elt_name = nullptr;
                    ply_get_element_info(element, &elt_name, nullptr);

                    if(!strcmp(elt_name, "vertex")) {
                        p_ply_property property = nullptr;
                        for(;;) {
                            property = ply_get_next_property(element, property);
                            if(property == nullptr) {
                                break;
                            }
                            const char* prop_name = nullptr;
                            ply_get_property_info(
                                property, &prop_name, nullptr, nullptr, nullptr
                            );
                            has_r = has_r || !strcmp(prop_name, "r");
                            has_g = has_g || !strcmp(prop_name, "g");
                            has_b = has_b || !strcmp(prop_name, "b");
                            has_red = has_red ||
                                !strcmp(prop_name, "red");
                            has_green = has_green ||
                                !strcmp(prop_name, "green");
                            has_blue = has_blue ||
                                !strcmp(prop_name, "blue");
                        }
                    }
                }

                if(has_r && has_g && has_b) {
                    has_colors_ = true;
                    color_mult_ = 1.0;
                    ply_set_read_cb(
                        ply, "vertex", "r", PlyLoader::color_cb, this, 0
                    );
                    ply_set_read_cb(
                        ply, "vertex", "g", PlyLoader::color_cb, this, 1
                    );
                    ply_set_read_cb(
                        ply, "vertex", "b", PlyLoader::color_cb, this, 2
                    );
                } else if(has_red && has_green && has_blue) {
                    has_colors_ = true;
                    color_mult_ = 1.0 / 255.0;
                    ply_set_read_cb(
                        ply, "vertex", "red", PlyLoader::color_cb, this, 0
                    );
                    ply_set_read_cb(
                        ply, "vertex", "green", PlyLoader::color_cb, this, 1
                    );
                    ply_set_read_cb(
                        ply, "vertex", "blue", PlyLoader::color_cb, this, 2
                    );
                } else {
                    has_colors_ = false;
                }

		if(!has_colors_) {
		    return;
		}
		
		vertex_color_.bind_if_is_defined(
		    mesh_.vertices.attributes(), "color"
		);
		if(vertex_color_.is_bound() &&
		   vertex_color_.dimension() != 3
		) {
		    Logger::warn("PLY")
			<< "Mesh already has a color attribute "
			<< "that is not of dimension 3"
			<< std::endl;
		    has_colors_ = false;
		    return;
		}
		if(!vertex_color_.is_bound()) {
		    vertex_color_.create_vector_attribute(
			mesh_.vertices.attributes(),
			"color",
			3
		    );
		}
            }

            /**
             * \brief Gets the PlyLoader associated with 
             *  an opaque p_ply_argument.
             * \details Used to pass a Plyloader through libply callbacks
             * \param[in] argument the opaque p_ply_argument
             * \return the PlyLoader associated with \p argument
             */
            static PlyLoader* loader(p_ply_argument argument) {
                PlyLoader* result = nullptr;
                ply_get_argument_user_data(
		    argument, (void**) (&result), nullptr
		);
                geo_debug_assert(result != nullptr);
                return result;
            }

            /**
             * \brief The vertex callback, called for each vertex 
             *  of the input file.
             * \param[in] argument the generic opaque argument 
             *  (from which this PlyLoader is retrieved).
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            static int vertex_cb(p_ply_argument argument) {
                return loader(argument)->add_vertex_data(argument);
            }

            /**
             * \brief The facet callback, called for each facet 
             *  of the input file.
             * \param[in] argument the generic opaque argument 
             *  (from which this PlyLoader is retrieved).
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            static int face_cb(p_ply_argument argument) {
                return loader(argument)->add_face_data(argument);
            }

            /**
             * \brief The triangle strip callback, 
             *  called for each triangle strip of the input file.
             * \param[in] argument the generic opaque argument 
             *  (from which this PlyLoader is retrieved).
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            static int tristrip_cb(p_ply_argument argument) {
                return loader(argument)->add_tristrip_data(argument);
            }

            /**
             * \brief The color callback, called for 
             *  each color data of the input file.
             * \param[in] argument the generic opaque argument 
             *  (from which this PlyLoader is retrieved).
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            static int color_cb(p_ply_argument argument) {
                return loader(argument)->add_color_data(argument);
            }

            /**
             * \brief Decodes vertex data from a generic callback argument.
             * \param[in] argument the generic callback argument.
             * \return callback status code, 
             *  zero for errors, non-zero for success.
             */
            int add_vertex_data(p_ply_argument argument) {
                long coord;
                ply_get_argument_user_data(argument, nullptr, &coord);
                if(coord == 0) {
                    geo_debug_assert(mesh_.vertices.dimension() >= 3);
                    if(
                        current_vertex_ >= mesh_.vertices.nb()
                    ) {
                        Logger::err("I/O")
                            << "File contains extraneous vertex data"
                            << std::endl;
                        return 0;
                    }
                    current_vertex_++;
                }

                if(coord == 0 || coord == 1 || coord == 2) {
                    // Note: current_vertex_ was incremented before,
                    // so we need to index by current_vertex_ - 1
                    if(mesh_.vertices.single_precision()) {
                        mesh_.vertices.single_precision_point_ptr(
                            current_vertex_-1
                        )[coord] = float(ply_get_argument_value(argument));
                    } else {
                        mesh_.vertices.point_ptr(
                            current_vertex_-1
                        )[coord] = ply_get_argument_value(argument);
                    }
                    return 1;
                }

                Logger::err("I/O")
                    << "In vertex #" << current_vertex_
                    << ": invalid coordinate index: " << coord
                    << std::endl;
                return 0;
            }

            /**
             * \brief Decodes facet data from a generic callback argument.
             * \param[in] argument the generic callback argument.
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            int add_face_data(p_ply_argument argument) {
                long length, value_index;
                ply_get_argument_property(
		    argument, nullptr, &length, &value_index
		);
                if(value_index < 0) {
                    // Ignore negative values -
                    // this is not an error! (facet markers)
                    return 1;
                }

                index_t vertex_index = index_t(
                    ply_get_argument_value(argument)
                );
                if(index_t(vertex_index) >= mesh_.vertices.nb()) {
                    Logger::err("I/O")
                        << "Facet corner #" << mesh_.facets.nb()
                        << " references an invalid vertex: "
                        << vertex_index
                        << std::endl;
                    return 0;
                }

                if(value_index == 0) {
                    begin_facet();
                }
                add_vertex_to_facet(vertex_index);
                if(value_index == length - 1) {
                    end_facet();
                }
                return 1;
            }

            /**
             * \brief Decodes triangle strip data from a 
             *  generic callback argument.
             * \param[in] argument the generic callback argument.
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            int add_tristrip_data(p_ply_argument argument) {
                long length, value_index;
                ply_get_argument_property(
		    argument, nullptr, &length, &value_index
		);
                if(value_index < 0) {
                    // Ignore negative values - this is not an error!
                    return 1;
                }

                // NOTE: negative vertex_index values have
                // a special meaning here:
                // they tell the loader to start a new strip

                signed_index_t vertex_index = signed_index_t(
                    ply_get_argument_value(argument)
                );
                if(vertex_index >=
                   signed_index_t(mesh_.vertices.nb())
                ) {
                    Logger::err("I/O")
                        << "Invalid vertex reference in tristrip: "
                        << vertex_index
                        << std::endl;
                    return 0;
                }

                if(value_index == 0) {
                    begin_tristrip();
                }
                if(vertex_index >= 0) {
                    add_to_tristrip(index_t(vertex_index));
                } else {
                    end_tristrip();
                    begin_tristrip();
                }
                if(value_index == length - 1) {
                    end_tristrip();
                }
                return 1;
            }

            /**
             * \brief Starts a new facet.
             */
            void begin_facet() {
                facet_vertices_.resize(0);
            }

            /**
             * \brief Adds a vertex to the current facet.
             * \param[in] v the index of the vertex to be added
             */
            void add_vertex_to_facet(index_t v) {
                facet_vertices_.push_back(v);
            }


            /**
             * \brief Terminates the currently constructed
             *  facet.
             */
            void end_facet() {
                index_t f = mesh_.facets.create_polygon(facet_vertices_.size());
                for(index_t lv=0; lv<facet_vertices_.size(); ++lv) {
                    mesh_.facets.set_vertex(f,lv,facet_vertices_[lv]);
                }
            }
            
            /**
             * \brief Starts a new triangle strip.
             */
            void begin_tristrip() {
                tristrip_index_ = 0;
            }

            /**
             * \brief Terminates the current triangle strip.
             */
            void end_tristrip() {
            }

            /**
             * \brief Adds a vertex to the current triangle strip.
             * \param[in] vertex_index the index of the vertex
             */
            void add_to_tristrip(index_t vertex_index) {
                if(tristrip_index_ >= 2) {
                    mesh_.facets.create_triangle(
                        tristrip_points_[0],
                        tristrip_points_[1],
                        vertex_index
                    );
                }
                tristrip_points_[tristrip_index_ & 1] = vertex_index;
                tristrip_index_++;
            }

            /**
             * \brief Adds color data to the current vertex
             * \param[in] argument the generic callback argument
             * \return callback status code, zero for errors, 
             *  non-zero for success.
             */
            int add_color_data(p_ply_argument argument) {
                long coord;
                ply_get_argument_user_data(argument, nullptr, &coord);
                if(coord == 0) {
                    geo_debug_assert(mesh_.vertices.dimension() >= 9);
                    if(current_color_ >= mesh_.vertices.nb()) {
                        Logger::err("I/O")
                            << "File contains extraneous color data"
                            << std::endl;
                        return 0;
                    }
                    current_color_++;
                }

                if(coord == 0 || coord == 1 || coord == 2) {
                    double value =
                        double(ply_get_argument_value(argument)) * color_mult_;
                    // (note: current_color_ - 1 because it was incremented before)
		    vertex_color_[3*(current_color_ - 1) + index_t(coord)] = value;
                    return 1;
                }

                Logger::err("I/O")
                    << "In vertex #" << current_color_
                    << ": invalid color index: " << coord
                    << std::endl;
                return 0;
            }

        protected:
            Mesh& mesh_;
            std::string filename_;
            MeshIOFlags flags_;
            
            index_t current_vertex_;

            bool has_colors_;
            double color_mult_;
            index_t current_color_;

            index_t tristrip_points_[2];
            index_t tristrip_index_;

            bool load_colors_;

            vector<index_t> facet_vertices_;

	    Attribute<double> vertex_color_;
        };


	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) override {
            PlyLoader loader(filename, M, ioflags);
            return loader.load();
        }
        
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
            p_ply oply = ply_create(
                filename.c_str(),
		(CmdLine::get_arg_bool("sys:ascii") ? PLY_ASCII : PLY_LITTLE_ENDIAN),
		nullptr, 0, nullptr
            );

            if(oply == nullptr) {
                return false;
            }

	    Attribute<double> vertex_color;
	    vertex_color.bind_if_is_defined(
		M.vertices.attributes(), "color"
	    );
	    if(vertex_color.is_bound() && vertex_color.dimension() != 3) {
		Logger::warn("IO")
		    << "Mesh has vertex colors but attribut dim is not 3 (ignoring them)"
		    << std::endl;
		vertex_color.unbind();
	    }

            // Create element and properties for vertices
            e_ply_type coord_type = PLY_FLOAT;
	    e_ply_type color_type = PLY_UINT8;
            ply_add_element(oply, "vertex", long(M.vertices.nb()));
            ply_add_property(oply, "x", coord_type, coord_type, coord_type);
            ply_add_property(oply, "y", coord_type, coord_type, coord_type);
            ply_add_property(oply, "z", coord_type, coord_type, coord_type);
	    if(vertex_color.is_bound()) {
		ply_add_property(oply, "red",   color_type, color_type, color_type);
		ply_add_property(oply, "green", color_type, color_type, color_type);
		ply_add_property(oply, "blue",  color_type, color_type, color_type);
	    }

            if(ioflags.has_element(MESH_FACETS)) {
                // Create element and properties for facets
                // (determine best index types)
                index_t max_facet_size = 0;
                for(index_t f = 0; f < M.facets.nb(); ++f) {
                    max_facet_size = std::max(
                        max_facet_size, M.facets.nb_vertices(f)
                    );
                }
                e_ply_type facet_len_type = PLY_UCHAR;
                if(max_facet_size > 65535) {
                    facet_len_type = PLY_UINT;
                } else if(max_facet_size > 255) {
                    facet_len_type = PLY_USHORT;
                }
                e_ply_type facet_idx_type = PLY_INT;
                ply_add_element(oply, "face", long(M.facets.nb()));
                ply_add_property(
                    oply, "vertex_indices",
                    PLY_LIST, facet_len_type, facet_idx_type
                );
            }

            std::vector<std::string> args;
            CmdLine::get_args(args);
            for(index_t i = 0; i < args.size(); i++) {
                ply_add_comment(oply, ("vorpaline " + args[i]).c_str());
            }

            // Write header
            if(!ply_write_header(oply)) {
                ply_close(oply);
                return false;
            }

            // Write vertices
            for(index_t v = 0; v < M.vertices.nb(); ++v) {
                double xyz[3];
                get_mesh_point(M, v, xyz, 3);
                for(index_t coord = 0; coord < 3; coord++) {
                    ply_write(oply, xyz[coord]);
                }
		if(vertex_color.is_bound()) {
		    const double* rgb = &vertex_color[v*3];
		    for(index_t coord = 0; coord < 3; coord++) {
			ply_write(oply, Numeric::uint8(255.0*rgb[coord]));
		    }
		}
            }

            if(ioflags.has_element(MESH_FACETS)) {
                // Write facets
                for(index_t f = 0; f < M.facets.nb(); ++f) {
                    ply_write(oply, double(M.facets.nb_vertices(f)));
                    for(index_t c = M.facets.corners_begin(f);
                        c < M.facets.corners_end(f); ++c
                        ) {
                        ply_write(oply, double(M.facet_corners.vertex(c)));
                    }
                }
            }

            ply_close(oply);
            return true;
        }
    };
    
    /************************************************************************/
    
    /**
     * \brief IO handler for the OFF file format
     * \see http://www.geomview.org/docs/html/OFF.html
     */
    class GEOGRAM_API OFFIOHandler : public MeshIOHandler {
    public:
        /**
         * \brief Loads a mesh from a file in OFF format.
         * \param[in] filename name of the file
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes 
         *  and elements should be read
         * \return true on success, false otherwise
         */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
            geo_argused(ioflags);
            
            // Note: Vertices indexes start by 0 in off format.

            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }

            if(!in.get_line()) {
                Logger::err("I/O")
                    << "Unexpected end of file"
                    << std::endl;
                return false;
            }
            in.get_fields();
            if(in.nb_fields() == 0 || !in.field_matches(0, "OFF")) {
                Logger::err("I/O")
                    << "Line " << in.line_number()
                    << ": unrecognized header"
                    << std::endl;
                return false;
            }

            if(!in.get_line()) {
                Logger::err("I/O")
                    << "Line " << in.line_number()
                    << ": unexpected end of file"
                    << std::endl;
                return false;
            }
            in.get_fields();
            if(in.nb_fields() != 3) {
                Logger::err("I/O")
                    << "Line " << in.line_number()
                    << ": unrecognized header"
                    << std::endl;
                return false;
            }

            index_t nb_vertices = in.field_as_uint(0);
            // second field is nb edges (unused)
            // index_t nb_facets = in.field_as_uint(2);

            M.vertices.create_vertices(nb_vertices);
            // TODO: reserve facets
            
            for(index_t i = 0; i < nb_vertices; i++) {
                do {
                    if(!in.get_line()) {
                        Logger::err("I/O")
                            << "Line " << in.line_number()
                            << ": unexpected end of file"
                            << std::endl;
                        return false;
                    }
                }
                while(in.current_line()[0] == '#');
                in.get_fields();
                if(in.nb_fields() != 3) {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << ": invalid number of fields:"
                        << " expected 3 coordinates, got "
                        << in.nb_fields()
                        << std::endl;
                    return false;
                }
                double xyz[3];
                xyz[0] = in.field_as_double(0);
                xyz[1] = in.field_as_double(1);
                xyz[2] = in.field_as_double(2);
                set_mesh_point(M, i, xyz, 3);
            }

            if(
		ioflags.has_element(MESH_FACETS) ||
		ioflags.has_element(MESH_EDGES)) {
                while(!in.eof() && in.get_line()) {
                    in.get_fields();
                   /* if(in.nb_fields() < 4) {
                        Logger::err("I/O")
                            << "Line " << in.line_number()
                            << ": facet line only has " << in.nb_fields()
                            << " fields (expected 1 count +"
                            << " at least 3 corner fields)"
                            << std::endl;
                        return false;
                    }*/
                    index_t nb_facet_vertices = in.field_as_uint(0);
                    
                    // Note: there can be more fields than the number
                    // of vertices, for instance some OFF files have
                    // a RGB color for each facet stored right after
                    // the vertices indices (we ignore it, thus the
                    // test here is '<' instead of '!=').
                    if(in.nb_fields() < nb_facet_vertices + 1) {
                        Logger::err("I/O")
                            << "Line " << in.line_number()
                            << ": facet has " << in.nb_fields() - 1
                            << " actual vertices ("
                            << nb_facet_vertices << " expected)"
                            << std::endl;
                        return false;
                    }
                    
                    if(nb_facet_vertices >= 3) {    
                        index_t f = M.facets.create_polygon(nb_facet_vertices);

                        for(index_t j = 0; j < nb_facet_vertices; j++) {
                            index_t vertex_index = in.field_as_uint(j + 1);
                            if(vertex_index >= M.vertices.nb()) {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << ": facet corner #" << j
                                    << " references an invalid vertex: "
                                    << vertex_index
                                    << std::endl;
                                return false;
                            }
                            M.facets.set_vertex(f, j, vertex_index);
                        }
                    } else if(nb_facet_vertices == 2) {    
                        index_t vertex_index0=in.field_as_uint(1);
                        index_t vertex_index1=in.field_as_uint(2);
                        
                        if(
			    vertex_index0 >= M.vertices.nb() ||
			    vertex_index1 >= M.vertices.nb()
			) {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << ": edge"
                                    << " references an invalid vertex: "
                                    << vertex_index0 <<" or "<<vertex_index1
                                    << std::endl;
                                return false; }
                        M.edges.create_edge(vertex_index0, vertex_index1);
                    }
                }
            }
            return true;
        }

        /**
         * \brief Saves a mesh into a file in OFF format.
         * \param[in] M The mesh to save
         * \param[in] filename name of the file
         * \param[in] ioflags specifies which attributes and elements 
         *  should be saved
         * \return true on success, false otherwise
         */
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
            std::ofstream output(filename.c_str());
            if(!output) {
                return false;
            }
            output << "OFF" << std::endl;

            /*output << M.vertices.nb() << " "
                << M.facets.nb() << " "
                << M.facet_corners.nb() / 2
                << std::endl;*/
            
            output << M.vertices.nb() << " "
                << M.facets.nb() << " "
                << M.edges.nb()
                << std::endl;

            // Output Vertices
            for(index_t v = 0; v < M.vertices.nb(); ++v) {
                double xyz[3];
                get_mesh_point(M, v, xyz, 3);
                output << xyz[0] << " " << xyz[1] << " " << xyz[2] << std::endl;
            }

            if(ioflags.has_element(MESH_FACETS)) {
                // Output facets
                for(index_t f = 0; f < M.facets.nb(); ++f) {
                    output << M.facets.nb_vertices(f) << " ";
                    for(
                        index_t c = M.facets.corners_begin(f);
                        c < M.facets.corners_end(f); ++c
                    ) {
                        output << M.facet_corners.vertex(c) << " ";
                    }
                    output << std::endl;
                }
            }
            
            if(ioflags.has_element(MESH_EDGES)) {
                // Output edges
                for(index_t e = 0; e < M.edges.nb(); ++e) 
                {
                    output << "2 " << M.edges.vertex(e, 0)
			   << " " << M.edges.vertex(e, 1)
			   << std::endl;
                }
            }
            return true;
        }
    };

    /************************************************************************/

    /**
     * \brief IO handler for the STL file format (ascii and binary)
     * \see http://en.wikipedia.org/wiki/STL_(file_format)
     */
    class GEOGRAM_API STLIOHandler : public MeshIOHandler {
    public:
        /**
         * \brief Loads a mesh from a file in STL format (ascii version).
         * \param[in] filename name of the file
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes and elements 
         *  should be read
         * \return true on success, false otherwise
         */
        bool load_ascii(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) {

            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }

            bind_attributes(M, ioflags, true);

            index_t current_chart = 0;
            bool facet_opened = false;
            vector<index_t> facet_vertices;
            
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                if(in.field_matches(0, "outer")) {
                    facet_vertices.resize(0);
                    facet_opened = true;
                } else if(in.field_matches(0, "endloop")) {
                    facet_opened = false;
                    if(ioflags.has_element(MESH_FACETS)) {
                        index_t f = M.facets.create_polygon(
                            facet_vertices.size()
                        );
                        for(index_t lv=0; lv<facet_vertices.size(); ++lv) {
                            M.facets.set_vertex(f,lv,facet_vertices[lv]);
                        }
                        if(facet_region_.is_bound()) {
                            facet_region_[f] = index_t(current_chart);
                        }
                    }
                } else if(in.field_matches(0, "vertex")) {
                    if(in.nb_fields() < 4) {
                        Logger::err("I/O")
                            << "Line " << in.line_number()
                            << ": vertex line has " << in.nb_fields() - 1
                            << " fields (at least 3 required)"
                            << std::endl;
                        unbind_attributes();
                        return false;
                    }
                    double xyz[3];
                    xyz[0] = in.field_as_double(1);
                    xyz[1] = in.field_as_double(2);
                    xyz[2] = in.field_as_double(3);
                    index_t v = M.vertices.create_vertex();
                    set_mesh_point(M, v, xyz, 3);
                    facet_vertices.push_back(v);
                } else if(in.field_matches(0, "solid")) {
                    current_chart++;
                }
            }

            if(facet_opened) {
                Logger::err("I/O")
                    << "Line " << in.line_number()
                    << ": current facet is not closed"
                    << std::endl;
                unbind_attributes();
                return false;
            }

            unbind_attributes();

            if(M.facets.nb() == 0) {
                Logger::err("I/O")
                    << "STL file does not contain any facet"
                    << std::endl;
                return false;
            }
            
            return true;
        }

        /**
         * \brief Loads a mesh from a file in STL format (binary version).
         * \param[in] filename name of the file
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes and elements 
         *  should be read
         * \return true on success, false otherwise
         */
        bool load_binary(
            const std::string& filename,
            Mesh& M, const MeshIOFlags& ioflags
        ) {
            BinaryInputStream in(filename, BinaryStream::GEO_LITTLE_ENDIAN);
            char header[80];
            in.read_opaque_data(header, 80);
            if(!in.OK()) {
                throw "failed to read header";
            }
            Numeric::uint32 nb_triangles;
            in >> nb_triangles;
            if(!in.OK()) {
                throw "failed to read number of triangles";
            }

            bind_attributes(M, ioflags, true);

            M.vertices.create_vertices(nb_triangles*3);
            if(ioflags.has_element(MESH_FACETS)) {
                M.facets.create_triangles(nb_triangles);
            }
            
            for(index_t t = 0; t < nb_triangles; t++) {
                Numeric::float32 N[3];
                Numeric::float32 XYZ[9];
                in >> N[0] >> N[1] >> N[2];
                for(index_t i = 0; i < 9; i++) {
                    in >> XYZ[i];
                }
                if(!in.OK()) {
                    throw "failed to read triangle";
                }
                Numeric::uint16 attrib;
                in >> attrib;

                set_mesh_point(M, 3*t,   XYZ,   3);
                set_mesh_point(M, 3*t+1, XYZ+3, 3);
                set_mesh_point(M, 3*t+2, XYZ+6, 3);

                if(ioflags.has_element(MESH_FACETS)) {                
                    M.facets.set_vertex(t, 0, 3*t);
                    M.facets.set_vertex(t, 1, 3*t+1);
                    M.facets.set_vertex(t, 2, 3*t+2);
                    if(facet_region_.is_bound()) {
                        facet_region_[t] = index_t(attrib);
                    }
                }
            }
            unbind_attributes();
            if(M.facets.nb() != nb_triangles) {
                Logger::err("I/O")
                    << "STL file does not have "
                    << "the required number of triangles"
                    << std::endl;

                return false;
            }
            
            return true;
        }

        /**
         * \brief Loads a mesh from a file in STL format.
         * \details Supports both ascii and binary STL.
         * \param[in] filename name of the file
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes and 
         *   elements should be read
         * \return true on success, false otherwise
         */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
            FILE* F = fopen(filename.c_str(), "rb");
            if(F == nullptr) {
                return false;
            }

            // The safe way of checking whether an STL file is
            // binary is to check whether the size of the file
            // matches the size deduced from the number of triangles
            // (many binary STL files start with SOLID although it
            //  is supposed to be only for ASCII STL files)
            fseek(F, 80, SEEK_SET);
            Numeric::uint32 nb_triangles;
            if(fread(&nb_triangles, sizeof(nb_triangles), 1, F) != 1) {
                Logger::err("I/O")
                    << "Cannot deduce the format of STL file"
                    << std::endl;
                fclose(F);
                return false;
            }
            fseek(F, 0, SEEK_END);
            long file_size = ftell(F);
            fclose(F);
            bool result;
            if(file_size == long(nb_triangles * 50 + 84)) {
                result = load_binary(filename, M, ioflags);
            } else {
                result = load_ascii(filename, M, ioflags);
            }
            return result;
        }

        /**
         * \brief Writes a point to a binary stream.
         * \param[in] out the binary stream
         * \param[in] V the vector to write
         */
        inline void write_stl_vector(BinaryOutputStream& out, const vec3& V) {
            out << Numeric::float32(V.x);
            out << Numeric::float32(V.y);
            out << Numeric::float32(V.z);
        }

        /**
         * \brief Saves a mesh into a file in STL format.
         * \param[in] M The mesh to save
         * \param[in] filename name of the file
         * \param[in] ioflags specifies which attributes 
         *   and elements should be saved
         * \return true on success, false otherwise
         */
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
	    bool result = true;
	    if(CmdLine::get_arg_bool("sys:ascii")) {
		result = save_ascii(M, filename, ioflags);
	    } else {
		result = save_binary(M, filename, ioflags);
	    }
	    return result;
	}

        /**
         * \brief Saves a mesh into a file in STL ASCII format.
         * \param[in] M The mesh to save
         * \param[in] filename name of the file
         * \param[in] ioflags specifies which attributes 
         *   and elements should be saved
         * \return true on success, false otherwise
         */
	bool save_ascii(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) {
	    geo_argused(ioflags);
	    std::ofstream out(filename.c_str());
	    if(!out) {
		return false;
	    }
	    out << "solid geogram" << std::endl;
            for(index_t f = 0; f < M.facets.nb(); ++f) {
                index_t c1 = M.facets.corners_begin(f);
                vec3 p1;
                get_mesh_point(M, M.facet_corners.vertex(c1), p1.data(), 3);
                for(index_t c2 = M.facets.corners_begin(f) + 1;
                    c2 + 1 < M.facets.corners_end(f); ++c2
                ) {
                    vec3 p2;
                    get_mesh_point(
                        M, M.facet_corners.vertex(c2), p2.data(), 3
                    );
                    vec3 p3;
                    get_mesh_point(
                        M, M.facet_corners.vertex(c2+1), p3.data(), 3
                    );

		    // Seriously, this ASCII STL format is soooo verbose !!!
		    // Crazy...
		    out << "facet normal " << normalize(cross(p2-p1,p3-p1))
			<< std::endl;
		    out << "outer loop" << std::endl;
		    out << "vertex " << p1 << std::endl;
		    out << "vertex " << p2 << std::endl;
		    out << "vertex " << p3 << std::endl;
		    out << "endloop" << std::endl;
		    out << "endfacet" << std::endl;
                }
            }
	    out << "endsolid" << std::endl;
            return true;
	}

        /**
         * \brief Saves a mesh into a file in STL binary format.
         * \param[in] M The mesh to save
         * \param[in] filename name of the file
         * \param[in] ioflags specifies which attributes 
         *   and elements should be saved
         * \return true on success, false otherwise
         */
	bool save_binary(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) {

            bind_attributes(M, ioflags, false);

            BinaryOutputStream out(filename, BinaryStream::GEO_LITTLE_ENDIAN);
            char header[80];
            Memory::clear(header, 80);
            strcpy(header, "generated with GEOGRAM");
            out.write_opaque_data(header, 80);
            Numeric::uint32 nb_triangles = 0;
            for(index_t f = 0; f < M.facets.nb(); ++f) {
                nb_triangles += Numeric::uint32((M.facets.nb_vertices(f) - 2));
            }
            out << nb_triangles;
            for(index_t f = 0; f < M.facets.nb(); ++f) {
                index_t c1 = M.facets.corners_begin(f);
                vec3 p1;
                get_mesh_point(M, M.facet_corners.vertex(c1), p1.data(), 3);
                for(index_t c2 = M.facets.corners_begin(f) + 1;
                    c2 + 1 < M.facets.corners_end(f); ++c2
                ) {
                    vec3 p2;
                    get_mesh_point(
                        M, M.facet_corners.vertex(c2), p2.data(), 3
                    );
                    vec3 p3;
                    get_mesh_point(
                        M, M.facet_corners.vertex(c2+1), p3.data(), 3
                    );
                    
                    Numeric::uint16 attribute = Numeric::uint16(
                        facet_region_.is_bound() ?
                        facet_region_[f] : 0
                    );
                    
                    write_stl_vector(out, normalize(cross(p2-p1,p3-p1)));
                    write_stl_vector(out, p1);
                    write_stl_vector(out, p2);
                    write_stl_vector(out, p3);
                    
                    out << attribute;
                }
            }
            unbind_attributes();
            return true;
        }
    };
    
    /************************************************************************/


    /**
     * \brief IO handler for the XYZ file format
     * \details Currtently only loading is supported
     */
    class GEOGRAM_API XYZIOHandler : public MeshIOHandler {
    public:
        /**
         * \brief Loads a pointset from a file in XYZ format.
         * \param[in] filename name of the file
         * \param[out] M the mesh where to store the points
         * \param[in] ioflags specifies which attributes and 
         *   elements should be read
         * \return true on success, false otherwise
         */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
            geo_argused(ioflags);
	    index_t nb_vertices = get_nb_vertices(filename);
	    if(nb_vertices == index_t(-1)) {
		return false;
	    }

	    M.vertices.create_vertices(nb_vertices);

            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }
            Attribute<double> normal;
            index_t cur_v = 0;
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                switch(in.nb_fields()) {
                    case 1:
                        break;
                    case 2:
                    case 3:
                    case 4:
                    case 6:
                    {
                        double xyz[3];
                        xyz[0] = in.field_as_double(0);
                        xyz[1] = in.field_as_double(1);
                        xyz[2] =
                            (in.nb_fields() >= 3) ? in.field_as_double(2) : 0.0;
                        //   Not all xyz files have the number of vertices
                        // specified on the first line. If it is unknown,
                        // then vertices are created dynamically.
                        if(cur_v+1 >= M.vertices.nb()) {
                            M.vertices.create_vertices(cur_v+1-M.vertices.nb());
                        }
                        set_mesh_point(M,cur_v,xyz,3);

                        if(in.nb_fields() == 6) {
                            if(!normal.is_bound()) {
                                normal.create_vector_attribute(
                                    M.vertices.attributes(), "normal", 3
                                );
                            }
                            normal[3*cur_v]   = in.field_as_double(3);
                            normal[3*cur_v+1] = in.field_as_double(4);
                            normal[3*cur_v+2] = in.field_as_double(5);
                        }
                        
                        ++cur_v;
                    }
                    break;
                    default:
                        Logger::err("I/O")
                            << "Line " << in.line_number()
                            << ": wrong number of fields"
                            << std::endl;
                        return false;
                }
            }
            return true;
        }

	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
            geo_argused(ioflags);

            if(M.vertices.dimension() < 3) {
                Logger::err("I/O")
                    << "XYZ format unsupported for dim < 3"
                    << std::endl;
                return false;
            }
            
            std::ofstream out(filename.c_str());
            if(!out) {
                Logger::err("I/O")
                    << "Could not create file : "
                    << filename
                    << std::endl;
                return false;
            }

            Attribute<double> normal;
            normal.bind_if_is_defined(M.vertices.attributes(), "normal");
            if(normal.is_bound() && normal.dimension() != 3) {
                normal.unbind();
            }
            
            out << M.vertices.nb() << std::endl;
            
            for(index_t v=0; v<M.vertices.nb(); ++v) {
                double point[3];
                get_mesh_point(M,v,point,3);
                if(normal.is_bound()) {
                    out << point[0] << ' '
                        << point[1] << ' '
                        << point[2] << ' '
                        << normal[3*v]   << ' ' 
                        << normal[3*v+1] << ' '
                        << normal[3*v+2] << ' '
                        << std::endl;
                } else if(
                    M.vertices.dimension() >= 6 &&
                    M.vertices.double_precision()
                ) {
                    const double* p = M.vertices.point_ptr(v);
                    out << p[0] << ' '
                        << p[1] << ' '
                        << p[2] << ' '
                        << p[3] << ' '
                        << p[4] << ' '
                        << p[5] << std::endl;
                } else {
                    out << point[0] << ' '
                        << point[1] << ' '
                        << point[2] << std::endl;
                }
            }
            
            return true;
        }

      protected:
	
	/**
	 * \brief Gets the number of vertices in the file.
	 * \details Some xyz files do not have the number of
	 *  points specified in them. For these files, this
	 *  function reads the entire file once and counts the
	 *  points. It is better to do so, because it makes it
	 *  possible to allocate the points once we known the 
	 *  required size, instead of growing.
	 * \param[in] filename the name of the file.
	 * \return the number of vertices in the file, or
	 *  index_t(-1) if the file could not be opened.
	 */
	index_t get_nb_vertices(const std::string& filename) {
	    index_t result = 0;
	    LineInput in(filename);
            if(!in.OK()) {
                return index_t(-1);
            }
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                switch(in.nb_fields()) {
                    case 1:
                        return in.field_as_uint(0);
                    case 2:
                    case 3:
                    case 4:
                    case 6:
			++result;
                    break;
                    default:
                        Logger::err("I/O")
                            << "Line " << in.line_number()
                            << ": wrong number of fields"
                            << std::endl;
                        return index_t(-1);
                }
            }
            return result;
        }
    };
    

    /************************************************************************/

    /**
     * \brief IO handler for the PTS file format
     */
    class GEOGRAM_API PTSIOHandler : public MeshIOHandler {
    public:
        /**
         * \brief Loads a pointset from a file in PTS format.
         * \param[in] filename name of the file
         * \param[out] M the mesh where to store the points
         * \param[in] ioflags specifies which attributes and 
         *   elements should be read
         * \return true on success, false otherwise
         */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
            geo_argused(ioflags);

            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }
            while(!in.eof() && in.get_line()) {
                in.get_fields();
                if(in.nb_fields() == 4 && in.field_matches(0,"v")) {
                    double xyz[3];
                    xyz[0] = in.field_as_double(1);
                    xyz[1] = in.field_as_double(2);
                    xyz[2] = in.field_as_double(3);                    
                    index_t v = M.vertices.create_vertex();
                    set_mesh_point(M,v,xyz,3);
                } else {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << ": wrong number of fields"
                        << std::endl;
                    return false;
                }
            }
            return true;
        }

	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags
        ) override {
            geo_argused(ioflags);

            std::ofstream out(filename.c_str());
            if(!out) {
                Logger::err("I/O")
                    << "Could not create file \'"
                    << filename << "\'" << std::endl;
                return false;
            }
            if(M.vertices.dimension() != 3) {
                Logger::err("I/O")
                    << "invalid dimension for pts file format"
                    << std::endl;
                return false;
            }

            for(index_t v = 0; v < M.vertices.nb(); ++v) {
                double p[3];
                get_mesh_point(M,v,p,3);
                out << "v ";
                for(index_t c = 0; c < 3; ++c) {
                    out << p[c] << ' ';
                }
                out << std::endl;
            }

            return true;
        }
    };
    
    /************************************************************************/

    /**
     * \brief IO handler for the TET file format
     */
    class GEOGRAM_API TETIOHandler : public MeshIOHandler {
    public:
        /**
         * \brief Creates a TET IO handler.
         * \param[in] dimension dimension of the vertices 
         *  (3 for regular 3d mesh)
         */
        TETIOHandler(coord_index_t dimension = 3) :
            dimension_(dimension) {
        }

        /**
         * \brief Loads a mesh from a file in TET format.
         * \details Only tetrahedral cells are supported for now.
         * \param[in] filename name of the file
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes and elements 
         *  should be read
         * \return true on success, false otherwise
         */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }
            if(!in.get_line()) {
                Logger::err("I/O")
                    << "Unexpected end of file"
                    << std::endl;
                return false;
            }
            in.get_fields();

            index_t nb_vertices = 0;
            index_t nb_cells = 0;
            bool has_arbitrary_cells = false;
            
            if(
                in.nb_fields() == 4 &&
                in.field_matches(1, "vertices") &&
                (
                    in.field_matches(3, "tets") ||
                    in.field_matches(3, "cells")
                )
            ) {
                nb_vertices = in.field_as_uint(0);
                nb_cells = in.field_as_uint(2);
                has_arbitrary_cells = in.field_matches(3, "cells");
            } else {
                if(in.nb_fields() != 2 || !in.field_matches(1, "vertices")) {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << " expected <number_of_vertices> vertices"
                        << std::endl;
                    return false;
                }
                nb_vertices = in.field_as_uint(0);
            
                if(!in.get_line()) {
                    Logger::err("I/O")
                        << "Unexpected end of file"
                        << std::endl;
                    return false;
                }
                in.get_fields();
                if(
                    in.nb_fields() != 2 || (
                        !in.field_matches(1, "tets") &&
                        !in.field_matches(1, "cells")
                    )
                ) {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << " expected <number_of_tets> tets"
                        << std::endl;
                    return false;
                }
                nb_cells = in.field_as_uint(0);
                has_arbitrary_cells = in.field_matches(1, "cells");
            } 

            M.vertices.set_dimension(dimension_);
            M.vertices.create_vertices(nb_vertices);
            vector<double> P(dimension_);
            for(index_t v = 0; v < nb_vertices; ++v) {
                if(!in.get_line()) {
                    Logger::err("I/O")
                        << "Unexpected end of file"
                        << std::endl;
                    return false;
                }
                in.get_fields();
                if(in.nb_fields() != index_t(dimension_)) {
                    Logger::err("I/O")
                        << "Line " << in.line_number()
                        << " expected " << dimension_ << " point coordinates"
                        << std::endl;
                }
                for(coord_index_t c = 0; c < dimension_; ++c) {
                    P[c] = in.field_as_double(c);
                }
                set_mesh_point(M,v,P.data(),dimension_);
            }
            
            if(ioflags.has_element(MESH_CELLS)) {
                if(has_arbitrary_cells) {
                    for(index_t t = 0; t < nb_cells; ++t) {
                        if(!in.get_line()) {
                            Logger::err("I/O")
                                << "Unexpected end of file"
                                << std::endl;
                            return false;
                        }
                        in.get_fields();
                        if(
                           in.nb_fields() >= 2 &&
                           in.field_matches(0,"#") && in.field_matches(1,"C")
                        ) {
                            if(in.nb_fields() != 6) {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << " expected # C v1 v2 v3 v4"
                                    << std::endl;
                                return false;
                            }
                            M.cells.create_connector(
                                in.field_as_uint(2),
                                in.field_as_uint(3),
                                in.field_as_uint(4),
                                in.field_as_uint(5)
                            );
                        } else {
                            if(in.nb_fields() > 0) {
                                index_t nb_vertices_in_cell =
                                    in.field_as_uint(0);
                                switch(nb_vertices_in_cell) {
                                case 4: {
                                    M.cells.create_tet(
                                        in.field_as_uint(1),
                                        in.field_as_uint(2),
                                        in.field_as_uint(3),
                                        in.field_as_uint(4)
                                    );                                    
                                } break;
                                case 8: {
                                    M.cells.create_hex(
                                        in.field_as_uint(1),
                                        in.field_as_uint(2),
                                        in.field_as_uint(3),
                                        in.field_as_uint(4),
                                        in.field_as_uint(5),
                                        in.field_as_uint(6),
                                        in.field_as_uint(7),
                                        in.field_as_uint(8)
                                    );
                                } break;
                                case 6: {
                                    M.cells.create_prism(
                                        in.field_as_uint(1),
                                        in.field_as_uint(2),
                                        in.field_as_uint(3),
                                        in.field_as_uint(4),
                                        in.field_as_uint(5),
                                        in.field_as_uint(6)
                                    );
                                } break;
                                case 5: {
                                    M.cells.create_pyramid(
                                        in.field_as_uint(1),
                                        in.field_as_uint(2),
                                        in.field_as_uint(3),
                                        in.field_as_uint(4),
                                        in.field_as_uint(5)
                                    );
                                } break;
                                default: {
                                Logger::err("I/O")
                                    << "Line " << in.line_number()
                                    << " unexpected number of vertices in cell:"
                                    << nb_vertices_in_cell
                                    << std::endl;
                                return false;
                                }
                                }
                            }
                        }
                    }
                } else {
                    M.cells.create_tets(nb_cells);
                    for(index_t t = 0; t < nb_cells; ++t) {
                        if(!in.get_line()) {
                            Logger::err("I/O")
                                << "Unexpected end of file"
                                << std::endl;
                            return false;
                        }
                        in.get_fields();
                        if(in.nb_fields() != 5 || in.field_as_int(0) != 4) {
                            Logger::err("I/O")
                                << "Line " << in.line_number()
                                << " expected 4 v1 v2 v3 v4"
                                << std::endl;
                        } else {
                            for(index_t i = 0; i < 4; ++i) {
                                index_t v = in.field_as_uint(i + 1);
                                if(i >= nb_vertices) {
                                    Logger::err("I/O")
                                        << "Line " << in.line_number()
                                        << "invalid vertex index"
                                        << std::endl;
                                    return false;
                                }
                                M.cells.set_vertex(t, i, v);
                            }
                        }
                    }
                }
            }
            return true;
        }

        /**
         * \brief Saves a mesh into a file in TET format.
         * \param[in] M The mesh to save
         * \param[in] filename name of the file
         * \param[in] ioflags specifies which attributes and elements 
         * should be saved
         * \return true on success, false otherwise
         */
	bool save(
            const Mesh& M, const std::string& filename, 
            const MeshIOFlags& ioflags
        ) override {
            geo_argused(ioflags);
            
            if(M.vertices.dimension() < dimension_) {
                return false;
            }
            std::ofstream out(filename.c_str());
            if(!out) {
                return false;
            }
            vector<double> P(dimension_);
            if(M.cells.are_simplices()) {
                out << M.vertices.nb() << " vertices" << std::endl;
                out << M.cells.nb() << " tets" << std::endl;
                for(index_t v = 0; v < M.vertices.nb(); ++v) {
                    get_mesh_point(M,v,P.data(),dimension_);
                    for(coord_index_t c = 0; c < dimension_; ++c) {
                        out << P[c] << " ";
                    }
                    out << std::endl;
                }
                for(index_t t = 0; t < M.cells.nb(); ++t) {
                    out << "4";
                    for(index_t i = 0; i < 4; ++i) {
                        out << " " << M.cells.vertex(t, i);
                    }
                    out << std::endl;
                }
            } else {
                out << M.vertices.nb() << " vertices" << std::endl;
                out << M.cells.nb() << " cells" << std::endl;
                for(index_t v = 0; v < M.vertices.nb(); ++v) {
                    get_mesh_point(M,v,P.data(),dimension_);                    
                    for(coord_index_t c = 0; c < dimension_; ++c) {
                        out << P[c] << " ";
                    }
                    out << std::endl;
                }
                bool has_connectors = false;
                for(index_t c=0; c<M.cells.nb(); ++c) {
                    switch(M.cells.type(c)) {
                    case MESH_TET: 
                    case MESH_HEX: 
                    case MESH_PRISM: 
                    case MESH_PYRAMID: {
                        out << M.cells.nb_vertices(c) << " ";
                        for(index_t lv=0; lv<M.cells.nb_vertices(c); ++lv) {
                            out << M.cells.vertex(c,lv) << " ";
                        }
                        out << std::endl;
                    } break;
                    case MESH_CONNECTOR: {
                        has_connectors=true;
                    } break;
                    case MESH_NB_CELL_TYPES:
                        geo_assert_not_reached;
                    }
                }
                if(has_connectors) {
                    for(index_t c=0; c<M.cells.nb(); ++c) {
                        if(M.cells.type(c) == MESH_CONNECTOR) {
                            out << "# C"
                                << " " << M.cells.vertex(c,0)
                                << " " << M.cells.vertex(c,1)
                                << " " << M.cells.vertex(c,2)
                                << " " << M.cells.vertex(c,3)
                                << std::endl;
                        }
                    }
                }
            }
            return true;
        }

    private:
        coord_index_t dimension_;
    };

    /************************************************************************/

    /**
     * \brief IO handler for the TET6 file format
     * \see TETIOHandler
     */
    class GEOGRAM_API TET6IOHandler : public TETIOHandler {
    public:
        TET6IOHandler() : TETIOHandler(6) {
        }
    };

    /**
     * \brief IO handler for the TET8 file format
     * \see TETIOHandler
     */
    class GEOGRAM_API TET8IOHandler : public TETIOHandler {
    public:
        TET8IOHandler() : TETIOHandler(8) {
        }
    };

    
    /************************************************************************/

    /**
     * \brief IO handler for the geogram native file format.
     */
    class GEOGRAM_API GeogramIOHandler : public MeshIOHandler {
    public:


        /**
         * \brief Loads a mesh from a GeoFile ('.geogram' file format).
         * \details
         * Loads the contents of the InputGeoFile \p geofile and stores the
         * resulting mesh to \p M. This function can be used to load several
         * meshes that are stored in the same GeoFile.
         * \param[in] in a reference to the InputGeoFile
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes and 
         *  elements should be loaded
         * \return true on success, false otherwise.
         */
        virtual bool load(
            InputGeoFile& in,
            Mesh& M,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) {

            M.clear();
            try {

                std::string chunk_class;
                for(
                    chunk_class = in.next_chunk();
                    chunk_class != "EOFL" && chunk_class != "SPTR";
                    chunk_class = in.next_chunk()
                ) {

                    if(chunk_class == "ATTS") {
                        read_attribute_set(in, M, ioflags);
                    } else if(chunk_class == "ATTR") {
                        if(
                            String::string_starts_with(
                                in.current_attribute().name, "GEO::Mesh::"
                            )
                        ) {
                            read_internal_attribute(in, M, ioflags);
                        } else {
                            read_user_attribute(in, M, ioflags);
                        }
                    } 
                }

		// Create facet "sentry"
		if(!M.facets.are_simplices()) {
		    M.facets.facet_ptr_[M.facets.nb()] = M.facet_corners.nb();
		}
		
		// Create cell "sentry"
		if(!M.cells.are_simplices()) {
		    M.cells.cell_ptr_[M.cells.nb()] = M.cell_corners.nb();
		}

//  This warning when loading a single mesh from a file that may
// contain several meshes -> deactivated for now.	       
//                if(chunk_class == "SPTR") {
//                    Logger::out("GeoFile")
//                        << "File may contain several objects"
//                        << std::endl;
//                }
		
            } catch(const GeoFileException& exc) {
                Logger::err("I/O") << exc.what() << std::endl;
                M.clear();
                return false;
            } catch(...) {
                Logger::err("I/O") << "Caught exception" << std::endl;
                M.clear();                
                return false;
            }


            return true;
        }

        /**
         * \brief Saves a mesh to a GeoFile ('.geogram' file format)
         * \details
         * Saves mesh \p M to the GeoFile \p geofile. This function can be
         * used to write several meshes into the same GeoFile.
         * \param[in] M the mesh to save
         * \param[in] out a reference to the OutputGeoFile
         * \param[in] ioflags specifies which attributes and elements 
         *  should be saved
         * \return true on success, false otherwise.
         */
	virtual bool save(
            const Mesh& M, OutputGeoFile& out,
            const MeshIOFlags& ioflags = MeshIOFlags(),
            bool save_command_line = false
        ) {
            try {

                if(save_command_line) {
                    // Save command line in file
                    std::vector<std::string> args;
                    CmdLine::get_args(args);
                    out.write_command_line(args);
                }
                
                if(ioflags.has_element(MESH_VERTICES) && M.vertices.nb() != 0) {
                    out.write_attribute_set(
                        "GEO::Mesh::vertices",
                        M.vertices.nb()
                    );
                    save_attributes(
                        out, "GEO::Mesh::vertices", M.vertices.attributes()
                    );
                }

                if(ioflags.has_element(MESH_EDGES) && M.edges.nb() != 0) {
                    out.write_attribute_set(
                        "GEO::Mesh::edges",
                        M.edges.nb()
                    );

                    out.write_attribute(
                        "GEO::Mesh::edges",
                        "GEO::Mesh::edges::edge_vertex",
                        "index_t",
                        sizeof(index_t),
                        2,
                        M.edges.edge_vertex_.data()
                    );
                    
                    save_attributes(
                        out, "GEO::Mesh::edges", M.edges.attributes()
                    );
                }

                if(ioflags.has_element(MESH_FACETS) && M.facets.nb() != 0) {
                    out.write_attribute_set(
                        "GEO::Mesh::facets",
                        M.facets.nb()
                    );

                    save_attributes(
                        out, "GEO::Mesh::facets", M.facets.attributes()
                    );

                    if(!M.facets.are_simplices()) {
                        out.write_attribute(
                            "GEO::Mesh::facets",
                            "GEO::Mesh::facets::facet_ptr",
                            "index_t",
                            sizeof(index_t),
                            1,
                            M.facets.facet_ptr_.data()
                        );
                    }
                    
                    out.write_attribute_set(
                        "GEO::Mesh::facet_corners",
                        M.facet_corners.nb()
                    );

                    out.write_attribute(
                        "GEO::Mesh::facet_corners",
                        "GEO::Mesh::facet_corners::corner_vertex",
                        "index_t",
                        sizeof(index_t),
                        1,
                        M.facet_corners.corner_vertex_.data()
                    );

                    out.write_attribute(
                        "GEO::Mesh::facet_corners",
                        "GEO::Mesh::facet_corners::corner_adjacent_facet",
                        "index_t",
                        sizeof(index_t),
                        1,
                        M.facet_corners.corner_adjacent_facet_.data()
                    );
                    
                    save_attributes(
                        out, "GEO::Mesh::facet_corners",
                        M.facet_corners.attributes()
                    );
                    
                }
                
                if(ioflags.has_element(MESH_CELLS) && M.cells.nb() != 0) {
                    out.write_attribute_set(
                        "GEO::Mesh::cells",
                        M.cells.nb()
                    );

                    save_attributes(
                        out, "GEO::Mesh::cells",
                        M.cells.attributes()
                    );
                    
                    if(!M.cells.are_simplices()) {
                        
                        out.write_attribute(
                            "GEO::Mesh::cells",
                            "GEO::Mesh::cells::cell_type",
                            "char",
                            sizeof(char),
                            1,
                            M.cells.cell_type_.data()
                        );
                        
                        out.write_attribute(
                            "GEO::Mesh::cells",
                            "GEO::Mesh::cells::cell_ptr",
                            "index_t",
                            sizeof(index_t),
                            1,
                            M.cells.cell_ptr_.data()
                        );
                    }

                    out.write_attribute_set(
                        "GEO::Mesh::cell_corners",
                        M.cell_corners.nb()
                    );

                    out.write_attribute(
                        "GEO::Mesh::cell_corners",
                        "GEO::Mesh::cell_corners::corner_vertex",
                        "index_t",
                        sizeof(index_t),
                        1,
                        M.cell_corners.corner_vertex_.data()
                    );

                    save_attributes(
                        out,
                        "GEO::Mesh::cell_corners",
                        M.cell_corners.attributes()
                    );

                    out.write_attribute_set(
                        "GEO::Mesh::cell_facets",
                        M.cell_facets.nb()
                    );

                    out.write_attribute(
                        "GEO::Mesh::cell_facets",
                        "GEO::Mesh::cell_facets::adjacent_cell",
                        "index_t",
                        sizeof(index_t),
                        1,
                        M.cell_facets.adjacent_cell_.data()
                    );

                    save_attributes(
                        out,
                        "GEO::Mesh::cell_facets",
                        M.cell_facets.attributes()
                    );
                }
                
            } catch(const GeoFileException& exc) {
                Logger::err("I/O") << exc.what() << std::endl;
                return false;
            }
            return true;
        }
        
        /**
         * \copydoc MeshIOHandler::load()
         */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) override {
            bool result = true;
            try {
                InputGeoFile in(filename);
                result = load(in, M, ioflags);
            }  catch(const GeoFileException& exc) {
                Logger::err("I/O") << exc.what() << std::endl;
                result = false;
            } catch(...) {
                Logger::err("I/O") << "Caught exception" << std::endl;
                result = false;
            }
            return result;
        }

        /**
         * \copydoc MeshIOHandler::save()
         */
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) override {
            bool result = true;
            try {
                OutputGeoFile out(
                    filename,
                    index_t(CmdLine::get_arg_int("sys:compression_level"))
                );
                result = save(M, out, ioflags, true);
            }  catch(const GeoFileException& exc) {
                Logger::err("I/O") << exc.what() << std::endl;
                result = false;
            } catch(...) {
                Logger::err("I/O") << "Caught exception" << std::endl;
                result = false;
            }
            return result;
        }

    protected:

        /**
         * \brief Reads an attribute set from a geogram file and
         *  creates the relevant elements in a mesh.
         * \param[in] in a reference to the InputGeoFile
         * \param[in] M a reference to the Mesh
         * \param[in] ioflags the MeshIOFlags that specify which
         *  attributes and mesh elements should be read
         */
        void read_attribute_set(
            InputGeoFile& in,
            Mesh& M,
            const MeshIOFlags& ioflags
        ) {
            const std::string& name =
                in.current_attribute_set().name;
            index_t nb_items = in.current_attribute_set().nb_items;

            if(
                name == "GEO::Mesh::vertices" &&
                ioflags.has_element(MESH_VERTICES)
            ) {
                M.vertices.resize_store(nb_items);
            } else if(
                name == "GEO::Mesh::edges" &&
                ioflags.has_element(MESH_EDGES)
            ) {
                M.edges.resize_store(nb_items);
            } else if(
                name == "GEO::Mesh::facets" &&
                ioflags.has_element(MESH_FACETS)
            ) {
                M.facets.resize_store(nb_items);
            } else if(
                name == "GEO::Mesh::facet_corners" &&
                ioflags.has_element(MESH_FACETS)
            ) {
                M.facet_corners.resize_store(nb_items);
            } else if(
                name == "GEO::Mesh::cells" &&
                ioflags.has_element(MESH_CELLS)
            ) {
                M.cells.resize_store(nb_items);
            } else if(
                name == "GEO::Mesh::cell_corners" &&
                ioflags.has_element(MESH_CELLS)
            ) {
                M.cell_corners.resize_store(nb_items);
            } else if(
                name == "GEO::Mesh::cell_facets" &&
                ioflags.has_element(MESH_CELLS)
            ) {
                M.cell_facets.resize_store(nb_items);
            }
        }

        /**
         * \brief Reads a user attribute from a geogram file and
         *  stores it in a mesh.
         * \param[in] in a reference to the InputGeoFile
         * \param[in] M a reference to the Mesh
         * \param[in] ioflags the MeshIOFlags that specify which
         *  attributes and mesh elements should be read
         */
        void read_user_attribute(
            InputGeoFile& in,
            Mesh& M,
            const MeshIOFlags& ioflags
        ) {
            const std::string& name =
                in.current_attribute().name;
            const std::string& set_name =
                in.current_attribute_set().name;
            if(set_name == "GEO::Mesh::vertices") {
                if(ioflags.has_element(MESH_VERTICES)) {
                    //   Vertex geometry is a special attribute, already
                    // created by the Mesh class, therefore we cannot use
                    // the generic read_attribute() function.
                    if(name == "point") {
                        M.vertices.set_double_precision();
                        M.vertices.set_dimension(
                            in.current_attribute().dimension
                        );
                        in.read_attribute(M.vertices.point_ptr(0));
                    } else if(name == "point_fp32") {
                        M.vertices.set_single_precision();
                        M.vertices.set_dimension(
                            in.current_attribute().dimension
                        );
                        in.read_attribute(
                            M.vertices.single_precision_point_ptr(0)
                        );                                    
                    } else {
                        read_attribute(in, M.vertices.attributes());
                    }
                } 
            } else if(set_name == "GEO::Mesh::edges") {
                if(ioflags.has_element(MESH_EDGES)) {
                    read_attribute(in, M.edges.attributes());
                } 
            } else if(set_name == "GEO::Mesh::facets") {
                if(ioflags.has_element(MESH_FACETS)) {
                    read_attribute(in, M.facets.attributes());
                } 
            } else if(set_name == "GEO::Mesh::facet_corners") {
                if(ioflags.has_element(MESH_FACETS)) {
                    read_attribute(
                        in, M.facet_corners.attributes()
                    );
                } 
            } else if(set_name == "GEO::Mesh::cells") {
                if(ioflags.has_element(MESH_CELLS)) {
                    read_attribute(in, M.cells.attributes());
                } 
            } else if(set_name == "GEO::Mesh::cell_corners") {
                if(ioflags.has_element(MESH_CELLS)) {
                    read_attribute(in, M.cell_corners.attributes());
                } 
            } else if(set_name == "GEO::Mesh::cell_facets") {
                if(ioflags.has_element(MESH_CELLS)) {
                    read_attribute(in, M.cell_facets.attributes());
                } 
            } 
        }

        /**
         * \brief Reads an internal attribute from a geogram file and
         *  stores it in a mesh.
         * \param[in] in a reference to the InputGeoFile
         * \param[in] M a reference to the Mesh
         * \param[in] ioflags the MeshIOFlags that specify which
         *  attributes and mesh elements should be read
         */
        void read_internal_attribute(
            InputGeoFile& in,
            Mesh& M,
            const MeshIOFlags& ioflags
        ) {
            const std::string& name =
                in.current_attribute().name;

            const std::string& set_name =
                in.current_attribute_set().name;

            if(!String::string_starts_with(name, set_name + "::")) {
                Logger::warn("I/O")
                    << "Invalid internal attribute (GEO::Mesh:: scoped): "
                    << name << " does not start with "
                    << set_name
                    << std::endl;
                return;
            }
            if(name == "GEO::Mesh::edges::edge_vertex") {
                if(ioflags.has_element(MESH_EDGES)) {
                    M.edges.edge_vertex_.resize(M.edges.nb()*2);
                    in.read_attribute(M.edges.edge_vertex_.data());
                }
            } else if(name == "GEO::Mesh::facets::facet_ptr") {
                if(ioflags.has_element(MESH_FACETS)) {
                    M.facets.is_simplicial_ = false;
                    M.facets.facet_ptr_.resize(M.facets.nb()+1);
                    in.read_attribute(M.facets.facet_ptr_.data());
                } 
            } else if(name == "GEO::Mesh::facet_corners::corner_vertex") {
                if(ioflags.has_element(MESH_FACETS)) {
                    in.read_attribute(M.facet_corners.corner_vertex_.data());
                } 
            } else if(
                name == "GEO::Mesh::facet_corners::corner_adjacent_facet"
            ) {
                if(ioflags.has_element(MESH_FACETS)) {
                    in.read_attribute(
                        M.facet_corners.corner_adjacent_facet_.data()
                    );
                } 
            } else if(name == "GEO::Mesh::cells::cell_type") {
                if(ioflags.has_element(MESH_CELLS)) {
                    M.cells.is_simplicial_ = false;
                    M.cells.cell_type_.resize(M.cells.nb());
                    in.read_attribute(M.cells.cell_type_.data());
                } 
            } else if(name == "GEO::Mesh::cells::cell_ptr") {
                if(ioflags.has_element(MESH_CELLS)) {
                    M.cells.is_simplicial_ = false;
                    M.cells.cell_ptr_.resize(M.cells.nb()+1);
                    in.read_attribute(M.cells.cell_ptr_.data());
                } 
            } else if(name == "GEO::Mesh::cell_corners::corner_vertex") {
                if(ioflags.has_element(MESH_CELLS)) {
                    in.read_attribute(M.cell_corners.corner_vertex_.data());
                } 
            } else if(name == "GEO::Mesh::cell_facets::adjacent_cell") {
                if(ioflags.has_element(MESH_CELLS)) {
                    in.read_attribute(M.cell_facets.adjacent_cell_.data());
                } 
            } 
        }

        /**
         * \brief Reads a user attribute from a geogram file and 
         *  stores it in an AttributesManager
         * \param[in] in a reference to the InputGeoFile
         * \param[in] attributes a reference to the AttributesManager
         *  where the read attribute should be stored
         */
        void read_attribute(
            InputGeoFile& in,
            AttributesManager& attributes
        ) {
            if(
                !AttributeStore::element_type_name_is_known(
                    in.current_attribute().element_type
                )
            ) {
                Logger::warn("I/O") << "Skipping attribute "
                                    << in.current_attribute().name
                                    << ":"
                                    << in.current_attribute().element_type
                                    << " (unknown type)"
                                    << std::endl;
                return;
            }
            AttributeStore* store =
                AttributeStore::create_attribute_store_by_element_type_name(
                    in.current_attribute().element_type,
                    in.current_attribute().dimension
                );
            attributes.bind_attribute_store(in.current_attribute().name,store);
            in.read_attribute(store->data());
        }

        /**
         * \brief Writes all the user attributes of an AttributesManager
         *  into a geogram file.
         * \param[out] out a reference to the OutputGeoFile
         * \param[in] attribute_set_name the name to be used for the attribute
         *  set in the geogram file
         * \param[in] attributes a reference to the AttributesManager
         */
        void save_attributes(
            OutputGeoFile& out,
            const std::string& attribute_set_name,
            AttributesManager& attributes
        ) {
            vector<std::string> attribute_names;
            attributes.list_attribute_names(attribute_names);
            for(index_t i=0; i<attribute_names.size(); ++i) {
                AttributeStore* store = attributes.find_attribute_store(
                    attribute_names[i]
                );
                if(
                    AttributeStore::element_typeid_name_is_known(
                        store->element_typeid_name()
                    )
                ) {
                    std::string element_type = 
                      AttributeStore::element_type_name_by_element_typeid_name(
                          store->element_typeid_name()
                      );

                    out.write_attribute(
                        attribute_set_name,
                        attribute_names[i],
                        element_type,
                        store->element_size(),
                        store->dimension(),
                        store->data()
                    );
                } else {
                    Logger::warn("I/O")
                        << "Skipping attribute: "
                        << attribute_names[i]
                        << " on "
                        << attribute_set_name
                        << std::endl;
                    Logger::warn("I/O")
                        << "Typeid "
                        << store->element_typeid_name()
                        << " unknown"
                        << std::endl;
                }
            }
        }
    };

    /************************************************************************/
    
    /**
     * \brief IO handler for graphite files.
     * \details Graphite files with a single object can be directly read
     *  by geogram. 
     */
    class GraphiteIOHandler : public GeogramIOHandler {
    public:
        /**
         * \copydoc MeshIOHandler::save()
         */
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) override {
            geo_argused(M);
            geo_argused(filename);
            geo_argused(ioflags);
            Logger::err("I/O")
                << "graphite file format not supported for writing"
                << std::endl;
            return false;
        }
    };

    /************************************************************************/
   
    /**
     * \brief IO handler for PDB (Protein DataBase) files.
     */ 
    class PDBIOHandler : public MeshIOHandler {
    public:
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
	    geo_argused(ioflags);
	    M.clear();
	    M.vertices.set_dimension(3);
	    std::ifstream in(filename.c_str());
	    if(!in) {
		return false;
	    }
	    Attribute<char> atom_type(M.vertices.attributes(), "atom_type");
	    Attribute<char> chain_id(M.vertices.attributes(), "chain_id");
	    while(in) {
		std::string line;
		getline(in, line);
		while(line.length() < 80) {
		    line.push_back(' ');
		}
		std::string record_name = get_columns(line, 1, 6) ;
		if(record_name == "ATOM  " || record_name == "HETATM") {
		    std::string serial      = get_columns(line, 7, 11);
		    std::string name        = get_columns(line, 11, 16);
		    std::string altLoc      = get_columns(line, 17, 17);
		    std::string resName     = get_columns(line, 18, 20);
		    std::string chainID     = get_columns(line, 22, 22);
		    std::string resSeq      = get_columns(line, 23, 26);
		    std::string iCode       = get_columns(line, 27, 27);
		    double x          = to_double(get_columns(line, 31, 38));
		    double y          = to_double(get_columns(line, 39, 46));
		    double z          = to_double(get_columns(line, 47, 54));
		    //double occupancy  = to_double(get_columns(line, 55, 60));
		    //double tempFactor = to_double(get_columns(line, 61, 66));
		    std::string element     = get_columns(line, 77, 78);
		    std::string charge      = get_columns(line, 79, 80);

		    index_t v = M.vertices.create_vertex();
		    if(M.vertices.single_precision()) {
			M.vertices.single_precision_point_ptr(v)[0] = float(x);
			M.vertices.single_precision_point_ptr(v)[1] = float(y);
			M.vertices.single_precision_point_ptr(v)[2] = float(z);
		    } else {
			M.vertices.point_ptr(v)[0] = x;
			M.vertices.point_ptr(v)[1] = y;
			M.vertices.point_ptr(v)[2] = z;
		    }
		    atom_type[v] = name[3];
		    chain_id[v] = chainID[0];
		}
	    }
	    return true;
	}
       
        /**
         * \copydoc MeshIOHandler::save()
         */
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) override {
            geo_argused(M);
            geo_argused(filename);
            geo_argused(ioflags);
            Logger::err("I/O")
                << "PDB file format not supported for writing"
                << std::endl;
            return false;
        }
    protected:
	
	inline std::string get_columns(
	    const std::string& s, unsigned int from_c, unsigned int to_c
	) const {
	    return s.substr(from_c - 1, to_c - from_c + 1) ;
	}
	
	inline double to_double(const std::string& s) {
	    return String::to_double(s);
	}
    };

    /************************************************************************/

    /**
     * \brief Mesh IO Handler for OpenVolumeMesh file format.
     * \details Saves a polyhedral mesh stored in a surfacic mesh that
     *  has all cell boundaries and a vertex_id vertex attribute and cell_id
     *  facet attribute. Note: the implementation is pretty inefficient (uses
     *  tables).
     */
    class OVMIOHandler : public MeshIOHandler {
    public:
	/**
	 * \copydoc MeshIOHandler::load()
	 */
	bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags
        ) override {
	    geo_argused(ioflags);
	    M.clear();
	    M.vertices.set_dimension(3);
            LineInput in(filename);
            if(!in.OK()) {
                return false;
            }
	    
	    in.get_line();
	    in.get_fields();
	    if(
		in.nb_fields() != 2 ||
		strcmp(in.field(0),"OVM") ||
		strcmp(in.field(1),"ASCII")
	    ) {
		Logger::err("OVM") << "Invalid file header" << std::endl;
		return false;
	    }


	    Attribute<int> vertex_id(M.vertices.attributes(), "vertex_id");
	    Attribute<int> cell_id(M.facets.attributes(), "cell_id");

	    index_t nb_vertices = 0;
	    vector<double> vertices;
	    vector<index_t> ovm_to_vertex_id;

	    index_t nb_edges = 0;
	    vector<index_t> edges;

	    index_t nb_facets = 0;
	    vector<index_t> facet_ptr;
	    vector<index_t> facet_edge;

	    index_t nb_cells = 0;

	    try {
		while(!in.eof()) {
		    std::string kw = get_keyword(in);
		    if(kw == "Vertices") {
			nb_vertices = get_number(in);
			vertices.resize(nb_vertices*3);
			ovm_to_vertex_id.assign(nb_vertices*3, index_t(-1));
			FOR(v, nb_vertices) {
			    in.get_line();
			    in.get_fields();
			    if(in.nb_fields() != 3) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid vertex, expected 3 coordinates"
				);
			    }
			    vertices[3*v]   = in.field_as_double(0);
			    vertices[3*v+1] = in.field_as_double(1);
			    vertices[3*v+2] = in.field_as_double(2);		
			}
		    } else if(kw == "Edges") {
			nb_edges = get_number(in);
			edges.resize(nb_edges*2);
			FOR(e, nb_edges) {
			    in.get_line();
			    in.get_fields();
			    if(in.nb_fields() != 2) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid edge, expected 2 indices"
				);
			    }
			    edges[2*e]   = in.field_as_uint(0);
			    edges[2*e+1] = in.field_as_uint(1);
			    if(
				edges[2*e] >= nb_vertices ||
				edges[2*e+1] >= nb_vertices) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid vertex id in edge"
				);
			    }
			}
		    } else if(kw == "Faces") {
			nb_facets = get_number(in);
			facet_ptr.resize(nb_facets+1);
			facet_ptr[0] = 0;
			FOR(f, nb_facets) {
			    in.get_line();
			    in.get_fields();
			    if(in.nb_fields() == 0) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid facet, empty line"
				);
			    }
			    index_t facet_size = in.field_as_uint(0);
			    facet_ptr[f+1] = facet_ptr[f] + facet_size;
			    if(in.nb_fields() != facet_size+1) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid facet, wrong number of elements"
				);
			    }
			    FOR(lf, facet_size) {
				index_t ie = in.field_as_uint(1+lf);
				if((ie/2) >= nb_edges) {
				    throw(
					"Line: " +
					String::to_string(in.line_number()) +
					":Invalid edge id in facet"
				    );
				}
				facet_edge.push_back(ie);
			    }
			}
		    } else if(kw == "Polyhedra") {
			nb_cells = get_number(in);
			FOR(c, nb_cells) {
			    in.get_line();
			    in.get_fields();
			    if(in.nb_fields() == 0) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid cell, empty line"
				);
			    }
			    index_t cell_size = in.field_as_uint(0);
			    if(in.nb_fields() != cell_size + 1) {
				throw(
				    "Line: " +
				    String::to_string(in.line_number()) +
				    ":Invalid cell, wrong number of elements"
				);
			    }
			    vector<index_t> cell_facets;
			    FOR(lf, cell_size) {
				index_t f = in.field_as_uint(lf+1);
				if((f/2) >= nb_facets) {
				    throw(
					"Line: " +
					String::to_string(in.line_number()) +
					":Invalid facet id in cell"
				    );
				}
				cell_facets.push_back(f);
			    }
			    
			    // Clear vertex ids
			    FOR(lf, cell_size) {
				index_t f = cell_facets[lf];		    
				bool inverse_f = (f & 1) != 0;		    
				f /= 2;
				for(index_t ee = facet_ptr[f]; ee<facet_ptr[f+1]; ++ee) {
				    index_t e = facet_edge[ee];
				    if(inverse_f) {
					e = e ^ index_t(1);
				    }
				    // Vertex index is directly obtained from edge[]
				    // (edge2vertices) table, and rule for orientation
				    // (2*e for direct edge, 2*e+1 for inversed edge)
				    // directly gives the index of the origin vertex.
				    // If the facet is inversed, then the edge is inversed
				    // (by inverting its least significant bit, with the
				    // XOR e ^(index_t(1)) operation).
				    index_t ovm_v = edges[e];			
				    ovm_to_vertex_id[ ovm_v ] = index_t(-1);
				}
			    }
			    
			    // Create vertices
			    FOR(lf, cell_size) {
				index_t f = cell_facets[lf];
				bool inverse_f = (f & 1) != 0;		    
				f /= 2;
				for(index_t ee = facet_ptr[f]; ee<facet_ptr[f+1]; ++ee) {
				    index_t e = facet_edge[ee];
				    if(inverse_f) {
					e = e ^ index_t(1);
				    }
				    index_t ovm_v = edges[e];
				    if(ovm_to_vertex_id[ovm_v] == index_t(-1)) {
					const double* p = &(vertices[ ovm_v*3 ]);
					index_t new_v = M.vertices.create_vertex();
					set_mesh_point(M, new_v, p, 3);
					ovm_to_vertex_id[ovm_v] = new_v;
					vertex_id[new_v] = int(ovm_v);
				    }
				}		    
			    }
			    
			    // Create facets
			    FOR(lf, cell_size) {
				index_t f = cell_facets[lf];
				bool inverse_f = (f & 1) != 0;
				f /= 2;
				index_t facet_size = facet_ptr[f+1] - facet_ptr[f];
				index_t new_f = M.facets.create_polygon(facet_size);
				cell_id[new_f] = int(c);
				FOR(le, facet_size) {
				    index_t ee = inverse_f ?
					(facet_ptr[f+1] - le - 1) :
					(facet_ptr[f] + le);
				    index_t e = facet_edge[ee];
				    if(inverse_f) {
					e = e ^ index_t(1);
				    }
				    index_t ovm_v = edges[e];
				    index_t geo_v = ovm_to_vertex_id[ovm_v];
				    M.facets.set_vertex(new_f, le, geo_v);
				}
			    }
			}
		    } else if(kw == "Vertex_Property") {
			skip_property(in,nb_vertices);
		    } else if(kw == "Edge_Property") {
			skip_property(in,nb_edges);
		    } else if(kw == "HalfEdge_Property") {
			skip_property(in,nb_edges*2);
		    } else if(kw == "Face_Property") {
			skip_property(in,nb_facets);
		    } else if(kw == "HalfFace_Property") {
			skip_property(in,nb_facets*2);		    
		    } else if(kw == "Polyhedron_Property") {
			skip_property(in,nb_cells);
		    } 
		}
	    } catch(const std::string& what) {
                Logger::err("I/O") << what << std::endl;
		return false;
	    } catch(const std::exception& ex) {
                Logger::err("I/O") << ex.what() << std::endl;
		return false;
            } catch(...) {
                Logger::err("I/O") << "Caught exception" << std::endl;
		return false;
	    }

	    M.facets.connect();
	    
	    return true;
	}
	
	/**
	 * \copydoc MeshIOHandler::save()
	 */
	bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags = MeshIOFlags()
	) override {
	    geo_argused(ioflags);
	    
	    Attribute<int> vertex_id;
	    vertex_id.bind_if_is_defined(M.vertices.attributes(), "vertex_id");
	    Attribute<int> cell_id;
	    cell_id.bind_if_is_defined(M.facets.attributes(), "cell_id");

	    if(!vertex_id.is_bound()) {
		Logger::err("OVM") << "Missing vertex ids" << std::endl;
		return false;
	    }

	    if(!cell_id.is_bound()) {
		Logger::err("OVM") << "Missing cell ids" << std::endl;
		return false;
	    }

	    std::ofstream out(filename.c_str());
	    if(!out) {
		return false;
	    }

	    out << "OVM ASCII" << std::endl;

	    // Output the vertices
	    {
		index_t nb_vertices = 0;
		FOR(v, M.vertices.nb()) {
		    nb_vertices = std::max(nb_vertices, index_t(vertex_id[v]));
		}
		++nb_vertices;
		vector<index_t> vid_to_v(nb_vertices);
		FOR(v, M.vertices.nb()) {
		    vid_to_v[vertex_id[v]] = v;
		}
		out << "Vertices" << std::endl;
		out << nb_vertices << std::endl;
		FOR(vid, nb_vertices) {
		    const double* p = M.vertices.point_ptr(vid_to_v[vid]);
		    FOR(d, M.vertices.dimension()) {
			out << std::setprecision(17) << p[d] << " ";
		    }
		    out << std::endl;
		}
	    }

	    std::map<bindex, index_t> edge_to_id;
	    vector<bindex> edges;
	    index_t nb_edges = 0;
	    
	    // Construct edge table and output edges
	    {
		FOR(f, M.facets.nb()) {
		    for(
			index_t c1=M.facets.corners_begin(f);
			c1<M.facets.corners_end(f); ++c1
		    ) {
			index_t c2 = M.facets.next_corner_around_facet(f,c1);
			index_t iv1 =
			    index_t(vertex_id[M.facet_corners.vertex(c1)]);
			index_t iv2 =
			    index_t(vertex_id[M.facet_corners.vertex(c2)]);
			bindex K(iv1, iv2);
			if(edge_to_id.find(K) == edge_to_id.end()) {
			    edge_to_id[K] = nb_edges;
			    edges.push_back(K);
			    ++nb_edges;
			}
		    }
		}
		out << "Edges" << std::endl;
		out << nb_edges << std::endl;
		FOR(e, nb_edges) {
		    out << edges[e].indices[0] << " "
			<< edges[e].indices[1] << std::endl;
		}
	    }

	    std::map<trindex, index_t> facet_to_id;
	    vector<index_t> facets;
	    index_t nb_facets = 0;

	    // Construct facet table and output facets
	    {
		FOR(f, M.facets.nb()) {
		    trindex K    = facet_key(M, f, vertex_id, false);
		    trindex Kinv = facet_key(M, f, vertex_id, true);		    
		    if(
			facet_to_id.find(K) == facet_to_id.end() &&
			facet_to_id.find(Kinv) == facet_to_id.end()
		    ) {
			facet_to_id[K]    = 2*nb_facets;
			facet_to_id[Kinv] = 2*nb_facets + 1;			
			++nb_facets;
			facets.push_back(f);
		    }
		}
		out << "Faces" << std::endl;
		out << nb_facets << std::endl;
		FOR(fi, nb_facets) {
		    index_t f = facets[fi];
		    out << M.facets.nb_vertices(f) << " ";
		    for(
			index_t c1=M.facets.corners_begin(f);
			c1<M.facets.corners_end(f); ++c1
		    ) {
			index_t c2 = M.facets.next_corner_around_facet(f,c1);
			index_t iv1 =
			    index_t(vertex_id[M.facet_corners.vertex(c1)]);
			index_t iv2 =
			    index_t(vertex_id[M.facet_corners.vertex(c2)]);
			bindex K(iv1, iv2, bindex::KEEP_ORDER);
			index_t ie = index_t(-1);
			auto it = edge_to_id.find(K);
			if(it == edge_to_id.end()) {
			    ie = 2*edge_to_id[bindex(iv1,iv2)]+1;
			} else {
			    ie = 2*it->second;
			}
			out << ie << " ";
		    }
		    out << std::endl;
		}
	    }

	    // Construct cell table and output cells
	    
	    index_t nb_cells = 0;
	    FOR(f, M.facets.nb()) {
		nb_cells = std::max(nb_cells, index_t(cell_id[f]));
	    }
	    ++nb_cells;
	    
	    // Ugly ! One could use compressed row storage instead.
	    // ... but anyway we got all these tables indexed by bindexes
	    // and trindexes that eat much memory, no need to optimize that
	    // for now since tables storage probably dominate...
	    vector< vector<index_t> >cell_to_f(nb_cells);
	    FOR(f, M.facets.nb()) {
		cell_to_f[cell_id[f]].push_back(f);
	    }

	    out << "Polyhedra" << std::endl;
	    out << nb_cells << std::endl;
	    
	    FOR(ci, nb_cells) {
		out << cell_to_f[ci].size() << " ";
		FOR(lf, cell_to_f[ci].size()) {
		    index_t f = cell_to_f[ci][lf];
		    trindex K = facet_key(M, f, vertex_id);
		    out << facet_to_id[K] << " ";
		}
		out << std::endl;
	    }

	    vertex_id.unbind();
	    cell_id.unbind();
	    
	    return true;
	}

    private:

	/**
	 * \brief Gets one keyword from the next line.
	 * \param[in] in a reference to the line input stream.
	 * \details Throws an exception if the line has
	 *  not exactly one keyword.
	 */
	std::string get_keyword(LineInput& in) {
	    in.get_line();
	    in.get_fields();
	    if(in.nb_fields() == 0 && in.eof()) {
		return std::string("");
	    }
	    if(in.nb_fields() < 1) {
		throw("Expected one keyword");
	    }
	    return std::string(in.field(0));
	}

	/**
	 * \brief Gets one unsigned integer from the next
	 *  line.
	 * \param[in] in a reference to the line input stream.
	 * \details Throws an exception if the next line
	 *  has not exactly one unsigned integer.
	 */
	index_t get_number(LineInput& in) {
	    in.get_line();
	    in.get_fields();
	    if(in.nb_fields() != 1) {
		throw("Expected one number");
	    }
	    index_t result = in.field_as_uint(0);
	    return result;
	}

	/*
	 * \brief Skips property.
	 * \param[in] in a reference to the line input stream.
	 * \param[in] nb_elements number of elements to be skipped
	 */
	void skip_property(LineInput& in, index_t nb_elements) {
	    // +1 because there is the type of the property
	    FOR(i, nb_elements+1) {
		in.get_line();
	    }
	}
	
	/**
	 * \brief Gets a key to be able to retrieve facet indices.
	 * \param[in] M a reference to a mesh
	 * \param[in] f a facet of the mesh
	 * \param[in] vertex_id an Id attribute attached to the vertices
	 * \param[in] invert if true, invert the order of the vertices
	 *  of the facet.
	 * \return a trindex composed of the ids of three corners of the
	 *  facet, formed by the id of the vertex with the lowest id, 
	 *  and the ids of its predecessor and successor around the facet.
	 */

	static trindex facet_key(
	    const Mesh& M, index_t f, const Attribute<int>& vertex_id,
	    bool invert=false
	) {
	    index_t min_iv = index_t(-1);
	    index_t min_corner = index_t(-1);
	    for(
		index_t c=M.facets.corners_begin(f);
		c<M.facets.corners_end(f); ++c
	    ) {
		index_t iv = index_t(vertex_id[M.facet_corners.vertex(c)]);
		if(min_iv == index_t(-1) || iv < min_iv) {
		    min_corner = c;
		    min_iv = iv;
		}
	    }
	    index_t c1 = min_corner;
	    index_t c2 = invert ?
		M.facets.prev_corner_around_facet(f,c1) :
		M.facets.next_corner_around_facet(f,c1) ;
	    index_t c3 = invert ?
		M.facets.prev_corner_around_facet(f,c2) :
		M.facets.next_corner_around_facet(f,c2) ;
	    index_t iv1 = index_t(vertex_id[M.facet_corners.vertex(c1)]);
	    index_t iv2 = index_t(vertex_id[M.facet_corners.vertex(c2)]);
	    index_t iv3 = index_t(vertex_id[M.facet_corners.vertex(c3)]);
	    return trindex(iv1,iv2,iv3,trindex::KEEP_ORDER);
	}
	
    };

/****************************************************************************/

    const index_t id_offset_msh = 1;
    const index_t msh2geo_hex[8] = {1, 3, 7, 5, 0, 2, 6, 4 };
    const index_t msh2geo_def[8] = {0, 1, 2, 3, 4, 5, 6, 7 };
    const index_t celltype_geo2msh[5] = {4, 5, 6, 7}; 

    /**
     * \brief Support for GMSH file format.
     * \details By Maxence Reberol.
     */
    class GEOGRAM_API MSHIOHandler : public MeshIOHandler {
      public:
	MSHIOHandler() {
	}

	bool verify_file_format(const std::string& filename) {
	    LineInput in(filename);
	    if (!in.OK()) return false;
	    in.get_line();
	    in.get_fields();
	    if (in.field_matches(0, "$MeshFormat")) {
		in.get_line();
		in.get_fields();
		if (in.field_as_double(0) == 2.2
		    && in.field_as_uint(1) == 0
		    && in.field_as_uint(2) == 8
		) return true;
	    }
	    return false;
	}

	bool read_vertices(const std::string& filename, Mesh& M) {
	    LineInput in(filename);
	    while (in.get_line()) {
		in.get_fields();
		if (in.field_matches(0, "$Nodes")) {
		    in.get_line();
		    in.get_fields();
		    geo_assert(in.nb_fields() == 1);
		    M.vertices.create_vertices(in.field_as_uint(0));
		    for (index_t v = 0; v < M.vertices.nb(); ++v){
			in.get_line();
			in.get_fields();
			geo_assert(in.nb_fields() == 4);
			double pt[4] = {
			    in.field_as_double(1),
			    in.field_as_double(2),
			    in.field_as_double(3)
			};
			set_mesh_point(M, v, pt, 3);
		    }
		} else if (in.field_matches(0, "$EndNodes"))  {
		    return true; 
		}
	    }
	    return false;
	}
	bool read_elements(const std::string& filename, Mesh& M) {
	    index_t nb_elements = 0;
	    index_t nb_edges = 0;
	    index_t nb_tri = 0;
	    index_t nb_quad = 0;
	    index_t nb_tet = 0;
	    index_t nb_hex = 0;
	    index_t nb_pyr = 0;
	    index_t nb_pri = 0;
	    index_t nb_oth = 0;
	    { /* First pass to get the number of each element type */
		LineInput in(filename);
		while (in.get_line()) {
		    in.get_fields();
		    if (in.field_matches(0, "$Elements")) {
			in.get_line();
			in.get_fields();
			geo_assert(in.nb_fields() == 1);
			nb_elements = in.field_as_uint(0);
			for (index_t e = 0; e < nb_elements; ++e){
			    in.get_line();
			    in.get_fields();
			    if      (in.field_as_uint(1) == 1) nb_edges += 1;
			    else if (in.field_as_uint(1) == 2) nb_tri += 1;
			    else if (in.field_as_uint(1) == 3) nb_quad += 1;
			    else if (in.field_as_uint(1) == 4) nb_tet += 1;
			    else if (in.field_as_uint(1) == 5) nb_hex += 1;
			    else if (in.field_as_uint(1) == 6) nb_pri += 1;
			    else if (in.field_as_uint(1) == 7) nb_pyr += 1;
			    else { nb_oth += 1; }
			}
			if (nb_oth > 0) {
			    Logger::warn("I/O")
				<< nb_oth
				<< " elements with type unsupported"
				<< std::endl;
			}
		    }
		}
		M.edges.create_edges(nb_edges);
		M.facets.create_triangles(nb_tri);
		M.facets.create_quads(nb_quad);
		M.cells.create_tets(nb_tet);
		M.cells.create_hexes(nb_hex);
		M.cells.create_pyramids(nb_pyr);
		M.cells.create_prisms(nb_pri);
	    }
	    { /* Second pass to fill the content of the mesh */
		/* Re-use the number of elts as counters */
		nb_edges = 0;
		index_t nb_facets = 0;
		index_t nb_cells = 0;
		LineInput in(filename);
		while (in.get_line()) {
		    in.get_fields();
		    if (in.field_matches(0, "$Elements")) {
			in.get_line();
			in.get_fields();
			geo_assert(in.field_as_uint(0) == nb_elements);
			for (index_t e = 0; e < nb_elements; ++e){
			    in.get_line();
			    in.get_fields();
			    index_t nb_tags = in.field_as_uint(2);
			    index_t offset = 3 + nb_tags;
			    if (in.field_as_uint(1) == 1) {
				index_t nbv = 2;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.edges.set_vertex(
					nb_edges, j,
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_edges += 1;
			    } else if (in.field_as_uint(1) == 2) {
				index_t nbv = 3;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.facets.set_vertex(
					nb_facets, j,
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_facets += 1;
			    } else if (in.field_as_uint(1) == 3) {
				index_t nbv = 4;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.facets.set_vertex(
					nb_facets, j,
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_facets += 1;
			    } else if (in.field_as_uint(1) == 4) {
				index_t nbv = 4;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.cells.set_vertex(
					nb_cells, j,
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_cells += 1;
			    } else if (in.field_as_uint(1) == 5) {
				index_t nbv = 8;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.cells.set_vertex(
					nb_cells, msh2geo_hex[j],
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_cells += 1;
			    } else if (in.field_as_uint(1) == 6) {
				index_t nbv = 6;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.cells.set_vertex(
					nb_cells, j,
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_cells += 1;
			    } else if (in.field_as_uint(1) == 7) {
				index_t nbv = 5;
				geo_debug_assert(
				    in.nb_fields() == offset + nbv
				);
				for (index_t j = 0; j < nbv; ++j) {
				    M.cells.set_vertex(
					nb_cells, j,
					in.field_as_uint(offset + j) - 1
				    );
				}
				nb_cells += 1;
			    }
			}
		    }
		}
	    }
	    return true;
	}

	bool load(
	    const std::string& filename, Mesh& M,
	    const MeshIOFlags& ioflags
	) override {
	    geo_argused(ioflags);
	    M.clear();
	    M.vertices.set_dimension(3);
	    try {
		if (!verify_file_format(filename)) {
		    Logger::err("I/O")
			<< "$MeshFormat not supported" << std::endl;
		    return false;
		}
		if (!read_vertices(filename, M)) {
		    Logger::err("I/O")
			<< "failed to read vertices" << std::endl;
		    return false;
		}
		if (!read_elements(filename, M)) {
		    Logger::err("I/O")
			<< "failed to read elements" << std::endl;
		    return false;
		}
	    } catch(const std::string& what) {
		Logger::err("I/O") << what << std::endl;
		return false;
	    } catch(const std::exception& ex) {
		Logger::err("I/O") << ex.what() << std::endl;
		return false;
	    } catch(...) {
		Logger::err("I/O") << "Caught exception" << std::endl;
		return false;
	    }
	    return true;
	}

	bool save(
	    const Mesh& M_in, const std::string& filename, 
	    const MeshIOFlags& ioflags
	) override {

	    Mesh M(M_in.vertices.dimension());
	    M.copy(M_in, true);

	    M.vertices.remove_isolated();

	    Attribute<int> region;
	    Attribute<int> bdr_region;
	    if (M.cells.attributes().is_defined("region")) {
		region.bind(M.cells.attributes(), "region");
	    }
	    if (M.facets.attributes().is_defined("bdr_region")) {
		bdr_region.bind(M.facets.attributes(), "bdr_region");
	    }

	    std::ofstream out( filename.c_str() ) ;
	    out.precision( 16 ) ;

	    /* Header */
	    out << "$MeshFormat\n";
	    out << "2.2 0 " << sizeof(double) << std::endl;
	    out << "$EndMeshFormat\n";

	    /* Vertices */
	    out << "$Nodes" << std::endl ;
	    out << M.vertices.nb() << std::endl ;
	    for( index_t v = 0; v < M.vertices.nb(); v++ ) {
		out << v + id_offset_msh << " "
		    << M.vertices.point_ptr(v)[0] << " "
		    << M.vertices.point_ptr(v)[1] << " "
		    << M.vertices.point_ptr(v)[2] << '\n' ;
	    }
	    out << "$EndNodes" << std::endl ;

	    /* Elements */
	    index_t nb_tet = 0;
	    index_t nb_hex = 0;
	    index_t nb_pyr = 0;
	    index_t nb_pri = 0;
	    for(index_t c = 0; c != M.cells.nb(); ++c){
		if(M.cells.type(c) == GEO::MESH_TET){
		    ++nb_tet;
		} else if(M.cells.type(c) == GEO::MESH_HEX){
		    ++nb_hex;
		} else if(M.cells.type(c) == GEO::MESH_PYRAMID){
		    ++nb_pyr;
		} else if(M.cells.type(c) == GEO::MESH_PRISM){
		    ++nb_pri;
		}
	    }
	    index_t nb_elt = nb_tet + nb_hex + nb_pyr + nb_pri;
	    if (ioflags.has_element(MESH_FACETS)) nb_elt += M.facets.nb();

	    out << "$Elements" << std::endl ;
	    out << nb_elt << std::endl ;
	    index_t elt_id = 0; /* starts at 1, common for faces and cells */
	    if (ioflags.has_element(MESH_FACETS)) {
		for (index_t f = 0; f < M.facets.nb(); ++f) {
		    int attr_value = 0;
		    if (bdr_region.is_bound()){
			attr_value = bdr_region[f];
		    }
		    int type = -1;
		    if (M.facets.nb_vertices(f) == 3) {
			type = 2;
		    } else if (M.facets.nb_vertices(f) == 4) {
			type = 3;
		    } else {
			geo_assert_not_reached
			    }
		    elt_id += 1;
		    out << elt_id << " " << type << " " << "2" << " "
			<< attr_value << " " << attr_value << " ";
		    for (index_t li = 0; li < M.facets.nb_vertices(f); ++li) {
			out << M.facets.vertex(f, li) + id_offset_msh << " ";
		    }
		    out << std::endl;
		}
	    }
	    for (index_t c = 0; c < M.cells.nb(); ++c) {
		if (M.cells.type(c) == GEO::MESH_CONNECTOR) {
		    continue;
		}
		int attr_value = 0;
		if (region.is_bound()) attr_value = region[c];
		const index_t* msh2geo =
		    (M.cells.type(c) == GEO::MESH_HEX) ?
		    msh2geo_hex : msh2geo_def;
		elt_id += 1;

		/* Write to file, format is:
		 *   elm-number elm-type number-of-tags < tag > ...
		 *   node-number-list
		 */
		out << elt_id << " " << celltype_geo2msh[M.cells.type(c)]
		    << " " << "1" << " " << attr_value << " ";
		for (index_t li = 0; li < M.cells.nb_vertices(c); ++li) {
		    out << M.cells.vertex(c, msh2geo[li]) + id_offset_msh
			<< " ";
		}
		out << std::endl;
	    }
	    out << "$EndElements" << std::endl;
	    
	    out.close();
	    return true;
	}
    };
    
}

/****************************************************************************/

namespace GEO {

    MeshIOFlags::MeshIOFlags() {
        dimension_ = 3;
        attributes_ = MESH_NO_ATTRIBUTES;
        elements_ = MESH_ALL_ELEMENTS;
    }

    /************************************************************************/
    
    bool GEOGRAM_API mesh_load(
        const std::string& filename, Mesh& M,
        const MeshIOFlags& ioflags
    ) {
        Logger::out("I/O")
            << "Loading file " << filename << "..."
            << std::endl;

        M.clear();

        bool result = false;
        MeshIOHandler_var handler = MeshIOHandler::get_handler(filename);
        if(handler != nullptr) {
            try {
                result = handler->load(filename, M, ioflags);
            }
            catch(const std::exception& ex) {
                Logger::err("I/O") << ex.what() << std::endl;
                result = false;
            }
        }

        if(!result) {
            Logger::err("I/O")
                << "Could not load file: " << filename
                << std::endl;
            return false;
        }

        if(!M.vertices.single_precision()) {
            index_t nb = M.vertices.nb() * M.vertices.dimension();
            double* p = M.vertices.point_ptr(0);
            bool has_nan = false;
            for(index_t i = 0; i < nb; i++) {
                if(Numeric::is_nan(*p)) {
                    has_nan = true;
                    *p = 0.0;
                }
                p++;
            }
            if(has_nan) {
                Logger::warn("I/O") << "Found NaNs in input file" << std::endl;
            }
        }

        if(
	    FileSystem::extension(filename) != "geogram" &&
	    FileSystem::extension(filename) != "geogram_ascii"
	) {
            M.facets.connect();
            M.cells.connect();
            if(M.cells.nb() != 0 && M.facets.nb() == 0) {
                M.cells.compute_borders();
            }
        }

        M.show_stats("I/O");
        
        return true;
    }

    bool GEOGRAM_API mesh_save(
        const Mesh& M, const std::string& filename,
        const MeshIOFlags& ioflags
    ) {
        Logger::out("I/O")
            << "Saving file " << filename << "..."
            << std::endl;

        MeshIOHandler_var handler = MeshIOHandler::get_handler(filename);
        if(handler != nullptr && handler->save(M, filename, ioflags)) {
            return true;
        }

        Logger::err("I/O")
            << "Could not save file: " << filename
            << std::endl;
        return false;
    }

    /************************************************************************/


    MeshIOHandler* MeshIOHandler::create(const std::string& format) {
        MeshIOHandler* handler = MeshIOHandlerFactory::create_object(
            format
        );
        if(handler != nullptr) {
            return handler;
        }

        Logger::err("I/O")
            << "Unsupported file format: " << format
            << std::endl;
        return nullptr;
    }

    MeshIOHandler* MeshIOHandler::get_handler(
        const std::string& filename
    ) {
        std::string ext = FileSystem::extension(filename);
        return create(ext);
    }

    MeshIOHandler::~MeshIOHandler() {
    }


    void MeshIOHandler::bind_attributes(
        const Mesh& M_in, const MeshIOFlags& flags, bool create
    ) {
        Mesh& M = const_cast<Mesh&>(M_in); // UGLY I know !!
        if(create) {
            if(flags.has_attribute(MESH_VERTEX_REGION)) {
                vertex_region_.bind(M.vertices.attributes(),"region");
            }
            if(flags.has_attribute(MESH_FACET_REGION)) {
                facet_region_.bind(M.facets.attributes(),"region");
            }
            if(flags.has_attribute(MESH_CELL_REGION)) {
                cell_region_.bind(M.cells.attributes(),"region");
            }
        } else {
            if(flags.has_attribute(MESH_VERTEX_REGION)) {
                vertex_region_.bind_if_is_defined(
                    M.vertices.attributes(),"region"
                );
            }
            if(flags.has_attribute(MESH_FACET_REGION)) {
                facet_region_.bind_if_is_defined(
                    M.facets.attributes(),"region"
                );
            }
            if(flags.has_attribute(MESH_CELL_REGION)) {
                cell_region_.bind_if_is_defined(
                    M.cells.attributes(),"region"
                );
            }
        }
    }

    void MeshIOHandler::unbind_attributes() {
        if(vertex_region_.is_bound()) {
            vertex_region_.unbind();
        }
        if(facet_region_.is_bound()) {
            facet_region_.unbind();
        }
        if(cell_region_.is_bound()) {
            cell_region_.unbind();
        }
    }
    

    void mesh_io_initialize() {
        geo_register_MeshIOHandler_creator(LMIOHandler,   "mesh");
        geo_register_MeshIOHandler_creator(LMIOHandler,   "meshb");
        geo_register_MeshIOHandler_creator(OBJIOHandler,  "obj");
        geo_register_MeshIOHandler_creator(OBJIOHandler,  "eobj");        
        geo_register_MeshIOHandler_creator(OBJ6IOHandler, "obj6");
        geo_register_MeshIOHandler_creator(PLYIOHandler,  "ply");
        geo_register_MeshIOHandler_creator(OFFIOHandler,  "off");
        geo_register_MeshIOHandler_creator(STLIOHandler,  "stl");
        geo_register_MeshIOHandler_creator(XYZIOHandler,  "xyz");
        geo_register_MeshIOHandler_creator(PTSIOHandler,  "pts");
        geo_register_MeshIOHandler_creator(TETIOHandler,  "tet");
        geo_register_MeshIOHandler_creator(TET6IOHandler, "tet6");
        geo_register_MeshIOHandler_creator(TET8IOHandler, "tet8");	
        geo_register_MeshIOHandler_creator(GeogramIOHandler, "geogram");
        geo_register_MeshIOHandler_creator(GeogramIOHandler, "geogram_ascii");
        geo_register_MeshIOHandler_creator(GraphiteIOHandler, "graphite");
        geo_register_MeshIOHandler_creator(PDBIOHandler, "pdb");
        geo_register_MeshIOHandler_creator(PDBIOHandler, "pdb1");
        geo_register_MeshIOHandler_creator(OVMIOHandler, "ovm");
        geo_register_MeshIOHandler_creator(MSHIOHandler, "msh");	
    }

    
    bool GEOGRAM_API mesh_load(
        InputGeoFile& geofile, Mesh& M,
        const MeshIOFlags& ioflags 
    ) {
        GeogramIOHandler geogram;
        return geogram.load(geofile, M, ioflags);
    }

    bool GEOGRAM_API mesh_save(
        const Mesh& M, OutputGeoFile& geofile,
        const MeshIOFlags& ioflags 
    ) {
        GeogramIOHandler geogram;
        return geogram.save(M, geofile, ioflags);
    }
    
}
