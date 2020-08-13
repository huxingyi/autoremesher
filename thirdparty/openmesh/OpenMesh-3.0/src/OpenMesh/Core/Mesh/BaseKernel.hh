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


//=============================================================================
//
//  CLASS BaseKernel
//
//=============================================================================


#ifndef OPENMESH_BASE_KERNEL_HH
#define OPENMESH_BASE_KERNEL_HH


//== INCLUDES =================================================================


#include <OpenMesh/Core/System/config.h>
// --------------------
#include <vector>
#include <string>
#include <algorithm>
// --------------------
#include <OpenMesh/Core/Utils/PropertyContainer.hh>


//== NAMESPACES ===============================================================


namespace OpenMesh {


//== CLASS DEFINITION =========================================================

/// This class provides the basic property management like adding/removing
/// properties and access to properties.
/// All operations provided by %BaseKernel need at least a property handle
/// (VPropHandleT, EPropHandleT, HPropHandleT, FPropHandleT, MPropHandleT).
/// which keeps the data type of the property, too.
///
/// There are two types of properties:
/// -# Standard properties - mesh data (e.g. vertex normal or face color)
/// -# Custom properties - user defined data
///
/// The differentiation is only semantically, technically both are
/// equally handled. Therefore the methods provided by the %BaseKernel
/// are applicable to both property types.
///
/// \attention Since the class PolyMeshT derives from a kernel, hence all public
/// elements of %BaseKernel are usable.

class OPENMESHDLLEXPORT BaseKernel
{
public: //-------------------------------------------- constructor / destructor

  BaseKernel() {}
  virtual ~BaseKernel() {
	vprops_.clear();
	eprops_.clear();
	hprops_.clear();
	fprops_.clear();
  }


public: //-------------------------------------------------- add new properties

  /// \name Add a property to a mesh item

  //@{

  /** Adds a property
   *
   *  Depending on the property handle type a vertex, (half-)edge, face or
   *  mesh property is added to the mesh. If the action fails the handle
   *  is invalid.
   *  On success the handle must be used to access the property data with
   *  property().
   *
   *  \param  _ph   A property handle defining the data type to bind to mesh.
   *                On success the handle is valid else invalid.
   *  \param  _name Optional name of property. Following restrictions apply
   *                to the name:
   *                -# Maximum length of name is 256 characters
   *                -# The prefixes matching "^[vhefm]:" are reserved for
   *                   internal usage.
   *                -# The expression "^<.*>$" is reserved for internal usage.
   *
   */

  template <class T>
  void add_property( VPropHandleT<T>& _ph, const std::string& _name="<vprop>")
  {
    _ph = VPropHandleT<T>( vprops_.add(T(), _name) );
    vprops_.resize(n_vertices());
  }

  template <class T>
  void add_property( HPropHandleT<T>& _ph, const std::string& _name="<hprop>")
  {
    _ph = HPropHandleT<T>( hprops_.add(T(), _name) );
    hprops_.resize(n_halfedges());
  }

  template <class T>
  void add_property( EPropHandleT<T>& _ph, const std::string& _name="<eprop>")
  {
    _ph = EPropHandleT<T>( eprops_.add(T(), _name) );
    eprops_.resize(n_edges());
  }

  template <class T>
  void add_property( FPropHandleT<T>& _ph, const std::string& _name="<fprop>")
  {
    _ph = FPropHandleT<T>( fprops_.add(T(), _name) );
    fprops_.resize(n_faces());
  }

  template <class T>
  void add_property( MPropHandleT<T>& _ph, const std::string& _name="<mprop>")
  {
    _ph = MPropHandleT<T>( mprops_.add(T(), _name) );
    mprops_.resize(1);
  }

  //@}


public: //--------------------------------------------------- remove properties

  /// \name Removing a property from a mesh tiem
  //@{

  /** Remove a property.
   *
   *  Removes the property represented by the handle from the apropriate
   *  mesh item.
   *  \param _ph Property to be removed. The handle is invalid afterwords.
   */

