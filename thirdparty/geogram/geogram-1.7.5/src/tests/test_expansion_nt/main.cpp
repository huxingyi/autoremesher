/*
 *  Copyright (c) 2012-2014, Bruno Levy
 *  All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *
 *  * Redistributions of source code must retain the above copyright notice,
 *  this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright notice,
 *  this list of conditions and the following disclaimer in the documentation
 *  and/or other materials provided with the distribution.
 *  * Neither the name of the ALICE Project-Team nor the names of its
 *  contributors may be used to endorse or promote products derived from this
 *  software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 *  LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 *  INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 *  CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 *  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 *
 *  If you modify this software, you should include a notice giving the
 *  name of the person performing the modification, the date of modification,
 *  and the reason for such modification.
 *
 *  Contact: Bruno Levy
 *
 *     Bruno.Levy@inria.fr
 *     http://www.loria.fr/~levy
 *
 *     ALICE Project
 *     LORIA, INRIA Lorraine, 
 *     Campus Scientifique, BP 239
 *     54506 VANDOEUVRE LES NANCY CEDEX 
 *     FRANCE
 *
 */

#include <geogram/numerics/expansion_nt.h>
#include <iostream>

/**
 * \brief Outputs an expansion_nt to a stream.
 * \param out a reference to the output stream
 * \param x a const reference to the expansion_nt to be printed
 */
static void print(std::ostream& out, const GEO::expansion_nt& x) {
    out << "expansion_nt(estimate="
        << x.estimate();
    out << ", components=[";
    for(GEO::index_t i=0; i<x.length(); ++i) {
        out << x.component(i);
        if(i != x.length()-1) {
            out << " ";
        }
    }
    out << "]";
    out << ")";
}

/**
 * \brief Outputs a rational_nt to a stream.
 * \param out a reference to the output stream
 * \param x a const reference to the expansion_nt to be printed
 */
static void print(std::ostream& out, const GEO::rational_nt& x) {
    out << "estimate=" << x.estimate() << ":";
    print(out,x.num());
    out << " / ";
    print(out,x.denom());
}

/**
 * \brief Outputs a double precision number to a stream.
 * \details this function is here so that the same generic code
 *  can be used with double and with expansion to show the difference.
 * \param out a reference to the output stream
 * \param x the double-precision number to be printed
 */
static void print(std::ostream& out, double x) {
    out << x;
}

/**
 * \brief performs a simple computation designed to
 *  give an errouenous result when using doubles.
 * \param zzz an ignored parameter, just there to 
 *  specify the type to be used for computations, i.e.
 *  use a double to test with doubles, and an expansion_nt 
 *  to test with expansion_nt.
 */
template <class T> inline void compute(const T& zzz) {
    GEO::geo_argused(zzz);

    T r = T(1e-30)+T(5.0)+T(1e30)+T(2e-30)-T(1e30);
    std::cout << "   sign(1e-30 + 5.0 + 1e30 + 2e-30 - 1e30) = "
              << GEO::geo_sgn(r) << std::endl;
    std::cout << "   result = ";
    print(std::cout,r);
    std::cout << std::endl;
}


template <class T> inline void compute2(const T& zzz) {
    GEO::geo_argused(zzz);

    T r = T(1e-30) / T(3.0) + T(5.0) / T(3.0) + T(1e30) / T(3.0) + T(2e-30) / T(3.0) - T(1e30) / T(3.0);
    std::cout << "   sign(1e-30/3.0 + 5.0/3.0 + 1e30/3.0 + 2e-30/3.0 - 1e30/3.0) = "
              << GEO::geo_sgn(r) << std::endl;
    std::cout << "   result = ";
    print(std::cout,r);
    std::cout << std::endl;
}


int main() {
    //   This function needs to be called before
    // using expansion_nt.
    GEO::expansion::initialize();

    std::cout << "Using double:" << std::endl;
    compute(double());
    
    std::cout << "Using expansion_nt:" << std::endl;    
    compute(GEO::expansion_nt());

    std::cout << "Using rational_nt:" << std::endl;    
    compute(GEO::rational_nt());

    std::cout << "With divisions:" << std::endl;
    
    std::cout << "Using double:" << std::endl;
    compute2(double());

    std::cout << "Using rational_nt:" << std::endl;    
    compute2(GEO::rational_nt());

    return 0;
}
