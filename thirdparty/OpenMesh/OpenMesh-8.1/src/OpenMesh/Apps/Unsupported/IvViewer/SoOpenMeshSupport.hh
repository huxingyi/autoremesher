/* ========================================================================= *
 *                                                                           *
 *                               OpenMesh                                    *
 *           Copyright (c) 2001-2015, RWTH-Aachen University                 *
 *           Department of Computer Graphics and Multimedia                  *
 *                          All rights reserved.                             *
 *                            www.openmesh.org                               *
 *                                                                           *
 *---------------------------------------------------------------------------*
 * This file is part of OpenMesh.                                            *
 *---------------------------------------------------------------------------*
 *                                                                           *
 * Redistribution and use in source and binary forms, with or without        *
 * modification, are permitted provided that the following conditions        *
 * are met:                                                                  *
 *                                                                           *
 * 1. Redistributions of source code must retain the above copyright notice, *
 *    this list of conditions and the following disclaimer.                  *
 *                                                                           *
 * 2. Redistributions in binary form must reproduce the above copyright      *
 *    notice, this list of conditions and the following disclaimer in the    *
 *    documentation and/or other materials provided with the distribution.   *
 *                                                                           *
 * 3. Neither the name of the copyright holder nor the names of its          *
 *    contributors may be used to endorse or promote products derived from   *
 *    this software without specific prior written permission.               *
 *                                                                           *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       *
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED *
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A           *
 * PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER *
 * OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,  *
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,       *
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR        *
 * PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF    *
 * LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING      *
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS        *
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.              *
 *                                                                           *
 * ========================================================================= */



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
const SoFieldData ** _class_::parentFieldData = nullptr; \
SO_NODE_SOURCE_TEMPLATE \
SoFieldData * _class_::fieldData = nullptr; \
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
