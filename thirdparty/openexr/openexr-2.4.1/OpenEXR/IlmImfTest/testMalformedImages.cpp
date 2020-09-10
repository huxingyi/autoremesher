///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2002-2012, Industrial Light & Magic, a division of Lucas
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

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <ImfAcesFile.h>
#include <ImfArray.h>
#include <ImfRgbaFile.h>
#include <IlmThread.h>
#include <stdio.h>
#include <assert.h>

#ifndef ILM_IMF_TEST_IMAGEDIR
    #define ILM_IMF_TEST_IMAGEDIR
#endif


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
readImage (const char inFileName[])
{
	Array2D<Rgba> p;
	Header h;
	Box2i dw;
	int width;
	int height;

	{
		cout << "Reading file " << inFileName << endl;

		AcesInputFile in (inFileName);

		h = in.header();
		dw = h.dataWindow();

		width  = dw.max.x - dw.min.x + 1;
		height = dw.max.y - dw.min.y + 1;
		p.resizeErase (height, width);

		in.setFrameBuffer (&p[0][0] - dw.min.x - dw.min.y * width, 1, width);
		in.readPixels (dw.min.y, dw.max.y);
	}
}

void
readBadBoundsImage (const char fileName[])
{
    cout << "file " << fileName << " " << flush;

    OPENEXR_IMF_NAMESPACE::RgbaInputFile in (fileName);

    cout << "version " << in.version() << " " << flush;

    //const Box2i &dw = in.dataWindow();
}

} // namespace

void
testMalformedImages (const std::string&)
{
    bool caught;

    try
    {
        // id:000012,sig:11,src:000328+001154,op:splice,rep:16
        caught = false;
        readImage (ILM_IMF_TEST_IMAGEDIR "comp_short_decode_piz.exr");
        cerr << "Malformed Images : InputFile : incorrect input file passed\n";
        assert (false);
    }
    catch (const IEX_NAMESPACE::BaseExc &e)
    {
        // expected behaviour
        caught = true;
    }
    assert (caught);

    try
    {
        // id:000077,sig:11,src:002575,op:havoc,rep:4
        caught = false;
        readImage (ILM_IMF_TEST_IMAGEDIR "comp_invalid_unknown.exr");
        cerr << "Malformed Images : InputFile : incorrect input file passed\n";
        assert (false);
    }
    catch (const IEX_NAMESPACE::IoExc &e)
    {
        // expected behaviour
        caught = true;
    }
    assert (caught);

    try
    {
        // id:000103,sig:11,src:002037+004745,op:splice,rep:2
        caught = false;
        readImage (ILM_IMF_TEST_IMAGEDIR "comp_early_eof_piz.exr");
        cerr << "Malformed Images : InputFile : incorrect input file passed\n";
        assert (false);
    }
    catch (const IEX_NAMESPACE::InputExc &e)
    {
        // expected behaviour
        caught = true;
    }
    assert (caught);

    // The files below expose a bug in the test code (readImage which uses the
    // logic taken from exr2aces) that calculates an invalid pointer for the
    // framebuffer.  The dataWindow and displayWindow values used in these files
    // seem valid based on a cursory reading of the OpenEXR specification. As
    // such, the best we can do is ensure that parsing the basic header
    // information doesn't cause any unexpected exceptions.

    // id:000087,sig:11,src:000562+000300,op:splice,rep:2
    readBadBoundsImage (ILM_IMF_TEST_IMAGEDIR "comp_bad_pos_bounds_piz.exr");

    // id:000104,sig:11,src:001329+000334,op:splice,rep:2
    readBadBoundsImage (ILM_IMF_TEST_IMAGEDIR "comp_bad_pos_bounds_pxr24.exr");

    // id:000131,sig:11,src:000514+002831,op:splice,rep:16
    readBadBoundsImage (ILM_IMF_TEST_IMAGEDIR "comp_bad_neg_bounds_pxr24.exr");

    // id:000132,sig:11,src:000895,op:havoc,rep:32
    readBadBoundsImage (ILM_IMF_TEST_IMAGEDIR "comp_bad_bounds_piz.exr");
}
