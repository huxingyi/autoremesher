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

#ifndef GEOGRAM_MESH_NEW_MESH_IO
#define GEOGRAM_MESH_NEW_MESH_IO

#include <geogram/basic/common.h>
#include <geogram/basic/numeric.h>
#include <geogram/basic/factory.h>
#include <geogram/basic/attributes.h>
#include <geogram/mesh/mesh.h>
#include <string>

/**
 * \file geogram/mesh/mesh_io.h
 * \brief Functions to load and save meshes
 */

namespace GEO {

    class InputGeoFile;
    class OutputGeoFile;
    
    /**
     * \brief Indicates the attributes stored in a mesh and attached
     *  to the mesh elements (vertices, facets or volumes).
     * \details The set of attributes attached to a mesh is represented
     *  by a bitwise or combination of the constants.
     */
    enum MeshAttributesFlags {
        MESH_NO_ATTRIBUTES = 0,
        MESH_VERTEX_REGION = 1,
        MESH_VERTEX_TEX_COORD = 2,
        MESH_VERTEX_COLOR = 4,
        MESH_FACET_REGION = 8,
        MESH_CELL_REGION = 16,
        MESH_ALL_ATTRIBUTES = 255
    };

    
    /**
     * \brief Mesh load/save flags
     * \details Represents the optional attributes of a Mesh that can be
     * loaded and stored in a file.
     */
    class GEOGRAM_API MeshIOFlags {
    public:
        /**
         * \brief Constructs a new MeshIOFlags with default attributes.
         */
        MeshIOFlags();

        /**
         * \brief Gets the dimension of the mesh (number of coordinates of 
         *  the vertices).
         * \return the dimension of the mesh 
         *  (i.e. number of coordinates of the verfices).
         */
        coord_index_t dimension() const {
            return dimension_;
        }

        /**
         * \brief Sets the dimension of the mesh (number of coordinates 
         *  of the vertices).
         * \param[in] x the dimension of the mesh
         */
        void set_dimension(coord_index_t x) {
            dimension_ = x;
        }

        /**
         * \brief Gets the attributes that should be loaded or saved.
         * \return a set of MeshAttributesFlags combined with bitwise or
         */
        MeshAttributesFlags attributes() const {
            return attributes_;
        }

        /**
         * \brief Sets the attributes that should be loaded or stored.
         * \param[in] x a set of MeshAttribute%s combined with bitwise or
         */
        void set_attributes(MeshAttributesFlags x) {
            attributes_ = x;
        }

        /**
         * \brief Sets a mesh attribute.
         * \details Indicates that \p x should be loaded or stored.
         * \param[in] x the attribute that should be set
         */
        void set_attribute(MeshAttributesFlags x) {
            attributes_ = MeshAttributesFlags(attributes_ | x);
        }

        /**
         * \brief Resets a mesh attribute..
         * \details Indicates that \p x should not be loaded nor stored.
         * \param[in] x the attribute that should be reset
         */
        void reset_attribute(MeshAttributesFlags& x) {
            attributes_ = MeshAttributesFlags(attributes_ & ~x);
        }

        /**
         * \brief Tests whether a mesh attribute is set.
         * \details If set, this means that the mesh attribute \p x will
         *  be loaded or stored.
         * \param[in] x the attribute to test
         * \retval true if attribute \p x is se
         * \retval false otherwise
         */
        bool has_attribute(MeshAttributesFlags x) const {
            return (attributes_ & x) != 0;
        }

        /**
         * \brief Gets the set of mesh elements that should be loaded or stored.
         * \return a set of MeshElement%s combined with bitwise or
         */
        MeshElementsFlags elements() const {
            return elements_;
        }

        /**
         * \brief Sets the set of mesh elements that should be loaded or stored.
         * \param[in] x a set of MeshElementsFlags combined with bitwise or
         */
        void set_elements(MeshElementsFlags x) {
            elements_ = x;
        }

        /**
         * \brief Sets a mesh element.
         * \details Indicates that mesh elements \p x should be loaded or 
         *  stored.
         * \param[in] x the element to set
         */
        void set_element(MeshElementsFlags x) {
            elements_ = MeshElementsFlags(elements_ | x);
        }

