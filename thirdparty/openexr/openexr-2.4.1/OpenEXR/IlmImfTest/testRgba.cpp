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
#include "compareDwa.h"

#include <ImfRgbaFile.h>
#include <ImfOutputFile.h>
#include <ImfChannelList.h>
#include <ImfFrameBuffer.h>
#include <ImfHeader.h>
#include <ImfArray.h>
#include <ImfThreading.h>
#include <IlmThread.h>
#include <string>
#include <stdio.h>
#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
rgbaMethods ()
{
    //
    // Verify that the constructors and the assignment
    // operator for struct Rgba work.
    //

    Rgba x (2.f, 3.f, 4.f);
    assert (x.r == 2.f && x.g == 3.f && x.b == 4.f && x.a == 1.f);

    Rgba y (5.f, 6.f, 7.f, 0.f);
    assert (y.r == 5.f && y.g == 6.f && y.b == 7.f && y.a == 0.f);

    Rgba z;

    z = x;
    assert (z.r == 2.f && z.g == 3.f && z.b == 4.f && z.a == 1.f);

    z = y;
    assert (z.r == 5.f && z.g == 6.f && z.b == 7.f && z.a == 0.f);

    Rgba w (z);
    assert (w.r == 5.f && w.g == 6.f && w.b == 7.f && w.a == 0.f);
}


void
fillPixels (Array2D<Rgba> &pixels, int w, int h)
{
    for (int y = 0; y < h; ++y)
    {
	for (int x = 0; x < w; ++x)
	{
	    Rgba &p = pixels[y][x];

	    p.r = 0.5 + 0.5 * sin (0.1 * x + 0.1 * y);
	    p.g = 0.5 + 0.5 * sin (0.1 * x + 0.2 * y);
	    p.b = 0.5 + 0.5 * sin (0.1 * x + 0.3 * y);
	    p.a = (p.r + p.b + p.g) / 3.0;
	}
    }
}


void
writeReadRGBA (const char fileName[],
	       int width,
	       int height,
	       const Array2D<Rgba> &p1,
	       RgbaChannels channels,
	       LineOrder lorder,
	       Compression comp)
{
    //
    // Save the selected channels of RGBA image p1; save the
    // scan lines in the specified order.  Read the image back
    // from the file, and compare the data with the orignal.
    //

    cout << "channels " <<
	    ((channels & WRITE_R)? "R": "") <<
	    ((channels & WRITE_G)? "G": "") <<
	    ((channels & WRITE_B)? "B": "") <<
	    ((channels & WRITE_A)? "A": "") <<
	    ", line order " << lorder <<
	    ", compression " << comp << endl;

    Header header (width, height);
    header.lineOrder() = lorder;
    header.compression() = comp;

    cout << "writing ";
    cout.flush();

    {
	remove (fileName);
	RgbaOutputFile out (fileName, header, channels);
	out.setFrameBuffer (&p1[0][0], 1, width);
	out.writePixels (height);
    }

    cout << "reading ";
    cout.flush();

    {
	RgbaInputFile in (fileName);
	const Box2i &dw = in.dataWindow();
	
	int w = dw.max.x - dw.min.x + 1;
	int h = dw.max.y - dw.min.y + 1;
	int dx = dw.min.x;
	int dy = dw.min.y;

	Array2D<Rgba> p2 (h, w);
	in.setFrameBuffer (&p2[-dy][-dx], 1, w);
	in.readPixels (dw.min.y, dw.max.y);

	assert (in.displayWindow() == header.displayWindow());
	assert (in.dataWindow() == header.dataWindow());
	assert (in.pixelAspectRatio() == header.pixelAspectRatio());
	assert (in.screenWindowCenter() == header.screenWindowCenter());
	assert (in.screenWindowWidth() == header.screenWindowWidth());
	assert (in.lineOrder() == header.lineOrder());
	assert (in.compression() == header.compression());
	assert (in.channels() == channels);

	if (in.compression() == B44_COMPRESSION ||
	    in.compression() == B44A_COMPRESSION)
	{
	    compareB44 (width, height, p1, p2, channels);
	}
	else if (in.compression() == DWAA_COMPRESSION ||
	         in.compression() == DWAB_COMPRESSION)
	{
	    compareDwa (width, height, p1, p2, channels);
	}
	else
	{
	    for (int y = 0; y < h; ++y)
	    {
		for (int x = 0; x < w; ++x)
		{
		    if (channels & WRITE_R)
			assert (p2[y][x].r == p1[y][x].r);
		    else
			assert (p2[y][x].r == 0);

		    if (channels & WRITE_G)
			assert (p2[y][x].g == p1[y][x].g);
		    else
			assert (p2[y][x].g == 0);

		    if (channels & WRITE_B)
			assert (p2[y][x].b == p1[y][x].b);
		    else
			assert (p2[y][x].b == 0);

		    if (channels & WRITE_A)
			assert (p2[y][x].a == p1[y][x].a);
		    else
			assert (p2[y][x].a == 1);
		}
	    }
	}
    }

    remove (fileName);
}


