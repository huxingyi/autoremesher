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

#include "PyImathAutovectorize.h"


namespace PyImath {

namespace detail {
//
// cheek possible vectorizations to ensure correctness
//
// single argument should be ((false),(true))
//
BOOST_STATIC_ASSERT(( size<possible_vectorizations<1>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( size<at_c<possible_vectorizations<1>::type,0>::type>::type::value == 1 ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<1>::type,0>::type,0>::type::value == false ));
BOOST_STATIC_ASSERT(( size<at_c<possible_vectorizations<1>::type,1>::type>::type::value == 1 ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<1>::type,1>::type,0>::type::value == true ));

//
// two argument should be ((false,false),(false,true),(true,false),(true,true))
//
BOOST_STATIC_ASSERT(( size<possible_vectorizations<2>::type>::type::value == 4 ));
BOOST_STATIC_ASSERT(( size<at_c<possible_vectorizations<2>::type,0>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,0>::type,0>::type::value == false ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,0>::type,1>::type::value == false ));
BOOST_STATIC_ASSERT(( size<at_c<possible_vectorizations<2>::type,1>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,1>::type,0>::type::value == false ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,1>::type,1>::type::value == true ));
BOOST_STATIC_ASSERT(( size<at_c<possible_vectorizations<2>::type,2>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,2>::type,0>::type::value == true ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,2>::type,1>::type::value == false ));
BOOST_STATIC_ASSERT(( size<at_c<possible_vectorizations<2>::type,3>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,3>::type,0>::type::value == true ));
BOOST_STATIC_ASSERT(( at_c<at_c<possible_vectorizations<2>::type,3>::type,1>::type::value == true ));
BOOST_STATIC_ASSERT(( size<possible_vectorizations<3>::type>::type::value == 8 ));

//
// Check disallow_vectorization for given vectorizable flags
//
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ > >::apply<vector<true_ > >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ > >::apply<vector<false_> >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_> >::apply<vector<true_ > >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_> >::apply<vector<false_> >::type::value == false  ));

BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , true_ > >::apply<vector<true_ , true_ > >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , true_ > >::apply<vector<false_, true_ > >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , true_ > >::apply<vector<true_ , false_> >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , true_ > >::apply<vector<false_, false_> >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , false_> >::apply<vector<true_ , true_ > >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , false_> >::apply<vector<false_, true_ > >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , false_> >::apply<vector<true_ , false_> >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<true_ , false_> >::apply<vector<false_, false_> >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, true_ > >::apply<vector<true_ , true_ > >::type::value == true   ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, true_ > >::apply<vector<false_, true_ > >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, true_ > >::apply<vector<true_ , false_> >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, true_ > >::apply<vector<false_, false_> >::type::value == false  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, false_> >::apply<vector<true_ , true_ > >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, false_> >::apply<vector<false_, true_ > >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, false_> >::apply<vector<true_ , false_> >::type::value == true  ));
BOOST_STATIC_ASSERT(( disallow_vectorization<vector<false_, false_> >::apply<vector<false_, false_> >::type::value == false  ));

//
// Check allowable_vectorizations, single argument not vectorizable, and two argument second argument vectorizable.
//
typedef allowable_vectorizations<vector<false_> > AV1f;
BOOST_STATIC_ASSERT(( size<AV1f::possible>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV1f::possible,0>::type,0>::type::value == false ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV1f::possible,1>::type,0>::type::value == true ));
BOOST_STATIC_ASSERT(( size<AV1f::type>::type::value == 1 ));
BOOST_STATIC_ASSERT(( size<at_c<AV1f::type,0>::type>::type::value == 1 ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV1f::type,0>::type,0>::type::value == false ));

typedef allowable_vectorizations<vector<false_,true_> > AV2ft;
BOOST_STATIC_ASSERT(( size<AV2ft::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( size<at_c<AV2ft::type,0>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV2ft::type,0>::type,0>::type::value == false ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV2ft::type,0>::type,1>::type::value == false ));
BOOST_STATIC_ASSERT(( size<at_c<AV2ft::type,1>::type>::type::value == 2 ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV2ft::type,1>::type,0>::type::value == false ));
BOOST_STATIC_ASSERT(( at_c<at_c<AV2ft::type,1>::type,1>::type::value == true ));

} // namespace detail

} // namespace PyImath