        /**
         * \brief Resets a mesh element.
         * \details Indicates that mesh elements \p x should not be 
         *  loaded nor stored.
         * \param[in] x the element to reset
         */
        void reset_element(MeshElementsFlags x) {
            elements_ = MeshElementsFlags(elements_ & ~x);
        }

        /**
         * \brief Tests whether a mesh element is set.
         * \details If set, this means that the mesh elements \p x will
         *  be loaded or stored.
         * \param[in] x the element to test
         * \retval true if element \p x is set
         * \retval false otherwise
         */
        bool has_element(MeshElementsFlags x) const {
            return (elements_ & x) != 0;
        }

        /**
	 * \brief Sets the name of the texture image file
	 *  associated with this mesh.
	 * \details Used by the OBJ file format. If present
	 *  generates a material lib.
	 * \param[in] x the name of the file with the texture.
	 */
        void set_texture_filename(const std::string& x) {
	    texture_filename_ = x;
	}

        /**
	 * \brief Gets the name of the texture image file.
	 * \return the name of the file.
	 * \see set_texture_filename()
	 */
        const std::string& get_texture_filename() const {
	    return texture_filename_;
	}
    
    private:
        coord_index_t dimension_;
        MeshAttributesFlags attributes_;
        MeshElementsFlags elements_;
        std::string texture_filename_;
    };

    
    /**
     * \brief Loads a mesh from a file.
     * \details
     * Loads the contents of the mesh file \p filename and stores the
     * resulting mesh to \p M. The file format is determined by the \p
     * filename's extension, which determines the appropriate 
     * MeshIOHandler to use to read the file.
     * \param[in] filename name of the file
     * \param[out] M the loaded mesh
     * \param[in] ioflags specifies which attributes and 
     *  elements should be loaded
     * \return true on success, false otherwise.
     * \see MeshIOHandler
     */
    bool GEOGRAM_API mesh_load(
        const std::string& filename, Mesh& M,
        const MeshIOFlags& ioflags = MeshIOFlags()
    );

    /**
     * \brief Loads a mesh from a GeoFile ('.geogram' file format).
     * \details
     * Loads the contents of the InputGeoFile \p geofile and stores the
     * resulting mesh to \p M. This function can be used to load several
     * meshes that are stored in the same GeoFile.
     * \param[in] geofile a reference to the InputGeoFile
     * \param[out] M the loaded mesh
     * \param[in] ioflags specifies which attributes and 
     *  elements should be loaded
     * \return true on success, false otherwise.
     */
    bool GEOGRAM_API mesh_load(
        InputGeoFile& geofile, Mesh& M,
        const MeshIOFlags& ioflags = MeshIOFlags()
    );
    

    /**
     * \brief Saves a mesh to a file.
     * \details
     * Saves mesh \p M to the file \p filename. The file format is determined
     * by the \p filename's extension, which determines the appropriate
     * MeshIOHandler to use to write the file.
     * \param[in] M the mesh to save
     * \param[in] filename name of the file
     * \param[in] ioflags specifies which attributes and elements 
     *  should be saved
     * \return true on success, false otherwise.
     * \see MeshIOHandler
     */
    bool GEOGRAM_API mesh_save(
        const Mesh& M, const std::string& filename,
        const MeshIOFlags& ioflags = MeshIOFlags()
    );

    /**
     * \brief Saves a mesh to a GeoFile ('.geogram' file format)
     * \details
     * Saves mesh \p M to the GeoFile \p geofile. This function can be
     * used to write several meshes into the same GeoFile.
     * \param[in] M the mesh to save
     * \param[in] geofile a reference to the OutputGeoFile
     * \param[in] ioflags specifies which attributes and elements 
     *  should be saved
     * \return true on success, false otherwise.
     */
    bool GEOGRAM_API mesh_save(
        const Mesh& M, OutputGeoFile& geofile,
        const MeshIOFlags& ioflags = MeshIOFlags()
    );
    
    
    /*************************************************************************/

