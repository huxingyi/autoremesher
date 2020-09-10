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

#ifndef INCLUDED_PY_IEX_H
#define INCLUDED_PY_IEX_H

//-----------------------------------------------------------------------------
//
//	PyIex -- support for mapping C++ exceptions to Python exceptions
//
//-----------------------------------------------------------------------------

#include <sstream>
#include <Python.h>
#include <boost/python.hpp>
#include <IexMathFloatExc.h>
#include <boost/python/errors.hpp>
#include <boost/format.hpp>
#include "PyIexTypeTranslator.h"
#include "PyIexExport.h"

namespace PyIex {

//
// Macros to catch C++ exceptions and translate them into Python exceptions
// for use in python C api code:
//
//	PY_TRY
//	PY_CATCH
//
// Usage:
//
//	Insert PY_TRY and PY_CATCH at the beginning and end of every
//	wrapper function to make sure that all possible exceptions
//	are caught and translated to corresponding Python exceptions.
//	Example:
//	
//	PyObject *
//	setSpeed (PyCar *self, PyObject *args)
//	{
//	    PY_TRY
//
//	    float length;
//	    PY_ARG_PARSE ((args, "f", &length));
//
//	    self->data->setSpeed (length);	// may throw
//
//	    PY_RETURN_NONE;
//	    PY_CATCH
//	}
//

#define PY_TRY						\
    try							\
    {							\
	IEX_NAMESPACE::MathExcOn mathexcon (IEX_NAMESPACE::IEEE_OVERFLOW |	\
			          IEX_NAMESPACE::IEEE_DIVZERO |	\
				  IEX_NAMESPACE::IEEE_INVALID);


#define PY_CATCH					\
    }							\
    catch (boost::python::error_already_set)		\
    {							\
	return 0;					\
    }							\
    catch (...)						\
    {							\
	boost::python::handle_exception();		\
	return 0;					\
    }


#define PY_CATCH_WITH_COMMENT(text)			\
    }							\
    catch (boost::python::error_already_set)		\
    {							\
        /* Can't use text here without messing with */	\
        /* the existing python exception state, so  */	\
        /* ignore                                   */	\
	return 0;					\
    }							\
    catch (...)						\
    {							\
	boost::python::handle_exception();		\
	return 0;					\
    }


// In most case, PY_CATCH should be used.  But in a few cases, the Python
// interpreter treats a return code of 0 as success rather than failure
// (e.g., the tp_print routine in a PyTypeObject struct). 

#define PY_CATCH_RETURN_CODE(CODE)			\
    }							\
    catch (...)						\
    {							\
	boost::python::handle_exception();		\
	return (CODE);					\
    }


PYIEX_EXPORT TypeTranslator<IEX_NAMESPACE::BaseExc> &baseExcTranslator();

// this should only be called from iexmodule.cpp during iex
// module initialization.
PYIEX_EXPORT void setBaseExcTranslator(TypeTranslator<IEX_NAMESPACE::BaseExc> *t);

//
// Since there's currently no mechanism in boost to inherit off of
// a python type (RuntimeError in this case), we instead use a
// parallel exception hierarchy defined in python, and create
// and register custom converters with boost::python to go between
// the c++ and corresponding python types.
//
// To register exceptions derived from IEX_NAMESPACE::BaseExc, call
// registerException with the type and base type as template
// parameters, and the name and module as arguments.  e.g.:
//
//     registerException<EpermExc,ErrnoExc>("EpermExc","iex");
//

//
// ExcTranslator provides the methods needed for boost to convert
// the parallel exception types between c++ and python.
//
template <class T>
class ExcTranslator
{
public:
    // to python
    static PyObject *convert(const T &exc)
    {
        using namespace boost::python;
        object excType = object(handle<>(borrowed(baseExcTranslator().typeObject(&exc))));
        return incref(excType(exc.what()).ptr());
    }

    // from python
    static void *convertible(PyObject *exc)
    {
#ifdef Py_TYPE
        if (!PyType_IsSubtype(Py_TYPE(exc),(PyTypeObject *)baseExcTranslator().baseTypeObject())) return 0;
#else
        // prior to python 2.6, access an object's type via it's ob_type member
        if (!PyType_IsSubtype(exc->ob_type,(PyTypeObject *)baseExcTranslator().baseTypeObject())) return 0;
#endif
        return exc;
    }

    static void construct(PyObject* raw_exc, boost::python::converter::rvalue_from_python_stage1_data* data)
    {
        using namespace boost::python;
        object exc(handle<>(borrowed(raw_exc)));
        std::string s = extract<std::string>(exc.attr("__str__")());
        void *storage = ((converter::rvalue_from_python_storage<T>*)data)->storage.bytes;
        new (storage) T(s);
        data->convertible = storage;
    }

};

//
// This function creates the proxy python type for a given exception.
//
static inline boost::python::object
createExceptionProxy(const std::string &name, const std::string &module,
                     const std::string &baseName, const std::string &baseModule,
                     PyObject *baseType)
{
    using namespace boost::python;
    dict tmpDict;
    tmpDict["__builtins__"] = handle<>(borrowed(PyEval_GetBuiltins()));

    std::string base = baseName;
    std::string definition;
 
    if (baseModule != module)
    {
        definition += (boost::format("import %s\n") % baseModule).str();
        base = (boost::format("%s.%s") % baseModule % baseName).str();
    }
    else
    {
        tmpDict[base] = object(handle<>(borrowed(baseType)));
    }

    definition += (boost::format("class %s (%s):\n"
                                 "  def __init__ (self, v=''):\n"
                                 "    super(%s,self).__init__(v)\n"
                                 "  def __repr__ (self):\n"
                                 "    return \"%s.%s('%%s')\"%%(self.args[0])\n")
                   % name % base % name % module % name).str();

    handle<> tmp(PyRun_String(definition.c_str(),Py_file_input,tmpDict.ptr(),tmpDict.ptr()));
    return tmpDict[name];
}

//
// register an excpetion derived from IEX_NAMESPACE::BaseExc out to python using
// the proxy mechanism described above.
//
template<class Exc, class ExcBase>
void
registerExc(const std::string &name, const std::string &module)
{
    using namespace boost::python;

    const TypeTranslator<IEX_NAMESPACE::BaseExc>::ClassDesc *baseDesc = baseExcTranslator().template findClassDesc<ExcBase>(baseExcTranslator().firstClassDesc());
    std::string baseName = baseDesc->typeName();
    std::string baseModule = baseDesc->moduleName();

    object exc_class = createExceptionProxy(name, module, baseName, baseModule, baseDesc->typeObject());
    scope().attr(name.c_str()) = exc_class;
    baseExcTranslator().registerClass<Exc,ExcBase>(name, module, exc_class.ptr());
    // to python
    to_python_converter<Exc,ExcTranslator<Exc> >();
    // from python
    converter::registry::push_back(&ExcTranslator<Exc>::convertible,
                                   &ExcTranslator<Exc>::construct,type_id<Exc>());
}

} // namespace PyIex

#endif
