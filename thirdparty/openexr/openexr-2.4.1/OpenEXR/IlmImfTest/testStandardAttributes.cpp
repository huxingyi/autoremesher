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

#include <ImfRgbaFile.h>
#include <ImfStandardAttributes.h>
#include <ImfFramesPerSecond.h>
#include <ImfArray.h>
#include "ImathRandom.h"
#include <fstream>
#include <iomanip>
#include <stdio.h>
#include <assert.h>

using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

void
convertRGBtoXYZ ()
{
    cout << "conversion from RGB to XYZ" << endl;

    Chromaticities c;
    float Y = 100;
    M44f M1 = RGBtoXYZ (c, Y);

    V3f R1 = V3f (1, 0, 0) * M1;
    V3f G1 = V3f (0, 1, 0) * M1;
    V3f B1 = V3f (0, 0, 1) * M1;
    V3f W1 = V3f (1, 1, 1) * M1;

    cout << "red   XYZ = " << R1 << endl;
    cout << "green XYZ = " << G1 << endl;
    cout << "blue  XYZ = " << B1 << endl;
    cout << "white XYZ = " << W1 << endl;

    V2f r1 (R1.x / (R1.x + R1.y + R1.z), R1.y / (R1.x + R1.y + R1.z));
    V2f g1 (G1.x / (G1.x + G1.y + G1.z), G1.y / (G1.x + G1.y + G1.z));
    V2f b1 (B1.x / (B1.x + B1.y + B1.z), B1.y / (B1.x + B1.y + B1.z));
    V2f w1 (W1.x / (W1.x + W1.y + W1.z), W1.y / (W1.x + W1.y + W1.z));

    cout << "red   xy = " << r1 << endl;
    cout << "green xy = " << g1 << endl;
    cout << "blue  xy = " << b1 << endl;
    cout << "white xy = " << w1 << endl;

    assert (equalWithRelError (W1.y, Y, 1e-5F));
    assert (r1.equalWithAbsError (c.red, 1e-5F));
    assert (g1.equalWithAbsError (c.green, 1e-5F));
    assert (b1.equalWithAbsError (c.blue, 1e-5F));
    assert (w1.equalWithAbsError (c.white, 1e-5F));

    cout << "conversion from XYZ to RGB" << endl;

    M44f M2 = XYZtoRGB (c, Y);

    V3f R2 = R1 * M2;
    V3f G2 = G1 * M2;
    V3f B2 = B1 * M2;
    V3f W2 = W1 * M2;

    cout << "red   RGB = " << R2 << endl;
    cout << "green RGB = " << G2 << endl;
    cout << "blue  RGB = " << B2 << endl;
    cout << "white RGB = " << W2 << endl;

    assert (R2.equalWithAbsError (V3f (1, 0, 0), 1e-3F));
    assert (G2.equalWithAbsError (V3f (0, 1, 0), 1e-3F));
    assert (B2.equalWithAbsError (V3f (0, 0, 1), 1e-3F));
    assert (W2.equalWithAbsError (V3f (1, 1, 1), 1e-3F));
}


void
writeReadChromaticities (const char fileName[])
{
    cout << "chromaticities attribute" << endl;

    cout << "writing, ";

    Chromaticities c1 (V2f (1, 2), V2f (3, 4), V2f (5, 6), V2f (7, 8));
    static const int W = 100;
    static const int H = 100;

    Header header (W, H);
    assert (hasChromaticities (header) == false);

    addChromaticities (header, c1);
    assert (hasChromaticities (header) == true);

    {
	RgbaOutputFile out (fileName, header);
	Rgba pixels[W];

	for (int i = 0; i < W; ++i)
	{
	    pixels[i].r = 1;
	    pixels[i].g = 1;
	    pixels[i].b = 1;
	    pixels[i].a = 1;
	}

	out.setFrameBuffer (pixels, 1, 0);
	out.writePixels (H);
    }

    cout << "reading, comparing" << endl;

    {
	RgbaInputFile in (fileName);
	const Chromaticities &c2 = chromaticities (in.header());

	assert (hasChromaticities (in.header()) == true);
	assert (c1.red == c2.red);
	assert (c1.green == c2.green);
	assert (c1.blue == c2.blue);
	assert (c1.white == c2.white);
    }

    remove (fileName);
}