    /**
     * \brief Mesh loader and saver
     * \details MeshIOHandler is responsible to load and save meshes in a
     * specific file format. MeshIOHandler is used internally by mesh_load()
     * and mesh_save(), so you don't normally need to use it directly, unless
     * you need to implement a new file format.
     *
     * Vorpaline currently supports thr following file formats:
     * - OBJ http://en.wikipedia.org/wiki/Wavefront_.obj_file
     * - mesh, meshb (ASCII and binary)
     *   http://www-roc.inria.fr/gamma/gamma/Membres/CIPD/Loic.Marechal/Research/LM5.html
     * - ply (ASCII and binary, single and double precision)
     *   http://en.wikipedia.org/wiki/PLY_(file_format)
     * - OFF http://www.geomview.org/docs/html/OFF.html
     * - STL (ASCII and binary)
     *   http://en.wikipedia.org/wiki/STL_(file_format)
     *
     * MeshIOHandler%s are created using method create() which uses the
     * Factory service. Handlers for new file formats can be implemented and
     * registered to the factory using geo_register_MeshIOHandler_creator().
     * \see MeshIOHandlerFactory
     * \see geo_register_MeshIOHandler_creator
     */
    class GEOGRAM_API MeshIOHandler : public Counted {
    public:
        /**
         * \brief Creates a MeshIOHandler
         * \param[in] format format of the file
         * \retval nullptr if \p format is not a supported file format.
         * \retval otherwise, a pointer to a IO handler. The returned pointer
         * must be stored in an MeshIOHandler_var that does automatic
         * destruction:
         * \code
         * MeshIOHandler_var handler = MeshIOHandler::create("mesh");
         * \endcode
         */
        static MeshIOHandler* create(const std::string& format);

        /**
         * \brief Gets the MeshIOHandler for a file
         * \details Determines the file format from the extension of file \p
         * filename and creates a MeshIOHandler for this format.
         * \param[in] filename a path to a mesh file
         * \retval nullptr if \p format is not a supported file format.
         * \retval a pointer to a IO handler if format is supported.
         * \retval nullptr otherwise
         * \see create()
         */
        static MeshIOHandler* get_handler(const std::string& filename);

        /**
         * \brief Loads a double precision mesh from a file.
         * \param[in] filename name of the file
         * \param[out] M the loaded mesh
         * \param[in] ioflags specifies which attributes and elements 
         *  should be loaded
         */
        virtual bool load(
            const std::string& filename, Mesh& M,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) = 0;

        /**
         * \brief Saves a mesh to a file.
         * \param[in] M the mesh to save
         * \param[in] filename name of the file
         * \param[in] ioflags specifies which attributes and elements 
         *  should be saved
         * \return true on success, false otherwise.
         */
        virtual bool save(
            const Mesh& M, const std::string& filename,
            const MeshIOFlags& ioflags = MeshIOFlags()
        ) = 0;

    protected:
        /**
         * \brief MeshIOHandler default constructor
         */
        MeshIOHandler() {
        }

        /**
         * \brief MeshIOHandler destructor
         */
        virtual ~MeshIOHandler();

        virtual void bind_attributes(
            const Mesh& M, const MeshIOFlags& flags, bool create
        );
        virtual void unbind_attributes();
        
    protected:
        Attribute<index_t> vertex_region_;
        Attribute<index_t> facet_region_;
        Attribute<index_t> cell_region_;
    };

    /**
     * \brief A smart pointer that contains a MeshIOHandler object.
     * \relates MeshIOHandler
     */
    typedef SmartPointer<MeshIOHandler> MeshIOHandler_var;

    /**
     * \brief MeshIOHandler Factory
     * \details
     * This Factory is used to create MeshIOHandler objects.
     * It can also be used to register new MeshIOHandler
     * implementations.
     * \see geo_register_MeshIOHandler_creator
     * \see Factory
     * \relates MeshIOHandler
     */
    typedef Factory0<MeshIOHandler> MeshIOHandlerFactory;

    /**
     * \brief Helper macro to register a MeshIOHandler implementation
     * \see MeshIOHandlerFactory
     * \relates MeshIOHandler
     */
#define geo_register_MeshIOHandler_creator(type, name) \
    geo_register_creator(GEO::MeshIOHandlerFactory, type, name)


    void GEOGRAM_API mesh_io_initialize() ;
}

#endif

