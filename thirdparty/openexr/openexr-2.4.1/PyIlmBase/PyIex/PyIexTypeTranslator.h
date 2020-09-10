///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2001-2011, Industrial Light & Magic, a division of Lucas
// Digital Ltd. LLC
// 
// All rights reserved.
// 
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
// *       Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
// *       Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
// *       Neither the name of Industrial Light & Magic nor the names of
// its contributors may be used to endorse or promote products derived
// from this software without specific prior written permission. 
// 
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
///////////////////////////////////////////////////////////////////////////

#ifndef INCLUDED_PYIEX_TYPETRANSLATOR_H
#define INCLUDED_PYIEX_TYPETRANSLATOR_H

//-----------------------------------------------------------------------------
//
//	PyIex -- support for mapping C++ exceptions to Python exceptions
//
//-----------------------------------------------------------------------------

#include <Python.h>
#include <string>
#include <vector>
#include <stdexcept>
#include <typeinfo>

namespace PyIex {

//
// TypeTranslator stores a class hierarchy along with corresponding
// python types and metadata for use in Python/C++ type translation.
//
template <class BaseClass>
class TypeTranslator
{
  public:

     TypeTranslator (const std::string &typeName, const std::string &moduleName, PyObject *typeObject);
    ~TypeTranslator ();

    PyObject *	typeObject (const BaseClass *ptr) const;
    PyObject *	baseTypeObject () const;

    template <class NewClass, class DerivedFrom>
    void	registerClass (const std::string &typeName,
                               const std::string &moduleName,
                               PyObject *typeObject);


    class ClassDesc
    {
      public:

	ClassDesc (const std::string &typeName,
                   const std::string &moduleName,
                   PyObject *typeObject,
		   const ClassDesc *baseClass);

	virtual ~ClassDesc ();

	virtual bool		typeMatches (const BaseClass *ptr) const = 0;

	virtual const std::type_info &
				typeInfo () const = 0;

        const std::string &     typeName() const;
        const std::string &     moduleName() const;

	PyObject *	        typeObject () const;
	const ClassDesc *	baseClass () const;

	int			numDerivedClasses () const;
	const ClassDesc *	derivedClass (int i) const;
	ClassDesc *		derivedClass (int i);

	void			addDerivedClass (ClassDesc *cd);

	const ClassDesc *	next () const;

      private:

        const std::string       _typeName;
        const std::string       _moduleName;
	PyObject *		_typeObject;
	const ClassDesc *	_baseClass;
	std::vector <ClassDesc *>	_derivedClasses;
	const ClassDesc *	_next;
    };

    template <class T>
    class ClassDescT: public ClassDesc
    {
      public:

	ClassDescT (const std::string &typeName,
                    const std::string &moduleName,
                    PyObject *typeObject,
		    const ClassDesc *baseClass);

	virtual bool		typeMatches (const BaseClass *ptr) const;

	virtual const std::type_info &
				typeInfo () const;
    };

    template <class T>
    ClassDesc *			findClassDesc (ClassDesc *cd);

    template <class T>
    const ClassDesc *		findClassDesc (const ClassDesc *cd) const;

    const ClassDesc *		firstClassDesc () const;
    const ClassDesc *		nextClassDesc (const ClassDesc *cd) const;

