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
#include <half.h>

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
	    ph[y][x] = sin (double (x)) + sin (y * 0.5);
}


void
writeCopyRead (const Array2D<half> &ph1,
	       const char fileName1[],
	       const char fileName2[],
	       int width,
	       int height,
	       int xOffset,
	       int yOffset,
	       Compression comp)
{
    //
    // Write the pixel data in ph1 to an image file using the
    // specified compression type and subsampling rates.
    // Then copy the image file using OutputFile::copyPixels()
    // Read the pixel data back from the copied file verify
    // that the data are the same as those in ph1.
    //

    cout << "compression " << comp << ":" << flush;

    Header hdr ((Box2i (V2i (0, 0),			// display window
		        V2i (width - 1, height -1))),
		(Box2i (V2i (xOffset, yOffset),		// data window
		        V2i (xOffset + width - 1, yOffset + height - 1))));

    hdr.compression() = comp;
    hdr.channels().insert ("H", Channel (HALF, 1, 1));

    {
	FrameBuffer fb; 

	fb.insert ("H",						// name
		   Slice (HALF,					// type
			  (char *) &ph1[-yOffset][-xOffset],	// base
			  sizeof (ph1[0][0]), 			// xStride
			  sizeof (ph1[0][0]) * width,		// yStride
			  1,					// xSampling
			  1)					// ySampling
		  );
	
	cout << " writing" << flush;

	remove (fileName1);
	OutputFile out (fileName1, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    {
	cout << " copying" << flush;

	remove (fileName2);
	InputFile in (fileName1);
	OutputFile out (fileName2, in.header());
	out.copyPixels (in);
    }

    {
	cout << " reading" << flush;

	InputFile in1 (fileName1);
	InputFile in2 (fileName2);
	
	const Box2i &dw = in2.header().dataWindow();
	int w = dw.max.x - dw.min.x + 1;
	int h = dw.max.y - dw.min.y + 1;
	int dx = dw.min.x;
	int dy = dw.min.y;

	Array2D<half> ph1 (h, w);
	Array2D<half> ph2 (h, w);

	FrameBuffer fb1;
	FrameBuffer fb2;

	fb1.insert ("H",				// name
		    Slice (HALF,			// type
			   (char *) &ph1[-dy][-dx],	// base
			   sizeof (ph1[0][0]), 		// xStride
			   sizeof (ph1[0][0]) * w,	// yStride
			   1,				// xSampling
			   1)				// ySampling
		   );

	fb2.insert ("H",				// name
		    Slice (HALF,			// type
			   (char *) &ph2[-dy][-dx],	// base
			   sizeof (ph2[0][0]), 		// xStride
			   sizeof (ph2[0][0]) * w,	// yStride
			   1,				// xSampling
			   1)				// ySampling
		   );
	
	in1.setFrameBuffer (fb1);
	in1.readPixels (dw.min.y, dw.max.y);
	in2.setFrameBuffer (fb2);
	in2.readPixels (dw.min.y, dw.max.y);

	cout << " comparing" << flush;

	assert (in2.header().displayWindow() == hdr.displayWindow());
	assert (in2.header().dataWindow() == hdr.dataWindow());
	assert (in2.header().pixelAspectRatio() == hdr.pixelAspectRatio());
	assert (in2.header().screenWindowCenter() == hdr.screenWindowCenter());
	assert (in2.header().screenWindowWidth() == hdr.screenWindowWidth());
	assert (in2.header().lineOrder() == hdr.lineOrder());
	assert (in2.header().compression() == hdr.compression());
	assert (in2.header().channels() == hdr.channels());

	for (int y = 0; y < h; ++y)
	    for (int x = 0; x < w; ++x)
		assert (ph1[y][x] == ph2[y][x]);
    }

    remove (fileName1);
    remove (fileName2);
    cout << endl;
}


void
writeCopyRead (const std::string &tempDir, const Array2D<half> &ph, int w, int h, int dx, int dy)
{
    std::string filename1 = tempDir + "imf_test_copy1.exr";
    std::string filename2 = tempDir + "imf_test_copy2.exr";

    for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
    {
	writeCopyRead (ph,
		       filename1.c_str(),
		       filename2.c_str(),
		       w, h,
		       dx, dy,
		       Compression (comp));
    }
}

} // namespace


void
testCopyPixels (const std::string &tempDir)
{
    try
    {
	cout << "Testing fast pixel copying" << endl;

	const int W = 371;
	const int H = 559;
	const int DX = 17;
	const int DY = 29;

	Array2D<half> ph (H, W);

	fillPixels (ph, W, H);
	writeCopyRead (tempDir, ph, W, H, 0,  0);
	writeCopyRead (tempDir, ph, W, H, 0,  DY);
	writeCopyRead (tempDir, ph, W, H, DX, 0);
	writeCopyRead (tempDir, ph, W, H, DX, DY);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
