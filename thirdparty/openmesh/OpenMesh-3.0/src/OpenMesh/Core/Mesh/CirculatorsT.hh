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

#ifndef OPENMESH_CIRCULATORS_HH
#define OPENMESH_CIRCULATORS_HH
//=============================================================================
//
//  Vertex and Face circulators for PolyMesh/TriMesh
//
//=============================================================================



//== INCLUDES =================================================================

#include <OpenMesh/Core/System/config.h>
#include <cassert>
#include <cstddef>
#include <iterator>

//== NAMESPACES ===============================================================

namespace OpenMesh {
namespace Iterators {

template<class Mesh, class CenterEntityHandle>
class GenericCirculator_CenterEntityFnsT {
    public:
        static void increment(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter);
        static void decrement(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter);
};

template<class Mesh>
class GenericCirculator_CenterEntityFnsT<Mesh, typename Mesh::VertexHandle> {
    public:
        inline static void increment(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            heh = mesh->cw_rotated_halfedge_handle(heh);
            if (heh == start) ++lap_counter;
        }
        inline static void decrement(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            if (heh == start) --lap_counter;
            heh = mesh->ccw_rotated_halfedge_handle(heh);
        }
};

template<class Mesh>
class GenericCirculator_CenterEntityFnsT<Mesh, typename Mesh::FaceHandle> {
    public:
        inline static void increment(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            heh = mesh->next_halfedge_handle(heh);
            if (heh == start) ++lap_counter;
        }
        inline static void decrement(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            if (heh == start) --lap_counter;
            heh = mesh->prev_halfedge_handle(heh);
        }
};

template<class Mesh, class CenterEntityHandle, class ValueHandle>
class GenericCirculator_DereferenciabilityCheckT {
    public:
        //inline static bool isDereferenciable(const Mesh *mesh, const typename Mesh::HalfedgeHandle &heh, const typename Mesh::HalfedgeHandle &start, const int &lap_counter);
};

template<class Mesh>
class GenericCirculator_DereferenciabilityCheckT<Mesh, typename Mesh::FaceHandle, typename Mesh::FaceHandle> {
    public:
        inline static bool isDereferenciable(const Mesh *mesh, const typename Mesh::HalfedgeHandle &heh) {
            return mesh->face_handle(mesh->opposite_halfedge_handle(heh)).is_valid();
        }
};

template<class Mesh>
class GenericCirculator_DereferenciabilityCheckT<Mesh, typename Mesh::VertexHandle, typename Mesh::FaceHandle> {
    public:
        inline static bool isDereferenciable(const Mesh *mesh, const typename Mesh::HalfedgeHandle &heh) {
            return mesh->face_handle(heh).is_valid();
        }
};

template<class Mesh, class CenterEntityHandle, class ValueHandle>
class GenericCirculator_ValueHandleFnsT {
    public:
        inline static bool is_valid(const typename Mesh::HalfedgeHandle &heh, const typename Mesh::HalfedgeHandle &start, const int &lap_counter) {
            return heh.is_valid() && ((start != heh) || (lap_counter == 0));
        }
        inline static void init(const Mesh*, typename Mesh::HalfedgeHandle&, typename Mesh::HalfedgeHandle&, int&) {};
        inline static void increment(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            GenericCirculator_CenterEntityFnsT<Mesh, CenterEntityHandle>::increment(mesh, heh, start, lap_counter);
        }
        inline static void decrement(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            GenericCirculator_CenterEntityFnsT<Mesh, CenterEntityHandle>::decrement(mesh, heh, start, lap_counter);
        }
};

template<class Mesh, class CenterEntityHandle>
class GenericCirculator_ValueHandleFnsT<Mesh, CenterEntityHandle, typename Mesh::FaceHandle> {
    public:
        typedef GenericCirculator_DereferenciabilityCheckT<Mesh, CenterEntityHandle, typename Mesh::FaceHandle> GenericCirculator_DereferenciabilityCheck;