void
latLongMap (const char fileName1[], const char fileName2[])
{
    cout << "latitude-longitude environment map" << endl;
    const int W = 360;
    const int H = 180;

    Header header (W, H);
    addEnvmap (header, ENVMAP_LATLONG);

    V2f pos;

    pos = LatLongMap::latLong (V3f (0, 1, 0));
    assert (equalWithAbsError (pos.x, float (M_PI/2), 1e-6f));

    pos = LatLongMap::latLong (V3f (0, -1, 0));
    assert (equalWithAbsError (pos.x, float (-M_PI/2), 1e-6f));

    pos = LatLongMap::latLong (V3f (0, 0, 1));
    assert (pos.equalWithAbsError (V2f (0, 0), 1e-6f));

    pos = LatLongMap::latLong (V3f (1, 0, 0));
    assert (pos.equalWithAbsError (V2f (0, M_PI/2), 1e-6f));

    pos = LatLongMap::latLong (V3f (-1, 0, 0));
    assert (pos.equalWithAbsError (V2f (0, -M_PI/2), 1e-6f));

    pos = LatLongMap::latLong (V3f (0, 1, 1));
    assert (pos.equalWithAbsError (V2f (M_PI/4, 0), 1e-6f));

    pos = LatLongMap::latLong (V3f (0, -1, 1));
    assert (pos.equalWithAbsError (V2f (-M_PI/4, 0), 1e-6f));

    pos = LatLongMap::pixelPosition (header.dataWindow(), V2f (M_PI/2, M_PI));
    assert (pos.equalWithAbsError (V2f (0, 0), 1e-6f * W));

    pos = LatLongMap::pixelPosition(header.dataWindow(), V2f (-M_PI/2, -M_PI));
    assert (pos.equalWithAbsError (V2f (header.dataWindow().max), 1e-6f * W));

    Array2D<Rgba> pixels (H, W);

    for (int y = 0; y < H; ++y)
    {
	for (int x = 0; x < W; ++x)
	{
	    Rgba &p = pixels[y][x];
	    V3f dir = LatLongMap::direction (header.dataWindow(), V2f (x, y));

	    p.r = dir.x + 1;
	    p.g = dir.y + 1;
	    p.b = dir.z + 1;
	}
    }

    {
	RgbaOutputFile out (fileName1, header, WRITE_RGB);
	out.setFrameBuffer (&pixels[0][0], 1, W);
	out.writePixels (H);
    }

    Rand48 rand (0);

    for (int i = 0; i < W * H * 3; ++i)
    {
	V3f dir = hollowSphereRand<V3f> (rand);
	V2f pos = LatLongMap::pixelPosition (header.dataWindow(), dir);

	Rgba &p = pixels[int (pos.y + 0.5)][int (pos.x + 0.5)];

	p.r = (dir.x + 1) * 0.8;
	p.g = (dir.y + 1) * 0.8;
	p.b = (dir.z + 1) * 0.8;

	V3f dir1 = LatLongMap::direction (header.dataWindow(), pos);
	assert (dir.equalWithAbsError (dir1.normalized(), 1e-5f));
    }

    {
	RgbaOutputFile out (fileName2, header, WRITE_RGB);
	out.setFrameBuffer (&pixels[0][0], 1, W);
	out.writePixels (H);
    }


    remove (fileName1);
    remove (fileName2);
}


