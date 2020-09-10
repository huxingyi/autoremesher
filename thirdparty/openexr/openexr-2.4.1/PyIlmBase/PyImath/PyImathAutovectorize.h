///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007-2011, Industrial Light & Magic, a division of Lucas
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

#ifndef _PyImathAutovectorize_h_
#define _PyImathAutovectorize_h_

#include <Python.h>
#include <boost/python.hpp>
#include <boost/mpl/size.hpp>
#include <boost/mpl/pop_front.hpp>
#include <boost/mpl/push_front.hpp>
#include <boost/mpl/front.hpp>
#include <boost/mpl/eval_if.hpp>
#include <boost/mpl/bool.hpp>
#include <boost/mpl/vector.hpp>
#include <boost/mpl/transform.hpp>
#include <boost/mpl/remove_if.hpp>
#include <boost/mpl/equal.hpp>
#include <boost/mpl/for_each.hpp>
#include <boost/mpl/not.hpp>
#include <boost/mpl/count.hpp>
#include <boost/mpl/or.hpp>
#include <boost/type_traits/is_base_of.hpp>
#include <boost/type_traits/function_traits.hpp>
#include <boost/static_assert.hpp>
#include <boost/python/args.hpp>
#include <iostream>
#include "PyImathFixedArray.h"
#include "PyImathTask.h"
#include "PyImathUtil.h"
#include <IexMathFloatExc.h>

