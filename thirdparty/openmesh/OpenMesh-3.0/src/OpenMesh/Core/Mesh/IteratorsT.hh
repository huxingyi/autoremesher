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
 *   $Revision$                                                         *
 *   $Date$                   *
 *                                                                           *
\*===========================================================================*/

#ifndef OPENMESH_ITERATORS_HH
#define OPENMESH_ITERATORS_HH

//=============================================================================
//
//  Iterators for PolyMesh/TriMesh
//
//=============================================================================



//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <OpenMesh/Core/Mesh/Status.hh>
#include <cassert>
#include <cstddef>
#include <iterator>


//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Iterators {


//== FORWARD DECLARATIONS =====================================================


template <class Mesh> class ConstVertexIterT;
template <class Mesh> class VertexIterT;
template <class Mesh> class ConstHalfedgeIterT;
template <class Mesh> class HalfedgeIterT;
template <class Mesh> class ConstEdgeIterT;
template <class Mesh> class EdgeIterT;
template <class Mesh> class ConstFaceIterT;
template <class Mesh> class FaceIterT;


template <class Mesh, class ValueHandle, class MemberOwner, bool (MemberOwner::*PrimitiveStatusMember)() const, size_t (MemberOwner::*PrimitiveCountMember)() const>
class GenericIteratorT {
    public:
        //--- Typedefs ---

        typedef ValueHandle                     value_handle;
        typedef value_handle                    value_type;
        typedef std::bidirectional_iterator_tag iterator_category;
        typedef std::ptrdiff_t                  difference_type;
        typedef const value_type&               reference;
        typedef const value_type*               pointer;
        typedef const Mesh*                     mesh_ptr;
        typedef const Mesh&                     mesh_ref;

        /// Default constructor.
        GenericIteratorT()
        : mesh_(0), skip_bits_(0)
        {}

        /// Construct with mesh and a target handle.
        GenericIteratorT(mesh_ref _mesh, value_handle _hnd, bool _skip=false)
        : mesh_(&_mesh), hnd_(_hnd), skip_bits_(0)
        {
            if (_skip) enable_skipping();

            // Set vertex handle invalid if the mesh contains no vertex
            if((mesh_->*PrimitiveCountMember)() == 0) hnd_ = value_handle(-1);
        }

        /// Standard dereferencing operator.
        reference operator*() const {
            return hnd_;
        }

        /// Standard pointer operator.
        pointer operator->() const {
            return &hnd_;
        }

        /**
         * \brief Get the handle of the item the iterator refers to.
         * \deprecated 
         * This function clutters your code. Use dereferencing operators -> and * instead.
         */
        DEPRECATED("This function clutters your code. Use dereferencing operators -> and * instead.")
        value_handle handle() const {
            return hnd_;
        }

        /**
         * \brief Cast to the handle of the item the iterator refers to.
         * \deprecated
         * Implicit casts of iterators are unsafe. Use dereferencing operators
         * -> and * instead.
         */
        DEPRECATED("Implicit casts of iterators are unsafe. Use dereferencing operators -> and * instead.")
        operator value_handle() const {
            return hnd_;
        }

        /// Are two iterators equal? Only valid if they refer to the same mesh!
        bool operator==(const GenericIteratorT& _rhs) const {
            return ((mesh_ == _rhs.mesh_) && (hnd_ == _rhs.hnd_));
        }

        /// Not equal?
        bool operator!=(const GenericIteratorT& _rhs) const {
            return !operator==(_rhs);
        }

        /// Standard pre-increment operator
        GenericIteratorT& operator++() {
            hnd_.__increment();
            if (skip_bits_)
                skip_fwd();
            return *this;
        }

        /// Standard post-increment operator
        GenericIteratorT operator++(int) {
            GenericIteratorT cpy(*this);
            ++(*this);
            return cpy;
        }

        /// Standard pre-decrement operator
        GenericIteratorT& operator--() {
            hnd_.__decrement();
            if (skip_bits_)
                skip_bwd();
            return *this;
        }

        /// Standard post-decrement operator
        GenericIteratorT operator--(int) {
            GenericIteratorT cpy(*this);
            --(*this);
            return cpy;
        }

        /// Turn on skipping: automatically skip deleted/hidden elements
        void enable_skipping() {
            if (mesh_ && (mesh_->*PrimitiveStatusMember)()) {
                Attributes::StatusInfo status;
                status.set_deleted(true);
                status.set_hidden(true);
                skip_bits_ = status.bits();
                skip_fwd();
            } else
                skip_bits_ = 0;
        }

        /// Turn on skipping: automatically skip deleted/hidden elements
        void disable_skipping() {
            skip_bits_ = 0;
        }

    private:

        void skip_fwd() {
            assert(mesh_ && skip_bits_);
            while ((hnd_.idx() < (signed) (mesh_->*PrimitiveCountMember)())
                    && (mesh_->status(hnd_).bits() & skip_bits_))
                hnd_.__increment();
        }

        void skip_bwd() {
            assert(mesh_ && skip_bits_);
            while ((hnd_.idx() >= 0) && (mesh_->status(hnd_).bits() & skip_bits_))
                hnd_.__decrement();
        }

    protected:
        mesh_ptr mesh_;
        value_handle hnd_;
        unsigned int skip_bits_;
};

//=============================================================================
} // namespace Iterators
} // namespace OpenMesh
//=============================================================================
#endif 
//=============================================================================
