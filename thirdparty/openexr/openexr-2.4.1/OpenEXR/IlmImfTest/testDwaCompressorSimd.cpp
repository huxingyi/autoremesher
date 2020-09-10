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

#include <half.h>
#include <math.h>
#include <string.h>
#include <ImfDwaCompressorSimd.h>
#include <ImfSystemSpecific.h>
#include <ImathRandom.h>
#include <iostream>
#include <assert.h>

using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace std;


namespace
{

void
dumpBuffer (const SimdAlignedBuffer64f &buffer)
{
    for (int i=0; i<8; ++i) 
    {
        for (int j=0; j<8; ++j) 
        {
            cout << buffer._buffer[i*8+j] << "  ";
        }
        cout << endl;
    }
}

void
compareBuffer (const SimdAlignedBuffer64f &src,
               const SimdAlignedBuffer64f &dst,
               const float                 errThresh)
{
    for (int i=0; i<64; ++i) 
    {
        double diff   = fabs(src._buffer[i] - dst._buffer[i]);

        if (diff > errThresh) 
        {
            cout << scientific;
            cout << "Error exceeded threshold on element "  << i << endl;
            cout << " diff: " << diff << endl;
            cout << "Goal (src): " << scientific << endl;
            dumpBuffer(src);
            cout << "Test (dst): " << endl;
            dumpBuffer(dst);

            assert(false);
        }
    }
}

void
compareBufferRelative (const SimdAlignedBuffer64f &src,
                       const SimdAlignedBuffer64f &dst,
                       const float                 relErrThresh,
                       const float                 absErrThresh)
{
    for (int i=0; i<64; ++i)
    {
        double diff    = fabs(src._buffer[i] - dst._buffer[i]);
        double relDiff = diff / fabs(src._buffer[i]);

        if (relDiff > relErrThresh && diff > absErrThresh)
        {
            cout << scientific;
            cout << "Error exceeded threshold on element "  << i << endl;
            cout << " diff: " << diff << " relErr: " << fixed << 100.0*relDiff << " %" << endl;
            cout << "Goal (src): " << scientific << endl;
            dumpBuffer(src);
            cout << "Test (dst): " << endl;
            dumpBuffer(dst);

            assert(false);
        }
    }
}

// 
// Test that we can round trip CSC data with reasonable precision
//
void
testCsc()
{
    const int            numIter = 1000000;
    Rand48               rand48(0);
    SimdAlignedBuffer64f orig[3];
    SimdAlignedBuffer64f test[3];

    cout << "   Color Space Conversion Round Trip " << endl;
    cout << "      csc709Forward64() - 64 x csc709Inverse()" << endl;
    for (int iter=0; iter<numIter; ++iter)
    {   
        for (int i=0; i<64; ++i)
        {
            test[0]._buffer[i] = orig[0]._buffer[i] = rand48.nextf();
            test[1]._buffer[i] = orig[1]._buffer[i] = rand48.nextf();
            test[2]._buffer[i] = orig[2]._buffer[i] = rand48.nextf();
        }
        
        csc709Forward64(test[0]._buffer, test[1]._buffer, test[2]._buffer);
        for (int i=0; i<64; ++i)
        {
            csc709Inverse(test[0]._buffer[i], test[1]._buffer[i], test[2]._buffer[i]);
        }

        compareBuffer(orig[0], test[0], 1e-3);
        compareBuffer(orig[1], test[1], 1e-3);
        compareBuffer(orig[2], test[2], 1e-3);

    } // iter

    cout << "      csc709Forward64() - csc709Inverse64()" << endl;
    for (int iter=0; iter<numIter; ++iter)
    {    
        for (int i=0; i<64; ++i)
        {
            test[0]._buffer[i] = orig[0]._buffer[i] = rand48.nextf();
            test[1]._buffer[i] = orig[1]._buffer[i] = rand48.nextf();
            test[2]._buffer[i] = orig[2]._buffer[i] = rand48.nextf();
        }
        
        csc709Forward64(test[0]._buffer, test[1]._buffer, test[2]._buffer);
        csc709Inverse64(test[0]._buffer, test[1]._buffer, test[2]._buffer);

        compareBuffer(orig[0], test[0], 1e-3);
        compareBuffer(orig[1], test[1], 1e-3);
        compareBuffer(orig[2], test[2], 1e-3);

    } // iter
}

//
// Test interleaving two byte arrays
//
void
testInterleave()
{
    const int bufferLen     = 100000;
    const int randomNumIter = 10000;
    const int lengthNumIter = 128;
    Rand48    rand48(0);
    char     *srcA    = new char[bufferLen];
    char     *srcB    = new char[bufferLen];
    char     *dst     = new char[2*bufferLen];
    char     *test    = new char[2*bufferLen];
    
    cout << "   Byte Interleaving " << endl;

    for (int i=0; i<bufferLen; ++i)
    {
        srcA[i]    = (char)rand48.nextf(0.0, 255.0);
        srcB[i]    = (char)rand48.nextf(0.0, 255.0);
        dst[2*i]   = srcA[i];
        dst[2*i+1] = srcB[i];
    }

    for (int iter=0; iter<randomNumIter; ++iter)
    {
        memset(test, 0, 2*bufferLen);

        int offset = (int)rand48.nextf(0.0, bufferLen/2);
        int len    = (int)rand48.nextf(1.0, bufferLen - 1 - offset);

        interleaveByte2( test+2*offset, srcA+offset, srcB+offset, len);
        for (int i=0; i<len; ++i) {
            assert( test[2*offset + 2*i]     == dst[2*offset + 2*i]);
            assert( test[2*offset + 2*i + 1] == dst[2*offset + 2*i + 1]);
        }
    }

    //
    // Test increasing length buffers, with varying alignment
    // on all the buffers.
    //
    for (int len=1; len<lengthNumIter; ++len) 
    {
        for (int offset=0; offset<16*16*16; ++offset) 
        {
            int offsetA    =  offset        % 16;
            int offsetB    = (offset /  16) % 16;
            int offsetTest = (offset / 256) % 16;

            memset(srcA, 255, bufferLen);
            memset(srcB, 255, bufferLen);
            memset(dst,  0,   2*bufferLen);
            memset(test, 0,   2*bufferLen);

            char *a   = srcA + offsetA;
            char *b   = srcB + offsetB;
            char *out = test + offsetTest;
            
            for (int i=0; i<len; ++i) 
            {
                a[i] = (char)rand48.nextf(0.0, 255.0);
                b[i] = (char)rand48.nextf(0.0, 255.0);

                dst[2*i]   = a[i];
                dst[2*i+1] = b[i];
            }

            interleaveByte2(out, a, b, len);
            
            for (int i=0; i<2*len+8; ++i) 
            {
                assert( dst[2*i]   == out[2*i] );
                assert( dst[2*i+1] == out[2*i+1] );
            }
        }
    }

    delete[] srcA;
    delete[] srcB;
    delete[] dst;
    delete[] test;
}

//
// Test that we can route trip DCT data with reasonable precision
//
void
testDct()
{
    const int            numIter = 1000000;
    Rand48               rand48(0);
    SimdAlignedBuffer64f orig;
    SimdAlignedBuffer64f test;
   
    cout << "   DCT Round Trip " << endl;
    for (int iter=0; iter<numIter; ++iter) 
    {
        for (int i=0; i<64; ++i) 
        {
            orig._buffer[i] = test._buffer[i] = rand48.nextf();
        }

        dctForward8x8(test._buffer);
        dctInverse8x8_scalar<0>(test._buffer);

        compareBufferRelative(orig, test, .02, 1e-3);
    } 

    cout << "      Inverse, DC Only" << endl;
    for (int iter=0; iter<numIter; ++iter) 
    {
        orig._buffer[0] = test._buffer[0] = rand48.nextf();
        for (int i=1; i<64; ++i) 
        {
            orig._buffer[i] = test._buffer[i] = 0;
        }

        dctInverse8x8_scalar<0>(orig._buffer);
        dctInverse8x8DcOnly(test._buffer);

        compareBufferRelative(orig, test, .01, 1e-6);
    } 


#define INVERSE_DCT_SCALAR_TEST_N(_func, _n, _desc)                \
    cout << "         " << _desc << endl;                          \
    for (int iter=0; iter<numIter; ++iter)                         \
    {                                                              \
        for (int i=0; i<64; ++i)                                   \
        {                                                          \
            if (i < 8*(8-_n))                                      \
            {                                                      \
               orig._buffer[i] = test._buffer[i] = rand48.nextf(); \
            } else {                                               \
               orig._buffer[i] = test._buffer[i] = 0;              \
            }                                                      \
        }                                                          \
        dctInverse8x8_scalar<0>(orig._buffer);                     \
        _func<_n>(test._buffer);                                   \
        compareBufferRelative(orig, test, .01, 1e-6);              \
    }

    cout << "      Inverse, Scalar: " << endl;
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 0, "8x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 1, "7x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 2, "6x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 3, "5x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 4, "4x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 5, "3x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 6, "2x8")
    INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_scalar, 7, "1x8")

    CpuId cpuid;
    if (cpuid.sse2) 
    {
        cout << "      Inverse, SSE2: " << endl;
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 0, "8x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 1, "7x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 2, "6x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 3, "5x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 4, "4x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 5, "3x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 6, "2x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_sse2, 7, "1x8")
    }

    if (cpuid.avx) 
    {
        cout << "      Inverse, AVX: " << endl;
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 0, "8x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 1, "7x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 2, "6x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 3, "5x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 4, "4x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 5, "3x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 6, "2x8")
        INVERSE_DCT_SCALAR_TEST_N(dctInverse8x8_avx, 7, "1x8")
    }
}

