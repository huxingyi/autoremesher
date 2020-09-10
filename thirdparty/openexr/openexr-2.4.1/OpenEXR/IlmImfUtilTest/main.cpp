///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014, Industrial Light & Magic, a division of Lucas
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

#include "ImfNamespace.h"
#include "OpenEXRConfigInternal.h"

#include "testFlatImage.h"
#include "testDeepImage.h"
#include "testIO.h"
#include "tmpDir.h"
#include <ImathRandom.h>

#include <cerrno>
#include <cstdlib>
#include <iostream>
#include <cstring>
#include <time.h>

#ifdef _WIN32
# include <windows.h>
#else
# include <unistd.h>
#endif

using namespace std;

#define TEST(test) {if (argc < 2 || !strcmp (argv[1], #test)) test(tempDir);}


int
main (int argc, char *argv[])
{
    //
    // Create temporary files in a uniquely named private temporary
    // subdirectory of IMF_TMP_DIR to avoid colliding with other running
    // instances of this program.
    //

    IMATH_NAMESPACE::Rand48 rand48 (time ((time_t*)0) );
    std::string tempDir;

    while (true)
    {
#ifdef _WIN32
        char tmpbuf[4096];
        DWORD len = GetTempPathA(4096, tmpbuf);
        if ( len == 0 || len > 4095 )
        {
            cerr << "Cannot retrieve temporary directory" << endl;
            return 1;
        }
        tempDir = tmpbuf;
        // windows does this automatically
        // tempDir += IMF_PATH_SEPARATOR;
        tempDir += "IlmImfTest_";
#else
        tempDir = IMF_TMP_DIR "IlmImfTest_";
#endif
        for (int i = 0; i < 8; ++i)
            tempDir += ('A' + rand48.nexti() % 26);

        if (mkdir (tempDir.c_str(), 0777) == 0)
        {
            tempDir += IMF_PATH_SEPARATOR;
            break; // success
        }

        if (errno != EEXIST)
        {
            cerr << "Cannot create directory " << tempDir << ". " <<
                    strerror (errno) << endl;

            return 1;
        }
    }

    cout << "using temporary directory " << tempDir << endl;

    TEST (testFlatImage);
    TEST (testDeepImage);
    TEST (testIO);

    cout << "removing temporary directory " << tempDir << endl;
    rmdir (tempDir.c_str());

    return 0;
}