  private:
    ClassDesc *			_classes;
};


template <class BaseClass>
TypeTranslator<BaseClass>::TypeTranslator
    (const std::string &typeName, const std::string &moduleName, PyObject *typeObject)
{
    _classes = new ClassDescT <BaseClass> (typeName, moduleName, typeObject, 0);
}


template <class BaseClass>
TypeTranslator<BaseClass>::~TypeTranslator ()
{
    delete _classes;
}


template <class BaseClass>
PyObject *
TypeTranslator<BaseClass>::typeObject (const BaseClass *ptr) const
{
    const ClassDesc *cd = _classes;

    assert (cd->typeMatches (ptr));

    while (true)
    {
	const ClassDesc *matchingCd = 0;

	for (int i = 0; i < cd->numDerivedClasses(); ++i)
	{
	    const ClassDesc *derivedCd = cd->derivedClass (i);

	    if (derivedCd->typeMatches (ptr))
	    {
		matchingCd = derivedCd;
		break;
	    }
	}

	if (matchingCd)
	    cd = matchingCd;
	else
	    break;
    }

    return cd->typeObject ();
}

template <class BaseClass>
PyObject *
TypeTranslator<BaseClass>::baseTypeObject () const
{
    return _classes->typeObject ();
}


template <class BaseClass>
template <class NewClass, class DerivedFrom>
void		
TypeTranslator<BaseClass>::registerClass
    (const std::string &typeName, const std::string &moduleName, PyObject *typeObject)
{
    ClassDesc *df = findClassDesc <DerivedFrom> (_classes);

    if (df == 0)
	throw std::invalid_argument ("PyIex::TypeTranslator: "
				     "Base class must be registered "
				     "before derived class.");

    ClassDesc *nc = findClassDesc <NewClass> (_classes);

    if (nc != 0)
    {
	//
	// Calling registerClass() more than once with the same
	// NewClass and DerivedFrom arguments is a no-op.
	// Calling registerClass() multiple times with the same
	// NewClass but with different DerivedFrom arguments is
	// an error.
	//

	for (int i = 0; i < df->numDerivedClasses(); ++i)
	{
	    if (df->derivedClass (i) == nc)
		return;
	}

	throw std::invalid_argument ("PyIex::TypeTranslator: "
				     "Derived class registered twice "
				     "with different base classes.");
    }

    df->addDerivedClass (new ClassDescT<NewClass> (typeName, moduleName, typeObject, df));
}


template <class BaseClass>
template <class T>
typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::findClassDesc (ClassDesc *cd)
{
    if (cd->typeInfo().hash_code() == typeid (T).hash_code())
	return cd;

    for (int i = 0; i < cd->numDerivedClasses(); ++i)
    {
	ClassDesc *match = findClassDesc<T> (cd->derivedClass(i));

	if (match)
	    return match;
    }

    return 0;
}

template <class BaseClass>
template <class T>
const typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::findClassDesc (const ClassDesc *cd) const
{
    if (cd->typeInfo().hash_code() == typeid (T).hash_code())
	return cd;

    for (int i = 0; i < cd->numDerivedClasses(); ++i)
    {
	const ClassDesc *match = findClassDesc<T> (cd->derivedClass(i));

	if (match)
	    return match;
    }

    return 0;
}


template <class BaseClass>
inline const typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::firstClassDesc () const
{
    return _classes;
}


template <class BaseClass>
inline const typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::nextClassDesc (const ClassDesc *cd) const
{
    return cd->next();
}


template <class BaseClass>
TypeTranslator<BaseClass>::ClassDesc::ClassDesc
    (const std::string &typeName,
     const std::string &moduleName,
     PyObject *typeObject,
     const ClassDesc *baseClass)
    : _typeName(typeName), _moduleName(moduleName),
      _typeObject (typeObject), _baseClass (baseClass), _next (0)
{
    // emtpy
}


template <class BaseClass>
TypeTranslator<BaseClass>::ClassDesc::~ClassDesc ()
{
    for (size_t i = 0; i < _derivedClasses.size(); ++i)
	delete _derivedClasses[i];
}

template <class BaseClass>
inline const std::string &
TypeTranslator<BaseClass>::ClassDesc::typeName () const
{
    return _typeName;
}

template <class BaseClass>
inline const std::string &
TypeTranslator<BaseClass>::ClassDesc::moduleName () const
{
    return _moduleName;
}


template <class BaseClass>
inline PyObject *
TypeTranslator<BaseClass>::ClassDesc::typeObject () const
{
    return _typeObject;
}


template <class BaseClass>
inline const typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::ClassDesc::baseClass () const
{
    return _baseClass;
}


template <class BaseClass>
int
TypeTranslator<BaseClass>::ClassDesc::numDerivedClasses () const
{
    return _derivedClasses.size();
}


template <class BaseClass>
inline const typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::ClassDesc::derivedClass (int i) const
{
    return _derivedClasses[i];
}


template <class BaseClass>
inline typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::ClassDesc::derivedClass (int i)
{
    return _derivedClasses[i];
}


template <class BaseClass>
void
TypeTranslator<BaseClass>::ClassDesc::addDerivedClass (ClassDesc *cd)
{
    _derivedClasses.push_back (cd);
    cd->_next = _next;
    _next = cd;
}


template <class BaseClass>
inline const typename TypeTranslator<BaseClass>::ClassDesc *
TypeTranslator<BaseClass>::ClassDesc::next () const
{
    return _next;
}


template <class BaseClass>
template <class T>
TypeTranslator<BaseClass>::ClassDescT<T>::ClassDescT
    (const std::string &typeName,
     const std::string &moduleName,
     PyObject *typeObject,
     const ClassDesc *baseClass):
    TypeTranslator<BaseClass>::ClassDesc (typeName, moduleName, typeObject, baseClass)
{
    // empty
}


template <class BaseClass>
template <class T>
bool	
TypeTranslator<BaseClass>::ClassDescT<T>::typeMatches (const BaseClass *ptr) const
{
    return 0 != dynamic_cast <const T *> (ptr);
}


template <class BaseClass>
template <class T>
const std::type_info &
TypeTranslator<BaseClass>::ClassDescT<T>::typeInfo () const
{
    return typeid (T);
}

} // namespace PyIex

#endif
