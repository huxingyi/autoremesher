///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2012, Industrial Light & Magic, a division of Lucas
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

#include "compareB44.h"

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImathRandom.h>
#include <half.h>
#include "compareFloat.h"

#include <stdio.h>
#include <assert.h>

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
fillPixels1 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "only zeroes" << endl;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = 0;
	    ph[y][x] = 0;
	    pf[y][x] = 0;
	}
}


void
fillPixels2 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "pattern 1" << endl;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = (x + y) & 1;
	    ph[y][x] = pi[y][x];
	    pf[y][x] = pi[y][x];
	}
}


void
fillPixels3 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "pattern 2" << endl;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = x % 100 + 100 * (y % 100);
	    ph[y][x] = sin (double (x)) + sin (y * 0.5);
	    pf[y][x] = sin (double (y)) + sin (x * 0.5);
	}
}


void
fillPixels4 (Array2D<unsigned int> &pi,
	     Array2D<half> &ph,
	     Array2D<float> &pf,
	     int width,
	     int height)
{
    cout << "random bits" << endl;

    //
    // Use of a union to extract the bit pattern from a float, as is
    // done below, works only if int and float have the same size.
    //

    assert (sizeof (int) == sizeof (float));

    Rand48 rand;

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	{
	    pi[y][x] = rand.nexti();

	    ph[y][x].setBits (rand.nexti());

	    union {int i; float f;} u;
	    u.i = rand.nexti();

	    pf[y][x] = u.f;
	}
}