namespace PyImath {

struct op_with_precomputation {};

namespace detail {

using boost::is_base_of;
using boost::is_same;
using boost::is_const;
using boost::remove_const;
using boost::remove_reference;
using boost::function_traits;

using boost::mpl::at;
using boost::mpl::at_c;
using boost::mpl::push_front;
using boost::mpl::vector;
using boost::mpl::push_back;
using boost::mpl::transform;
using boost::mpl::fold;
using boost::mpl::_;
using boost::mpl::_1;
using boost::mpl::_2;
using boost::mpl::long_;
using boost::mpl::false_;
using boost::mpl::true_;
using boost::mpl::not_;
using boost::mpl::or_;
using boost::mpl::and_;
using boost::mpl::size;
using boost::mpl::remove_if;
using boost::mpl::if_;
using boost::mpl::for_each;

template <class T> struct name_of_type;

template <> struct name_of_type<int>
{
   static const char *apply() { return "int"; }
};
template <> struct name_of_type<float>
{
    static const char *apply() { return "float"; }
};
template <> struct name_of_type<double>
{
    static const char *apply() { return "double"; }
};
template <class T> struct name_of_type<PyImath::FixedArray<T> >
{
    static const char *apply() { return PyImath::FixedArray<T>::name(); }
};


struct null_precomputation {
    static void precompute(size_t len) { return; }
};

template <class T> struct op_precompute
{
    static void
    apply(size_t len)
    {
        if_<is_base_of<op_with_precomputation,T>,
            T,
            null_precomputation>::type::precompute(len);
    }
};

template <int N>
struct possible_vectorizations
{
    typedef typename fold<
        typename possible_vectorizations<N-1>::type,
        vector<>,
        push_back<push_back<_1,push_back<_2,false_> >,push_back<_2,true_> >
    >::type type;
};

template <>
struct possible_vectorizations<0>
{
    typedef vector<vector<> > type;    
};

template <class Vectorizable>
struct disallow_vectorization
{
    template <class Vectorize>
    struct apply
    {
        // Valid = !Vectorize || Vectorizable
        typedef typename transform<Vectorize,not_<_> >::type DontVectorize;
        typedef typename transform<DontVectorize,Vectorizable,or_<_,_> >::type Valid;
        typedef typename not_<fold<Valid,true_,and_<_,_> > >::type type;
    };
};

template <class Vectorizable>
struct allowable_vectorizations
{
    typedef typename possible_vectorizations<size<Vectorizable>::value>::type possible;
    typedef typename remove_if<possible,disallow_vectorization<Vectorizable> >::type type;
};

template <class T>
bool any_masked(const T &value)
{
    return false;
};

template <class T>
bool any_masked(const PyImath::FixedArray<T> &value)
{
    return value.isMaskedReference();
};

template <class T1, class T2>
bool any_masked(const T1 &a, const T2 &b)
{
    return any_masked(a) || any_masked(b);
}

template <class T1, class T2, class T3>
bool any_masked(const T1 &a, const T2 &b, const T3 &c)
{
    return any_masked(a,b) || any_masked(c);
}

template <class T1, class T2, class T3, class T4>
bool any_masked(const T1 &a, const T2 &b, const T3 &c, const T4 &d)
{
    return any_masked(a,b) || any_masked(c,d);
}

template <class T>
struct access_value
{
    static inline T & apply(T &arg, size_t i) { return arg; }
};

template <class T>
struct access_value<T &>
{
    static inline T & apply(T &arg, size_t i) { return arg; }
};

template <class T>
struct access_value<PyImath::FixedArray<T> &>
{
    static inline T & apply(PyImath::FixedArray<T> &arg, size_t i) { return arg[i]; }
};

template <class T>
struct access_value<const PyImath::FixedArray<T> &>
{
    static inline const T & apply(const PyImath::FixedArray<T> &arg, size_t i) { return arg[i]; }
};

template <class T>
struct direct_access_value
{
    static inline T & apply(T &arg, size_t i) { return arg; }
};

template <class T>
struct direct_access_value<T &>
{
    static inline T & apply(T &arg, size_t i) { return arg; }
};

template <class T>
struct direct_access_value<PyImath::FixedArray<T> &>
{
    static inline T & apply(PyImath::FixedArray<T> &arg, size_t i) { return arg.direct_index(i); }
};

template <class T>
struct direct_access_value<const PyImath::FixedArray<T> &>
{
    static inline const T & apply(const PyImath::FixedArray<T> &arg, size_t i) { return arg.direct_index(i); }
};


//-----------------------------------------------------------------------------------------

//
// measure_argument returns a pair indicating the integral length of the argument
// (scalar arguments have implicit length 1), and a bool indicating whether the argument
// is a vectorized argument.
//
template <class T>
struct measure_argument
{
    static inline std::pair<size_t,bool> apply(T arg) { return std::make_pair(1,false); }
};

template <class T>
struct measure_argument<PyImath::FixedArray<T> >
{
    static inline std::pair<size_t,bool> apply(const PyImath::FixedArray<T> &arg) { return std::make_pair(arg.len(),true); }
};

//
// match_lengths returns the compatible length given two argument lengths
//
static inline std::pair<size_t,bool>
match_lengths(const std::pair<size_t,bool> &len1, const std::pair<size_t,bool> &len2)
{
    // scalar arguemnts are always compatible with other arguments
    if (len1.second == false) return len2;
    if (len2.second == false) return len1;

    // now both arguments are vectorized, check for dimension match
    if (len1.first != len2.first)
        throw IEX_NAMESPACE::ArgExc("Array dimensions passed into function do not match");

    return len1;
}


//
// measure_arguments finds the length that a return value from a given
// set of arguments should have, throwing an exception if the lengths
// are incompatible.  If all arguments are scalar, length 1 is returned.
//
template <class arg1_type>
size_t
measure_arguments(const arg1_type &arg1)
{
    std::pair<size_t,bool> len = measure_argument<arg1_type>::apply(arg1);
    return len.first;
}

template <class arg1_type, class arg2_type>
size_t
measure_arguments(const arg1_type &arg1, const arg2_type &arg2)
{
    std::pair<size_t,bool> len = measure_argument<arg1_type>::apply(arg1);
    len = match_lengths(len,measure_argument<arg2_type>::apply(arg2));
    return len.first;
}

template <class arg1_type, class arg2_type, class arg3_type>
size_t
measure_arguments(const arg1_type &arg1, const arg2_type &arg2, const arg3_type &arg3)
{
    std::pair<size_t,bool> len = measure_argument<arg1_type>::apply(arg1);
    len = match_lengths(len,measure_argument<arg2_type>::apply(arg2));
    len = match_lengths(len,measure_argument<arg3_type>::apply(arg3));
    return len.first;
}

//-----------------------------------------------------------------------------------------

template <class T>
struct create_uninitalized_return_value
{
    static T apply(size_t length)
    {
        return T();
    }
};

template <class T>
struct create_uninitalized_return_value<PyImath::FixedArray<T> >
{
    static PyImath::FixedArray<T> apply(size_t length)
    {
        return PyImath::FixedArray<T>(Py_ssize_t(length),PyImath::UNINITIALIZED);
    }
};

template <class T, class VectorizeArg>
struct vectorized_result_type
{
    typedef typename if_<VectorizeArg,PyImath::FixedArray<T>,T>::type type;
};

template <class T, class VectorizeArg>
struct vectorized_argument_type
{
    typedef typename remove_const<typename remove_reference<T>::type>::type base_type;
    typedef typename if_<VectorizeArg,const PyImath::FixedArray<base_type> &,T>::type type;
};

template <class Op, class result_type, class arg1_type>
struct VectorizedOperation1 : public Task
{
    result_type &retval;
    arg1_type arg1;

