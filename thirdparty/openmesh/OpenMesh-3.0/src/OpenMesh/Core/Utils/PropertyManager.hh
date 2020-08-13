/*===========================================================================*\
 *                                                                           *
 *                               OpenMesh                                    *
 *      Copyright (C) 2001-2014 by Computer Graphics Group, RWTH Aachen      *
 *                           www.openmesh.org                                *
 *                                                                           *
 *---------------------------------------------------------------------------*
 *  This file is part of OpenMesh.                                           *
 *                                                                           *
 *  OpenMesh is free software: you can redistribute it and/or modify         *
 *  it under the terms of the GNU Lesser General Public License as           *
 *  published by the Free Software Foundation, either version 3 of           *
 *  the License, or (at your option) any later version with the              *
 *  following exceptions:                                                    *
 *                                                                           *
 *  If other files instantiate templates or use macros                       *
 *  or inline functions from this file, or you compile this file and         *
 *  link it with other files to produce an executable, this file does        *
 *  not by itself cause the resulting executable to be covered by the        *
 *  GNU Lesser General Public License. This exception does not however       *
 *  invalidate any other reasons why the executable file might be            *
 *  covered by the GNU Lesser General Public License.                        *
 *                                                                           *
 *  OpenMesh is distributed in the hope that it will be useful,              *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of           *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the            *
 *  GNU Lesser General Public License for more details.                      *
 *                                                                           *
 *  You should have received a copy of the GNU LesserGeneral Public          *
 *  License along with OpenMesh.  If not,                                    *
 *  see <http://www.gnu.org/licenses/>.                                      *
 *                                                                           *
\*===========================================================================*/

/*===========================================================================*\
 *                                                                           *
 *   $Revision$                                                              *
 *   $Date$                                                                  *
 *                                                                           *
\*===========================================================================*/

#ifndef PROPERTYMANAGER_HH_
#define PROPERTYMANAGER_HH_

#include <sstream>
#include <stdexcept>
#include <string>

namespace OpenMesh {

/**
 * This class is intended to manage the lifecycle of properties.
 * It also defines convenience operators to access the encapsulated
 * property's value.
 *
 * Usage example:
 *
 * \code
 * TriMesh mesh;
 * PropertyManager<VPropHandleT<bool>, MeshT> visited(mesh, "visited.plugin-example.i8.informatik.rwth-aachen.de");
 *
 * for (TriMesh::VertexIter vh_it = mesh.begin(); ... ; ...) {
 *     if (!visited[*vh_it]) {
 *         visitComponent(mesh, *vh_it, visited);
 *     }
 * }
 * \endcode
 *
 */
template<typename PROPTYPE, typename MeshT>
class PropertyManager {
#if __cplusplus > 199711L or __GXX_EXPERIMENTAL_CXX0X__
    public:
        PropertyManager(const PropertyManager&) = delete;
        PropertyManager& operator=(const PropertyManager&) = delete;
#else
    private:
        /**
         * Noncopyable because there aren't no straightforward copy semantics.
         */
        PropertyManager(const PropertyManager&);

        /**
         * Noncopyable because there aren't no straightforward copy semantics.
         */
        PropertyManager& operator=(const PropertyManager&);
#endif

    public:
        /**
         * Constructor.
         *
         * Throws an \p std::runtime_error if \p existing is true and
         * no property named \p propname of the appropriate property type
         * exists.
         *
         * @param mesh The mesh on which to create the property.
         * @param propname The name of the property.
         * @param existing If false, a new property is created and its lifecycle is managed (i.e.
         * the property is deleted upon destruction of the PropertyManager instance). If true,
         * the instance merely acts as a convenience wrapper around an existing property with no
         * lifecycle management whatsoever.
         */
        PropertyManager(MeshT &mesh, const char *propname, bool existing = false) : mesh_(&mesh), retain_(existing), name_(propname) {
            if (existing) {
                if (!mesh_->get_property_handle(prop_, propname)) {
                    std::ostringstream oss;
                    oss << "Requested property handle \"" << propname << "\" does not exist.";
                    throw std::runtime_error(oss.str());
                }
            } else {
                mesh_->add_property(prop_, propname);
            }
        }

        PropertyManager() : mesh_(0), retain_(false) {
        }

        ~PropertyManager() {
            deleteProperty();
        }

        void swap(PropertyManager &rhs) {
            std::swap(mesh_, rhs.mesh_);
            std::swap(prop_, rhs.prop_);
            std::swap(retain_, rhs.retain_);
            std::swap(name_, rhs.name_);
        }

