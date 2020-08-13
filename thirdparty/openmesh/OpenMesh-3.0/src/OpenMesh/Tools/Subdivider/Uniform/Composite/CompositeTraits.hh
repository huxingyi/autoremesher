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

/** \file Uniform/Composite/CompositeTraits.hh
    Mesh traits for uniform composite subdivision.
 */

//=============================================================================
//
//  CLASS Traits
//
//=============================================================================

#ifndef OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITETRAITS_HH
#define OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITETRAITS_HH


//== INCLUDES =================================================================

//#include "Config.hh"
// --------------------
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Mesh/Attributes.hh>


//== NAMESPACE ================================================================

namespace OpenMesh   { // BEGIN_NS_OPENMESH
namespace Subdivider { // BEGIN_NS_DECIMATER
namespace Uniform    { // BEGIN_NS_UNIFORM


//== CLASS DEFINITION =========================================================
	      

/** Uniform Composite Subdivision framework.
*/

struct CompositeTraits : public OpenMesh::DefaultTraits
{
  FaceAttributes( OpenMesh::Attributes::Normal );
  
  VertexAttributes( OpenMesh::Attributes::Normal );
  
  //HalfedgeAttributes( OpenMesh::Attributes::PrevHalfedge );

  FaceTraits
  {
  private:
    typedef typename Refs::HalfedgeHandle HalfedgeHandle;
    typedef typename Refs::Scalar Scalar;
    typedef typename Refs::Point Point;
    HalfedgeHandle red_halfedge_handle_;
    unsigned int generation_;
    bool red_;
    Scalar quality_;
    Point midpoint_;
    Point position_;
    
  public:
    const unsigned int& generation() { return generation_; }
    void set_generation(const unsigned int& _g) { generation_ = _g; }
    void inc_generation() { ++generation_; }
    void set_red() { red_ = 1; }
    void set_green() {red_ = 0; }
    bool is_red() { return red_; }
    bool is_green() { return !red_; }
    void set_red_halfedge_handle(HalfedgeHandle& _heh) 
      { red_halfedge_handle_ = _heh; }
    HalfedgeHandle& red_halfedge_handle() { return red_halfedge_handle_; }
    void set_quality(Scalar& _q) { quality_ = _q; }
    Scalar& quality() { return quality_; }
    const Point& midpoint() const { return midpoint_; }
    void set_midpoint(const Point& _p) { midpoint_ = _p; }
    const Point& position() const { return position_; }
    void set_position(const Point& _p) { position_ = _p; }
  };

  EdgeTraits
  {
  private:
    typedef typename Refs::Point Point;
    typedef typename Refs::Scalar Scalar;
    Point midpoint_;
    Scalar length_;
    Point position_;
  public: 
    const Point& midpoint() const { return midpoint_; }
    void set_midpoint(const Point& _vh) { midpoint_ = _vh; }
    const Scalar& length() const { return length_; }
    void set_length(const Scalar& _s) { length_ = _s; }
    const Point& position() const { return position_; }
    void set_position(const Point& _p) { position_ = _p; }
  };

  VertexTraits
  {
  private:
    typedef typename Refs::Point Point;
    Point new_pos_;
    Point orig_pos_;
    Point position_;
    unsigned int generation_;
  public:
    const Point& new_pos() const { return new_pos_; }
    void set_new_pos(const Point& _p) { new_pos_ = _p; }
    const unsigned int& generation() const { return generation_; }
    void set_generation(const unsigned int& _i) { generation_ = _i; }
    const Point& orig_pos() const { return orig_pos_; }
    void set_orig_pos(const Point& _p) { orig_pos_ = _p; }
    const Point& position() const { return position_; }
    void set_position(const Point& _p) { position_ = _p; }
  };
};

//=============================================================================
} // END_NS_UNIFORM
} // END_NS_SUBDIVIDER
} // END_NS_OPENMESH
//=============================================================================
#endif // OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITETRAITS_HH defined
//=============================================================================