void
writeRead (const Array2D<unsigned int> &pi1,
	   const Array2D<half> &ph1,
	   const Array2D<float> &pf1,
	   const char fileName[],
	   int width,
	   int height,
	   int xOffset,
	   int yOffset,
	   Compression comp,
	   int xs,
	   int ys)
{
    //
    // Write the pixel data in pi1, ph1 and ph2 to an
    // image file using the specified compression type
    // and subsampling rates.  Read the pixel data back
    // from the file and verify that the data did not
    // change.
    //

    cout << "compression " << comp <<
	    ", x sampling " << xs <<
	    ", y sampling " << ys <<
	    ":" << flush;


    Header hdr ((Box2i (V2i (0, 0),			// display window
		        V2i (width - 1, height -1))),
		(Box2i (V2i (xOffset, yOffset),		// data window
		        V2i (xOffset + width - 1, yOffset + height - 1))));

    hdr.compression() = comp;

    hdr.channels().insert ("I",			// name
			   Channel (IMF::UINT,	// type
				    xs,		// xSampling
				    ys)		// ySampling
			  );

    hdr.channels().insert ("H",			// name
			   Channel (IMF::HALF,	// type
				    xs,		// xSampling
				    ys)		// ySampling
			  );

    hdr.channels().insert ("F",			// name
			   Channel (IMF::FLOAT,	// type
				    xs,		// xSampling
				    ys)		// ySampling
			  );

    {
	FrameBuffer fb; 

	fb.insert ("I",						// name
		   Slice (IMF::UINT,				// type
			  (char *) &pi1[-yOffset / ys][-xOffset / xs], // base
			  sizeof (pi1[0][0]), 			// xStride
			  sizeof (pi1[0][0]) * (width / xs),	// yStride
			  xs,					// xSampling
			  ys)					// ySampling
		  );
	
	fb.insert ("H",						// name
		   Slice (IMF::HALF,				// type
			  (char *) &ph1[-yOffset / ys][-xOffset / xs], // base
			  sizeof (ph1[0][0]), 			// xStride
			  sizeof (ph1[0][0]) * (width / xs),	// yStride
			  xs,					// xSampling
			  ys)					// ySampling
		  );
	
	fb.insert ("F",						// name
		   Slice (IMF::FLOAT,				// type
			  (char *) &pf1[-yOffset / ys][-xOffset / xs], // base
			  sizeof (pf1[0][0]), 			// xStride
			  sizeof (pf1[0][0]) * (width / xs),	// yStride
			  xs,					// xSampling
			  ys)					// ySampling
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

	Array2D<unsigned int> pi2 (h / ys, w / xs);
	Array2D<half>         ph2 (h / ys, w / xs);
	Array2D<float>        pf2 (h / ys, w / xs);

	FrameBuffer fb;

	{
	    int xs = in.header().channels()["I"].xSampling;
	    int ys = in.header().channels()["I"].ySampling;

	    fb.insert ("I",					// name
		       Slice (IMF::UINT,			// type
			      (char *) &pi2[-dy / ys][-dx / xs], // base
			      sizeof (pi2[0][0]), 		// xStride
			      sizeof (pi2[0][0]) * (w / xs),	// yStride
			      xs,				// xSampling
			      ys)				// ySampling
		      );
	}
	    
	{
	    int xs = in.header().channels()["H"].xSampling;
	    int ys = in.header().channels()["H"].ySampling;

	    fb.insert ("H",					// name
		       Slice (IMF::HALF,			// type
			      (char *) &ph2[-dy / ys][-dx / xs], // base
			      sizeof (ph2[0][0]), 		// xStride
			      sizeof (ph2[0][0]) * (w / xs),	// yStride
			      xs,				// xSampling
			      ys)				// ySampling
		      );
	}
	    
	{
	    int xs = in.header().channels()["F"].xSampling;
	    int ys = in.header().channels()["F"].ySampling;

	    fb.insert ("F",					// name
		       Slice (IMF::FLOAT,			// type
			      (char *) &pf2[-dy / ys][-dx / xs], // base
			      sizeof (pf2[0][0]), 		// xStride
			      sizeof (pf2[0][0]) * (w / xs),	// yStride
			      xs,				// xSampling
			      ys)				// ySampling
		      );
	}
	
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

	for (int y = 0; y < h / ys; ++y)
	{
	    for (int x = 0; x < w / xs; ++x)
	    {
		assert (pi1[y][x] == pi2[y][x]);
		assert (equivalent (pf1[y][x], pf2[y][x], comp));

		if (comp != B44_COMPRESSION &&
                    comp != B44A_COMPRESSION)
                {
		    assert (ph1[y][x].bits() == ph2[y][x].bits());
                }
	    }
	}

	if (comp == B44_COMPRESSION ||
            comp == B44A_COMPRESSION)
	{
	    Array2D<half> ph3 (h / ys, w / xs);

	    for (int y = 0; y < h / ys; ++y)
		for (int x = 0; x < w / xs; ++x)
		    ph3[y][x] = ph1[y][x];

	    compareB44 (w / xs, h / ys, ph3, ph2);
	}
    }

    remove (fileName);
    cout << endl;
}


void
writeRead (const std::string &tempDir,
           const Array2D<unsigned int> &pi,
	   const Array2D<half> &ph,
	   const Array2D<float> &pf,
	   int w,
	   int h,
	   int dx,
	   int dy)
{
    std::string filename = tempDir + "imf_test_comp.exr";

    for (int xs = 1; xs <= 2; ++xs)
    {
	for (int ys = 1; ys <= 2; ++ys)
	{
	    for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
	    {
		writeRead (pi, ph, pf,
			   filename.c_str(),
			   w  * xs, h  * ys,
			   dx * xs, dy * ys,
			   Compression (comp),
			   xs, ys);
	    }
	}
    }
}

} // namespace


void
testCompression (const std::string &tempDir)
{
    try
    {
	cout << "Testing pixel data types, "
		"subsampling and "
		"compression schemes" << endl;

	const int W = 1371;
	const int H = 159;
	const int DX = 17;
	const int DY = 29;

	Array2D<unsigned int> pi (H, W);
	Array2D<half> ph (H, W);
	Array2D<float> pf (H, W);

	//
	// If the following assertion fails, new pixel types have
	// been added to the Imf library; testing code for the new
	// pixel types should be added to this file.
	//

	assert (NUM_PIXELTYPES == 3);

	fillPixels1 (pi, ph, pf, W, H);
	writeRead (tempDir, pi, ph, pf, W, H, DX, DY);

	fillPixels2 (pi, ph, pf, W, H);
	writeRead (tempDir, pi, ph, pf, W, H, DX, DY);

	fillPixels3 (pi, ph, pf, W, H);
	writeRead (tempDir, pi, ph, pf, W, H, DX, DY);

	fillPixels4 (pi, ph, pf, W, H);
	writeRead (tempDir, pi, ph, pf, W, H, DX, DY);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
