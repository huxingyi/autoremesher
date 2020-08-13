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

/** \file CompositeLoopT.hh
    
 */

//=============================================================================
//
//  CLASS LoopT
//
//=============================================================================

#ifndef OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITELOOPT_HH
#define OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITELOOPT_HH


//== INCLUDES =================================================================

#include "Composite/CompositeT.hh"
#include "Composite/CompositeTraits.hh"


//== NAMESPACE ================================================================

namespace OpenMesh   { // BEGIN_NS_OPENMESH
namespace Subdivider { // BEGIN_NS_DECIMATER
namespace Uniform    { // BEGIN_NS_DECIMATER


//== CLASS DEFINITION =========================================================

/** Uniform composite Loop subdivision algorithm
 */
template <class MeshType, class RealType=float>
class CompositeLoopT : public CompositeT<MeshType, RealType>
{
public:

  typedef CompositeT<MeshType, RealType>  Inherited;
  
public:

  CompositeLoopT() : Inherited() {};
  CompositeLoopT(MeshType& _mesh) : Inherited(_mesh) {};
  ~CompositeLoopT() {}

public:
  
  const char *name() const { return "Uniform Composite Loop"; }
  
protected: // inherited interface

  void apply_rules(void)  
  { 
    Inherited::Tvv4(); 
    Inherited::VdE(); 
    Inherited::EVc(coeffs_); 
    Inherited::VdE(); 
    Inherited::EVc(coeffs_); 
  }

protected:
  
 typedef typename Inherited::Coeff Coeff;

  
  /** Helper struct
   *  \internal
   */
  struct EVCoeff : public Coeff 
  {
    EVCoeff() : Coeff() { init(50); }

    void init(size_t _max_valence)
    {
      weights_.resize(_max_valence);
      std::generate(weights_.begin(), 
                    weights_.end(), compute_weight() );
    }
    
    double operator()(size_t _valence) { return weights_[_valence]; }

    /// \internal
    struct compute_weight 
    {
      compute_weight() : val_(0) { }

      double operator()(void) // Loop weights for non-boundary vertices
      {
        // 1      3          2 * pi
        // - * ( --- + cos ( ------- ) )� - 1.0
        // 2      2          valence        
        double f1 = 1.5 + cos(2.0*M_PI/val_++);
        return 0.5 * f1 * f1 - 1.0;
      }
      size_t val_;

    };

    std::vector<double> weights_;
  } coeffs_;

};
  
//=============================================================================
} // END_NS_UNIFORM
} // END_NS_SUBDIVIDER
} // END_NS_OPENMESH
//=============================================================================
#endif // OPENMESH_SUBDIVIDER_UNIFORM_COMPOSITELOOPT_HH defined
//=============================================================================
