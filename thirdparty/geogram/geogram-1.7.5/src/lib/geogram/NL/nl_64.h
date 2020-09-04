/*
 *  Copyright (c) 2004-2014, Bruno Levy
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

/*
 * Wrappers to avoid warnings when using OpenNL functions with 64 bit 
 * indices. Note: dimension of matrix still limited to 32 bit indices
 * (but not NNZ).
 */

#ifndef NL_64_H
#define NL_64_H

#if defined(__cplusplus) && defined(GARGANTUA)

#include <limits>
#include <cassert>

inline NLuint nlTo32(NLulong x) {
    assert(x <= NLulong(std::numeric_limits<NLuint>::max()));
    return NLuint(x);
}

inline double nlGetVariable(NLulong i) {
    return nlGetVariable(nlTo32(i));
}

inline void nlSetVariable(NLulong i, double a) {
    nlSetVariable(nlTo32(i), a);
}

inline void nlLockVariable(NLulong i) {
    nlLockVariable(nlTo32(i));
}

inline NLboolean nlVariableIsLocked(NLulong index) {
    return nlVariableIsLocked(nlTo32(index));
}

inline void nlCoefficient(NLulong i, double a) {
    nlCoefficient(nlTo32(i), a);
}

inline void nlAddIJCoefficient(NLulong i, NLulong j, double a) {
    nlAddIJCoefficient(nlTo32(i), nlTo32(j), a);
}

inline double nlGetEigenValue(int i) {
    return nlGetEigenValue(NLuint(i));
}

inline double nlGetEigenValue(NLulong i) {
    return nlGetEigenValue(nlTo32(i));
}

inline double nlMultiGetVariable(NLulong i, NLulong j) {
    return nlMultiGetVariable(nlTo32(i),nlTo32(j));
}

#endif

#endif