        static bool propertyExists(MeshT &mesh, const char *propname) {
            PROPTYPE dummy;
            return mesh.get_property_handle(dummy, propname);
        }

        bool isValid() const { return mesh_ != 0; }
        operator bool() const { return isValid(); }

        const PROPTYPE &getRawProperty() const { return prop_; }

        const std::string &getName() const { return name_; }

        MeshT &getMesh() const { return *mesh_; }

#if __cplusplus > 199711L or __GXX_EXPERIMENTAL_CXX0X__
        /**
         * Move constructor. Transfers ownership (delete responsibility).
         */
        PropertyManager(PropertyManager &&rhs) : mesh_(rhs.mesh_), prop_(rhs.prop_), retain_(rhs.retain_), name_(rhs.name_) {
            rhs.retain_ = true;
        }

        /**
         * Move assignment. Transfers ownership (delete responsibility).
         */
        PropertyManager &operator=(PropertyManager &&rhs) {

            deleteProperty();

            mesh_ = rhs.mesh_;
            prop_ = rhs.prop_;
            retain_ = rhs.retain_;
            name_ = rhs.name_;
            rhs.retain_ = true;

            return *this;
        }

        /**
         * Create a property manager for the supplied property and mesh.
         * If the property doesn't exist, it is created. In any case,
         * lifecycle management is disabled.
         */
        static PropertyManager createIfNotExists(MeshT &mesh, const char *propname) {
            PROPTYPE dummy_prop;
            PropertyManager pm(mesh, propname, mesh.get_property_handle(dummy_prop, propname));
            pm.retain();
            return std::move(pm);
        }

#else
        class Proxy {
            private:
                Proxy(MeshT *mesh_, PROPTYPE prop_, bool retain_, const std::string &name_) :
                    mesh_(mesh_), prop_(prop_), retain_(retain_), name_(name_) {}
                MeshT *mesh_;
                PROPTYPE prop_;
                bool retain_;
                std::string name_;

                friend class PropertyManager;
        };

        operator Proxy() {
            Proxy p(mesh_, prop_, retain_, name_);
            mesh_ = 0;
            retain_ = true;
            return p;
        }

        PropertyManager(Proxy p) : mesh_(p.mesh_), prop_(p.prop_), retain_(p.retain_), name_(p.name_) {}

        PropertyManager &operator=(Proxy p) {
            PropertyManager(p).swap(*this);
            return *this;
        }

        /**
         * Create a property manager for the supplied property and mesh.
         * If the property doesn't exist, it is created. In any case,
         * lifecycle management is disabled.
         */
        static Proxy createIfNotExists(MeshT &mesh, const char *propname) {
            PROPTYPE dummy_prop;
            PropertyManager pm(mesh, propname, mesh.get_property_handle(dummy_prop, propname));
            pm.retain();
            return (Proxy)pm;
        }
#endif

        /**
         * \brief Disable lifecycle management for this property.
         *
         * If this method is called, the encapsulated property will not be deleted
         * upon destruction of the PropertyManager instance.
         */
        inline void retain(bool doRetain = true) {
            retain_ = doRetain;
        }

        /**
         * Access the encapsulated property.
         */
        inline PROPTYPE &operator* () {
            return prop_;
        }

        /**
         * Access the encapsulated property.
         */
        inline const PROPTYPE &operator* () const {
            return prop_;
        }

        /**
         * Enables convenient access to the encapsulated property.
         *
         * For a usage example see this class' documentation.
         *
         * @param handle A handle of the appropriate handle type. (I.e. \p VertexHandle for \p VPropHandleT, etc.)
         */
        template<typename HandleType>
        inline typename PROPTYPE::reference operator[] (const HandleType &handle) {
            return mesh_->property(prop_, handle);
        }

        /**
         * Enables convenient access to the encapsulated property.
         *
         * For a usage example see this class' documentation.
         *
         * @param handle A handle of the appropriate handle type. (I.e. \p VertexHandle for \p VPropHandleT, etc.)
         */
        template<typename HandleType>
        inline typename PROPTYPE::const_reference operator[] (const HandleType &handle) const {
            return mesh_->property(prop_, handle);
        }

    private:
        void deleteProperty() {
            if (!retain_)
                mesh_->remove_property(prop_);
        }

    private:
        MeshT *mesh_;
        PROPTYPE prop_;
        bool retain_;
        std::string name_;
};

} /* namespace OpenMesh */
#endif /* PROPERTYMANAGER_HH_ */