  template <typename T>
  void remove_property(VPropHandleT<T>& _ph)
  {
    if (_ph.is_valid())
      vprops_.remove(_ph);
    _ph.reset();
  }

  template <typename T>
  void remove_property(HPropHandleT<T>& _ph)
  {
    if (_ph.is_valid())
      hprops_.remove(_ph);
    _ph.reset();
  }

  template <typename T>
  void remove_property(EPropHandleT<T>& _ph)
  {
    if (_ph.is_valid())
      eprops_.remove(_ph);
    _ph.reset();
  }

  template <typename T>
  void remove_property(FPropHandleT<T>& _ph)
  {
    if (_ph.is_valid())
      fprops_.remove(_ph);
    _ph.reset();
  }

  template <typename T>
  void remove_property(MPropHandleT<T>& _ph)
  {
    if (_ph.is_valid())
      mprops_.remove(_ph);
    _ph.reset();
  }

  //@}
  
public: //------------------------------------------------ get handle from name

  /// \name Get property handle by name
  //@{

  /** Retrieves the handle to a named property by it's name.
   *
   *  \param _ph    A property handle. On success the handle is valid else
   *                invalid.
   *  \param _name  Name of wanted property.
   *  \return \c true if such a named property is available, else \c false.
   */

  template <class T>
  bool get_property_handle(VPropHandleT<T>& _ph,
         const std::string& _name) const
  {
    return (_ph = VPropHandleT<T>(vprops_.handle(T(), _name))).is_valid();
  }

  template <class T>
  bool get_property_handle(HPropHandleT<T>& _ph,
         const std::string& _name) const
  {
    return (_ph = HPropHandleT<T>(hprops_.handle(T(), _name))).is_valid();
  }

  template <class T>
  bool get_property_handle(EPropHandleT<T>& _ph,
         const std::string& _name) const
  {
    return (_ph = EPropHandleT<T>(eprops_.handle(T(), _name))).is_valid();
  }

  template <class T>
  bool get_property_handle(FPropHandleT<T>& _ph,
         const std::string& _name) const
  {
    return (_ph = FPropHandleT<T>(fprops_.handle(T(), _name))).is_valid();
  }

  template <class T>
  bool get_property_handle(MPropHandleT<T>& _ph,
         const std::string& _name) const
  {
    return (_ph = MPropHandleT<T>(mprops_.handle(T(), _name))).is_valid();
  }

  //@}

public: //--------------------------------------------------- access properties

  /// \name Access a property
  //@{

  /** Access a property
   *
   *  This method returns a reference to property. The property handle
   *  must be valid! The result is unpredictable if the handle is invalid!
   *
   *  \param  _ph     A \em valid (!) property handle.
   *  \return The wanted property if the handle is valid.
   */

  template <class T>
  PropertyT<T>& property(VPropHandleT<T> _ph) {
    return vprops_.property(_ph);
  }
  template <class T>
  const PropertyT<T>& property(VPropHandleT<T> _ph) const {
    return vprops_.property(_ph);
  }

  template <class T>
  PropertyT<T>& property(HPropHandleT<T> _ph) {
    return hprops_.property(_ph);
  }
  template <class T>
  const PropertyT<T>& property(HPropHandleT<T> _ph) const {
    return hprops_.property(_ph);
  }

  template <class T>
  PropertyT<T>& property(EPropHandleT<T> _ph) {
    return eprops_.property(_ph);
  }
  template <class T>
  const PropertyT<T>& property(EPropHandleT<T> _ph) const {
    return eprops_.property(_ph);
  }

  template <class T>
  PropertyT<T>& property(FPropHandleT<T> _ph) {
    return fprops_.property(_ph);
  }
  template <class T>
  const PropertyT<T>& property(FPropHandleT<T> _ph) const {
    return fprops_.property(_ph);
  }

  template <class T>
  PropertyT<T>& mproperty(MPropHandleT<T> _ph) {
    return mprops_.property(_ph);
  }
  template <class T>
  const PropertyT<T>& mproperty(MPropHandleT<T> _ph) const {
    return mprops_.property(_ph);
  }

