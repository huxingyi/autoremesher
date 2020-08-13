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

#ifndef OPENMESH_PROPERTYCONTAINER
#define OPENMESH_PROPERTYCONTAINER

// Use static casts when not debugging
#ifdef NDEBUG
#define OM_FORCE_STATIC_CAST
#endif

#include <OpenMesh/Core/Utils/Property.hh>

//-----------------------------------------------------------------------------
namespace OpenMesh
{
//== FORWARDDECLARATIONS ======================================================
  class BaseKernel;

//== CLASS DEFINITION =========================================================
/// A a container for properties.
class PropertyContainer
{
public:

  //-------------------------------------------------- constructor / destructor

  PropertyContainer() {}
  virtual ~PropertyContainer() { std::for_each(properties_.begin(), properties_.end(), Delete()); }


  //------------------------------------------------------------- info / access

  typedef std::vector<BaseProperty*> Properties;
  const Properties& properties() const { return properties_; }
  size_t size() const { return properties_.size(); }



  //--------------------------------------------------------- copy / assignment

  PropertyContainer(const PropertyContainer& _rhs) { operator=(_rhs); }

  PropertyContainer& operator=(const PropertyContainer& _rhs)
  {
    // The assignment below relies on all previous BaseProperty* elements having been deleted
    std::for_each(properties_.begin(), properties_.end(), Delete());
    properties_ = _rhs.properties_;
    Properties::iterator p_it=properties_.begin(), p_end=properties_.end();
    for (; p_it!=p_end; ++p_it)
      if (*p_it)
        *p_it = (*p_it)->clone();
    return *this;
  }



  //--------------------------------------------------------- manage properties

  template <class T>
  BasePropHandleT<T> add(const T&, const std::string& _name="<unknown>")
  {
    Properties::iterator p_it=properties_.begin(), p_end=properties_.end();
    int idx=0;
    for ( ; p_it!=p_end && *p_it!=NULL; ++p_it, ++idx ) {};
    if (p_it==p_end) properties_.push_back(NULL);
    properties_[idx] = new PropertyT<T>(_name);
    return BasePropHandleT<T>(idx);
  }


  template <class T>
  BasePropHandleT<T> handle(const T&, const std::string& _name) const
  {
    Properties::const_iterator p_it = properties_.begin();
    for (int idx=0; p_it != properties_.end(); ++p_it, ++idx)
    {
      if (*p_it != NULL &&
         (*p_it)->name() == _name  //skip deleted properties
// Skip type check
#ifndef OM_FORCE_STATIC_CAST
          && dynamic_cast<PropertyT<T>*>(properties_[idx]) != NULL //check type
#endif
         )
      {
        return BasePropHandleT<T>(idx);
      }
    }
    return BasePropHandleT<T>();
  }

  BaseProperty* property( const std::string& _name ) const
  {
    Properties::const_iterator p_it = properties_.begin();
    for (int idx=0; p_it != properties_.end(); ++p_it, ++idx)
    {
      if (*p_it != NULL && (*p_it)->name() == _name) //skip deleted properties
      {
        return *p_it;
      }
    }
    return NULL;
  }

  template <class T> PropertyT<T>& property(BasePropHandleT<T> _h)
  {
    assert(_h.idx() >= 0 && _h.idx() < (int)properties_.size());
    assert(properties_[_h.idx()] != NULL);
#ifdef OM_FORCE_STATIC_CAST
    return *static_cast  <PropertyT<T>*> (properties_[_h.idx()]);
#else
    PropertyT<T>* p = dynamic_cast<PropertyT<T>*>(properties_[_h.idx()]);
    assert(p != NULL);
    return *p;
#endif
  }


  template <class T> const PropertyT<T>& property(BasePropHandleT<T> _h) const
  {
    assert(_h.idx() >= 0 && _h.idx() < (int)properties_.size());
    assert(properties_[_h.idx()] != NULL);
#ifdef OM_FORCE_STATIC_CAST
    return *static_cast<PropertyT<T>*>(properties_[_h.idx()]);
#else
    PropertyT<T>* p = dynamic_cast<PropertyT<T>*>(properties_[_h.idx()]);
    assert(p != NULL);
    return *p;
#endif
  }


  template <class T> void remove(BasePropHandleT<T> _h)
  {
    assert(_h.idx() >= 0 && _h.idx() < (int)properties_.size());
    delete properties_[_h.idx()];
    properties_[_h.idx()] = NULL;
  }


  void clear()
  {
	// Clear properties vector:
	// Replaced the old version with new one
	// which performs a swap to clear values and
	// deallocate memory.

	// Old version (changed 22.07.09) {
	// std::for_each(properties_.begin(), properties_.end(), Delete());
    // }

	std::for_each(properties_.begin(), properties_.end(), ClearAll());
  }


  //---------------------------------------------------- synchronize properties

  void reserve(size_t _n) const {
    std::for_each(properties_.begin(), properties_.end(), Reserve(_n));
  }

  void resize(size_t _n) const {
    std::for_each(properties_.begin(), properties_.end(), Resize(_n));
  }

  void swap(size_t _i0, size_t _i1) const {
    std::for_each(properties_.begin(), properties_.end(), Swap(_i0, _i1));
  }



protected: // generic add/get

  size_t _add( BaseProperty* _bp )
  {
    Properties::iterator p_it=properties_.begin(), p_end=properties_.end();
    size_t idx=0;
    for (; p_it!=p_end && *p_it!=NULL; ++p_it, ++idx) {};
    if (p_it==p_end) properties_.push_back(NULL);
    properties_[idx] = _bp;
    return idx;
  }

  BaseProperty& _property( size_t _idx )
  {
    assert( _idx < properties_.size());
    assert( properties_[_idx] != NULL);
    BaseProperty *p = properties_[_idx];
    assert( p != NULL );
    return *p;
  }

  const BaseProperty& _property( size_t _idx ) const
  {
    assert( _idx < properties_.size());
    assert( properties_[_idx] != NULL);
    BaseProperty *p = properties_[_idx];
    assert( p != NULL );
    return *p;
  }


  typedef Properties::iterator       iterator;
  typedef Properties::const_iterator const_iterator;
  iterator begin() { return properties_.begin(); }
  iterator end()   { return properties_.end(); }
  const_iterator begin() const { return properties_.begin(); }
  const_iterator end() const   { return properties_.end(); }

  friend class BaseKernel;

private:

  //-------------------------------------------------- synchronization functors

#ifndef DOXY_IGNORE_THIS
  struct Reserve
  {
    Reserve(size_t _n) : n_(_n) {}
    void operator()(BaseProperty* _p) const { if (_p) _p->reserve(n_); }
    size_t n_;
  };

  struct Resize
  {
    Resize(size_t _n) : n_(_n) {}
    void operator()(BaseProperty* _p) const { if (_p) _p->resize(n_); }
    size_t n_;
  };

  struct ClearAll
  {
    ClearAll() {}
    void operator()(BaseProperty* _p) const { if (_p) _p->clear(); }
  };

  struct Swap
  {
    Swap(size_t _i0, size_t _i1) : i0_(_i0), i1_(_i1) {}
    void operator()(BaseProperty* _p) const { if (_p) _p->swap(i0_, i1_); }
    size_t i0_, i1_;
  };

  struct Delete
  {
    Delete() {}
    void operator()(BaseProperty* _p) const { if (_p) delete _p; _p=NULL; }
  };
#endif

  Properties   properties_;
};

}//namespace OpenMesh

#endif//OPENMESH_PROPERTYCONTAINER

