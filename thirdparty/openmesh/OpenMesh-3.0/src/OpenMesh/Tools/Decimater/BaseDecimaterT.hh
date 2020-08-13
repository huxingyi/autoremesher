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

/** \file BaseDecimaterT.hh
 */

//=============================================================================
//
//  CLASS McDecimaterT
//
//=============================================================================

#ifndef OPENMESH_BASE_DECIMATER_DECIMATERT_HH
#define OPENMESH_BASE_DECIMATER_DECIMATERT_HH


//== INCLUDES =================================================================

#include <memory>

#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Tools/Decimater/ModBaseT.hh>
#include <OpenMesh/Core/Utils/Noncopyable.hh>



//== NAMESPACE ================================================================

namespace OpenMesh  {
namespace Decimater {


//== CLASS DEFINITION =========================================================


/** base class decimater framework
    \see BaseDecimaterT, \ref decimater_docu
*/
class BaseDecimaterModule
{
};

template < typename MeshT >
class BaseDecimaterT : private Utils::Noncopyable
{
public: //-------------------------------------------------------- public types

  typedef BaseDecimaterT< MeshT >       Self;
  typedef MeshT                         Mesh;
  typedef CollapseInfoT<MeshT>          CollapseInfo;
  typedef ModBaseT<MeshT>                Module;
  typedef std::vector< Module* >        ModuleList;
  typedef typename ModuleList::iterator ModuleListIterator;

public: //------------------------------------------------------ public methods
  BaseDecimaterT(Mesh& _mesh);
  virtual ~BaseDecimaterT();

  /** Initialize decimater and decimating modules.

      Return values:
      true   ok
      false  No ore more than one non-binary module exist. In that case
             the decimater is uninitialized!
   */
  bool initialize();


  /// Returns whether decimater has been successfully initialized.
  bool is_initialized() const { return initialized_; }


  /// Print information about modules to _os
  void info( std::ostream& _os );

public: //--------------------------------------------------- module management

  /// access mesh. used in modules.
  Mesh& mesh() { return mesh_; }

  /// add module to decimater
  template < typename _Module >
  bool add( ModHandleT<_Module>& _mh )
  {
    if (_mh.is_valid())
      return false;

    _mh.init( new _Module(mesh()) );
    all_modules_.push_back( _mh.module() );

    set_uninitialized();

    return true;
  }


  /// remove module
  template < typename _Module >
  bool remove( ModHandleT<_Module>& _mh )
  {
    if (!_mh.is_valid())
      return false;

    typename ModuleList::iterator it = std::find(all_modules_.begin(),
                                                 all_modules_.end(),
                                                 _mh.module() );

    if ( it == all_modules_.end() ) // module not found
      return false;

    delete *it;
    all_modules_.erase( it ); // finally remove from list
    _mh.clear();

    set_uninitialized();
    return true;
  }


  /// get module referenced by handle _mh
  template < typename Module >
  Module& module( ModHandleT<Module>& _mh )
  {
    assert( _mh.is_valid() );
    return *_mh.module();
  }


protected:

  // Reset the initialized flag, and clear the bmodules_ and cmodule_
  void set_uninitialized() {
    initialized_ = false;
    cmodule_ = 0;
    bmodules_.clear();
  }

  void update_modules(CollapseInfo& _ci)
  {
    typename ModuleList::iterator m_it, m_end = bmodules_.end();
    for (m_it = bmodules_.begin(); m_it != m_end; ++m_it)
      (*m_it)->postprocess_collapse(_ci);
    cmodule_->postprocess_collapse(_ci);
  }


protected: //---------------------------------------------------- private methods

  /// Is an edge collapse legal?  Performs topological test only.
  /// The method evaluates the status bit Locked, Deleted, and Feature.
  /// \attention The method temporarily sets the bit Tagged. After usage
  ///            the bit will be disabled!
  bool is_collapse_legal(const CollapseInfo& _ci);

  /// Calculate priority of an halfedge collapse (using the modules)
  float collapse_priority(const CollapseInfo& _ci);

  /// Pre-process a collapse
  void preprocess_collapse(CollapseInfo& _ci);

  /// Post-process a collapse
  void postprocess_collapse(CollapseInfo& _ci);

  /**
   * This provides a function that allows the setting of a percentage
   * of the original constraint of the modules
   *
   * Note that some modules might re-initialize in their
   * set_error_tolerance_factor function as necessary
   * @param _factor has to be in the closed interval between 0.0 and 1.0
   */
  void set_error_tolerance_factor(double _factor);

  /** Reset the status of this class
   *
   * You have to call initialize again!!
   */
  void reset(){ initialized_ = false; };


private: //------------------------------------------------------- private data


  // reference to mesh
  Mesh&      mesh_;

  // list of binary modules
  ModuleList bmodules_;

  // the current priority module
  Module*    cmodule_;

  // list of all allocated modules (including cmodule_ and all of bmodules_)
  ModuleList all_modules_;

  // Flag if all modules were initialized
  bool       initialized_;


};

//=============================================================================
} // END_NS_DECIMATER
} // END_NS_OPENMESH
//=============================================================================
#if defined(OM_INCLUDE_TEMPLATES) && !defined(OPENMESH_BASE_DECIMATER_DECIMATERT_CC)
#define OPENMESH_BASE_DECIMATER_TEMPLATES
#include "BaseDecimaterT.cc"
#endif
//=============================================================================
#endif // OPENMESH_BASE_DECIMATER_DECIMATERT_HH defined
//=============================================================================