void
cubeMap (const char fileName1[], const char fileName2[])
{
    cout << "cube environment map" << endl;
    const int N = 128;
    const int W = N;
    const int H = N * 6;

    Header header (W, H);
    addEnvmap (header, ENVMAP_CUBE);

    int sof = CubeMap::sizeOfFace (header.dataWindow());

    assert (sof == N);

    for (int face1 = 0; face1 < 6; ++face1)
    {
	Box2i dw1 = CubeMap::dataWindowForFace (CubeMapFace (face1),
						header.dataWindow());

	assert (dw1.max.x - dw1.min.x == sof - 1);
	assert (dw1.max.y - dw1.min.y == sof - 1);
	assert (header.dataWindow().intersects (dw1.min));
	assert (header.dataWindow().intersects (dw1.max));

	for (int face2 = face1 + 1; face2 < 6; ++face2)
	{
	    Box2i dw2 = CubeMap::dataWindowForFace (CubeMapFace (face2),
						    header.dataWindow());
	    assert (!dw1.intersects (dw2));
	}
    }

    CubeMapFace face;
    V2f pos;

    CubeMap::faceAndPixelPosition (V3f (1, 0, 0),
				   header.dataWindow(),
				   face, pos);
    
    assert (face == CUBEFACE_POS_X);
    assert (pos.equalWithAbsError (V2f ((sof - 1), (sof - 1)) / 2, 1e-6 * W));

    CubeMap::faceAndPixelPosition (V3f (-1, 0, 0),
	                           header.dataWindow(),
				   face, pos);
    
    assert (face == CUBEFACE_NEG_X);
    assert (pos.equalWithAbsError (V2f ((sof - 1), (sof - 1)) / 2, 1e-6 * W));

    CubeMap::faceAndPixelPosition (V3f (0, 1, 0),
	    			   header.dataWindow(),
				   face, pos);
    
    assert (face == CUBEFACE_POS_Y);
    assert (pos.equalWithAbsError (V2f ((sof - 1), (sof - 1)) / 2, 1e-6 * W));

    CubeMap::faceAndPixelPosition (V3f (0, -1, 0),
	    			   header.dataWindow(),
				   face, pos);
    
    assert (face == CUBEFACE_NEG_Y);
    assert (pos.equalWithAbsError (V2f ((sof - 1), (sof - 1)) / 2, 1e-6 * W));

    CubeMap::faceAndPixelPosition (V3f (0, 0, 1),
	    			   header.dataWindow(),
				   face, pos);
    
    assert (face == CUBEFACE_POS_Z);
    assert (pos.equalWithAbsError (V2f ((sof - 1), (sof - 1)) / 2, 1e-6 * W));

    CubeMap::faceAndPixelPosition (V3f (0, 0, -1),
	    			   header.dataWindow(),
				   face, pos);
    
    assert (face == CUBEFACE_NEG_Z);
    assert (pos.equalWithAbsError (V2f ((sof - 1), (sof - 1)) / 2, 1e-6 * W));

    Array2D<Rgba> pixels (H, W);

    for (int y = 0; y < H; ++y)
    {
	for (int x = 0; x < W; ++x)
	{
	    Rgba &p = pixels[y][x];
	    p.r = p.g = p.b = 0;
	}
    }

    for (int face = 0; face < 6; ++face)
    {
	for (int y = 0; y < sof; ++y)
	{
	    for (int x = 0; x < sof; ++x)
	    {
		V2f px = CubeMap::pixelPosition (CubeMapFace (face),
						 header.dataWindow(),
						 V2f (x, y));

		Rgba &p = pixels[int (px.y + 0.5)][int (px.x + 0.5)];

		V3f dir = CubeMap::direction (CubeMapFace (face),
			                      header.dataWindow(),
					      V2f (x, y));
		dir.normalize();

		p.r = dir.x + 1;
		p.g = dir.y + 1;
		p.b = dir.z + 1;
	    }
	}
    }

    {
	RgbaOutputFile out (fileName1, header, WRITE_RGB);
	out.setFrameBuffer (&pixels[0][0], 1, W);
	out.writePixels (H);
    }

    for (int y = 0; y < H; ++y)
    {
	for (int x = 0; x < W; ++x)
	{
	    Rgba &p = pixels[y][x];
	    assert (p.r != 0 || p.g != 0 || p.b != 0);
	}
    }

    Rand48 rand (0);

    for (int i = 0; i < W * H * 3; ++i)
    {
	V3f dir = hollowSphereRand<V3f> (rand);

	CubeMapFace face;
	V2f pif;

	CubeMap::faceAndPixelPosition (dir, header.dataWindow(), face, pif);

	V2f pos = CubeMap::pixelPosition (face, header.dataWindow(), pif);

	Rgba &p = pixels[int (pos.y + 0.5)][int (pos.x + 0.5)];

	p.r = (dir.x + 1) * 0.8;
	p.g = (dir.y + 1) * 0.8;
	p.b = (dir.z + 1) * 0.8;

	V3f dir1 = CubeMap::direction (face, header.dataWindow(), pif);
	assert (dir.equalWithAbsError (dir1.normalized(), 1e-6f));
    }

    {
	RgbaOutputFile out (fileName2, header, WRITE_RGB);
	out.setFrameBuffer (&pixels[0][0], 1, W);
	out.writePixels (H);
    }

    remove (fileName1);
    remove (fileName2);
}