  //@}

public: //-------------------------------------------- access property elements

  /// \name Access a property element using a handle to a mesh item
  //@{

  /** Return value of property for an item
   */

  template <class T>
  typename VPropHandleT<T>::reference
  property(VPropHandleT<T> _ph, VertexHandle _vh) {
    return vprops_.property(_ph)[_vh.idx()];
  }

  template <class T>
  typename VPropHandleT<T>::const_reference
  property(VPropHandleT<T> _ph, VertexHandle _vh) const {
    return vprops_.property(_ph)[_vh.idx()];
  }


  template <class T>
  typename HPropHandleT<T>::reference
  property(HPropHandleT<T> _ph, HalfedgeHandle _hh) {
    return hprops_.property(_ph)[_hh.idx()];
  }

  template <class T>
  typename HPropHandleT<T>::const_reference
  property(HPropHandleT<T> _ph, HalfedgeHandle _hh) const {
    return hprops_.property(_ph)[_hh.idx()];
  }


  template <class T>
  typename EPropHandleT<T>::reference
  property(EPropHandleT<T> _ph, EdgeHandle _eh) {
    return eprops_.property(_ph)[_eh.idx()];
  }

  template <class T>
  typename EPropHandleT<T>::const_reference
  property(EPropHandleT<T> _ph, EdgeHandle _eh) const {
    return eprops_.property(_ph)[_eh.idx()];
  }


  template <class T>
  typename FPropHandleT<T>::reference
  property(FPropHandleT<T> _ph, FaceHandle _fh) {
    return fprops_.property(_ph)[_fh.idx()];
  }

  template <class T>
  typename FPropHandleT<T>::const_reference
  property(FPropHandleT<T> _ph, FaceHandle _fh) const {
    return fprops_.property(_ph)[_fh.idx()];
  }


  template <class T>
  typename MPropHandleT<T>::reference
  property(MPropHandleT<T> _ph) {
    return mprops_.property(_ph)[0];
  }

  template <class T>
  typename MPropHandleT<T>::const_reference
  property(MPropHandleT<T> _ph) const {
    return mprops_.property(_ph)[0];
  }

  //@}


public: //------------------------------------------------ copy property

  /** Copies a single property from one mesh element to another (of the same type)
   *
   * @param _ph       A vertex property handle
   * @param _vh_from  From vertex handle
   * @param _vh_to    To vertex handle
   */
  template <class T>
  void copy_property(VPropHandleT<T>& _ph, VertexHandle _vh_from, VertexHandle _vh_to) {
    if(_vh_from.is_valid() && _vh_to.is_valid())
      vprops_.property(_ph)[_vh_to.idx()] = vprops_.property(_ph)[_vh_from.idx()];
  }

  /** Copies a single property from one mesh element to another (of the same type)
    *
    * @param _ph       A halfedge property handle
    * @param _hh_from  From halfedge handle
    * @param _hh_to    To halfedge handle
    */
  template <class T>
  void copy_property(HPropHandleT<T> _ph, HalfedgeHandle _hh_from, HalfedgeHandle _hh_to) {
    if(_hh_from.is_valid() && _hh_to.is_valid())
      hprops_.property(_ph)[_hh_to.idx()] = hprops_.property(_ph)[_hh_from.idx()];
  }

  /** Copies a single property from one mesh element to another (of the same type)
    *
    * @param _ph       An edge property handle
    * @param _eh_from  From edge handle
    * @param _eh_to    To edge handle
    */
  template <class T>
  void copy_property(EPropHandleT<T> _ph, EdgeHandle _eh_from, EdgeHandle _eh_to) {
    if(_eh_from.is_valid() && _eh_to.is_valid())
      eprops_.property(_ph)[_eh_to.idx()] = eprops_.property(_ph)[_eh_from.idx()];
  }

