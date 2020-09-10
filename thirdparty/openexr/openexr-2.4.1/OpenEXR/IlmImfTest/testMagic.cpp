//////////////////////////////////////////////////////////////////////////
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

#include <ImfVersion.h>
#include <ImfTestFile.h>
#include <ImfStdIO.h>
#include <iostream>
#include <exception>
#include <stdio.h>
#include <assert.h>

#include "TestUtilFStream.h"

#ifndef ILM_IMF_TEST_IMAGEDIR
    #define ILM_IMF_TEST_IMAGEDIR
#endif


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;


namespace {

void
testFile1 (const char fileName[], bool isImfFile)
{
    cout << fileName << " " << flush;

    ifstream f;
    testutil::OpenStreamWithUTF8Name (f, fileName, ios::in | ios_base::binary);
    assert (!!f);

    char bytes[4];
    f.read (bytes, sizeof (bytes));

    assert (!!f && isImfFile == isImfMagic (bytes));

    cout << "is " << (isImfMagic (bytes)? "": "not ") << "an OpenEXR file\n";
}


void
testFile2 (const char fileName[], bool exists, bool exrFile, bool tiledFile)
{
    cout << fileName << " " << flush;

    bool exr, tiled;

    exr = isOpenExrFile (fileName, tiled);
    assert (exr == exrFile && tiled == tiledFile);

    exr = isOpenExrFile (fileName);
    assert (exr == exrFile);

    tiled = isTiledOpenExrFile (fileName);
    assert (tiled == tiledFile);

    if (exists)
    {
	StdIFStream is (fileName);

	exr = isOpenExrFile (is, tiled);
	assert (exr == exrFile && tiled == tiledFile);

	if (exr)
	    assert (is.tellg() == 0);

	exr = isOpenExrFile (is);
	assert (exr == exrFile);

	if (exr)
	    assert (is.tellg() == 0);

	tiled = isTiledOpenExrFile (is);
	assert (tiled == tiledFile);

	if (tiled)
	    assert (is.tellg() == 0);
    }

    cout << (exists? "exists": "does not exist") << ", " <<
	    (exrFile? "is an OpenEXR file": "is not an OpenEXR file") << ", " <<
	    (tiledFile? "is tiled": "is not tiled") << endl;
}

} // namespace


void
testMagic (const std::string &)
{
    try
    {
	cout << "Testing magic number" << endl;

	testFile1 (ILM_IMF_TEST_IMAGEDIR "comp_none.exr", true);
	testFile1 (ILM_IMF_TEST_IMAGEDIR "invalid.exr", false);

	testFile2 (ILM_IMF_TEST_IMAGEDIR "tiled.exr", true, true, true);
	testFile2 (ILM_IMF_TEST_IMAGEDIR "comp_none.exr", true, true, false);
	testFile2 (ILM_IMF_TEST_IMAGEDIR "invalid.exr", true, false, false);
	testFile2 (ILM_IMF_TEST_IMAGEDIR "does_not_exist.exr", false, false, false);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