        inline static bool is_valid(const typename Mesh::HalfedgeHandle &heh, const typename Mesh::HalfedgeHandle &start, const int &lap_counter) {
            return heh.is_valid() && ((start != heh) || (lap_counter == 0));
        }
        inline static void init(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            if (heh.is_valid() && !GenericCirculator_DereferenciabilityCheck::isDereferenciable(mesh, heh) && lap_counter == 0)
                increment(mesh, heh, start, lap_counter);
        };
        inline static void increment(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            do {
                GenericCirculator_CenterEntityFnsT<Mesh, CenterEntityHandle>::increment(mesh, heh, start, lap_counter);
            } while (is_valid(heh, start, lap_counter) && !GenericCirculator_DereferenciabilityCheck::isDereferenciable(mesh, heh));
        }
        inline static void decrement(const Mesh *mesh, typename Mesh::HalfedgeHandle &heh, typename Mesh::HalfedgeHandle &start, int &lap_counter) {
            do {
                GenericCirculator_CenterEntityFnsT<Mesh, CenterEntityHandle>::decrement(mesh, heh, start, lap_counter);
            } while (is_valid(heh, start, lap_counter) && !GenericCirculator_DereferenciabilityCheck::isDereferenciable(mesh, heh));
        }
};

template<class Mesh>
class GenericCirculatorBaseT {
    public:
        typedef const Mesh* mesh_ptr;
        typedef const Mesh& mesh_ref;

    public:
        GenericCirculatorBaseT() : mesh_(0), lap_counter_(0) {}

        GenericCirculatorBaseT(mesh_ref mesh, HalfedgeHandle heh, bool end = false) :
            mesh_(&mesh), start_(heh), heh_(heh), lap_counter_(static_cast<int>(end)) {}

        GenericCirculatorBaseT(const GenericCirculatorBaseT &rhs) :
            mesh_(rhs.mesh_), start_(rhs.start_), heh_(rhs.heh_), lap_counter_(rhs.lap_counter_) {}

        inline typename Mesh::FaceHandle toFaceHandle() const {
            return mesh_->face_handle(heh_);
        }

        inline typename Mesh::FaceHandle toOppositeFaceHandle() const {
            return mesh_->face_handle(toOppositeHalfedgeHandle());
        }

        inline typename Mesh::EdgeHandle toEdgeHandle() const {
            return mesh_->edge_handle(heh_);
        }

        inline typename Mesh::HalfedgeHandle toHalfedgeHandle() const {
            return heh_;
        }

        inline typename Mesh::HalfedgeHandle toOppositeHalfedgeHandle() const {
            return mesh_->opposite_halfedge_handle(heh_);
        }

        inline typename Mesh::VertexHandle toVertexHandle() const {
            return mesh_->to_vertex_handle(heh_);
        }

        inline GenericCirculatorBaseT &operator=(const GenericCirculatorBaseT &rhs) {
            mesh_ = rhs.mesh_;
            start_ = rhs.start_;
            heh_ = rhs.heh_;
            lap_counter_ = rhs.lap_counter_;
            return *this;
        }

        inline bool operator==(const GenericCirculatorBaseT &rhs) const {
            return mesh_ == rhs.mesh_ && start_ == rhs.start_ && heh_ == rhs.heh_ && lap_counter_ == rhs.lap_counter_;
        }

        inline bool operator!=(const GenericCirculatorBaseT &rhs) const {
            return !operator==(rhs);
        }

    protected:
        mesh_ptr mesh_;
        typename Mesh::HalfedgeHandle start_, heh_;
        int lap_counter_;
};

template<class Mesh, class CenterEntityHandle, class ValueHandle,
        ValueHandle (GenericCirculatorBaseT<Mesh>::*Handle2Value)() const>
class GenericCirculatorT : protected GenericCirculatorBaseT<Mesh> {
    public:
        typedef std::ptrdiff_t difference_type;
        typedef ValueHandle value_type;
        typedef const value_type& reference;
        typedef const value_type* pointer;
        typedef std::bidirectional_iterator_tag iterator_category;

        typedef typename GenericCirculatorBaseT<Mesh>::mesh_ptr mesh_ptr;
        typedef typename GenericCirculatorBaseT<Mesh>::mesh_ref mesh_ref;
        typedef GenericCirculator_ValueHandleFnsT<Mesh, CenterEntityHandle, ValueHandle> GenericCirculator_ValueHandleFns;