    VectorizedOperation1(result_type &r, arg1_type a1) : retval(r), arg1(a1) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(retval,arg1)) {
            for (size_t i=start; i<end; ++i) {
                access_value<result_type &>::apply(retval,i) = Op::apply(access_value<arg1_type>::apply(arg1,i));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                direct_access_value<result_type &>::apply(retval,i) = Op::apply(direct_access_value<arg1_type>::apply(arg1,i));
            }
        }
    }
};

template <class Op, class result_type, class arg1_type, class arg2_type>
struct VectorizedOperation2 : public Task
{
    result_type &retval;
    arg1_type arg1;
    arg2_type arg2;

    VectorizedOperation2(result_type &r, arg1_type a1, arg2_type a2) : retval(r), arg1(a1), arg2(a2) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(retval,arg1,arg2)) {
            for (size_t i=start; i<end; ++i) {
                access_value<result_type &>::apply(retval,i) = Op::apply(access_value<arg1_type>::apply(arg1,i),
                                                                         access_value<arg2_type>::apply(arg2,i));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                direct_access_value<result_type &>::apply(retval,i) = Op::apply(direct_access_value<arg1_type>::apply(arg1,i),
                                                                                direct_access_value<arg2_type>::apply(arg2,i));
            }
        }
    }
};

template <class Op, class result_type, class arg1_type, class arg2_type, class arg3_type>
struct VectorizedOperation3 : public Task
{
    result_type &retval;
    arg1_type arg1;
    arg2_type arg2;
    arg3_type arg3;

    VectorizedOperation3(result_type &r, arg1_type a1, arg2_type a2, arg3_type a3) : retval(r), arg1(a1), arg2(a2), arg3(a3) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(retval,arg1,arg2,arg3)) {
            for (size_t i=start; i<end; ++i) {
                access_value<result_type &>::apply(retval,i) = Op::apply(access_value<arg1_type>::apply(arg1,i),
                                                                         access_value<arg2_type>::apply(arg2,i),
                                                                         access_value<arg3_type>::apply(arg3,i));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                direct_access_value<result_type &>::apply(retval,i) = Op::apply(direct_access_value<arg1_type>::apply(arg1,i),
                                                                                direct_access_value<arg2_type>::apply(arg2,i),
                                                                                direct_access_value<arg3_type>::apply(arg3,i));
            }
        }
    }
};


template <class Op, class Vectorize, class Func>
struct VectorizedFunction1 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value == function_traits<Func>::arity));

    typedef function_traits<Func> traits;
    typedef typename fold<Vectorize,false_,or_<_,_> >::type any_vectorized;

    typedef typename vectorized_result_type<typename traits::result_type,any_vectorized>::type result_type;
    typedef typename vectorized_argument_type<typename traits::arg1_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;

    static result_type
    apply(arg1_type arg1)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(arg1);
        op_precompute<Op>::apply(len);
        result_type retval = create_uninitalized_return_value<result_type>::apply(len);
        VectorizedOperation1<Op,result_type,arg1_type> vop(retval,arg1);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return retval;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<1> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+") - ";
    }
};

