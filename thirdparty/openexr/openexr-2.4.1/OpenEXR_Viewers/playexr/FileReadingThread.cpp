///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006, Industrial Light & Magic, a division of Lucas
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

//-----------------------------------------------------------------------------
//
//	class FileReadingThread
//
//-----------------------------------------------------------------------------

#include "FileReadingThread.h"
#include "fileNameForFrame.h"
#include "ImageBuffers.h"
#include <ImfInputFile.h>
#include <Iex.h>
#include <iostream>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;


FileReadingThread::FileReadingThread
    (const char fileNameTemplate[],
     int firstFrame,
     int lastFrame,
     ImageBuffers &imageBuffers)
:
    Thread(),
    _fileNameTemplate (fileNameTemplate),
    _firstFrame (firstFrame),
    _lastFrame (lastFrame),
    _imageBuffers (imageBuffers)
{
    start();	// start() calls run()
}


void
FileReadingThread::run ()
{
    try
    {
	int i = 0;	// index of the image buffer we will fill next
	int frame = _firstFrame;

	while (true)
	{
	    //
	    // Check if the display thread wants us to exit.
	    //

	    if (_imageBuffers.exitSemaphore1.tryWait())
	    {
		_imageBuffers.exitSemaphore2.post();
		return;
	    }

	    //
	    // Wait for an image buffer to become available.
	    //

	    _imageBuffers.emptyBuffersSemaphore.wait();

	    //
	    // Generate the file name for this frame
	    // and open the corresponding OpenEXR file.
	    //

	    string fileName = fileNameForFrame (_fileNameTemplate, frame);
	    InputFile in (fileName.c_str());

	    //
	    // Verify that this frame has the same data window
	    // as all other frames. (We do not dynamically resize
	    // our image buffers.)
	    //

	    if (in.header().dataWindow() != _imageBuffers.dataWindow)
		THROW (ArgExc,
		       "Data window of frame " << frame << " "
		       "differs from data window of "
		       "frame " << _firstFrame << ".");

	    //
	    // Read the OpenEXR file, storing the pixels in
	    // image buffer i.
	    //

	    in.setFrameBuffer (_imageBuffers.frameBuffer (i));

	    in.readPixels (_imageBuffers.dataWindow.min.y,
			   _imageBuffers.dataWindow.max.y);

	    //
	    // Mark the image buffer as full; the display
	    // thread can now display this frame.
	    //

	    _imageBuffers.frameNumber (i) = frame;
	    _imageBuffers.fullBuffersSemaphore.post();

	    //
	    // Advance to the next frame
	    //

	    if (_imageBuffers.forward)
	    {
		if (frame >= _lastFrame)
		    frame = _firstFrame;
		else
		    frame += 1;
	    }
	    else
	    {
		if (frame <= _firstFrame)
		    frame = _lastFrame;
		else
		    frame -= 1;
	    }

	    i = (i + 1) % _imageBuffers.numBuffers();
	}
    }
    catch (const std::exception &exc)
    {
	//
	// If anything goes wrong, print an eror message and exit.
	//

	cerr << exc.what() << endl;

	_imageBuffers.exitSemaphore2.post();
	_imageBuffers.fullBuffersSemaphore.post();
	return;
    }
}
