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

#ifndef TIMER_HH
#define TIMER_HH
// ----------------------------------------------------------------------------

/** \file Timer.hh
    A timer class
*/


// ----------------------------------------------------------------------------

#include <OpenMesh/Core/System/config.hh>
//
#include <iostream>
#include <string>
#if defined(OM_CC_MIPS)
#  include <assert.h>
#else
#  include <cassert>
#endif


// ------------------------------------------------------------- namespace ----

namespace OpenMesh {
namespace Utils {


// -------------------------------------------------------------- forwards ----


class TimerImpl;


// ----------------------------------------------------------------- class ----

/** Timer class
 */
class OPENMESHDLLEXPORT Timer
{
public:

  /// Formatting options for member Timer::as_string()
  enum Format {
    Automatic,
    Long,
    Hours,
    Minutes,
    Seconds,
    HSeconds,
    MSeconds,
    MicroSeconds,
    NanoSeconds
  };

  Timer(void);
  ~Timer(void);

  /// Returns true if self is in a valid state!
  bool is_valid() const { return state_!=Invalid; }

  bool is_stopped() const { return state_==Stopped; }

  /// Reset the timer
  void reset(void);

  /// Start measurement
  void start(void);

  /// Stop measurement
  void stop(void);

  /// Continue measurement
  void cont(void);

  /// Give resolution of timer. Depends on the underlying measurement method.
  float resolution() const;
    
  /// Returns measured time in seconds, if the timer is in state 'Stopped'
  double seconds(void) const;

  /// Returns measured time in hundredth seconds, if the timer is in state 'Stopped'
  double hseconds(void) const { return seconds()*1e2; }

  /// Returns measured time in milli seconds, if the timer is in state 'Stopped'
  double mseconds(void) const { return seconds()*1e3; }

  /// Returns measured time in micro seconds, if the timer is in state 'Stopped'
  double useconds(void) const { return seconds()*1e6; }
  
  /** Returns the measured time as a string. Use the format flags to specify
      a wanted resolution.
   */
  std::string as_string(Format format = Automatic);
  
  /** Returns a given measured time as a string. Use the format flags to 
      specify a wanted resolution.
   */
  static std::string as_string(double seconds, Format format = Automatic);

public:

  //@{
  /// Compare timer values
  bool operator < (const Timer& t2) const 
  { 
    assert( is_stopped() && t2.is_stopped() ); 
    return (seconds() < t2.seconds()); 
  }

  bool operator > (const Timer& t2) const
  { 
    assert( is_stopped() && t2.is_stopped() ); 
    return (seconds() > t2.seconds()); 
  }

  bool operator == (const Timer& t2) const
  { 
    assert( is_stopped() && t2.is_stopped() ); 
    return (seconds() == t2.seconds()); 
  }

  bool operator <= (const Timer& t2) const
  {
    assert( is_stopped() && t2.is_stopped() ); 
    return (seconds() <= t2.seconds()); 
  }

  bool operator >=(const Timer& t2) const
  { 
    assert( is_stopped() && t2.is_stopped() ); 
    return (seconds() >= t2.seconds()); 
  }
  //@}

protected:

  TimerImpl *impl_;

  enum {
    Invalid = -1,
    Stopped =  0,
    Running =  1
  } state_;

};


/** Write seconds to output stream. 
 *  Timer must be stopped before.
 *  \relates Timer
 */
inline std::ostream& operator << (std::ostream& _o, const Timer& _t)
{
   return (_o << _t.seconds());
}


// ============================================================================
} // END_NS_UTILS
} // END_NS_OPENMESH
// ============================================================================
#endif
// end of Timer.hh
// ===========================================================================

