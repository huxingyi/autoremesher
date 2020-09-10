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
#include "half.h"

#include <stdio.h>
#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;

namespace {

void
fillPixels (Array2D<half> &ph1,
	    Array2D<half> &ph2,
	    int width, int height)
{
    for (int y = 0; y < height; ++y)
    {
	for (int x = 0; x < width; ++x)
	{
	    ph1[y][x] = x % 10 + 10 * (y % 17);
	    ph2[y][x] = x % 11 + 11 * (y % 15);
	}
    }
}


void
writeRead (const Array2D<half> &h1out,
	   const Array2D<half> &h2out,
	   const char fileName[],
	   int width,
	   int height)
{
    //
    // Write an image file with three channels, H1, H2 and H3.
    // Our frame buffer contains pixel data only for H1 and H2;
    // the file's H3 channel should be filled with zeroes.
    //

    Header hdr (width, height);

    hdr.channels().insert ("H1",			// name
			   Channel (HALF,		// type
				    1,			// xSampling
				    1)			// ySampling
			  );

    hdr.channels().insert ("H2",			// name
			   Channel (HALF,		// type
				    1,			// xSampling
				    1)			// ySampling
			  );

    hdr.channels().insert ("H3",			// name
			   Channel (HALF,		// type
				    1,			// xSampling
				    1)			// ySampling
			  );

    {
	FrameBuffer fb; 

	fb.insert ("H1",				// name
		   Slice (HALF,				// type
			  (char *) &h1out[0][0],	// base
			  sizeof (h1out[0][0]), 	// xStride
			  sizeof (h1out[0][0]) * width,	// yStride
			  1,				// xSampling
			  1)				// ySampling
		  );

	fb.insert ("H2",				// name
		   Slice (HALF,				// type
			  (char *) &h2out[0][0],	// base
			  sizeof (h2out[0][0]), 	// xStride
			  sizeof (h2out[0][0]) * width,	// yStride
			  1,				// xSampling
			  1)				// ySampling
		  );
	
	cout << "writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    //
    // Read the image back from the file.  Our frame buffer now
    // contains space for the three channels, H1, H3 and H4.
    // H1 and H3 should be read back from the file (but H3
    // should contain only zeroes), and H4 should be filled
    // with a default value, 3.0, because the file contains
    // no data for it.  The file's H2 channel should be ignored.
    //

    {
	cout << " reading" << flush;

	InputFile in (fileName);
	
	const Box2i &dw = in.header().dataWindow();
	int w = dw.max.x - dw.min.x + 1;
	int h = dw.max.y - dw.min.y + 1;
	int dx = dw.min.x;
	int dy = dw.min.y;

	Array2D<half> h1in (h, w);
	Array2D<half> h3in (h, w);
	Array2D<half> h4in (h, w);

	FrameBuffer fb;

	fb.insert ("H1",				// name
		   Slice (HALF,				// type
			  (char *) &h1in[-dy][-dx],	// base
			  sizeof (h1in[0][0]), 		// xStride
			  sizeof (h1in[0][0]) * w,	// yStride
			  1,				// xSampling
			  1,				// ySampling
			  3.0)				// fillValue
		  );
	    
	fb.insert ("H3",				// name
		   Slice (HALF,				// type
			  (char *) &h3in[-dy][-dx],	// base
			  sizeof (h3in[0][0]), 		// xStride
			  sizeof (h3in[0][0]) * w,	// yStride
			  1,				// xSampling
			  1,				// ySampling
			  3.0)				// fillValue
		  );
	    
	fb.insert ("H4",				// name
		   Slice (HALF,				// type
			  (char *) &h4in[-dy][-dx],	// base
			  sizeof (h4in[0][0]), 		// xStride
			  sizeof (h4in[0][0]) * w,	// yStride
			  1,				// xSampling
			  1,				// ySampling
			  3.0)				// fillValue
		  );
	    
	in.setFrameBuffer (fb);
	in.readPixels (dw.min.y, dw.max.y);

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
	{
	    for (int x = 0; x < w; ++x)
	    {
		assert (h1in[y][x] == h1out[y][x]);
		assert (h3in[y][x] == 0.0);
		assert (h4in[y][x] == 3.0);
	    }
	}
    }

    remove (fileName);
    cout << endl;
}


} // namespace


void
testChannels (const std::string &tempDir)
{
    try
    {
	cout << "Testing filling of missing channels" << endl;

	const int W = 117;
	const int H = 97;

	Array2D<half> ph1 (H, W);
	Array2D<half> ph2 (H, W);
	fillPixels (ph1, ph2, W, H);

	std::string filename = tempDir + "imf_test_channels.exr";

	writeRead (ph1, ph2, filename.c_str(), W, H);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
