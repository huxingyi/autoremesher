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

/** \file Tools/Utils/HeapT.hh
    A generic heap class
**/
/** Martin, 26.12.2004: 
    1) replaced resize(size()-1) with pop_back(), since the later is more efficient
    2) replaced interface_.set_heap_position(entry(0), -1); with reset_heap_position()
    3) added const modifier to various functions
    TODO: in the moment the heap does not conform to the HeapInterface specification, 
          i.e., copies are passed instead of references. This is especially important 
          for set_heap_position(), where the reference argument is non-const. The 
          specification should be changed to reflect that the heap actually (only?)
          works when the heap entry is nothing more but a handle.
    TODO: change the specification of HeapInterface to make less(), greater() and
          get_heap_position() const. Needs changing DecimaterT. Might break 
          someone's code.
*/
    

//=============================================================================
//
//  CLASS HeapT
//
//=============================================================================

#ifndef OPENMESH_UTILS_HEAPT_HH
#define OPENMESH_UTILS_HEAPT_HH


//== INCLUDES =================================================================

#include "Config.hh"
#include <vector>
#include <OpenMesh/Core/System/omstream.hh>

//== NAMESPACE ================================================================

namespace OpenMesh { // BEGIN_NS_OPENMESH
namespace Utils { // BEGIN_NS_UTILS

//== CLASS DEFINITION =========================================================


/** This class demonstrates the HeapInterface's interface.  If you
 *  want to build your customized heap you will have to specify a heap
 *  interface class and use this class as a template parameter for the
 *  class HeapT. This class defines the interface that this heap
 *  interface has to implement.
 *   
 *  \see HeapT
 */
template <class HeapEntry>
struct HeapInterfaceT
{
  /// Comparison of two HeapEntry's: strict less
  bool less(const HeapEntry& _e1, const HeapEntry& _e2);

  /// Comparison of two HeapEntry's: strict greater
  bool greater(const HeapEntry& _e1, const HeapEntry& _e2);

  /// Get the heap position of HeapEntry _e
  int  get_heap_position(const HeapEntry& _e);

  /// Set the heap position of HeapEntry _e
  void set_heap_position(HeapEntry& _e, int _i);
};



/** \class HeapT HeapT.hh <OSG/Utils/HeapT.hh>
 *
 *  An efficient, highly customizable heap.
 *
 *  The main difference (and performance boost) of this heap compared
 *  to e.g. the heap of the STL is that here the positions of the
 *  heap's elements are accessible from the elements themself.
 *  Therefore if one changes the priority of an element one does not
 *  have to remove and re-insert this element, but can just call the
 *  update(HeapEntry) method.
 *
 *  This heap class is parameterized by two template arguments: 
 *  \li the class \c HeapEntry, that will be stored in the heap 
 *  \li the HeapInterface telling the heap how to compare heap entries and
 *      how to store the heap positions in the heap entries.
 *
 *  As an example how to use the class see declaration of class 
 *  Decimater::DecimaterT.
 *
 *  \see HeapInterfaceT
 */
 
template <class HeapEntry, class HeapInterface=HeapEntry>
class HeapT : private std::vector<HeapEntry>
{
private: 
  typedef std::vector<HeapEntry>            Base;
  
public:

  /// Constructor
  HeapT() : HeapVector() {}
  
  /// Construct with a given \c HeapIterface. 
  HeapT(const HeapInterface& _interface)
  : HeapVector(),  interface_(_interface)
  {}
  
  /// Destructor.
  ~HeapT(){};


  /// clear the heap
  void clear() { HeapVector::clear(); }

  /// is heap empty?
  bool empty() const { return HeapVector::empty(); }

  /// returns the size of heap
  size_t size() const { return HeapVector::size(); }

  /// reserve space for _n entries
  void reserve(size_t _n) { HeapVector::reserve(_n); }

  /// reset heap position to -1 (not in heap)
  void reset_heap_position(HeapEntry _h)
  { interface_.set_heap_position(_h, -1); }
  
  /// is an entry in the heap?
  bool is_stored(HeapEntry _h)
  { return interface_.get_heap_position(_h) != -1; }
  
  /// insert the entry _h
  void insert(HeapEntry _h)  
  { 
    this->push_back(_h); 
    upheap(size()-1); 
  }

