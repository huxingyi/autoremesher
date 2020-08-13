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

#include <OpenMesh/Core/Mesh/BaseKernel.hh>
namespace OpenMesh
{

void BaseKernel::property_stats(std::ostream& _ostr) const
{
  const PropertyContainer::Properties& vps = vprops_.properties();
  const PropertyContainer::Properties& hps = hprops_.properties();
  const PropertyContainer::Properties& eps = eprops_.properties();
  const PropertyContainer::Properties& fps = fprops_.properties();
  const PropertyContainer::Properties& mps = mprops_.properties();

  PropertyContainer::Properties::const_iterator it;

  _ostr << vprops_.size() << " vprops:\n";
  for (it=vps.begin(); it!=vps.end(); ++it)
  {
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);
  }
  _ostr << hprops_.size() << " hprops:\n";
  for (it=hps.begin(); it!=hps.end(); ++it)
  {
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);
  }
  _ostr << eprops_.size() << " eprops:\n";
  for (it=eps.begin(); it!=eps.end(); ++it)
  {
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);
  }
  _ostr << fprops_.size() << " fprops:\n";
  for (it=fps.begin(); it!=fps.end(); ++it)
  {
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);
  }
  _ostr << mprops_.size() << " mprops:\n";
  for (it=mps.begin(); it!=mps.end(); ++it)
  {
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);
  }
}



void BaseKernel::vprop_stats( std::string& _string ) const
{
  _string.clear();

  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& vps = vprops_.properties();
  for (it=vps.begin(); it!=vps.end(); ++it)
    if ( *it == NULL )
      _string += "[deleted] \n";
    else {
      _string += (*it)->name();
      _string += "\n";
    }

}

void BaseKernel::hprop_stats( std::string& _string ) const
{
  _string.clear();

  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& hps = hprops_.properties();
  for (it=hps.begin(); it!=hps.end(); ++it)
    if ( *it == NULL )
      _string += "[deleted] \n";
    else {
      _string += (*it)->name();
      _string += "\n";
    }

}

void BaseKernel::eprop_stats( std::string& _string ) const
{
  _string.clear();

  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& eps = eprops_.properties();
  for (it=eps.begin(); it!=eps.end(); ++it)
    if ( *it == NULL )
      _string += "[deleted] \n";
    else {
      _string += (*it)->name();
      _string += "\n";
    }

}
void BaseKernel::fprop_stats( std::string& _string ) const
{
  _string.clear();

  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& fps = fprops_.properties();
  for (it=fps.begin(); it!=fps.end(); ++it)
    if ( *it == NULL )
      _string += "[deleted] \n";
    else {
      _string += (*it)->name();
      _string += "\n";
    }

}

void BaseKernel::mprop_stats( std::string& _string ) const
{
  _string.clear();

  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& mps = mprops_.properties();
  for (it=mps.begin(); it!=mps.end(); ++it)
    if ( *it == NULL )
      _string += "[deleted] \n";
    else {
      _string += (*it)->name();
      _string += "\n";
    }

}

void BaseKernel::vprop_stats(std::ostream& _ostr ) const
{
  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& vps = vprops_.properties();
  for (it=vps.begin(); it!=vps.end(); ++it)
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);

}

void BaseKernel::hprop_stats(std::ostream& _ostr ) const
{
  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& hps = hprops_.properties();
  for (it=hps.begin(); it!=hps.end(); ++it)
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);

}
void BaseKernel::eprop_stats(std::ostream& _ostr ) const
{
  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& eps = eprops_.properties();
  for (it=eps.begin(); it!=eps.end(); ++it)
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);

}
void BaseKernel::fprop_stats(std::ostream& _ostr ) const
{
  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& fps = fprops_.properties();
  for (it=fps.begin(); it!=fps.end(); ++it)
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);

}
void BaseKernel::mprop_stats(std::ostream& _ostr ) const
{
  PropertyContainer::Properties::const_iterator it;
  const PropertyContainer::Properties& mps = mprops_.properties();
  for (it=mps.begin(); it!=mps.end(); ++it)
    *it == NULL ? (void)(_ostr << "[deleted]" << "\n") : (*it)->stats(_ostr);

}


}
