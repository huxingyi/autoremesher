///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2013, Industrial Light & Magic, a division of Lucas
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

#include "testXdr.h"
#include "testMagic.h"
#include "testHuf.h"
#include "testWav.h"
#include "testChannels.h"
#include "testAttributes.h"
#include "testCustomAttributes.h"
#include "testLineOrder.h"
#include "testCompression.h"
#include "testCopyPixels.h"
#include "testRgba.h"
#include "testRgbaThreading.h"
#include "testLut.h"
#include "testSampleImages.h"
#include "testPreviewImage.h"
#include "testConversion.h"
#include "testStandardAttributes.h"
#include "testNativeFormat.h"
#include "testTiledRgba.h"
#include "testTiledCompression.h"
#include "testTiledCopyPixels.h"
#include "testTiledLineOrder.h"
#include "testScanLineApi.h"
#include "testExistingStreams.h"
#include "testYca.h"
#include "testTiledYa.h"
#include "testIsComplete.h"
#include "testLargeDataWindowOffsets.h"
#include "testSharedFrameBuffer.h"
#include "testMultiView.h"
#include "testMultiPartApi.h"
#include "testMultiPartSharedAttributes.h"
#include "testMultiPartThreading.h"
#include "testMultiScanlinePartThreading.h"
#include "testMultiTiledPartThreading.h"
#include "testDeepScanLineBasic.h"
#include "testCopyDeepScanLine.h"
#include "testDeepScanLineMultipleRead.h"
#include "testDeepScanLineHuge.h"
#include "testDeepTiledBasic.h"
#include "testCopyDeepTiled.h"
#include "testCompositeDeepScanLine.h"
#include "testMultiPartFileMixingBasic.h"
#include "testInputPart.h"
#include "testBackwardCompatibility.h"
#include "testCopyMultiPartFile.h"
#include "testPartHelper.h"
#include "testOptimized.h"
#include "testOptimizedInterleavePatterns.h"
#include "testBadTypeAttributes.h"
#include "testFutureProofing.h"
#include "testPartHelper.h"
#include "testDwaCompressorSimd.h"
#include "testRle.h"

#include "tmpDir.h"
#include "ImathRandom.h"

// system includes

#include <errno.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#    include <windows.h>
#else
#    include <unistd.h>
#endif
#include <sstream>

using namespace std;

#define TEST(x,y)                                                       \
    if (argc < 2 || (!strcmp (argv[1], #x) || !strcmp (argv[1], y)))    \
    {                                                                   \
        cout << "\n=======\nRunning " << #x <<endl;                     \
        x(tempDir);                                                     \
    }

int
main (int argc, char *argv[])
{
    // Create temporary files in a uniquely named private temporary
    // subdirectory of IMF_TMP_DIR to avoid colliding with other
    // running instances of this program.

    IMATH_NAMESPACE::Rand48 rand48 (time ((time_t*)0) );
    std::string tempDir;

    while (true)
    {
#ifdef _WIN32
        char  tmpbuf[4096];
        DWORD len = GetTempPathA (4096, tmpbuf);
        if (len == 0 || len > 4095)
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

        std::cout << "tempDir = " << tempDir << std::endl;

        int status = mkdir(tempDir.c_str(), 0777);

        if (status == 0)
        {
            tempDir += IMF_PATH_SEPARATOR;
            break; // success
        }

        if (errno != EEXIST)
        {
            std::cerr << "ERROR -- mkdir(" << tempDir << ") failed: "
                         "errno = " << errno << std::endl;
            return 1;
        }
    }

    TEST (testMagic, "core");
    TEST (testXdr, "core");
    TEST (testHuf, "core");
    TEST (testWav, "core");
    TEST (testRgba, "basic");
    TEST (testLargeDataWindowOffsets, "basic");
    TEST (testSharedFrameBuffer, "basic");
    TEST (testRgbaThreading, "basic");
    TEST (testChannels, "basic");
    TEST (testAttributes, "core");
    TEST (testCustomAttributes, "core");
    TEST (testLineOrder, "basic");
    TEST (testCompression, "basic");
    TEST (testCopyPixels, "basic");
    TEST (testLut, "basic");
    TEST (testSampleImages, "basic");
    TEST (testPreviewImage, "basic");
    TEST (testConversion, "basic");
    TEST (testTiledRgba, "basic");
    TEST (testTiledCopyPixels, "basic");
    TEST (testTiledCompression, "basic");
    TEST (testTiledLineOrder, "basic");
    TEST (testScanLineApi, "basic");
    TEST (testExistingStreams, "core");
    TEST (testStandardAttributes, "core");
    TEST (testOptimized, "basic");
    TEST (testOptimizedInterleavePatterns, "basic");
    TEST (testYca, "basic");
    TEST (testTiledYa, "basic");
    TEST (testNativeFormat, "basic");
    TEST (testMultiView, "basic");
    TEST (testIsComplete, "basic");
    TEST (testDeepScanLineBasic, "deep");
    TEST (testCopyDeepScanLine, "deep");
    TEST (testDeepScanLineMultipleRead, "deep");
    TEST (testDeepTiledBasic, "deep");
    TEST (testCopyDeepTiled, "deep");
    TEST (testCompositeDeepScanLine, "deep");
    TEST (testMultiPartFileMixingBasic, "multi");
    TEST (testInputPart, "multi");
    TEST (testPartHelper, "multi");
    TEST (testBadTypeAttributes, "multi");
    TEST (testMultiScanlinePartThreading, "multi");
    TEST (testMultiTiledPartThreading, "multi");
    TEST (testMultiPartThreading, "multi");
    TEST (testMultiPartApi, "multi");
    TEST (testMultiPartSharedAttributes, "multi");
    TEST (testCopyMultiPartFile, "multi");
    TEST (testBackwardCompatibility, "core");
    TEST (testFutureProofing, "core");
    TEST (testDwaCompressorSimd, "basic");
    TEST (testRle, "core");


    //#ifdef ENABLE_IMFHUGETEST
    // defined via configure with --enable-imfhugetest=yes/no
#if 0
        TEST (testDeepScanLineHuge, "deep");
#endif    


    std::cout << "removing temp dir " << tempDir << std::endl;
    rmdir (tempDir.c_str());

#ifdef OPENEXR_IMF_HAVE_LINUX_PROCFS

        //
        // Allow the user to check for file descriptor leaks
        //

        std::cout << "open file descriptors:" << std::endl;

        std::stringstream ss;
        ss << "ls -lG /proc/" << getpid() << "/fd";
            
        if (system (ss.str().c_str()) == -1)
        {
            std::cout << "failed to run ls\n";
        }

        std::cout << std::endl;

#endif

    return 0;
}