void
writeReadKeyCode (const char fileName[])
{
    cout << "key code attribute" << endl;

    cout << "writing, ";

    KeyCode k1 (12,	// filmMfcCode
	        34,	// filmType
		123456,	// prefix
		1234,	// count
		45,	// perfOffset
		3,	// perfsPerFrame
		80);	// perfsPerCount

    assert (k1.filmMfcCode() == 12);
    assert (k1.filmType() == 34);
    assert (k1.prefix() == 123456);
    assert (k1.count() == 1234);
    assert (k1.perfOffset() == 45);
    assert (k1.perfsPerFrame() == 3);
    assert (k1.perfsPerCount() == 80);

    static const int W = 100;
    static const int H = 100;

    Header header (W, H);
    assert (hasKeyCode (header) == false);

    addKeyCode (header, k1);
    assert (hasKeyCode (header) == true);

    {
	RgbaOutputFile out (fileName, header);
	Rgba pixels[W];

	for (int i = 0; i < W; ++i)
	{
	    pixels[i].r = 1;
	    pixels[i].g = 1;
	    pixels[i].b = 1;
	    pixels[i].a = 1;
	}

	out.setFrameBuffer (pixels, 1, 0);
	out.writePixels (H);
    }

    cout << "reading, comparing" << endl;

    {
	RgbaInputFile in (fileName);
	const KeyCode &k2 = keyCode (in.header());

	assert (hasKeyCode (in.header()) == true);
	assert (k1.filmMfcCode() == k2.filmMfcCode());
	assert (k1.filmType() == k2.filmType());
	assert (k1.prefix() == k2.prefix());
	assert (k1.count() == k2.count());
	assert (k1.perfOffset() == k2.perfOffset());
	assert (k1.perfsPerFrame() == k2.perfsPerFrame());
	assert (k1.perfsPerCount() == k2.perfsPerCount());
    }

    remove (fileName);
}


