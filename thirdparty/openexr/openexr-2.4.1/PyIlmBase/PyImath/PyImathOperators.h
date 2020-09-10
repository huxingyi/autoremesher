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

#ifndef _PyImathOperators_h_
#define _PyImathOperators_h_

#include "PyImathFixedArray.h"
#include "PyImathAutovectorize.h"

namespace PyImath {

template <class T1, class T2=T1, class Ret=T1>
struct op_add {
    static inline Ret apply(const T1 &a, const T2 &b) { return a+b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_sub {
    static inline Ret apply(const T1 &a, const T2 &b) { return a-b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_rsub {
    static inline Ret apply(const T1 &a, const T2 &b) { return b-a; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_mul {
    static inline Ret apply(const T1 &a, const T2 &b) { return a*b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_div {
    static inline Ret apply(const T1 &a, const T2 &b) { return a/b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_mod {
    static inline Ret apply(const T1 &a, const T2 &b) { return a%b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_pow {
    static inline Ret apply(const T1 &a, const T2 &b) { return std::pow(a,b); }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_rpow {
    static inline Ret apply(const T1 &a, const T2 &b) { return std::pow(b,a); }
};

template <class T1, class Ret=T1>
struct op_neg {
    static inline Ret apply(const T1 &a) { return -a; }
};

template <class T1, class Ret=T1>
struct op_abs {
    static inline Ret apply(const T1 &a) { return std::abs(a); }
};

template <class T1, class Ret=T1>
struct op_inverse {
    static inline Ret apply(const T1 &a) { return ~a; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_lshift {
    static inline Ret apply(const T1 &a, const T2 &b) { return a << b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_rshift {
    static inline Ret apply(const T1 &a, const T2 &b) { return a >> b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_bitand {
    static inline Ret apply(const T1 &a, const T2 &b) { return a & b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_xor {
    static inline Ret apply(const T1 &a, const T2 &b) { return a ^ b; }
};

template <class T1, class T2=T1, class Ret=T1>
struct op_bitor {
    static inline Ret apply(const T1 &a, const T2 &b) { return a | b; }
};

template <class T1, class T2=T1>
struct op_iadd {
    static inline void apply(T1 &a, const T2 &b) { a += b; }
};

template <class T1, class T2=T1>
struct op_isub {
    static inline void apply(T1 &a, const T2 &b) { a -= b; }
};

template <class T1, class T2=T1>
struct op_imul {
    static inline void apply(T1 &a, const T2 &b) { a *= b; }
};

template <class T1, class T2=T1>
struct op_idiv {
    static inline void apply(T1 &a, const T2 &b) { a /= b; }
};

template <class T1, class T2=T1>
struct op_imod {
    static inline void apply(T1 &a, const T2 &b) { a %= b; }
};

template <class T1, class T2=T1>
struct op_ipow {
    static inline void apply(T1 &a, const T2 &b) { a = std::pow(a,b); }
};

template <class T1, class T2=T1>
struct op_ilshift {
    static inline void apply(T1 &a, const T2 &b) { a <<= b; }
};

template <class T1, class T2=T1>
struct op_irshift {
    static inline void apply(T1 &a, const T2 &b) { a >>= b; }
};

template <class T1, class T2=T1>
struct op_ixor {
    static inline void apply(T1 &a, const T2 &b) { a ^= b; }
};

template <class T1, class T2=T1>
struct op_ibitand {
    static inline void apply(T1 &a, const T2 &b) { a &= b; }
};

template <class T1, class T2=T1>
struct op_ibitor {
    static inline void apply(T1 &a, const T2 &b) { a |= b; }
};

// the logical function return values default to 'int' for use
// as mask arrays.
template <class T1, class T2=T1, class Ret=int>
struct op_lt {
    static inline Ret apply(const T1 &a, const T2 &b) { return a < b; }
};

template <class T1, class T2=T1, class Ret=int>
struct op_gt {
    static inline Ret apply(const T1 &a, const T2 &b) { return a > b; }
};

template <class T1, class T2=T1, class Ret=int>
struct op_le {
    static inline Ret apply(const T1 &a, const T2 &b) { return a <= b; }
};

template <class T1, class T2=T1, class Ret=int>
struct op_ge {
    static inline Ret apply(const T1 &a, const T2 &b) { return a >= b; }
};

template <class T1, class T2=T1, class Ret=int>
struct op_eq {
    static inline Ret apply(const T1 &a, const T2 &b) { return a == b; }
};

template <class T1, class T2=T1, class Ret=int>
struct op_ne {
    static inline Ret apply(const T1 &a, const T2 &b) { return a != b; }
};

template <class T>
static T fa_reduce(const FixedArray<T> &a) {
    T tmp(T(0)); // should use default construction but V3f doens't initialize
    size_t len = a.len();
    for (size_t i=0; i < len; ++i) tmp += a[i];
    return tmp;
}

template <class T>
static T fa_min(const FixedArray<T> &a) {
    T tmp(T(0));
    size_t len = a.len();
    if (len > 0)
        tmp = a[0];
    for (size_t i=1; i < len; ++i)
        if (a[i] < tmp)
            tmp = a[i];
    return tmp;
}

template <class T>
static T fa_max(const FixedArray<T> &a) {
    T tmp(T(0));
    size_t len = a.len();
    if (len > 0)
        tmp = a[0];
    for (size_t i=1; i < len; ++i)
        if (a[i] > tmp)
            tmp = a[i];
    return tmp;
}

template <class T>
static void add_arithmetic_math_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    using boost::mpl::false_;
    generate_member_bindings<op_add<T>, true_ >(c,"__add__", "self+x", boost::python::args("x"));
    generate_member_bindings<op_add<T>, false_>(c,"__radd__","x+self", boost::python::args("x"));
    generate_member_bindings<op_sub<T>, true_ >(c,"__sub__", "self-x", boost::python::args("x"));
    generate_member_bindings<op_rsub<T>,false_>(c,"__rsub__","x-self", boost::python::args("x"));
    generate_member_bindings<op_mul<T>, true_ >(c,"__mul__", "self*x", boost::python::args("x"));
    generate_member_bindings<op_mul<T>, false_>(c,"__rmul__","x*self", boost::python::args("x"));
    generate_member_bindings<op_div<T>, true_ >(c,"__div__", "self/x", boost::python::args("x"));
    generate_member_bindings<op_div<T>, true_ >(c,"__truediv__", "self/x", boost::python::args("x"));
    generate_member_bindings<op_neg<T> >(c,"__neg__", "-x");
    generate_member_bindings<op_iadd<T>,true_ >(c,"__iadd__","self+=x",boost::python::args("x"));
    generate_member_bindings<op_isub<T>,true_ >(c,"__isub__","self-=x",boost::python::args("x"));
    generate_member_bindings<op_imul<T>,true_ >(c,"__imul__","self*=x",boost::python::args("x"));
    generate_member_bindings<op_idiv<T>,true_ >(c,"__idiv__","self/=x",boost::python::args("x"));
    generate_member_bindings<op_idiv<T>,true_ >(c,"__itruediv__","self/=x",boost::python::args("x"));

    c.def("reduce",&fa_reduce<T>);
}

template <class T>
static void add_reduction_functions(boost::python::class_<FixedArray<T> > &c) {
    c.def("min",&fa_min<T>);
    c.def("max",&fa_max<T>);
}

template <class T>
static void add_pow_math_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    using boost::mpl::false_;
    generate_member_bindings<op_pow<T>, true_ >(c,"__pow__", "self**x", boost::python::args("x"));
    generate_member_bindings<op_rpow<T>,false_>(c,"__rpow__","x**self", boost::python::args("x"));
    generate_member_bindings<op_ipow<T>,true_ >(c,"__ipow__","x**=self",boost::python::args("x"));
}

template <class T>
static void add_mod_math_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    generate_member_bindings<op_mod<T>, true_>(c,"__mod__", "self%x", boost::python::args("x"));
    generate_member_bindings<op_imod<T>,true_>(c,"__imod__","self%=x",boost::python::args("x"));
}

template <class T>
static void add_shift_math_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    generate_member_bindings<op_lshift<T>, true_>(c,"__lshift__", "self<<x", boost::python::args("x"));
    generate_member_bindings<op_ilshift<T>,true_>(c,"__ilshift__","self<<=x",boost::python::args("x"));
    generate_member_bindings<op_rshift<T>, true_>(c,"__rshift__", "self>>x", boost::python::args("x"));
    generate_member_bindings<op_irshift<T>,true_>(c,"__irshift__","self>>=x",boost::python::args("x"));
}

template <class T>
static void add_bitwise_math_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    generate_member_bindings<op_bitand<T>, true_>(c,"__and__", "self&x", boost::python::args("x"));
    generate_member_bindings<op_ibitand<T>,true_>(c,"__iand__","self&=x",boost::python::args("x"));
    generate_member_bindings<op_bitor<T>,  true_>(c,"__or__",  "self|x", boost::python::args("x"));
    generate_member_bindings<op_ibitor<T>, true_>(c,"__ior__", "self|=x",boost::python::args("x"));
    generate_member_bindings<op_xor<T>,    true_>(c,"__xor__", "self^x", boost::python::args("x"));
    generate_member_bindings<op_ixor<T>,   true_>(c,"__ixor__","self^=x",boost::python::args("x"));
}

template <class T>
static void add_comparison_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    generate_member_bindings<op_eq<T>, true_>(c,"__eq__","self==x",boost::python::args("x"));
    generate_member_bindings<op_ne<T>, true_>(c,"__ne__","self!=x",boost::python::args("x"));
}

template <class T>
static void add_ordered_comparison_functions(boost::python::class_<FixedArray<T> > &c) {
    using boost::mpl::true_;
    generate_member_bindings<op_lt<T>, true_>(c,"__lt__","self<x", boost::python::args("x"));
    generate_member_bindings<op_le<T>, true_>(c,"__le__","self<=x",boost::python::args("x"));
    generate_member_bindings<op_gt<T>, true_>(c,"__gt__","self>x", boost::python::args("x"));
    generate_member_bindings<op_ge<T>, true_>(c,"__ge__","self>=x",boost::python::args("x"));
}

template <class S,class T>
static void add_explicit_construction_from_type(boost::python::class_<FixedArray<T> > &c) {
    using namespace boost::python;
    c.def(init<FixedArray<S> >("copy contents of other array into this one"));
}

}

#endif // _PyImathOperators_h_