template <class Op, class Vectorize, class Func>
struct VectorizedFunction2 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value == function_traits<Func>::arity));

    typedef function_traits<Func> traits;
    typedef typename fold<Vectorize,false_,or_<_,_> >::type any_vectorized;

    typedef typename vectorized_result_type<typename traits::result_type,any_vectorized>::type result_type;
    typedef typename vectorized_argument_type<typename traits::arg1_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,typename at<Vectorize,long_<1> >::type>::type arg2_type;

    static result_type
    apply(arg1_type arg1, arg2_type arg2)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(arg1,arg2);
        op_precompute<Op>::apply(len);
        result_type retval = create_uninitalized_return_value<result_type>::apply(len);
        VectorizedOperation2<Op,result_type,arg1_type,arg2_type> vop(retval,arg1,arg2);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return retval;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<2> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+","+args.elements[1].name+") - ";
    }
};

template <class Op, class Vectorize, class Func>
struct VectorizedFunction3 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value == function_traits<Func>::arity));

    typedef function_traits<Func> traits;
    typedef typename fold<Vectorize,false_,or_<_,_> >::type any_vectorized;

    typedef typename vectorized_result_type<typename traits::result_type,any_vectorized>::type result_type;
    typedef typename vectorized_argument_type<typename traits::arg1_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,typename at<Vectorize,long_<1> >::type>::type arg2_type;
    typedef typename vectorized_argument_type<typename traits::arg3_type,typename at<Vectorize,long_<2> >::type>::type arg3_type;

    static result_type
    apply(arg1_type arg1, arg2_type arg2, arg3_type arg3)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(arg1,arg2,arg3);
        op_precompute<Op>::apply(len);
        result_type retval = create_uninitalized_return_value<result_type>::apply(len);
        VectorizedOperation3<Op,result_type,arg1_type,arg2_type,arg3_type> vop(retval,arg1,arg2,arg3);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return retval;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<3> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+","+args.elements[1].name+","+args.elements[2].name+") - ";
    }
};

template <class Op, class Func, class Keywords>
struct function_binding
{
    std::string _name, _doc;
    const Keywords &_args;


    function_binding(const std::string &name, const std::string &doc,const Keywords &args)
        : _name(name), _doc(doc), _args(args)
    {}

    template <class Vectorize>
    void operator()(Vectorize) const
    {
        typedef typename at<vector<
             int,  // unused, arity 0
             VectorizedFunction1<Op,Vectorize,Func>,
             VectorizedFunction2<Op,Vectorize,Func>,
             VectorizedFunction3<Op,Vectorize,Func>
            >,
            long_<function_traits<Func>::arity> >::type vectorized_function_type;
        std::string doc = _name + vectorized_function_type::format_arguments(_args) + _doc;
        boost::python::def(_name.c_str(),&vectorized_function_type::apply,doc.c_str(),_args);
    }
};

template <class Op,class Func,class Keywords>
function_binding<Op,Func,Keywords>
build_function_binding(Func *func,const std::string &name,const std::string &doc,const Keywords &args)
{
    return function_binding<Op,Func,Keywords>(name,doc,args);
}

template <class Op,class Vectorizable,class Keywords>
struct generate_bindings_struct
{
    //BOOST_STATIC_ASSERT(size<Vectorizable>::value == function_traits<Op::apply>::arity);
    static void apply(const std::string &name,const std::string &doc,const Keywords &args) {
        for_each<typename allowable_vectorizations<Vectorizable>::type>(build_function_binding<Op>(Op::apply,name,doc,args));
    }
};


template <class T>
struct vectorized_class_reference_type
{
    typedef typename remove_const<typename remove_reference<T>::type>::type base_type;
    typedef typename if_<is_const<T>,const PyImath::FixedArray<base_type> &,PyImath::FixedArray<base_type> &>::type type;
};

template <class Op, class class_type>
struct VectorizedVoidOperation0 : public Task
{
    class_type cls;

    VectorizedVoidOperation0(class_type c) : cls(c) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(cls)) {
            for (size_t i=start; i<end; ++i) {
                Op::apply(access_value<class_type>::apply(cls,i));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                Op::apply(direct_access_value<class_type>::apply(cls,i));
            }
        }
    }
};

template <class Op, class class_type, class arg1_type>
struct VectorizedVoidOperation1 : public Task
{
    class_type cls;
    arg1_type arg1;