    public:
        GenericCirculatorT() {}
        GenericCirculatorT(mesh_ref mesh, CenterEntityHandle start, bool end = false) :
            GenericCirculatorBaseT<Mesh>(mesh, mesh.halfedge_handle(start), end) {

            GenericCirculator_ValueHandleFns::init(this->mesh_, this->heh_, this->start_, this->lap_counter_);
        }
        GenericCirculatorT(mesh_ref mesh, HalfedgeHandle heh, bool end = false) :
            GenericCirculatorBaseT<Mesh>(mesh, heh, end) {

            GenericCirculator_ValueHandleFns::init(this->mesh_, this->heh_, this->start_, this->lap_counter_);
        }
        GenericCirculatorT(const GenericCirculatorT &rhs) : GenericCirculatorBaseT<Mesh>(rhs) {}

        GenericCirculatorT& operator++() {
            assert(this->mesh_);
            GenericCirculator_ValueHandleFns::increment(this->mesh_, this->heh_, this->start_, this->lap_counter_);
            return *this;
        }
        GenericCirculatorT& operator--() {
            assert(this->mesh_);
            GenericCirculator_ValueHandleFns::decrement(this->mesh_, this->heh_, this->start_, this->lap_counter_);
            return *this;
        }

        /// Post-increment
        GenericCirculatorT operator++(int) {
            assert(this->mesh_);
            GenericCirculatorT cpy(*this);
            ++(*this);
            return cpy;
        }

        /// Post-decrement
        GenericCirculatorT operator--(int) {
            assert(this->mesh_);
            GenericCirculatorT cpy(*this);
            --(*this);
            return cpy;
        }

        /// Standard dereferencing operator.
        value_type operator*() const {
#ifndef NDEBUG
            assert(this->heh_.is_valid());
            value_type res = (this->*Handle2Value)();
            assert(res.is_valid());
            return res;
#else
            return (this->*Handle2Value)();
#endif
        }

        /**
         * @brief Pointer dereferentiation.
         *
         * This returns a pointer which points to a handle
         * that loses its validity once this dereferentiation is
         * invoked again. Thus, do not store the result of
         * this operation.
         */
        pointer operator->() const {
            pointer_deref_value = **this;
            return &pointer_deref_value;
        }

        GenericCirculatorT &operator=(const GenericCirculatorT &rhs) {
            GenericCirculatorBaseT<Mesh>::operator=(rhs);
            return *this;
        };

        bool operator==(const GenericCirculatorT &rhs) const {
            return GenericCirculatorBaseT<Mesh>::operator==(rhs);
        }

        bool operator!=(const GenericCirculatorT &rhs) const {
            return GenericCirculatorBaseT<Mesh>::operator!=(rhs);
        }

        bool is_valid() const {
            return GenericCirculator_ValueHandleFns::is_valid(this->heh_, this->start_, this->lap_counter_);
        }

        DEPRECATED("current_halfedge_handle() is an implementation detail and should not be accessed from outside the iterator class.")
        /**
         * \deprecated
         * current_halfedge_handle() is an implementation detail and should not
         * be accessed from outside the iterator class.
         */
        const HalfedgeHandle &current_halfedge_handle() const {
            return this->heh_;
        }

        DEPRECATED("Do not use this error prone implicit cast. Compare to end-iterator or use is_valid(), instead.")
        /**
         * \deprecated
         * Do not use this error prone implicit cast. Compare to the
         * end-iterator or use is_valid() instead.
         */
        operator bool() const {
            return is_valid();
        }

        /**
         * \brief Return the handle of the current target.
         * \deprecated
         * This function clutters your code. Use dereferencing operators -> and * instead.
         */
        DEPRECATED("This function clutters your code. Use dereferencing operators -> and * instead.")
        value_type handle() const {
          return **this;
        }

        /**
         * \brief Cast to the handle of the current target.
         * \deprecated
         * Implicit casts of iterators are unsafe. Use dereferencing operators
         * -> and * instead.
         */
        DEPRECATED("Implicit casts of iterators are unsafe. Use dereferencing operators -> and * instead.")
        operator value_type() const {
          return **this;
        }

        template<typename STREAM>
        friend STREAM &operator<< (STREAM &s, const GenericCirculatorT &self) {
            return s << self.mesh_ << ", " << self.start_.idx() << ", " << self.heh_.idx() << ", " << self.lap_counter_;
        }

    private:
        mutable value_type pointer_deref_value;
};

} // namespace Iterators
} // namespace OpenMesh

#endif