//
// Test FLOAT -> HALF conversion, mostly for F16C enabled processors
//
void
testFloatToHalf()
{
    cout << "   FLOAT -> HALF conversion" << endl;

    const int             numIter = 1000000;
    Rand48                rand48(0);
    SimdAlignedBuffer64f  src;
    SimdAlignedBuffer64us dst;

    cout << "      convertFloatToHalf64_scalar()" << endl;
    for (int iter=0; iter<numIter; ++iter)
    {
        for (int i=0; i<64; ++i)
        {
            if (i < 32)
            {
                src._buffer[i] = (float)140000*(rand48.nextf()-.5);
            } else
            {
                src._buffer[i] = (float)(rand48.nextf()-.5);
            }
            dst._buffer[i] = 0;
        }

        convertFloatToHalf64_scalar(dst._buffer, src._buffer);

        for (int i=0; i<64; ++i)
        {
            half value = (half)src._buffer[i];
            if (value.bits() != dst._buffer[i])
            {
                cout << src._buffer[i] << " -> " << dst._buffer[i] 
                                 << " expected " << value.bits() << endl;
                assert(false);
            }
        }
    }


    CpuId cpuid;
    if (cpuid.avx && cpuid.f16c)
    {
        cout << "      convertFloatToHalf64_f16c()" << endl;
        for (int iter=0; iter<numIter; ++iter)
        {
            for (int i=0; i<64; ++i)
            {
                if (i < 32)
                {
                    src._buffer[i] = (float)140000*(rand48.nextf()-.5);
                } 
                else
                {
                    src._buffer[i] = (float)(rand48.nextf()-.5);
                }
                dst._buffer[i] = 0;
            }

            convertFloatToHalf64_f16c(dst._buffer, src._buffer);

            for (int i=0; i<64; ++i)
            {
                half value = (half)src._buffer[i];
                if (value.bits() != dst._buffer[i])
                {
                    cout << src._buffer[i] << " -> " << dst._buffer[i] 
                                     << " expected " << value.bits() << endl;
                    assert(false);
                }
            }
        }
    }
}