    VectorizedVoidOperation1(class_type c, arg1_type a1) : cls(c), arg1(a1) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(cls,arg1)) {
            for (size_t i=start; i<end; ++i) {
                Op::apply(access_value<class_type>::apply(cls,i),
                          access_value<arg1_type>::apply(arg1,i));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                Op::apply(direct_access_value<class_type>::apply(cls,i),
                          direct_access_value<arg1_type>::apply(arg1,i));
            }
        }
    }
};

template <class Op, class class_type, class arg1_type>
struct VectorizedMaskedVoidOperation1 : public Task
{
    class_type cls;
    arg1_type arg1;

    VectorizedMaskedVoidOperation1(class_type c, arg1_type a1) : cls(c), arg1(a1) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(arg1)) {
            for (size_t i=start; i<end; ++i) {
                Op::apply(access_value<class_type>::apply(cls,i),
                          access_value<arg1_type>::apply(arg1,cls.raw_ptr_index(i)));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                Op::apply(access_value<class_type>::apply(cls,i),
                          direct_access_value<arg1_type>::apply(arg1,cls.raw_ptr_index(i)));
            }
        }
    }
};

template <class Op, class class_type, class arg1_type, class arg2_type>
struct VectorizedVoidOperation2 : public Task
{
    class_type cls;
    arg1_type arg1;
    arg2_type arg2;

    VectorizedVoidOperation2(class_type c, arg1_type a1, arg2_type a2) : cls(c), arg1(a1), arg2(a2) {}

    void execute(size_t start, size_t end)
    {
        if (any_masked(cls,arg1,arg2)) {
            for (size_t i=start; i<end; ++i) {
                Op::apply(access_value<class_type>::apply(cls,i),
                          access_value<arg1_type>::apply(arg1,i),
                          access_value<arg2_type>::apply(arg2,i));
            }
        } else {
            for (size_t i=start; i<end; ++i) {
                Op::apply(direct_access_value<class_type>::apply(cls,i),
                          direct_access_value<arg1_type>::apply(arg1,i),
                          direct_access_value<arg2_type>::apply(arg2,i));
            }
        }
    }
};


template <class Op, class Vectorize, class Func>
struct VectorizedVoidMemberFunction0 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value+1 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;

    static class_type
    apply(class_type cls)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(cls);
        op_precompute<Op>::apply(len);
        VectorizedVoidOperation0<Op,class_type> vop(cls);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return cls;
    }
};

template <class Op, class Vectorize, class Func>
struct VectorizedVoidMemberFunction1 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value+1 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;

    static class_type
    apply(class_type cls, arg1_type arg1)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(cls,arg1);
        op_precompute<Op>::apply(len);
        VectorizedVoidOperation1<Op,class_type,arg1_type> vop(cls,arg1);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return cls;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<1> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+") - ";
    }
};

//
// special class to handle single argument void memberfunctions, such as those
// used for the inplace operators like +=, -=, etc.  In this case we allow additional
// compatibilty between a masked class and an unmasked right hand side, using the
// mask to select results.
//
template <class Op, class Func>
struct VectorizedVoidMaskableMemberFunction1 {
    BOOST_STATIC_ASSERT((2 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,boost::mpl::true_>::type arg1_type;

    static class_type
    apply(class_type cls, arg1_type arg1)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = cls.match_dimension(arg1, false);
        op_precompute<Op>::apply(len);

        if (cls.isMaskedReference() && (size_t) arg1.len() == cls.unmaskedLength())
        {
            // class is masked, and the unmasked length matches the right hand side
            VectorizedMaskedVoidOperation1<Op,class_type,arg1_type> vop(cls,arg1);
            dispatchTask(vop,len);
        }
        else
        {
            // the two arrays match length (masked or otherwise), use the standard path.
            VectorizedVoidOperation1<Op,class_type,arg1_type> vop(cls,arg1);
            dispatchTask(vop,len);
        }
           
        PY_IMATH_RETURN_PYTHON;
        return cls;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<1> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+") - ";
    }
};

template <class Op, class Vectorize, class Func>
struct VectorizedVoidMemberFunction2 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value+1 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;
    typedef typename vectorized_argument_type<typename traits::arg3_type,typename at<Vectorize,long_<1> >::type>::type arg2_type;