  /** Copies a single property from one mesh element to another (of the same type)
    *
    * @param _ph       A face property handle
    * @param _fh_from  From face handle
    * @param _fh_to    To face handle
    */
  template <class T>
  void copy_property(FPropHandleT<T> _ph, FaceHandle _fh_from, FaceHandle _fh_to) {
    if(_fh_from.is_valid() && _fh_to.is_valid())
      fprops_.property(_ph)[_fh_to.idx()] = fprops_.property(_ph)[_fh_from.idx()];
  }


public:
  //------------------------------------------------ copy all properties

  /** Copies all properties from one mesh element to another (of the same type)
   *
   *
   * @param _vh_from A vertex handle - source
   * @param _vh_to   A vertex handle - target
   * @param _copyBuildIn Should the internal properties (position, normal, texture coordinate,..) be copied?
   */
  void copy_all_properties(VertexHandle _vh_from, VertexHandle _vh_to, bool _copyBuildIn = false) {

    for( PropertyContainer::iterator p_it = vprops_.begin();
        p_it != vprops_.end(); ++p_it) {

      // Copy all properties, if build in is true
      // Otherwise, copy only properties without build in specifier
      if ( *p_it && ( _copyBuildIn || (*p_it)->name().substr(0,2) != "v:" ) )
        (*p_it)->copy(_vh_from.idx(), _vh_to.idx());

    }
  }

  /** Copies all properties from one mesh element to another (of the same type)
   *
   * @param _hh_from A halfedge handle - source
   * @param _hh_to   A halfedge handle - target
   * @param _copyBuildIn Should the internal properties (position, normal, texture coordinate,..) be copied?
   */
  void copy_all_properties(HalfedgeHandle _hh_from, HalfedgeHandle _hh_to, bool _copyBuildIn = false) {

    for( PropertyContainer::iterator p_it = hprops_.begin();
        p_it != hprops_.end(); ++p_it) {

      // Copy all properties, if build in is true
      // Otherwise, copy only properties without build in specifier
      if ( *p_it && ( _copyBuildIn || (*p_it)->name().substr(0,2) != "h:") )
        (*p_it)->copy(_hh_from.idx(), _hh_to.idx());

    }
  }

  /** Copies all properties from one mesh element to another (of the same type)
   *
   * @param _eh_from An edge handle - source
   * @param _eh_to   An edge handle - target
   * @param _copyBuildIn Should the internal properties (position, normal, texture coordinate,..) be copied?
   */
  void copy_all_properties(EdgeHandle _eh_from, EdgeHandle _eh_to, bool _copyBuildIn = false) {
    for( PropertyContainer::iterator p_it = eprops_.begin();
        p_it != eprops_.end(); ++p_it) {

      // Copy all properties, if build in is true
      // Otherwise, copy only properties without build in specifier
      if ( *p_it && ( _copyBuildIn || (*p_it)->name().substr(0,2) != "e:") )
        (*p_it)->copy(_eh_from.idx(), _eh_to.idx());

    }
  }

  /** Copies all properties from one mesh element to another (of the same type)
    *
    * @param _fh_from A face handle - source
    * @param _fh_to   A face handle - target
    * @param _copyBuildIn Should the internal properties (position, normal, texture coordinate,..) be copied?
    *
    */
  void copy_all_properties(FaceHandle _fh_from, FaceHandle _fh_to, bool _copyBuildIn = false) {

    for( PropertyContainer::iterator p_it = fprops_.begin();
        p_it != fprops_.end(); ++p_it) {

      // Copy all properties, if build in is true
      // Otherwise, copy only properties without build in specifier
      if ( *p_it && ( _copyBuildIn || (*p_it)->name().substr(0,2) != "f:") )
        (*p_it)->copy(_fh_from.idx(), _fh_to.idx());
    }

  }

protected: //------------------------------------------------- low-level access

public: // used by non-native kernel and MeshIO, should be protected

  size_t n_vprops(void) const { return vprops_.size(); }

  size_t n_eprops(void) const { return eprops_.size(); }

  size_t n_hprops(void) const { return hprops_.size(); }

  size_t n_fprops(void) const { return fprops_.size(); }

  size_t n_mprops(void) const { return mprops_.size(); }