void
writeReadIncomplete (const std::string &tempDir)
{
    cout << "\nfile with missing and broken scan lines" << endl;

    std::string fileName = tempDir + "imf_test_incomplete.exr";

    //
    // Write a file where some scan lines are missing or broken.
    // Then try read the file and verify that all existing good
    // scan lines can actually be read.
    //

    const int width = 400;
    const int height = 300;

    Array2D<Rgba> p1 (height, width);

    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	    p1[y][x] = Rgba (x % 5, x % 17, y % 23, y % 29);

    {
        cout << "writing" << endl;
 
        remove (fileName.c_str());

	Header header (width, height);
	header.compression() = ZIPS_COMPRESSION;

        RgbaOutputFile out (fileName.c_str(), header, WRITE_RGBA);
        
        out.setFrameBuffer (&p1[0][0], 1, width);
	out.writePixels (height / 2);		// write only half of the
						// scan lines in the image

	out.breakScanLine (10, 10, 10, 0xff);	// destroy scan lines
	out.breakScanLine (25, 10, 10, 0xff);	// 10 and 25
    }

    {
	Array2D<Rgba> p2 (height, width);

	for (int y = 0; y < height; ++y)
	    for (int x = 0; x < width; ++x)
		p2[y][x] = Rgba (-1, -1, -1, -1);

        cout << "reading one scan line at a time," << flush;

        RgbaInputFile in (fileName.c_str());
        const Box2i &dw = in.dataWindow();

        assert (dw.max.x - dw.min.x + 1 == width);
        assert (dw.max.y - dw.min.y + 1 == height);
	assert (dw.min.x == 0);
	assert (dw.min.y == 0);
	
        in.setFrameBuffer (&p2[0][0], 1, width);

	for (int y = 0; y < height; ++y)
	{
	    bool scanLinePresent = true;
	    bool scanLineBroken = false;

	    try
	    {
		in.readPixels (y);
	    }
	    catch (const IEX_NAMESPACE::InputExc &)
	    {
		scanLinePresent = false;	// scan line is missing
	    }
	    catch (const IEX_NAMESPACE::IoExc &)
	    {
		scanLineBroken = true;		// scan line cannot be decoded
	    }

	    if (y == 10 || y == 25)
		assert (scanLineBroken);
	    else
		assert (scanLinePresent == (y < height / 2));
	}

	cout << " comparing" << endl << flush;               

	for (int y = 0; y < height; ++y)
	{
	    for (int x = 0; x < width; ++x)
	    {
		const Rgba &s = p1[y][x];
		const Rgba &t = p2[y][x];

		if (y < height / 2 && y != 10 && y != 25)
		{
		    assert (t.r == s.r &&
			    t.g == s.g &&
			    t.b == s.b &&
			    t.a == s.a);
		}
		else
		{
		    assert (t.r == -1 &&
			    t.g == -1 &&
			    t.b == -1 &&
			    t.a == -1);
		}
	    }
	}
    }

    {
	Array2D<Rgba> p2 (height, width);

	for (int y = 0; y < height; ++y)
	    for (int x = 0; x < width; ++x)
		p2[y][x] = Rgba (-1, -1, -1, -1);

        cout << "reading multiple scan lines at a time," << flush;

        RgbaInputFile in (fileName.c_str());
        const Box2i &dw = in.dataWindow();

        assert (dw.max.x - dw.min.x + 1 == width);
        assert (dw.max.y - dw.min.y + 1 == height);
	assert (dw.min.x == 0);
	assert (dw.min.y == 0);
	
        in.setFrameBuffer (&p2[0][0], 1, width);

	bool scanLinesMissing = false;
	bool scanLinesBroken = false;

	try
	{
	    in.readPixels (0, height - 1);
	}
	catch (const IEX_NAMESPACE::InputExc &)
	{
	    scanLinesMissing = true;
	}
	catch (const IEX_NAMESPACE::IoExc &)
	{
	    scanLinesBroken = true;
	}

	assert (scanLinesMissing || scanLinesBroken);

	cout << " comparing" << endl << flush;               

	for (int y = 0; y < height; ++y)
	{
	    for (int x = 0; x < width; ++x)
	    {
		const Rgba &s = p1[y][x];
		const Rgba &t = p2[y][x];

		assert ((t.r == -1  && t.g == -1  && t.b == -1  && t.a == -1) ||
			(t.r == s.r && t.g == s.g && t.b == s.b && t.a == s.a));
	    }
	}
    }

    remove (fileName.c_str());
}


