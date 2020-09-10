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
#include <ImfArray.h>
#include <string>
#include <ImathRandom.h>
#include <ImfThreading.h>
#include <IlmThread.h>
#include <IlmThreadMutex.h>
#include <IlmThreadSemaphore.h>
#include <ImfThreading.h>

#include <stdio.h>
#include <assert.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;


namespace {

Rand48 rand1 (27);
Mutex scanlineMutex;
Semaphore threadSemaphore;
int remainingScanlines = 0;


void
fillPixels (Array2D<Rgba> &pixels, int w, int h)
{
    assert (sizeof (int) == sizeof (float));
    
    for (int y = 0; y < h; ++y)
    {
	for (int x = 0; x < w; ++x)
	{
	    Rgba &p = pixels[y][x];

	    p.r = x % 100 + 100 * (y % 100) +
		  rand1.nextf (-0.01f, -0.01f);

	    p.g = 0.5 + 0.5 * sin (0.1 * x + 0.2 * y) +
                  rand1.nextf (-0.01f, -0.01f);

	    p.b = 0.5 + 0.5 * sin (0.1 * x + 0.3 * y) +
                  rand1.nextf (-0.01f, -0.01f);

	    p.a = rand1.nextf (0.0f, 1.0f);
	}
    }
}


class WriterThread: public Thread
{
  public:

    WriterThread (RgbaOutputFile *outfile);

    virtual void	run ();

  private:

    RgbaOutputFile *	_outfile;
};


WriterThread::WriterThread (RgbaOutputFile *outfile): _outfile (outfile)
{
    start();
}

void
WriterThread::run ()
{
    //
    // Signal that the thread has started
    //

    threadSemaphore.post();

    while (true)
    {
	Lock lock (scanlineMutex);

	if (remainingScanlines)
	{
	    int n = int (rand1.nextf (0.0f, remainingScanlines) + 0.5f);
	    _outfile->writePixels (n);
	    remainingScanlines -= n;
	}
	else
	{
	    break;
	}
    }
}
    

class ReaderThread : public Thread
{
  public:

    ReaderThread (RgbaInputFile *infile, int start, int step);

    virtual void	run ();
    
  private:

    RgbaInputFile *	_infile;
    int			_start;
    int			_step;
};


ReaderThread::ReaderThread (RgbaInputFile *infile, int start, int step):
    _infile (infile), _start (start), _step (step)
{
    Thread::start();
}


void
ReaderThread::run ()
{
    //
    // Signal that the thread has started
    //

    threadSemaphore.post ();

    int num = _infile->header().dataWindow().max.y -
	      _infile->header().dataWindow().min.y + 1;

    for (int i = _start; i < num; i += _step)
	_infile->readPixels (i);
}
    

void
writeReadRGBA (const char fileName[],
	       int width,
	       int height,
	       const Array2D<Rgba> &p1,
	       RgbaChannels channels,
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
	    ", compression " << comp << ", " << flush;

    Header header (width, height);
    header.compression() = comp;

    {
	remove (fileName);
        cout << "writing " << flush;
	RgbaOutputFile out (fileName, header, channels);
        out.setFrameBuffer (&p1[0][0], 1, width);
        
        remainingScanlines = height;
        WriterThread writer1 (&out);
        WriterThread writer2 (&out);
        threadSemaphore.wait();
        threadSemaphore.wait();
    }

    {
        cout << "reading " << flush;
	RgbaInputFile in (fileName);
	const Box2i &dw = in.dataWindow();
	
	int w = dw.max.x - dw.min.x + 1;
	int h = dw.max.y - dw.min.y + 1;
	int dx = dw.min.x;
	int dy = dw.min.y;

	Array2D<Rgba> p2 (h, w);
	in.setFrameBuffer (&p2[-dy][-dx], 1, w);
        
        {
            ReaderThread reader1 (&in, 0, 2);
            ReaderThread reader2 (&in, 1, 2);
            threadSemaphore.wait();
            threadSemaphore.wait();
        }

	assert (in.displayWindow() == header.displayWindow());
	assert (in.dataWindow() == header.dataWindow());
	assert (in.pixelAspectRatio() == header.pixelAspectRatio());
	assert (in.screenWindowCenter() == header.screenWindowCenter());
	assert (in.screenWindowWidth() == header.screenWindowWidth());
	assert (in.lineOrder() == header.lineOrder());
	assert (in.compression() == header.compression());
	assert (in.channels() == channels);

        cout << "comparing " << endl;

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


} // namespace


void
testSharedFrameBuffer (const std::string &tempDir)
{
    try
    {
	cout << "Testing reading from and writing to files using\n"
		"multiple threads and a shared framebuffer" << endl;

        if (!ILMTHREAD_NAMESPACE::supportsThreads ())
        {
            cout << "   Threading not supported!" << endl << endl;
            return;
        }
                
	const int W = 1371;
	const int H = 159;
        
	Array2D<Rgba> p1 (H, W);
	fillPixels (p1, W, H);
        
        for (int n = 0; n <= 8; n++)
        {
            int numThreads = (n * 3) % 8;

            setGlobalThreadCount (numThreads);
            cout << "number of threads: " << globalThreadCount () << endl;

            for (int comp = 0; comp < NUM_COMPRESSION_METHODS; ++comp)
            {
                writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
                                W, H, p1,
                                WRITE_RGBA,
                                Compression (comp));

                writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
                                W, H, p1,
                                WRITE_RGB,
                                Compression (comp));

                writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
                                W, H, p1,
                                WRITE_A,
                                Compression (comp));

                writeReadRGBA ((tempDir + "imf_test_rgba.exr").c_str(),
                                W, H, p1,
                                RgbaChannels (WRITE_R | WRITE_B),
                                Compression (comp));
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
