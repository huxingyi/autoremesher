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

#ifndef SOOPENMESHSUPPORT_H
#define SOOPENMESHSUPPORT_H

//== REDEFINE DEFINES SO THEY WORK WITH TEMPLATES ============================

#define SO_NODE_SOURCE_TEMPLATE template <class Mesh>

// ----------------------------------------------------------------- COIN ----
//
// define __COIN__ is set by coin headers
#ifdef __COIN__

#ifdef PRIVATE_NODE_TYPESYSTEM_SOURCE
#  undef PRIVATE_NODE_TYPESYSTEM_SOURCE
#endif

#define PRIVATE_NODE_TYPESYSTEM_SOURCE(_class_) \
SO_NODE_SOURCE_TEMPLATE \
SoType _class_::getClassTypeId(void) { return _class_::classTypeId; } \
SO_NODE_SOURCE_TEMPLATE \
SoType _class_::getTypeId(void) const { return _class_::classTypeId; } \
/* Don't set value explicitly to SoType::badType(), to avoid a bug in */ \
/* Sun CC v4.0. (Bitpattern 0x0000 equals SoType::badType()). */ \
SO_NODE_SOURCE_TEMPLATE \
SoType _class_::classTypeId

// FIXME: document. 20000103 mortene.
#ifdef SO_NODE_ABSTRACT_SOURCE
#  undef SO_NODE_ABSTRACT_SOURCE
#endif

#define SO_NODE_ABSTRACT_SOURCE(_class_) \
PRIVATE_NODE_TYPESYSTEM_SOURCE(_class_); \
 \
SO_NODE_SOURCE_TEMPLATE \
unsigned int _class_::classinstances = 0; \
SO_NODE_SOURCE_TEMPLATE \
const SoFieldData ** _class_::parentFieldData = NULL; \
SO_NODE_SOURCE_TEMPLATE \
SoFieldData * _class_::fieldData = NULL; \
 \
SO_NODE_SOURCE_TEMPLATE \
const SoFieldData ** \
_class_::getFieldDataPtr(void) \
{ \
  return (const SoFieldData **)(&_class_::fieldData); \
} \
 \
SO_NODE_SOURCE_TEMPLATE \
const SoFieldData * \
_class_::getFieldData(void) const \
{ \
  return _class_::fieldData; \
}



// FIXME: document. 20000103 mortene.
#ifdef SO_NODE_SOURCE
#  undef SO_NODE_SOURCE
#endif
#define SO_NODE_SOURCE(_class_) \
SO_NODE_ABSTRACT_SOURCE(_class_); \
 \
SO_NODE_SOURCE_TEMPLATE \
void * \
_class_::createInstance(void) \
{ \
  return new _class_; \
}

// ------------------------------------------------------------------ SGI ----
#else

#ifdef SO_NODE_SOURCE
# undef SO_NODE_SOURCE
#endif

#define SO_NODE_SOURCE(_class_) \
  SO_NODE_SOURCE_TEMPLATE \
  SoType _class_::classTypeId; \
  \
  SO_NODE_SOURCE_TEMPLATE \
  SoFieldData *_class_::fieldData; \
  \
  SO_NODE_SOURCE_TEMPLATE \
  const SoFieldData **_class_::parentFieldData; \
  \
  SO_NODE_SOURCE_TEMPLATE \
  SbBool _class_::firstInstance = TRUE; \
  \
  SO_NODE_SOURCE_TEMPLATE \
  SoType _class_::getTypeId() const { \
    return classTypeId; \
  } \
  \
  SO_NODE_SOURCE_TEMPLATE \
  const SoFieldData* \
  _class_::getFieldData() const { \
    SO__NODE_CHECK_CONSTRUCT("SoOpenMeshNodeT"); \
    return fieldData; \
  } \
  \
  SO_NODE_SOURCE_TEMPLATE \
  void* _class_::createInstance() { \
    return (void *)(new _class_); \
  } \

#endif

#endif