  BaseProperty* _get_vprop( const std::string& _name)
  { return vprops_.property(_name); }

  BaseProperty* _get_eprop( const std::string& _name)
  { return eprops_.property(_name); }

  BaseProperty* _get_hprop( const std::string& _name)
  { return hprops_.property(_name); }

  BaseProperty* _get_fprop( const std::string& _name)
  { return fprops_.property(_name); }

  BaseProperty* _get_mprop( const std::string& _name)
  { return mprops_.property(_name); }

  const BaseProperty* _get_vprop( const std::string& _name) const
  { return vprops_.property(_name); }

  const BaseProperty* _get_eprop( const std::string& _name) const
  { return eprops_.property(_name); }

  const BaseProperty* _get_hprop( const std::string& _name) const
  { return hprops_.property(_name); }

  const BaseProperty* _get_fprop( const std::string& _name) const
  { return fprops_.property(_name); }

  const BaseProperty* _get_mprop( const std::string& _name) const
  { return mprops_.property(_name); }

  BaseProperty& _vprop( size_t _idx ) { return vprops_._property( _idx ); }
  BaseProperty& _eprop( size_t _idx ) { return eprops_._property( _idx ); }
  BaseProperty& _hprop( size_t _idx ) { return hprops_._property( _idx ); }
  BaseProperty& _fprop( size_t _idx ) { return fprops_._property( _idx ); }
  BaseProperty& _mprop( size_t _idx ) { return mprops_._property( _idx ); }

  const BaseProperty& _vprop( size_t _idx ) const
  { return vprops_._property( _idx ); }
  const BaseProperty& _eprop( size_t _idx ) const
  { return eprops_._property( _idx ); }
  const BaseProperty& _hprop( size_t _idx ) const
  { return hprops_._property( _idx ); }
  const BaseProperty& _fprop( size_t _idx ) const
  { return fprops_._property( _idx ); }
  const BaseProperty& _mprop( size_t _idx ) const
  { return mprops_._property( _idx ); }

  size_t _add_vprop( BaseProperty* _bp ) { return vprops_._add( _bp ); }
  size_t _add_eprop( BaseProperty* _bp ) { return eprops_._add( _bp ); }
  size_t _add_hprop( BaseProperty* _bp ) { return hprops_._add( _bp ); }
  size_t _add_fprop( BaseProperty* _bp ) { return fprops_._add( _bp ); }
  size_t _add_mprop( BaseProperty* _bp ) { return mprops_._add( _bp ); }

protected: // low-level access non-public

  BaseProperty& _vprop( BaseHandle _h )
  { return vprops_._property( _h.idx() ); }
  BaseProperty& _eprop( BaseHandle _h )
  { return eprops_._property( _h.idx() ); }
  BaseProperty& _hprop( BaseHandle _h )
  { return hprops_._property( _h.idx() ); }
  BaseProperty& _fprop( BaseHandle _h )
  { return fprops_._property( _h.idx() ); }
  BaseProperty& _mprop( BaseHandle _h )
  { return mprops_._property( _h.idx() ); }

  const BaseProperty& _vprop( BaseHandle _h ) const
  { return vprops_._property( _h.idx() ); }
  const BaseProperty& _eprop( BaseHandle _h ) const
  { return eprops_._property( _h.idx() ); }
  const BaseProperty& _hprop( BaseHandle _h ) const
  { return hprops_._property( _h.idx() ); }
  const BaseProperty& _fprop( BaseHandle _h ) const
  { return fprops_._property( _h.idx() ); }
  const BaseProperty& _mprop( BaseHandle _h ) const
  { return mprops_._property( _h.idx() ); }


public: //----------------------------------------------------- element numbers


  virtual size_t n_vertices()  const { return 0; }
  virtual size_t n_halfedges() const { return 0; }
  virtual size_t n_edges()     const { return 0; }
  virtual size_t n_faces()     const { return 0; }


protected: //------------------------------------------- synchronize properties

