///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004, Industrial Light & Magic, a division of Lucas
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
//	Code examples that show how implement custom low-level
//	file input and output for OpenEXR files:
//
//	Classes C_IStream and C_OStream are derived from IlmImf's
//	abstract IStream and OStreamd classes.  They allow OpenEXR
//	file input and output via C stdio files (FILE *).
//
//-----------------------------------------------------------------------------

#include <ImfRgbaFile.h>
#include <ImfIO.h>
#include "Iex.h"

#include "drawImage.h"

#include <iostream>
#include <stdio.h>

#include "namespaceAlias.h"
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


class C_IStream: public IStream
{
  public:

    C_IStream (FILE *file, const char fileName[]):
	IStream (fileName), _file (file) {}

    virtual bool	read (char c[/*n*/], int n);
    virtual Int64	tellg ();
    virtual void	seekg (Int64 pos);
    virtual void	clear ();

  private:

    FILE *		_file;
};


class C_OStream: public OStream
{
  public:

    C_OStream (FILE *file, const char fileName[]):
	OStream (fileName), _file (file) {}

    virtual void	write (const char c[/*n*/], int n);
    virtual Int64	tellp ();
    virtual void	seekp (Int64 pos);

  private:

    FILE *		_file;
};


bool
C_IStream::read (char c[/*n*/], int n)
{
    if (n != static_cast<int>(fread (c, 1, n, _file)))
    {
        //
        // fread() failed, but the return value does not distinguish
        // between I/O errors and end of file, so we call ferror() to
        // determine what happened.
        //

        if (ferror (_file))
            IEX_NAMESPACE::throwErrnoExc();
        else
            throw IEX_NAMESPACE::InputExc ("Unexpected end of file.");
    }

    return feof (_file);
}


Int64
C_IStream::tellg ()
{
    return ftell (_file);
}


void
C_IStream::seekg (Int64 pos)
{
    clearerr (_file);
    fseek (_file, pos, SEEK_SET);
}


void
C_IStream::clear ()
{
    clearerr (_file);
}


void
C_OStream::write (const char c[/*n*/], int n)
{
    clearerr (_file);

    if (n != static_cast<int>(fwrite (c, 1, n, _file)))
        IEX_NAMESPACE::throwErrnoExc();
}


Int64
C_OStream::tellp ()
{
    return ftell (_file);
}


void
C_OStream::seekp (Int64 pos)
{
    clearerr (_file);
    fseek (_file, pos, SEEK_SET);
}


void
writeRgbaFILE (FILE *cfile,
	       const char fileName[],
	       const Rgba *pixels,
	       int width,
	       int height)
{
    //
    // Store an RGBA image in a C stdio file that has already been opened:
    //
    //	- create a C_OStream object for writing to the file
    //	- create an RgbaOutputFile object, and attach it to the C_OStream
    //	- describe the memory layout of the pixels
    //	- store the pixels in the file
    //


    C_OStream ostr (cfile, fileName);
    RgbaOutputFile file (ostr, Header (width, height), WRITE_RGBA);
    file.setFrameBuffer (pixels, 1, width);
    file.writePixels (height);
}


void
readRgbaFILE (FILE *cfile,
	      const char fileName[],
	      Array2D<Rgba> &pixels,
	      int &width,
	      int &height)
{
    //
    // Read an RGBA image from a C stdio file that has already been opened:
    //
    //	- create a C_IStream object for reading from the file
    //	- create an RgbaInputFile object, and attach it to the C_IStream
    //	- allocate memory for the pixels
    //	- describe the memory layout of the pixels
    //	- read the pixels from the file
    //

    C_IStream istr (cfile, fileName);
    RgbaInputFile file (istr);
    Box2i dw = file.dataWindow();

    width  = dw.max.x - dw.min.x + 1;
    height = dw.max.y - dw.min.y + 1;
    pixels.resizeErase (height, width);

    file.setFrameBuffer (&pixels[0][0] - dw.min.x - dw.min.y * width, 1, width);
    file.readPixels (dw.min.y, dw.max.y);
}


void
lowLevelIoExamples ()
{
    cout << "\nCustom low-level file I/O\n" << endl;
    cout << "drawing image" << endl;

    int w = 800;
    int h = 600;
    const char *fileName = "rgba4.exr";

    Array2D<Rgba> p (h, w);
    drawImage1 (p, w, h);

    //
    // The following code is somewhat complicated because
    // fopen() returns 0 on error, but writeRgbaFILE() and
    // readRgbaFILE() throw exceptions.  Also, if a FILE *
    // goes out of scope, the corresponding file is not
    // automatically closed.  In order to avoid leaking
    // file descriptors, we have to make sure fclose() is
    // called in all cases.
    //

    cout << "writing image" << endl;

    {
	FILE *file = fopen (fileName, "wb");

	if (file == 0)
	{
	    THROW_ERRNO ("Cannot open file " << fileName << " (%T).");
	}
	else
	{
	    try
	    {
		writeRgbaFILE (file, fileName, &p[0][0], w, h);
	    }
	    catch (...)
	    {
		fclose (file);
		throw;
	    }

	    fclose (file);
	}
    }

    cout << "reading image" << endl;

    {
	FILE *file = fopen (fileName, "rb");

	if (file == 0)
	{
	    THROW_ERRNO ("Cannot open file " << fileName << " (%T).");
	}
	else
	{
	    try
	    {
		readRgbaFILE (file, fileName, p, w, h);
	    }
	    catch (...)
	    {
		fclose (file);
		throw;
	    }

	    fclose (file);
	}
    }
}