//
// Test ZigZag reordering + HALF -> FLOAT conversion
//
void
testFromHalfZigZag()
{
    SimdAlignedBuffer64us src;
    SimdAlignedBuffer64f  dst;

    cout << "   ZigZag re-ordering with HALF -> FLOAT conversion" << endl;

    // First off, simple check to see that the reordering is working
    // This pattern, when converted, should give 0.0 - 63.0 as floats
    // in order.
    unsigned short pattern[] = {
        0x0000, 0x3c00, 0x4800, 0x4c00, 0x4880, 0x4000, 0x4200, 0x4900,
        0x4c40, 0x4e00, 0x5000, 0x4e40, 0x4c80, 0x4980, 0x4400, 0x4500,
        0x4a00, 0x4cc0, 0x4e80, 0x5020, 0x5100, 0x5200, 0x5120, 0x5040,
        0x4ec0, 0x4d00, 0x4a80, 0x4600, 0x4700, 0x4b00, 0x4d40, 0x4f00,
        0x5060, 0x5140, 0x5220, 0x5300, 0x5320, 0x5240, 0x5160, 0x5080,
        0x4f40, 0x4d80, 0x4b80, 0x4dc0, 0x4f80, 0x50a0, 0x5180, 0x5260,
        0x5340, 0x5360, 0x5280, 0x51a0, 0x50c0, 0x4fc0, 0x50e0, 0x51c0,
        0x52a0, 0x5380, 0x53a0, 0x52c0, 0x51e0, 0x52e0, 0x53c0, 0x53e0
    };

    cout << "      fromHalfZigZag_scaler()" << endl;
    for (int i=0; i<64; ++i)
    {
        src._buffer[i] = pattern[i];
    }
    fromHalfZigZag_scalar(src._buffer, dst._buffer);
    for (int i=0; i<64; ++i)
    {
        if ( fabsf(dst._buffer[i] - (float)i) > 1e-5 )
        {
            cout << "At index " << i << ": ";
            cout << "expecting " << (float)i << "; got " << dst._buffer[i] << endl;
            assert(false);
        }
    }
       
    // Then compare the two implementations, if supported
    CpuId cpuid;
    if (cpuid.avx && cpuid.f16c)
    {
        const int             numIter = 1000000;
        Rand48                rand48(0);
        half                  h;
        SimdAlignedBuffer64f  dstF16c;

        cout << "      fromHalfZigZag_f16c()" << endl;

        for (int iter=0; iter<numIter; ++iter)
        {
            for (int i=0; i<64; ++i)
            {
                if (i < 32)
                {
                    h = (half)(140000.*(rand48.nextf() - .5));
                }
                else 
                {
                    h = (half)(rand48.nextf() - .5);
                }
                src._buffer[i] = h.bits();
            }

            fromHalfZigZag_scalar(src._buffer, dst._buffer);
            fromHalfZigZag_f16c(src._buffer, dstF16c._buffer);

            for (int i=0; i<64; ++i)
            {
                if ( fabsf(dst._buffer[i] - dstF16c._buffer[i]) > 1e-5 )
                {
                    cout << "At index " << i << ": ";
                    cout << "expecting " << dst._buffer[i] << "; got "
                         << dstF16c._buffer[i] << endl;
                    assert(false);
                }
            }
        } // iter
    } // f16c
}


} // namespace

void 
testDwaCompressorSimd (const string&)
{
    cout << "SIMD helper functions for DwaCompressor:" << endl;

    try
    {
    
        testCsc();
        testInterleave();
        testFloatToHalf();
        testFromHalfZigZag();

        testDct();

    }
    catch (const exception &e)
    {
        cout << "unexpected exception: " << e.what() << endl;
        assert (false);
    }
    catch (...)
    {
        cout << "unexpected exception" << endl;
        assert (false);
    }

    cout << "ok\n" << endl;
}
