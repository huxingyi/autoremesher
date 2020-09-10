///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 1998-2011, Industrial Light & Magic, a division of Lucas
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

#include "PyImathBasicTypes.h"
#include <Python.h>
#include <boost/python.hpp>
#include "PyImath.h"
#include "PyImathExport.h"
#include "PyImathFixedArray.h"
#include "PyImathFixedVArray.h"

using namespace boost::python;

namespace PyImath {

void
register_basicTypes()
{
    class_<BoolArray> bclass = BoolArray::register_("Fixed length array of bool");
    add_comparison_functions(bclass);

    class_<SignedCharArray> scclass = SignedCharArray::register_("Fixed length array of signed chars");
    add_arithmetic_math_functions(scclass);
    add_mod_math_functions(scclass);
    add_comparison_functions(scclass);
    add_ordered_comparison_functions(scclass);

    class_<UnsignedCharArray> ucclass = UnsignedCharArray::register_("Fixed length array of unsigned chars");
    add_arithmetic_math_functions(ucclass);
    add_mod_math_functions(ucclass);
    add_comparison_functions(ucclass);
    add_ordered_comparison_functions(ucclass);

    class_<ShortArray> sclass = ShortArray::register_("Fixed length array of shorts");
    add_arithmetic_math_functions(sclass);
    add_mod_math_functions(sclass);
    add_comparison_functions(sclass);
    add_ordered_comparison_functions(sclass);

    class_<UnsignedShortArray> usclass = UnsignedShortArray::register_("Fixed length array of unsigned shorts");
    add_arithmetic_math_functions(usclass);
    add_mod_math_functions(usclass);
    add_comparison_functions(usclass);
    add_ordered_comparison_functions(usclass);

    class_<IntArray> iclass = IntArray::register_("Fixed length array of ints");
    add_arithmetic_math_functions(iclass);
    add_mod_math_functions(iclass);
    add_comparison_functions(iclass);
    add_ordered_comparison_functions(iclass);
    add_explicit_construction_from_type<float>(iclass);
    add_explicit_construction_from_type<double>(iclass);

    class_<UnsignedIntArray> uiclass = UnsignedIntArray::register_("Fixed length array of unsigned ints");
    add_arithmetic_math_functions(uiclass);
    add_mod_math_functions(uiclass);
    add_comparison_functions(uiclass);
    add_ordered_comparison_functions(uiclass);
    add_explicit_construction_from_type<float>(uiclass);
    add_explicit_construction_from_type<double>(uiclass);

    class_<FloatArray> fclass = FloatArray::register_("Fixed length array of floats");
    add_arithmetic_math_functions(fclass);
    add_pow_math_functions(fclass);
    add_comparison_functions(fclass);
    add_ordered_comparison_functions(fclass);
    add_explicit_construction_from_type<int>(fclass);
    add_explicit_construction_from_type<double>(fclass);

    class_<DoubleArray> dclass = DoubleArray::register_("Fixed length array of doubles");
    add_arithmetic_math_functions(dclass);
    add_pow_math_functions(dclass);
    add_comparison_functions(dclass);
    add_ordered_comparison_functions(dclass);
    add_explicit_construction_from_type<int>(dclass);
    add_explicit_construction_from_type<float>(dclass);

    class_<VIntArray> ivclass = VIntArray::register_("Variable fixed length array of ints");
    // Don't add other functionality until its defined better.
}

} // namespace PyImath