void
timeCodeMethods ()
{
    cout << "time code methods" << endl;

    TimeCode t;

    assert (t.timeAndFlags() == 0);
    assert (t.userData() == 0);

    // Frames

    t.setTimeAndFlags (0x00000000);
    t.setFrame (29);
    assert (t.frame() == 29);
    assert (t.timeAndFlags() == 0x00000029);

    t.setTimeAndFlags (0xffffffff);
    t.setFrame (0);
    assert (t.frame() == 0);
    assert (t.timeAndFlags() == 0xffffffc0);

    // Seconds

    t.setTimeAndFlags (0x00000000);
    t.setSeconds (59);
    assert (t.seconds() == 59);
    assert (t.timeAndFlags() == 0x00005900);

    t.setTimeAndFlags (0xffffffff);
    t.setSeconds (0);
    assert (t.seconds() == 0);
    assert (t.timeAndFlags() == 0xffff80ff);

    // Minutes

    t.setTimeAndFlags (0x00000000);
    t.setMinutes (59);
    assert (t.minutes() == 59);
    assert (t.timeAndFlags() == 0x00590000);

    t.setTimeAndFlags (0xffffffff);
    t.setMinutes (0);
    assert (t.minutes() == 0);
    assert (t.timeAndFlags() == 0xff80ffff);

    // Hours

    t.setTimeAndFlags (0x00000000);
    t.setHours (23);
    assert (t.hours() == 23);
    assert (t.timeAndFlags() == 0x23000000);

    t.setTimeAndFlags (0xffffffff);
    t.setHours (0);
    assert (t.hours() == 0);
    assert (t.timeAndFlags() == 0xc0ffffff);

    // Drop frame flag

    t.setTimeAndFlags (0x00000000);
    t.setDropFrame (true);
    assert (t.dropFrame() == true);
    assert (t.timeAndFlags() == 0x00000040);

    t.setTimeAndFlags (0xffffffff);
    t.setDropFrame (false);
    assert (t.dropFrame() == false);
    assert (t.timeAndFlags() == 0xffffffbf);

    // Color frame flag

    t.setTimeAndFlags (0x00000000);
    t.setColorFrame (true);
    assert (t.colorFrame() == true);
    assert (t.timeAndFlags() == 0x00000080);

    t.setTimeAndFlags (0xffffffff);
    t.setColorFrame (false);
    assert (t.colorFrame() == false);
    assert (t.timeAndFlags() == 0xffffff7f);

    // Field/phase flag

    t.setTimeAndFlags (0x00000000);
    t.setFieldPhase (true);
    assert (t.fieldPhase() == true);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0x00008000);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0x80000000);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0x00008000);

    t.setTimeAndFlags (0xffffffff);
    t.setFieldPhase (false);
    assert (t.fieldPhase() == false);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0xffff7fff);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0x7fffffbf);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0xffff7f3f);

    t.setTimeAndFlags (0x23595929 | 0x00008000, TimeCode::TV60_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x00008000));

    t.setTimeAndFlags (0x23595929 | 0x80000000, TimeCode::TV50_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x00008000));

    t.setTimeAndFlags (0x23595929 | 0x00008000, TimeCode::FILM24_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x00008000));

    // bgf0

    t.setTimeAndFlags (0x00000000);
    t.setBgf0 (true);
    assert (t.bgf0() == true);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0x00800000);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0x00008000);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0x00800000);

    t.setTimeAndFlags (0xffffffff);
    t.setBgf0 (false);
    assert (t.bgf0() == false);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0xff7fffff);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0xffff7fbf);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0xff7fff3f);

    t.setTimeAndFlags (0x23595929 | 0x00800000, TimeCode::TV60_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x00800000));

    t.setTimeAndFlags (0x23595929 | 0x00008000, TimeCode::TV50_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x00800000));

    t.setTimeAndFlags (0x23595929 | 0x00800000, TimeCode::FILM24_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x00800000));

    // bgf1

    t.setTimeAndFlags (0x00000000);
    t.setBgf1 (true);
    assert (t.bgf1() == true);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0x40000000);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0x40000000);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0x40000000);

    t.setTimeAndFlags (0xffffffff);
    t.setBgf1 (false);
    assert (t.bgf1() == false);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0xbfffffff);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0xbfffffbf);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0xbfffff3f);

    t.setTimeAndFlags (0x23595929 | 0x40000000, TimeCode::TV60_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x40000000));

    t.setTimeAndFlags (0x23595929 | 0x40000000, TimeCode::TV50_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x40000000));

    t.setTimeAndFlags (0x23595929 | 0x40000000, TimeCode::FILM24_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x40000000));

    // bgf2

    t.setTimeAndFlags (0x00000000);
    t.setBgf2 (true);
    assert (t.bgf2() == true);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0x80000000);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0x00800000);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0x80000000);

    t.setTimeAndFlags (0xffffffff);
    t.setBgf2 (false);
    assert (t.bgf2() == false);
    assert (t.timeAndFlags (TimeCode::TV60_PACKING)   == 0x7fffffff);
    assert (t.timeAndFlags (TimeCode::TV50_PACKING)   == 0xff7fffbf);
    assert (t.timeAndFlags (TimeCode::FILM24_PACKING) == 0x7fffff3f);

    t.setTimeAndFlags (0x23595929 | 0x80000000, TimeCode::TV60_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x80000000));

    t.setTimeAndFlags (0x23595929 | 0x00800000, TimeCode::TV50_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x80000000));

    t.setTimeAndFlags (0x23595929 | 0x80000000, TimeCode::FILM24_PACKING);
    assert (t.timeAndFlags() == (0x23595929 | 0x80000000));

    // User-defined data
    
    t.setUserData (0x87654321);
    assert (t.userData() == 0x87654321);

    assert (t.binaryGroup (1) == 1);
    assert (t.binaryGroup (2) == 2);
    assert (t.binaryGroup (3) == 3);
    assert (t.binaryGroup (4) == 4);
    assert (t.binaryGroup (5) == 5);
    assert (t.binaryGroup (6) == 6);
    assert (t.binaryGroup (7) == 7);

    t.setBinaryGroup (1, 2);
    t.setBinaryGroup (2, 3);
    t.setBinaryGroup (3, 4);
    t.setBinaryGroup (4, 5);
    t.setBinaryGroup (5, 6);
    t.setBinaryGroup (6, 7);
    t.setBinaryGroup (7, 8);
    t.setBinaryGroup (8, 9);

    assert (t.userData() == 0x98765432);
    
    // Assignment

    TimeCode t1 (12, 17, 57, 14,	   // hours, minutes, seconds, frame
		 false, false, false,	   // dropFrame, colorFrame, fieldPhase
		 false, false, false,	   // bgf0, bgf1, bgf2
		 1, 2, 3, 4, 5, 6, 7, 8);  // binary groups 1 to 8
    t = t1;

    assert (t.timeAndFlags() == 0x12175714);
    assert (t.userData() == 0x87654321);
}


