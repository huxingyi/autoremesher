///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007, Industrial Light & Magic, a division of Lucas
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

#include <fuzzFile.h>

#include <ImfRgbaFile.h>
#include <ImfArray.h>
#include <Iex.h>
#include <half.h>

#include <iostream>
#include <fstream>
#include "../IlmImfTest/TestUtilFStream.h"

// Handle the case when the custom namespace is not exposed
#include <OpenEXRConfig.h>
using namespace OPENEXR_IMF_INTERNAL_NAMESPACE;
using namespace std;
using namespace IMATH_NAMESPACE;


namespace {

Int64
lengthOfFile (const char fileName[])
{
    ifstream ifs;
    testutil::OpenStreamWithUTF8Name (
        ifs, fileName, ios::in | ios_base::binary);

    if (!ifs)
	return 0;

    ifs.seekg (0, ios_base::end);
    return ifs.tellg();
}


void
fuzzFile (const char goodFile[],
          const char brokenFile[],
	  Int64 offset,
	  Int64 windowSize,
	  Rand48 &random,
	  double fuzzAmount)
{
    //
    // Read the input file.
    //

    ifstream ifs;
    testutil::OpenStreamWithUTF8Name (
        ifs, goodFile, ios::in | ios_base::binary);

    if (!ifs)
	THROW_ERRNO ("Cannot open file " << goodFile << " (%T).");

    ifs.seekg (0, ios_base::end);
    Int64 fileLength = ifs.tellg();
    ifs.seekg (0, ios_base::beg);

    Array<char> data (fileLength);
    ifs.read (data, fileLength);

    if (!ifs)
	THROW_ERRNO ("Cannot read file " << goodFile << " (%T)." << endl);

    //
    // Damage the contents of the file by overwriting some of the bytes
    // in a window of size windowSize, starting at the specified offset.
    // 

    for (Int64 i = offset; i < offset + windowSize; ++i)
    {
	if (random.nextf() < fuzzAmount)
	    data[i] = char (random.nexti());
    }

    //
    // Save the damaged file contents in the output file.
    //

    ofstream ofs;
    testutil::OpenStreamWithUTF8Name (
        ofs, brokenFile, ios::out | ios_base::binary);

    if (!ofs)
	THROW_ERRNO ("Cannot open file " << brokenFile << " (%T)." << endl);

    ofs.write (data, fileLength);

    if (!ofs)
	THROW_ERRNO ("Cannot write file " << brokenFile << " (%T)." << endl);
}

} // namespace


void
fuzzFile (const char goodFile[],
          const char brokenFile[],
	  void (*readFile) (const char[]),
	  int nSlidingWindow,
	  int nFixedWindow,
	  Rand48 &random)
{
    //
    // We want to test how resilient the IlmImf library is with respect
    // to malformed OpenEXR input files.  In order to do this we damage
    // a good input file by overwriting parts of it with random data.
    // We then call function readFile() to try and read the damaged file.
    // Provided the IlmImf library works as advertised, a try/catch(...)
    // block in readFile() should be able to handle all errors that could
    // possibly result from reading a broken OpenEXR file.  We repeat
    // this damage/read cycle many times, overwriting different parts
    // of the file:
    //
    // First we slide a window along the file.  The size of the window
    // is fileSize*2/nSlidingWindow bytes.  In each damage/read cycle
    // we overwrite up to 10% of the bytes the window, try to read the
    // file, and advance the window by fileSize/nSlidingWindow bytes.
    //
    // Next we overwrite up to 10% of the file's first 2048 bytes and
    // try to read the file.  We repeat this nFixedWindow times.
    //

    {
	Int64 fileSize = lengthOfFile (goodFile);
	Int64 windowSize = fileSize * 2 / nSlidingWindow;
	Int64 lastWindowOffset = fileSize - windowSize;

	cout << "sliding " << windowSize << "-byte window" << endl;

	for (int i = 0; i < nSlidingWindow; ++i)
	{
	    if (i % 100 == 0)
		cout << i << "\r" << flush;

	    Int64 offset = lastWindowOffset * i / (nSlidingWindow - 1);
	    double fuzzAmount = random.nextf (0.0, 0.1);

	    fuzzFile (goodFile, brokenFile,
		      offset, windowSize,
		      random, fuzzAmount);

	    readFile (brokenFile);
	}

	cout << nSlidingWindow << endl;
    }

    {
	Int64 windowSize = 2048;

	cout << windowSize << "-byte window at start of file" << endl;

	for (int i = 0; i < nFixedWindow; ++i)
	{
	    if (i % 100 == 0)
		cout << i << "\r" << flush;

	    double fuzzAmount = random.nextf (0.0, 0.1);

	    fuzzFile (goodFile, brokenFile,
		      0, windowSize,
		      random, fuzzAmount);

	    readFile (brokenFile);
	}

	cout << nFixedWindow << endl;
    }
}