void
writeReadLayers (const std::string &tempDir)
{
    cout << "\nreading multi-layer file" << endl;

    std::string fileName = tempDir + "imf_test_multi_layer_rgba.exr";

    const int W = 237;
    const int H = 119;
    
    Array2D<half> p1 (H, W);
    Array2D<half> p2 (H, W);

    for (int y = 0; y < H; ++y)
    {
	for (int x = 0; x < W; ++x)
	{
	    p1[y][x] = half (y % 23 + x % 17);
	    p2[y][x] = half (y % 29 + x % 19);
	}
    }

    {
	Header hdr (W, H);
	hdr.channels().insert ("R", Channel (HALF));
	hdr.channels().insert ("foo.R", Channel (HALF));

	FrameBuffer fb;

	fb.insert ("R",
		   Slice (HALF,			// type
			  (char *) &p1[0][0], 	// base
			  sizeof (half),	// xStride
			  sizeof (half) * W));	// yStride

	fb.insert ("foo.R",
		   Slice (HALF,			// type
			  (char *) &p2[0][0], 	// base
			  sizeof (half),	// xStride
			  sizeof (half) * W));	// yStride

	OutputFile out (fileName.c_str(), hdr);
	out.setFrameBuffer (fb);
	out.writePixels (H);
    }

    {
	RgbaInputFile in (fileName.c_str(), "");

	Array2D<Rgba> p3 (H, W);
	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (0, H - 1);

	for (int y = 0; y < H; ++y)
	{
	    for (int x = 0; x < W; ++x)
	    {
		assert (p3[y][x].r == p1[y][x]);
		assert (p3[y][x].g == 0);
		assert (p3[y][x].b == 0);
		assert (p3[y][x].a == 1);
	    }
	}
    }

    {
	RgbaInputFile in (fileName.c_str(), "foo");

	Array2D<Rgba> p3 (H, W);
	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (0, H - 1);

	for (int y = 0; y < H; ++y)
	{
	    for (int x = 0; x < W; ++x)
	    {
		assert (p3[y][x].r == p2[y][x]);
		assert (p3[y][x].g == 0);
		assert (p3[y][x].b == 0);
		assert (p3[y][x].a == 1);
	    }
	}
    }

    {
	RgbaInputFile in (fileName.c_str(), "");

	Array2D<Rgba> p3 (H, W);

	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (0, H / 2 - 1);

	in.setLayerName ("foo");

	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (H / 2, H - 1);

	for (int y = 0; y < H; ++y)
	{
	    for (int x = 0; x < W; ++x)
	    {
		if (y < H / 2)
		    assert (p3[y][x].r == p1[y][x]);
		else
		    assert (p3[y][x].r == p2[y][x]);

		assert (p3[y][x].g == 0);
		assert (p3[y][x].b == 0);
		assert (p3[y][x].a == 1);
	    }
	}
    }

    {
	Header hdr (W, H);
	hdr.channels().insert ("Y", Channel (HALF));
	hdr.channels().insert ("foo.Y", Channel (HALF));

	FrameBuffer fb;

	fb.insert ("Y",
		   Slice (HALF,			// type
			  (char *) &p1[0][0], 	// base
			  sizeof (half),	// xStride
			  sizeof (half) * W));	// yStride

	fb.insert ("foo.Y",
		   Slice (HALF,			// type
			  (char *) &p2[0][0], 	// base
			  sizeof (half),	// xStride
			  sizeof (half) * W));	// yStride

	OutputFile out (fileName.c_str(), hdr);
	out.setFrameBuffer (fb);
	out.writePixels (H);
    }

    {
	RgbaInputFile in (fileName.c_str(), "");

	Array2D<Rgba> p3 (H, W);
	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (0, H - 1);

	for (int y = 0; y < H; ++y)
	{
	    for (int x = 0; x < W; ++x)
	    {
		assert (p3[y][x].r == p1[y][x]);
		assert (p3[y][x].g == p1[y][x]);
		assert (p3[y][x].b == p1[y][x]);
		assert (p3[y][x].a == 1);
	    }
	}
    }

    {
	RgbaInputFile in (fileName.c_str(), "foo");

	Array2D<Rgba> p3 (H, W);
	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (0, H - 1);

	for (int y = 0; y < H; ++y)
	{
	    for (int x = 0; x < W; ++x)
	    {
		assert (p3[y][x].r == p2[y][x]);
		assert (p3[y][x].g == p2[y][x]);
		assert (p3[y][x].b == p2[y][x]);
		assert (p3[y][x].a == 1);
	    }
	}
    }

    {
	RgbaInputFile in (fileName.c_str(), "");

	Array2D<Rgba> p3 (H, W);

	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (0, H / 2 - 1);

	in.setLayerName ("foo");

	in.setFrameBuffer (&p3[0][0], 1, W);
	in.readPixels (H / 2, H - 1);

	for (int y = 0; y < H; ++y)
	{
	    for (int x = 0; x < W; ++x)
	    {
		if (y < H / 2)
		{
		    assert (p3[y][x].r == p1[y][x]);
		    assert (p3[y][x].g == p1[y][x]);
		    assert (p3[y][x].b == p1[y][x]);
		}
		else
		{
		    assert (p3[y][x].r == p2[y][x]);
		    assert (p3[y][x].g == p2[y][x]);
		    assert (p3[y][x].b == p2[y][x]);
		}

		assert (p3[y][x].a == 1);
	    }
	}
    }

    remove (fileName.c_str());
}


} // namespace