void
writeReadTimeCode (const char fileName[])
{
    cout << "time code attribute" << endl;

    cout << "writing, ";

    TimeCode t1 (0x23595829, 0x12345678, TimeCode::FILM24_PACKING);

    assert (t1.timeAndFlags (TimeCode::FILM24_PACKING) == 0x23595829);
    assert (t1.userData() == 0x12345678);

    static const int W = 100;
    static const int H = 100;

    Header header (W, H);
    assert (hasTimeCode (header) == false);

    addTimeCode (header, t1);
    assert (hasTimeCode (header) == true);

    {
	RgbaOutputFile out (fileName, header);
	Rgba pixels[W];

	for (int i = 0; i < W; ++i)
	{
	    pixels[i].r = 1;
	    pixels[i].g = 1;
	    pixels[i].b = 1;
	    pixels[i].a = 1;
	}

	out.setFrameBuffer (pixels, 1, 0);
	out.writePixels (H);
    }

    cout << "reading, comparing" << endl;

    {
	RgbaInputFile in (fileName);
	const TimeCode &t2 = timeCode (in.header());

	assert (hasTimeCode (in.header()) == true);
	assert (t1.timeAndFlags() == t2.timeAndFlags());
	assert (t1.userData() == t2.userData());
    }

    remove (fileName);
}


bool
equal (const Rational &a, const Rational &b)
{
    return a.n == b.n && a.d == b.d;
}


void
rationalMethods ()
{
    cout << "rational methods" << endl;

    Rational r0 (0, 1);
    assert (r0 == 0);

    Rational r1 (1, 1);
    assert (r1 == 1);

    Rational r2 (1, 4);
    assert (r2 == 0.25);

    Rational r3 (-8, 2);
    assert (r3 == -4);

    Rational r4 (0.0);
    assert (r4 == 0);

    Rational r5 (1e-50);
    assert (r5 == 0);

    Rational r6 (1.0);
    assert (r6 == 1.0 && r6.n == 1 && r6.d == 1);

    Rational r7 (-10.0);
    assert (r7 == -10.0 && r7.n == -10 && r7.d == 1);

    Rational r8 (0.03125);
    assert (r8 == 0.03125 && r8.n == 1 && r8.d == 32);

    Rational r9 (0.53125);
    assert (r9 == 0.53125 && r9.n == 17 && r9.d == 32);

    Rational r10 (10.1);
    assert (equalWithAbsError (double (r10), 10.1, 1e-8));

    Rational r11 (double ((1U << 30) - 1));
    assert (r11 == double ((1U << 30) - 1));

    assert (equal (guessExactFps (23.976), fps_23_976()));
    assert (equal (guessExactFps (24.000), fps_24()));
    assert (equal (guessExactFps (25.000), fps_25()));
    assert (equal (guessExactFps (29.970), fps_29_97()));
    assert (equal (guessExactFps (30.000), fps_30()));
    assert (equal (guessExactFps (47.952), fps_47_952()));
    assert (equal (guessExactFps (48.000), fps_48()));
    assert (equal (guessExactFps (50.000), fps_50()));
    assert (equal (guessExactFps (59.940), fps_59_94()));
    assert (equal (guessExactFps (60.000), fps_60()));
    assert (equal (guessExactFps (70.500), Rational (141, 2)));
}


