///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2003-2012, Industrial Light & Magic, a division of Lucas
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
#include <ImfRgbaFile.h>
#include <ImfArray.h>

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
readImage (const char fileName[],
           Array2D<Rgba>& pixels,
           int& w,
           int& h,
           unsigned int correctChecksum)
{
    RgbaInputFile in (fileName);

    const Box2i &dw = in.dataWindow();

    w = dw.max.x - dw.min.x + 1;
    h = dw.max.y - dw.min.y + 1;
    int dx = dw.min.x;
    int dy = dw.min.y;

    pixels.resizeErase (h, w);
    in.setFrameBuffer (&pixels[0][0] - dx - dy * w, 1, w);
    in.readPixels (in.dataWindow().min.y, in.dataWindow().max.y);

    unsigned int checksum = 0;

    for (int y = 0; y < h; ++y)
        for (int x = 0; x < w; ++x)
        {
            checksum ^= pixels[y][x].r.bits();
            checksum ^= pixels[y][x].g.bits();
            checksum ^= pixels[y][x].b.bits();
            checksum ^= pixels[y][x].a.bits();
        }

    cout << "checksum = " << checksum << flush;

    assert (checksum == correctChecksum);
    
    cout << ", ok" << flush;
}


void
readBackImage (const char fileName[],
               Array2D<Rgba>& pixels,
	       const Array2D<Rgba>& pixels2,
	       int& w,
	       int& h,
	       const int& xs,
	       const int& ys)
{
    InputFile file (fileName);

    Box2i dw = file.header().dataWindow();
    w = dw.max.x - dw.min.x + 1;
    h = dw.max.y - dw.min.y + 1;

    pixels.resizeErase (h, w);

    FrameBuffer frameBuffer;

    Rgba *base = &pixels[0][0] - dw.min.x - dw.min.y * w;
    int xStride = sizeof (pixels[0][0]) * xs;
    int yStride = sizeof (pixels[0][0]) * w * ys;

    frameBuffer.insert ("R",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].r,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys,                       // x/y sampling
                        0.0));                        // fillValue

    frameBuffer.insert ("G",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].g,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys,                       // x/y sampling
                        0.0));                        // fillValue
                        
    frameBuffer.insert ("B",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].b,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys,                       // x/y sampling
                        0.0));                        // fillValue
                        
    frameBuffer.insert ("A",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].a,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys,                       // x/y sampling
                        1.0));                        // fillValue

    file.setFrameBuffer (frameBuffer);
    file.readPixels (dw.min.y, dw.max.y);
    
    cout << "comparing, " << flush;
    for (int y = 0; y < h; y+=ys)
        for (int x = 0; x < w; x+=xs)
        {
            assert(pixels2[y][x].r.bits() == pixels[y][x].r.bits());
            assert(pixels2[y][x].g.bits() == pixels[y][x].g.bits());
            assert(pixels2[y][x].b.bits() == pixels[y][x].b.bits());
            assert(pixels2[y][x].a.bits() == pixels[y][x].a.bits());
        }
    cout << "ok" << endl << flush;

}


void
writeImage (const char fileName[],
	    const Array2D<OPENEXR_IMF_NAMESPACE::Rgba>& pixels,
	    const int& width,
	    const int& height,
	    const int& xs = 1,
	    const int& ys = 1)
{
    //
    // Write the image to fileName one scanline at a time
    //

    Header header (width, height);
    header.compression() = PIZ_COMPRESSION;
    header.channels().insert ("R", Channel (HALF,xs,ys));
    header.channels().insert ("G", Channel (HALF,xs,ys));
    header.channels().insert ("B", Channel (HALF,xs,ys));
    header.channels().insert ("A", Channel (HALF,xs,ys));

    OutputFile file (fileName, header);    
    FrameBuffer frameBuffer;

    const Rgba *base = &pixels[0][0];
    int xStride = sizeof (pixels[0][0]) * xs;
    int yStride = sizeof (pixels[0][0]) * 0;

    frameBuffer.insert ("R",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].r,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys));                     // x/y sampling

    frameBuffer.insert ("G",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].g,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys));                     // x/y sampling

    frameBuffer.insert ("B",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].b,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys));                     // x/y sampling

    frameBuffer.insert ("A",                          // name
                        Slice (HALF,                  // type
                        (char *) &base[0].a,          // base
                        xStride,                      // xStride
                        yStride,                      // yStride
                        xs, ys));                     // x/y sampling

    // iterate over all scanlines, and write them out
    for (int y = 0; y < height; ++y)
    {
        // set the base address for this scanline
        base = &pixels[y][0];
        frameBuffer["R"].base = (char *) &base[0].r;
        frameBuffer["G"].base = (char *) &base[0].g;
        frameBuffer["B"].base = (char *) &base[0].b;
        frameBuffer["A"].base = (char *) &base[0].a;
        
        // set the framebuffer and write the pixels
        file.setFrameBuffer (frameBuffer);
        file.writePixels (1);
    }
}


void
readCopyRead (const std::string &tempDir,
              const char* infilename,
              unsigned int correctChecksum)
{
    std::string outfilename = tempDir + "imf_test_native.exr";

    int w, h;
    Array2D<OPENEXR_IMF_NAMESPACE::Rgba> pixels (1,1);
    
    cout << "   reading, " << flush;
    readImage(infilename, pixels, w, h, correctChecksum);
    cout << endl;
    
    for (int xs = 1; xs <= 2; ++xs)
    {
        for (int ys = 1; ys <= 2; ++ys)
        {
            cout << "   x sampling " << xs << ", y sampling " << ys <<
                    ": writing image, " << flush;
            writeImage(outfilename.c_str(), pixels, w, h, xs, ys);
            
            Array2D<OPENEXR_IMF_NAMESPACE::Rgba> pixels2 (1,1);
            cout << "reading back, " << flush;
            readBackImage(outfilename.c_str(), pixels2, pixels, w, h, xs, ys);

            remove(outfilename.c_str());
        }
    }            
}

} // namespace


void
testNativeFormat (const std::string &tempDir)
{
    try
    {
        cout << "Testing if uncompressible pixel data are written "
		"in Xdr, not native format" << endl;

        cout << "image 1:" << endl;
        readCopyRead(tempDir, ILM_IMF_TEST_IMAGEDIR "test_native1.exr", 54435);
        
        cout << "image 2:" << endl;
        readCopyRead(tempDir, ILM_IMF_TEST_IMAGEDIR "test_native2.exr", 37639);
        
	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
        cerr << "ERROR -- caught exception: " << e.what() << endl;
        assert (false);
    }
}