  /// get the first entry
  HeapEntry front() const
  { 
    assert(!empty()); 
    return entry(0); 
  }

  /// delete the first entry
  void pop_front()
  {    
    assert(!empty());
    reset_heap_position(entry(0));
    if (size() > 1)
    {
      entry(0, entry(size()-1));
      Base::pop_back();
      downheap(0);
    }
    else
    {
      Base::pop_back();
    }
  }

  /// remove an entry
  void remove(HeapEntry _h)
  {
    int pos = interface_.get_heap_position(_h);
    reset_heap_position(_h);

    assert(pos != -1);
    assert((unsigned int) pos < size());
    
    // last item ?
    if ((unsigned int) pos == size()-1)
    {
      Base::pop_back();    
    }
    else 
    {
      entry(pos, entry(size()-1)); // move last elem to pos
      Base::pop_back();
      downheap(pos);
      upheap(pos);
    }
  }

  /** update an entry: change the key and update the position to
      reestablish the heap property.
  */
  void update(HeapEntry _h)
  {
    int pos = interface_.get_heap_position(_h);
    assert(pos != -1);
    assert((unsigned int)pos < size());
    downheap(pos);
    upheap(pos);
  }
  
  /// check heap condition
  bool check()
  {
    bool ok(true);
    unsigned int i, j;
    for (i=0; i<size(); ++i)
    {
      if (((j=left(i))<size()) && interface_.greater(entry(i), entry(j))) 
      {
        omerr() << "Heap condition violated\n";
        ok=false;
      }
      if (((j=right(i))<size()) && interface_.greater(entry(i), entry(j)))
      {
        omerr() << "Heap condition violated\n";
        ok=false;
      }
    }
    return ok;
  }

protected:  
  /// Instance of HeapInterface
  HeapInterface interface_;

private:
  // typedef
  typedef std::vector<HeapEntry> HeapVector;

  
  /// Upheap. Establish heap property.
  void upheap(size_t _idx);

  
  /// Downheap. Establish heap property.
  void downheap(size_t _idx);

  
  /// Get the entry at index _idx
  inline HeapEntry entry(size_t _idx) const
  {
    assert(_idx < size());
    return (Base::operator[](_idx));
  }

  
  /// Set entry _h to index _idx and update _h's heap position.
  inline void entry(size_t _idx, HeapEntry _h)
  {
    assert(_idx < size());
    Base::operator[](_idx) = _h;
    interface_.set_heap_position(_h, int(_idx));
  }

  
  /// Get parent's index
  inline size_t parent(size_t _i) { return (_i-1)>>1; }
  /// Get left child's index
  inline size_t left(size_t _i)   { return (_i<<1)+1; }
  /// Get right child's index
  inline size_t right(size_t _i)  { return (_i<<1)+2; }

};




//== IMPLEMENTATION ========================================================== 


template <class HeapEntry, class HeapInterface>
void
HeapT<HeapEntry, HeapInterface>::
upheap(size_t _idx)
{
  HeapEntry     h = entry(_idx);
  size_t        parentIdx;

  while ((_idx>0) && interface_.less(h, entry(parentIdx=parent(_idx))))
  {
    entry(_idx, entry(parentIdx));
    _idx = parentIdx;    
  }
  
  entry(_idx, h);
}
  

//-----------------------------------------------------------------------------

  
template <class HeapEntry, class HeapInterface>
void
HeapT<HeapEntry, HeapInterface>::
downheap(size_t _idx)
{
  HeapEntry     h = entry(_idx);
  size_t        childIdx;
  size_t        s = size();
  
  while(_idx < s)
  {
    childIdx = left(_idx);
    if (childIdx >= s) break;
    
    if ((childIdx + 1 < s) && (interface_.less(entry(childIdx + 1), entry(childIdx))))
      ++childIdx;
    
    if (interface_.less(h, entry(childIdx))) break;

    entry(_idx, entry(childIdx));
    _idx = childIdx;
  }  

  entry(_idx, h);
}


//=============================================================================
} // END_NS_UTILS
} // END_NS_OPENMESH
//=============================================================================
#endif // OSG_HEAP_HH defined
//=============================================================================