void
testRgba (const std::string &tempDir)
{
    try
    {
	cout << "Testing the RGBA image interface" << endl;

	rgbaMethods ();

	const int W = 237;
	const int H = 119;

	Array2D<Rgba> p1 (H, W);
	fillPixels (p1, W, H);

	int maxThreads = ILMTHREAD_NAMESPACE::supportsThreads()? 3: 0;

	for (int n = 0; n <= maxThreads; ++n)
	{
	    if (ILMTHREAD_NAMESPACE::supportsThreads())
	    {
		setGlobalThreadCount (n);
		cout << "\nnumber of threads: " << globalThreadCount() << endl;
	    }

	    for (int lorder = 0; lorder < RANDOM_Y; ++lorder)
	    {
		for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
		{
		    writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
				   W, H, p1,
				   WRITE_RGBA,
				   LineOrder (lorder),
				   Compression (comp));

		    writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
				   W, H, p1,
				   WRITE_RGB,
				   LineOrder (lorder),
				   Compression (comp));

		    writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
				   W, H, p1,
				   WRITE_A,
				   LineOrder (lorder),
				   Compression (comp));

		    writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
				   W, H, p1,
				   RgbaChannels (WRITE_R | WRITE_B),
				   LineOrder (lorder),
				   Compression (comp));
		}
	    }

	    writeReadIncomplete (tempDir);
	}

	writeReadLayers (tempDir);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
