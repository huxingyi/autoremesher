///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2009-2014 DreamWorks Animation LLC. 
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
// *       Neither the name of DreamWorks Animation nor the names of
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

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <math.h>
#include <assert.h>

#include "half.h"
#include "compareDwa.h"

using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;

//
// Convert from linear to a nonlinear representation,
//

half
toNonlinear(half linear)
{
    float sign    = 1;
    float logBase = pow(2.7182818, 2.2);

    if ((float)linear < 0) {
        sign = -1;
    } 

    if ((linear.bits() & 0x7c00) == 0x7c00) {
        return (half)0.0;                   
    }

    if ( fabs( (float)linear ) <= 1.0f) {
        return (half)(sign * pow(fabs((float)linear), 1.f/2.2f));
    } else {
        return (half)(sign * ( log(fabs((float)linear)) / log(logBase) + 1.0f) );
    }

    return (half)0.0f;
}


void
compareDwa(int width, 
           int height,
           const Array2D<Rgba> &src,
           const Array2D<Rgba> &test,
           RgbaChannels channels)
{
    half  srcNonlin, testNonlin;
    float relError;

    for (int y=0; y<height; ++y) {
        for (int x=0; x<width; ++x) {

            for (int comp=0; comp<3; ++comp) {
                switch (comp) {
                    case 0:
                        if (!(channels & WRITE_R)) continue;

                        srcNonlin  = toNonlinear(src[y][x].r);
                        testNonlin = toNonlinear(test[y][x].r);
                        break;
                    case 1:
                        if (!(channels & WRITE_G)) continue;

                        srcNonlin  = toNonlinear(src[y][x].g);
                        testNonlin = toNonlinear(test[y][x].g);
                        break;
                    case 2:
                        if (!(channels & WRITE_B)) continue;

                        srcNonlin  = toNonlinear(src[y][x].b);
                        testNonlin = toNonlinear(test[y][x].b);
                        break;
                }

                //
                // Try to compare with relative error. This breaks down
                // for small numbers, which could be quantiezed to 0 
                // giving 100% error. 
                //
                if (srcNonlin.bits() != 0x00) {

                    relError = fabs( (float)srcNonlin - (float)testNonlin ) /
                                    fabs((float)srcNonlin);


                    if (fabs(srcNonlin) < .1) continue;
 
                    if (fabs(srcNonlin) < .25) {
                        assert( relError < .25);
                    } else {
                        assert( relError < .1);
                    }

                } else {
                    assert( srcNonlin != testNonlin );
                }
            }

        }
    }


    //
    // Test alpha, if necessary
    //
    if (channels & WRITE_A) {
        for (int y=0; y<height; ++y) {
            for (int x=0; x<width; ++x) {
                assert( src[y][x].a == test[y][x].a );
            }
        }
    }
}
