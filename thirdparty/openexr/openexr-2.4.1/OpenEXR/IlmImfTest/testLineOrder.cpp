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

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImfThreading.h>
#include "IlmThread.h"
#include "half.h"

#include <stdio.h>
#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
fillPixels (Array2D<half> &ph, int width, int height)
{
    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	    ph[y][x] = x % 10 + 10 * (y % 17);
}


void
writeRead (const Array2D<half> &ph1,
	   const char fileName[],
	   int width,
	   int height,
	   LineOrder lorder)
{
    //
    // Write the pixel data in ph1 to an image file using
    // the specified line order.  Read the pixel data back
    // from the file in pseudo-random order and verify that
    // the data did not change.
    //

    cout << "line order " << lorder <<
	    ":" << flush;

    Header hdr (width, height);
    hdr.lineOrder() = lorder;

    hdr.channels().insert ("H",				// name
			   Channel (HALF,		// type
				    1,			// xSampling
				    1)			// ySampling
			  );

    {
	FrameBuffer fb; 

	fb.insert ("H",					// name
		   Slice (HALF,				// type
			  (char *) &ph1[0][0],		// base
			  sizeof (ph1[0][0]), 		// xStride
			  sizeof (ph1[0][0]) * width,	// yStride
			  1,				// xSampling
			  1)				// ySampling
		  );
	
	cout << " writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    {
	cout << " reading" << flush;

	InputFile in (fileName);
	
	const Box2i &dw = in.header().dataWindow();
	int w = dw.max.x - dw.min.x + 1;
	int h = dw.max.y - dw.min.y + 1;
	int dx = dw.min.x;
	int dy = dw.min.y;

	Array2D<half> ph2 (h, w);

	FrameBuffer fb;

	fb.insert ("H",					// name
		   Slice (HALF,				// type
			  (char *) &ph2[-dy][-dx],	// base
			  sizeof (ph2[0][0]), 		// xStride
			  sizeof (ph2[0][0]) * w,	// yStride
			  1,				// xSampling
			  1)				// ySampling
		  );
	    
	in.setFrameBuffer (fb);

	//
	// Read the scan lines in this order:
	// 0, N, 2N, 3N, ... 1, N+1, 2N+1, ... 2, N+2, 2N+2, ...
	//

	const int N = 7;

	for (int i = 0; i < N; ++i)
	    for (int y = dw.min.y + i; y <= dw.max.y; y += N)
		in.readPixels (y);

	cout << " comparing" << flush;

	assert (in.header().displayWindow() == hdr.displayWindow());
	assert (in.header().dataWindow() == hdr.dataWindow());
	assert (in.header().pixelAspectRatio() == hdr.pixelAspectRatio());
	assert (in.header().screenWindowCenter() == hdr.screenWindowCenter());
	assert (in.header().screenWindowWidth() == hdr.screenWindowWidth());
	assert (in.header().lineOrder() == hdr.lineOrder());
	assert (in.header().compression() == hdr.compression());

	ChannelList::ConstIterator hi = hdr.channels().begin();
	ChannelList::ConstIterator ii = in.header().channels().begin();

	while (hi != hdr.channels().end())
	{
	    assert (!strcmp (hi.name(), ii.name()));
	    assert (hi.channel().type == ii.channel().type);
	    assert (hi.channel().xSampling == ii.channel().xSampling);
	    assert (hi.channel().ySampling == ii.channel().ySampling);

	    ++hi;
	    ++ii;
	}

	assert (ii == in.header().channels().end());

	for (int y = 0; y < h; ++y)
	    for (int x = 0; x < w; ++x)
		assert (ph1[y][x] == ph2[y][x]);
    }

    remove (fileName);
    cout << endl;
}


} // namespace


void
testLineOrder (const std::string &tempDir)
{
    try
    {
	cout << "Testing line order and random access to scan lines" << endl;

	const int W = 117;
	const int H = 97;

	Array2D<half> ph (H, W);
	fillPixels (ph, W, H);

	int maxThreads = ILMTHREAD_NAMESPACE::supportsThreads()? 3: 0;

	for (int n = 0; n <= maxThreads; ++n)
	{
	    if (ILMTHREAD_NAMESPACE::supportsThreads())
	    {
		setGlobalThreadCount (n);
		cout << "\nnumber of threads: " << globalThreadCount() << endl;
	    }

	    std::string filename = tempDir + "imf_test_lorder.exr";

	    for (int lorder = 0; lorder < RANDOM_Y; ++lorder)
	    {
		writeRead (ph,
			   filename.c_str(),
			   W, H,
			   LineOrder (lorder));
	    }
	}

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