    static class_type
    apply(class_type cls, arg1_type arg1, arg2_type arg2)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(cls,arg1,arg2);
        op_precompute<Op>::apply(len);
        VectorizedVoidOperation2<Op,class_type,arg1_type,arg2_type> vop(cls,arg1,arg2);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return cls;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<2> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+","+args.elements[1].name+") - ";
    }
};


template <class Op, class Vectorize, class Func>
struct VectorizedMemberFunction0 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value+1 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_result_type<typename traits::result_type,true_>::type result_type;
    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;

    static result_type
    apply(class_type cls)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(cls);
        op_precompute<Op>::apply(len);
        result_type retval = create_uninitalized_return_value<result_type>::apply(len);
        VectorizedOperation1<Op,result_type,class_type> vop(retval,cls);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return retval;
    }
};

template <class Op, class Vectorize, class Func>
struct VectorizedMemberFunction1 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value+1 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_result_type<typename traits::result_type,true_>::type result_type;
    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;

    static result_type
    apply(class_type cls, arg1_type arg1)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(cls,arg1);
        op_precompute<Op>::apply(len);
        result_type retval = create_uninitalized_return_value<result_type>::apply(len);
        VectorizedOperation2<Op,result_type,class_type,arg1_type> vop(retval,cls,arg1);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return retval;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<1> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+") - ";
    }
};

template <class Op, class Vectorize, class Func>
struct VectorizedMemberFunction2 {
    BOOST_STATIC_ASSERT((size<Vectorize>::value+1 == function_traits<Func>::arity));

    typedef function_traits<Func> traits;

    typedef typename vectorized_result_type<typename traits::result_type,true_>::type result_type;
    typedef typename vectorized_class_reference_type<typename traits::arg1_type>::type class_type;
    typedef typename vectorized_argument_type<typename traits::arg2_type,typename at<Vectorize,long_<0> >::type>::type arg1_type;
    typedef typename vectorized_argument_type<typename traits::arg3_type,typename at<Vectorize,long_<1> >::type>::type arg2_type;

    static result_type
    apply(class_type cls, arg1_type arg1, arg2_type arg2)
    {
        PY_IMATH_LEAVE_PYTHON;
        size_t len = measure_arguments(cls,arg1,arg2);
        op_precompute<Op>::apply(len);
        result_type retval = create_uninitalized_return_value<result_type>::apply(len);
        VectorizedOperation3<Op,result_type,class_type,arg1_type,arg2_type> vop(retval,cls,arg1,arg2);
        dispatchTask(vop,len);
        PY_IMATH_RETURN_PYTHON;
        return retval;
    }

    static std::string
    format_arguments(const boost::python::detail::keywords<2> &args)
    {
        // TODO: add types here
        return std::string("(")+args.elements[0].name+","+args.elements[1].name+") - ";
    }
};

template <class Op, class Cls, class Func, class Keywords>
struct member_function_binding
{
    Cls &_cls;
    std::string _name, _doc;
    const Keywords &_args;

    member_function_binding(Cls &cls,const std::string &name, const std::string &doc,const Keywords &args)
        : _cls(cls), _name(name), _doc(doc), _args(args)
    {}

    template <class Vectorize>
    void operator()(Vectorize) const
    {
        typedef typename if_<is_same<void,typename function_traits<Func>::result_type>,
                             typename if_<boost::mpl::equal<Vectorize,boost::mpl::vector<boost::mpl::true_> >,
                                 VectorizedVoidMaskableMemberFunction1<Op,Func>,
                                 VectorizedVoidMemberFunction1<Op,Vectorize,Func> >::type,
                             VectorizedMemberFunction1<Op,Vectorize,Func>
                         >::type member_func1_type;

        typedef typename if_<is_same<void,typename function_traits<Func>::result_type>,
                         VectorizedVoidMemberFunction2<Op,Vectorize,Func>,
                         VectorizedMemberFunction2<Op,Vectorize,Func> >::type member_func2_type;

        typedef typename if_<is_same<void,typename function_traits<Func>::result_type>,
                         boost::python::return_internal_reference<>,  // the void vectorizations return a reference to self
                         boost::python::default_call_policies>::type call_policies;

        typedef typename at<vector<
            int,  // unused, arity 0
            int,  // unused, arity 1 - first argument corresponds to the class type
            member_func1_type,
            member_func2_type
            >,
            long_<function_traits<Func>::arity> >::type vectorized_function_type;
        std::string doc = _name + vectorized_function_type::format_arguments(_args) + _doc;
        _cls.def(_name.c_str(),&vectorized_function_type::apply,doc.c_str(),_args,call_policies());
    }
};