void
writeReadRational (const char fileName[])
{
    cout << "rational attribute" << endl;

    cout << "writing, ";

    Rational r1 (12, 17);
    Rational r2 (-12, 3);

    static const int W = 100;
    static const int H = 100;

    Header header (W, H);
    header.insert ("r1", RationalAttribute (r1));
    header.insert ("r2", RationalAttribute (r2));

    {
	RgbaOutputFile out (fileName, header);
	Rgba pixels[W];

	for (int i = 0; i < W; ++i)
	{
	    pixels[i].r = 1;
	    pixels[i].g = 1;
	    pixels[i].b = 1;
	    pixels[i].a = 1;
	}

	out.setFrameBuffer (pixels, 1, 0);
	out.writePixels (H);
    }

    cout << "reading, comparing" << endl;

    {
	RgbaInputFile in (fileName);

	const Rational &r3 =
	    in.header().typedAttribute<RationalAttribute>("r1").value();

	const Rational &r4 =
	    in.header().typedAttribute<RationalAttribute>("r2").value();

	assert (equal (r1, r3));
	assert (equal (r2, r4));
    }

    remove (fileName);
}


void
generatedFunctions ()
{
    //
    // Most optional standard attributes are of type string, float,
    // etc.  The attribute types are already being tested elsewhere
    // (testAttributes.C), and the convenience functions to access
    // the standard attributes are all generated via macros.  Here
    // we just verify that all the convenience functions exist
    // (that is, ImfStandardAttributes.C and ImfStandardAttributes.h
    // contain the right macro invocations).  If any functions are
    // missing, we should get an error during compiling or linking.
    //

    cout << "automatically generated functions" << endl;

    Header header;

    assert (hasChromaticities (header) == false);
    assert (hasWhiteLuminance (header) == false);
    assert (hasAdoptedNeutral (header) == false);
    assert (hasRenderingTransform (header) == false);
    assert (hasLookModTransform (header) == false);
    assert (hasXDensity (header) == false);
    assert (hasOwner (header) == false);
    assert (hasComments (header) == false);
    assert (hasCapDate (header) == false);
    assert (hasUtcOffset (header) == false);
    assert (hasLongitude (header) == false);
    assert (hasLatitude (header) == false);
    assert (hasAltitude (header) == false);
    assert (hasFocus (header) == false);
    assert (hasExpTime (header) == false);
    assert (hasAperture (header) == false);
    assert (hasIsoSpeed (header) == false);
    assert (hasEnvmap (header) == false);
    assert (hasKeyCode (header) == false);
    assert (hasTimeCode (header) == false);
    assert (hasWrapmodes (header) == false);
    assert (hasFramesPerSecond (header) == false);
    assert (hasMultiView (header) == false);
    assert (hasWorldToCamera (header) == false);
    assert (hasWorldToNDC (header) == false);
    assert (hasDeepImageState (header) == false);
    assert (hasOriginalDataWindow (header) == false);
}


} // namespace


void
testStandardAttributes (const std::string &tempDir)
{
    try
    {
	cout << "Testing optional standard attributes" << endl;

	convertRGBtoXYZ();

	{
	    std::string filename = tempDir + "imf_test_chromaticities.exr";
	    writeReadChromaticities (filename.c_str());
	}

	{
	    std::string fn1 = tempDir + "imf_test_latlong1.exr";
	    std::string fn2 = tempDir + "imf_test_latlong2.exr";
	    latLongMap (fn1.c_str(), fn2.c_str());
	}

	{
	    std::string fn1 = tempDir + "imf_test_cube1.exr";
	    std::string fn2 = tempDir + "imf_test_cube2.exr";
	    cubeMap (fn1.c_str(), fn2.c_str());
	}

	{
	    std::string filename = tempDir + "imf_test_keycode.exr";
	    writeReadKeyCode (filename.c_str());
	}

	{
	    timeCodeMethods();
	    std::string filename = tempDir + "imf_test_timecode.exr";
	    writeReadTimeCode (filename.c_str());
	}

	{
	    rationalMethods();
	    std::string filename = tempDir + "imf_test_rational.exr";
	    writeReadRational (filename.c_str());
	}

	generatedFunctions();

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