  void vprops_reserve(size_t _n) const { vprops_.reserve(_n); }
  void vprops_resize(size_t _n) const { vprops_.resize(_n); }
  void vprops_clear() {
    vprops_.clear();
  }
  void vprops_swap(unsigned int _i0, unsigned int _i1) const {
    vprops_.swap(_i0, _i1);
  }

  void hprops_reserve(size_t _n) const { hprops_.reserve(_n); }
  void hprops_resize(size_t _n) const { hprops_.resize(_n); }
  void hprops_clear() {
    hprops_.clear();
  }
  void hprops_swap(unsigned int _i0, unsigned int _i1) const {
    hprops_.swap(_i0, _i1);
  }

  void eprops_reserve(size_t _n) const { eprops_.reserve(_n); }
  void eprops_resize(size_t _n) const { eprops_.resize(_n); }
  void eprops_clear() {
    eprops_.clear();
  }
  void eprops_swap(unsigned int _i0, unsigned int _i1) const {
    eprops_.swap(_i0, _i1);
  }

  void fprops_reserve(size_t _n) const { fprops_.reserve(_n); }
  void fprops_resize(size_t _n) const { fprops_.resize(_n); }
  void fprops_clear() {
    fprops_.clear();
  }
  void fprops_swap(unsigned int _i0, unsigned int _i1) const {
    fprops_.swap(_i0, _i1);
  }

  void mprops_resize(size_t _n) const { mprops_.resize(_n); }
  void mprops_clear() {
    mprops_.clear();
  }

public:

  void property_stats(std::ostream& _ostr = std::clog) const;

  void vprop_stats( std::string& _string ) const;
  void hprop_stats( std::string& _string ) const;
  void eprop_stats( std::string& _string ) const;
  void fprop_stats( std::string& _string ) const;
  void mprop_stats( std::string& _string ) const;

  void vprop_stats(std::ostream& _ostr = std::clog) const;
  void hprop_stats(std::ostream& _ostr = std::clog) const;
  void eprop_stats(std::ostream& _ostr = std::clog) const;
  void fprop_stats(std::ostream& _ostr = std::clog) const;
  void mprop_stats(std::ostream& _ostr = std::clog) const;

public:

  typedef PropertyContainer::iterator prop_iterator;
  typedef PropertyContainer::const_iterator const_prop_iterator;

  prop_iterator vprops_begin() { return vprops_.begin(); }
  prop_iterator vprops_end()   { return vprops_.end(); }
  const_prop_iterator vprops_begin() const { return vprops_.begin(); }
  const_prop_iterator vprops_end()   const { return vprops_.end(); }

  prop_iterator eprops_begin() { return eprops_.begin(); }
  prop_iterator eprops_end()   { return eprops_.end(); }
  const_prop_iterator eprops_begin() const { return eprops_.begin(); }
  const_prop_iterator eprops_end()   const { return eprops_.end(); }

  prop_iterator hprops_begin() { return hprops_.begin(); }
  prop_iterator hprops_end()   { return hprops_.end(); }
  const_prop_iterator hprops_begin() const { return hprops_.begin(); }
  const_prop_iterator hprops_end()   const { return hprops_.end(); }

  prop_iterator fprops_begin() { return fprops_.begin(); }
  prop_iterator fprops_end()   { return fprops_.end(); }
  const_prop_iterator fprops_begin() const { return fprops_.begin(); }
  const_prop_iterator fprops_end()   const { return fprops_.end(); }

  prop_iterator mprops_begin() { return mprops_.begin(); }
  prop_iterator mprops_end()   { return mprops_.end(); }
  const_prop_iterator mprops_begin() const { return mprops_.begin(); }
  const_prop_iterator mprops_end()   const { return mprops_.end(); }

private:

  PropertyContainer  vprops_;
  PropertyContainer  hprops_;
  PropertyContainer  eprops_;
  PropertyContainer  fprops_;
  PropertyContainer  mprops_;
};


//=============================================================================
} // namespace OpenMesh
//=============================================================================
#endif // OPENMESH_BASE_KERNEL_HH defined
//=============================================================================