template <class Op,class Cls,class Func,class Keywords>
member_function_binding<Op,Cls,Func,Keywords>
build_member_function_binding(Cls &cls,Func *func,const std::string &name,const std::string &doc,const Keywords &args)
{
    return member_function_binding<Op,Cls,Func,Keywords>(cls,name,doc,args);
}

template <class Op,class Cls,class Vectorizable,class Keywords>
struct generate_member_bindings_struct
{
    //BOOST_STATIC_ASSERT(size<Vectorizable>::value+1 == function_traits<Op::apply>::arity);
    static void apply(Cls &cls,const std::string &name,const std::string &doc,const Keywords &args) {
        for_each<typename allowable_vectorizations<Vectorizable>::type>(build_member_function_binding<Op>(cls,Op::apply,name,doc,args));
    }
};

template <class Op,class Cls,class Func>
void
generate_single_member_binding(Cls &cls,Func *func,const std::string &name,const std::string &doc)
{
    typedef typename if_<is_same<void,typename function_traits<Func>::result_type>,
                         VectorizedVoidMemberFunction0<Op,boost::mpl::vector<>,Func>,
                         VectorizedMemberFunction0<Op,boost::mpl::vector<>,Func> >::type vectorized_function_type;

    typedef typename if_<is_same<void,typename function_traits<Func>::result_type>,
                         boost::python::return_internal_reference<>,  // the void vectorizations return a reference to self
                         boost::python::default_call_policies>::type call_policies;

    cls.def(name.c_str(),&vectorized_function_type::apply,doc.c_str(),call_policies());
}

} // namespace detail

// TODO: update for arg("name")=default_value syntax
template <class Op,class Vectorizable0>
void generate_bindings(const std::string &name,const std::string &doc,const boost::python::detail::keywords<1> &args) {
    using namespace detail;
    generate_bindings_struct<Op,vector<Vectorizable0>,boost::python::detail::keywords<1> >::apply(name,doc,args);
}

template <class Op,class Vectorizable0, class Vectorizable1>
void generate_bindings(const std::string &name,const std::string &doc,const boost::python::detail::keywords<2> &args) {
    using namespace detail;
    generate_bindings_struct<Op,vector<Vectorizable0,Vectorizable1>,boost::python::detail::keywords<2> >::apply(name,doc,args);
}

template <class Op,class Vectorizable0, class Vectorizable1, class Vectorizable2>
void generate_bindings(const std::string &name,const std::string &doc,const boost::python::detail::keywords<3> &args) {
    using namespace detail;
    generate_bindings_struct<Op,vector<Vectorizable0,Vectorizable1,Vectorizable2>,boost::python::detail::keywords<3> >::apply(name,doc,args);
}

template <class Op,class Cls>
void
generate_member_bindings(Cls &cls,const std::string &name,const std::string &doc)
{
    using namespace detail;
    generate_single_member_binding<Op>(cls,&Op::apply,name,doc);
}

template <class Op,class Vectorizable0,class Cls>
void
generate_member_bindings(Cls &cls,const std::string &name,const std::string &doc,const boost::python::detail::keywords<1> &args)
{
    using boost::mpl::vector;
    detail::generate_member_bindings_struct<Op,Cls,vector<Vectorizable0>,boost::python::detail::keywords<1> >::apply(cls,name,doc,args);
}

template <class Op,class Vectorizable0,class Vectorizable1,class Cls>
void
generate_member_bindings(Cls &cls,const std::string &name,const std::string &doc,const boost::python::detail::keywords<2> &args)
{
    using boost::mpl::vector;
    detail::generate_member_bindings_struct<Op,Cls,vector<Vectorizable0,Vectorizable1>,boost::python::detail::keywords<2> >::apply(cls,name,doc,args);
}

} // namespace PyImath

#endif // _PyImathAutovectorize_h_
