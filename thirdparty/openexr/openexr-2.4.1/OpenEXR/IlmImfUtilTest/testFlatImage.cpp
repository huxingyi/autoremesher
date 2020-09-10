///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2014 Industrial Light & Magic, a division of Lucas
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

#include <ImfFlatImage.h>
#include <ImfFlatImageIO.h>
#include <ImfHeader.h>
#include <ImathRandom.h>
#include <Iex.h>

#include <cstdio>
#include <cassert>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

namespace {


template <class T>
void
verifyPixelsAreEqual
    (const FlatImageChannel &c1,
     const FlatImageChannel &c2,
     int dx,
     int dy)
{
    const TypedFlatImageChannel<T> &tc1 =
        dynamic_cast <const TypedFlatImageChannel<T>&> (c1);

    const TypedFlatImageChannel<T> &tc2 =
        dynamic_cast <const TypedFlatImageChannel<T>&> (c2);

    const Box2i &dataWindow = c1.level().dataWindow();
    int xStep = c1.xSampling();
    int yStep = c1.ySampling();

    for (int y = dataWindow.min.y; y <= dataWindow.max.y; y += yStep)
        for (int x = dataWindow.min.x; x <= dataWindow.max.x; x += xStep)
            if (tc1.at (x, y) != tc2.at (x + dx, y + dy))
                throw ArgExc ("different pixel values");
}


void
verifyLevelsAreEqual
    (const FlatImageLevel &level1,
     const FlatImageLevel &level2,
     int dx,
     int dy)
{
    if (level1.dataWindow().min.x != level2.dataWindow().min.x - dx ||
        level1.dataWindow().min.y != level2.dataWindow().min.y - dy ||
        level1.dataWindow().max.x != level2.dataWindow().max.x - dx ||
        level1.dataWindow().max.y != level2.dataWindow().max.y - dy)
    {
        throw ArgExc ("different data windows");
    }

    FlatImageLevel::ConstIterator i1 = level1.begin();
    FlatImageLevel::ConstIterator i2 = level2.begin();

    while (i1 != level1.end() && i2 != level2.end())
    {
        cout << "            channel " << i1.name() << endl;

        if (i1.name() != i2.name())
            throw ArgExc ("different channel names");

        if (i1.channel().pixelType() != i2.channel().pixelType())
            throw ArgExc ("different channel types");

        if (i1.channel().xSampling() != i2.channel().xSampling() ||
            i1.channel().ySampling() != i2.channel().ySampling())
            throw ArgExc ("different channel sampling rates");

        if (i1.channel().pLinear() != i2.channel().pLinear())
            throw ArgExc ("different channel types");

        switch (i1.channel().pixelType())
        {
          case HALF:

            verifyPixelsAreEqual <half>
                (i1.channel(), i2.channel(), dx, dy);

            break;

          case FLOAT:

            verifyPixelsAreEqual <float>
                (i1.channel(), i2.channel(), dx, dy);

            break;

          case UINT:

            verifyPixelsAreEqual <unsigned int>
                (i1.channel(), i2.channel(), dx, dy);

            break;

          default:
             assert (false);
        }

        ++i1;
        ++i2;
    }

    if (i1 != level1.end() || i2 != level2.end())
        throw ArgExc ("different channel lists");
}


void
verifyImagesAreEqual
    (const FlatImage &img1,
     const FlatImage &img2,
     int dx = 0,
     int dy = 0)
{
    if (img1.levelMode() != img2.levelMode())
        throw ArgExc ("different level modes");

    if (img1.levelRoundingMode() != img2.levelRoundingMode())
        throw ArgExc ("different level rounding modes");

    if (img1.numXLevels() != img2.numXLevels() ||
        img1.numYLevels() != img2.numYLevels())
        throw ArgExc ("different number of levels");

    switch (img1.levelMode())
    {
      case ONE_LEVEL:

        cout << "        level 0" << endl;

        verifyLevelsAreEqual
            (img1.level(), img2.level(), dx, dy);

        break;

      case MIPMAP_LEVELS:

        for (int x = 0; x < img1.numLevels(); ++x)
        {
            cout << "        level " << x << "" << endl;

            verifyLevelsAreEqual
                (img1.level (x), img2.level (x), dx, dy);
        }

        break;

      case RIPMAP_LEVELS:

        for (int y = 0; y < img1.numYLevels(); ++y)
        {
            for (int x = 0; x < img1.numXLevels(); ++x)
            {
                cout << "        level (" << x << ", " << y << ")" << endl;

                verifyLevelsAreEqual
                    (img1.level (x, y), img2.level (x, y), dx, dy);
            }
        }

        break;

      default:

        assert (false);
    }
}


template <class T>
void
fillChannel (Rand48 &random, FlatImageChannel &c)
{
    //
    // Fill image channel tc with random numbers
    //

    TypedFlatImageChannel<T> &tc =
        dynamic_cast <TypedFlatImageChannel<T>&> (c);

    const Box2i &dataWindow = tc.level().dataWindow();
    int xStep = tc.xSampling();
    int yStep = tc.ySampling();

    for (int y = dataWindow.min.y; y <= dataWindow.max.y; y += yStep)
        for (int x = dataWindow.min.x; x <= dataWindow.max.x; x += xStep)
            tc.at (x, y) = T (random.nextf (0.0, 100.0));
}


void
fillChannels (Rand48 &random, FlatImageLevel &level)
{
    for (FlatImageLevel::Iterator i = level.begin(); i != level.end(); ++i)
    {
        cout << "             channel " << i.name() << endl;

        switch (i.channel().pixelType())
        {
          case HALF:
            fillChannel <half> (random, i.channel());
            break;

          case FLOAT:
            fillChannel <float> (random, i.channel());
            break;

          case UINT:
            fillChannel <unsigned int> (random, i.channel());
            break;

          default:
             assert (false);
        }
    }
}


void
fillChannels (Rand48 &random, FlatImage &img)
{
    switch (img.levelMode())
    {
      case ONE_LEVEL:

        cout << "        level 0" << endl;
        fillChannels (random, img.level());

        break;

      case MIPMAP_LEVELS:

        for (int x = 0; x < img.numLevels(); ++x)
        {
            cout << "        level " << x << "" << endl;
            fillChannels (random, img.level (x));
        }

        break;

      case RIPMAP_LEVELS:

        for (int y = 0; y < img.numYLevels(); ++y)
        {
            for (int x = 0; x < img.numXLevels(); ++x)
            {
                cout << "        level (" << x << ", " << y << ")" << endl;
                fillChannels (random, img.level (x, y));
            }
        }

        break;

      default:

        assert (false);
    }
}


void
testScanLineImage
    (const Box2i &dataWindow,
     const string &fileName)
{
    cout << "scan lines, data window = "
            "(" << dataWindow.min.x << ", " << dataWindow.min.y << ") - "
            "(" << dataWindow.max.x << ", " << dataWindow.max.y << ")" << endl;

    FlatImage img1;
    img1.resize (dataWindow);

    img1.insertChannel ("H11", HALF, 1, 1, false);
    img1.insertChannel ("H22", HALF, 2, 2, true);
    img1.insertChannel ("H12", HALF, 1, 2, true);
    img1.insertChannel ("H21", HALF, 2, 1, true);
    img1.insertChannel ("F", FLOAT, 1, 1, false);
    img1.insertChannel ("UI", UINT, 1, 1, false);

    Rand48 random (0);
    cout << "    generating random pixel values" << endl;
    fillChannels (random, img1);

    cout << "    saving file" << endl;
    saveFlatScanLineImage (fileName, img1);
    
    FlatImage img2;

    cout << "    loading file" << endl;
    loadFlatImage (fileName, img2);

    cout << "    comparing" << endl;
    verifyImagesAreEqual (img1, img2);

    remove (fileName.c_str());
}


void
testScanLineImages (const string &fileName)
{
    testScanLineImage (Box2i (V2i (0, 0), V2i (399, 499)), fileName);
    testScanLineImage (Box2i (V2i (-10, -50), V2i (499, 599)), fileName);
    testScanLineImage (Box2i (V2i (50, 10), V2i (699, 199)), fileName);
}


void
testTiledImage
    (const Box2i &dataWindow,
     const string &fileName,
     LevelMode levelMode,
     LevelRoundingMode levelRoundingMode)
{
    cout << "tiles, data window = "
            "(" << dataWindow.min.x << ", " << dataWindow.min.y << ") - "
            "(" << dataWindow.max.x << ", " << dataWindow.max.y << "), "
            "level mode = " << levelMode << ", "
            "rounding mode = " << levelRoundingMode << endl;

    FlatImage img1;
    img1.resize (dataWindow, levelMode, levelRoundingMode);

    img1.insertChannel ("H1", HALF, 1, 1, false);
    img1.insertChannel ("H2", HALF, 1, 1, true);
    img1.insertChannel ("F", FLOAT, 1, 1, false);
    img1.insertChannel ("UI", UINT, 1, 1, false);

    Rand48 random (0);
    cout << "    generating random pixel values" << endl;
    fillChannels (random, img1);

    cout << "    saving file" << endl;
    saveFlatTiledImage (fileName, img1);
    
    FlatImage img2;

    cout << "    loading file" << endl;
    loadFlatImage (fileName, img2);

    cout << "    comparing" << endl;
    verifyImagesAreEqual (img1, img2);

    remove (fileName.c_str());
}


void
testTiledImage
    (const Box2i &dataWindow,
     const string &fileName)
{
    testTiledImage (dataWindow, fileName, ONE_LEVEL, ROUND_DOWN);
    testTiledImage (dataWindow, fileName, MIPMAP_LEVELS, ROUND_DOWN);
    testTiledImage (dataWindow, fileName, MIPMAP_LEVELS, ROUND_UP);
    testTiledImage (dataWindow, fileName, RIPMAP_LEVELS, ROUND_DOWN);
    testTiledImage (dataWindow, fileName, RIPMAP_LEVELS, ROUND_UP);
}


void
testTiledImages (const string &fileName)
{
    testTiledImage (Box2i (V2i (0, 0), V2i (399, 499)), fileName);
    testTiledImage (Box2i (V2i (-10, -50), V2i (499, 599)), fileName);
    testTiledImage (Box2i (V2i (50, 10), V2i (699, 199)), fileName);
}


void
testShiftPixels ()
{
    cout << "pixel shifting" << endl;

    FlatImage img1 (Box2i (V2i (15, 20), V2i (45, 60)), MIPMAP_LEVELS);
    img1.insertChannel ("A", HALF);
    img1.insertChannel ("B", HALF);

    FlatImage img2 (Box2i (V2i (15, 20), V2i (45, 60)), MIPMAP_LEVELS);
    img2.insertChannel ("A", HALF);
    img2.insertChannel ("B", HALF);

    cout << "    generating random pixel values" << endl;

    {
        Rand48 random (1);
        fillChannels (random, img1);
    }

    {
        Rand48 random (1);
        fillChannels (random, img2);
    }

    int DX = 5;
    int DY = 7;

    cout << "    shifting, dx = " << DX << ", dy = " << DY << endl;
    img2.shiftPixels (DX, DY);

    cout << "    comparing" << endl;
    verifyImagesAreEqual (img1, img2, DX, DY);
}


void
testCropping (const string &fileName)
{
    cout << "cropping an image" << endl;

    FlatImage img1 (Box2i (V2i (10, 20), V2i (110, 120)), ONE_LEVEL);
    img1.insertChannel ("A", HALF);

    Rand48 random (0);
    cout << "    generating random pixel values" << endl;
    fillChannels (random, img1);

    Header hdr;
    hdr.dataWindow() = Box2i (V2i (40, 50), V2i (60, 70));

    cout << "    saving scan line file" << endl;
    saveFlatScanLineImage (fileName, hdr, img1, USE_HEADER_DATA_WINDOW);

    cout << "    loading file" << endl;
    FlatImage img2;
    loadFlatImage (fileName, img2);

    assert (img2.dataWindow() != img1.dataWindow());
    assert (img2.dataWindow() == hdr.dataWindow());

    cout << "    comparing" << endl;

    verifyPixelsAreEqual <half> (img2.level().channel ("A"),
                                 img1.level().channel ("A"),
                                 0, 0);


    cout << "    saving tiled file" << endl;
    saveFlatTiledImage (fileName, hdr, img1, USE_HEADER_DATA_WINDOW);

    cout << "    loading file" << endl;
    FlatImage img3;
    loadFlatImage (fileName, img3);

    assert (img3.dataWindow() != img1.dataWindow());
    assert (img3.dataWindow() == hdr.dataWindow());

    cout << "    comparing" << endl;

    verifyPixelsAreEqual <half> (img3.level(0).channel ("A"),
                                 img1.level(0).channel ("A"),
                                 0, 0);
    remove (fileName.c_str ());
}

void
testRenameChannel ()
{
    cout << "renaming a single channel" << endl;

    FlatImage img (Box2i (V2i (15, 20), V2i (45, 60)), MIPMAP_LEVELS);
    img.insertChannel ("A", HALF);
    img.insertChannel ("B", HALF);

    for (int i = 0; i < img.numLevels(); ++i)
    {
        const FlatImageLevel &level = img.level (i);
        assert (level.findTypedChannel <half> ("A") != 0);
        assert (level.findTypedChannel <half> ("B") != 0);
        assert (level.findTypedChannel <half> ("C") == 0);
    }

    img.renameChannel ("A", "C");

    for (int i = 0; i < img.numLevels(); ++i)
    {
        const FlatImageLevel &level = img.level (i);
        assert (level.findTypedChannel <half> ("A") == 0);
        assert (level.findTypedChannel <half> ("B") != 0);
        assert (level.findTypedChannel <half> ("C") != 0);
    }

    bool caught = false;

    try
    {
        img.renameChannel ("A", "D");   // "A" doesn't exist
        assert (false);
    }
    catch (...)
    {
        // expecting exception
        caught = true;
    }
    assert (caught);

    caught = false;
    try
    {
        img.renameChannel ("C", "B");   // "B" exists already
        assert (false);
    }
    catch (...)
    {
        // expecting exception
        caught = true;
    }
    assert (caught);
}


void
testRenameChannels ()
{
    cout << "renaming multiple channels at the same time" << endl;

    FlatImage img (Box2i (V2i (0, 0), V2i (10, 10)), MIPMAP_LEVELS);
    img.insertChannel ("A", HALF);
    img.insertChannel ("B", HALF);
    img.insertChannel ("C", HALF);
    img.insertChannel ("D", HALF);

    img.level(0).typedChannel<half>("A").at (0, 0) = 1;
    img.level(0).typedChannel<half>("B").at (0, 0) = 2;
    img.level(0).typedChannel<half>("C").at (0, 0) = 3;
    img.level(0).typedChannel<half>("D").at (0, 0) = 4;

    img.level(1).typedChannel<half>("A").at (0, 0) = 1;
    img.level(1).typedChannel<half>("B").at (0, 0) = 2;
    img.level(1).typedChannel<half>("C").at (0, 0) = 3;
    img.level(1).typedChannel<half>("D").at (0, 0) = 4;

    {
        RenamingMap oldToNewNames;
        oldToNewNames["A"] = "B";
        oldToNewNames["B"] = "A";
        oldToNewNames["C"] = "E";
        oldToNewNames["X"] = "Y";

        img.renameChannels (oldToNewNames);
    }

    assert (img.level(0).findChannel("A") != 0);
    assert (img.level(0).findChannel("B") != 0);
    assert (img.level(0).findChannel("C") == 0);
    assert (img.level(0).findChannel("D") != 0);
    assert (img.level(0).findChannel("E") != 0);

    assert (img.level(0).typedChannel<half>("A").at (0, 0) == 2);
    assert (img.level(0).typedChannel<half>("B").at (0, 0) == 1);
    assert (img.level(0).typedChannel<half>("D").at (0, 0) == 4);
    assert (img.level(0).typedChannel<half>("E").at (0, 0) == 3);

    assert (img.level(1).typedChannel<half>("A").at (0, 0) == 2);
    assert (img.level(1).typedChannel<half>("B").at (0, 0) == 1);
    assert (img.level(1).typedChannel<half>("D").at (0, 0) == 4);
    assert (img.level(1).typedChannel<half>("E").at (0, 0) == 3);

    bool caught = false;

    try
    {
        RenamingMap oldToNewNames;
        oldToNewNames["A"] = "F";
        oldToNewNames["B"] = "F";   // duplicate new name "F"

        img.renameChannels (oldToNewNames);
        assert (false);
    }
    catch (...)
    {
        // expecting exception
        caught = true;
    }
    assert (caught);

    caught = false;
    try
    {
        RenamingMap oldToNewNames;
        oldToNewNames["A"] = "B";   // duplicate new name "B"

        img.renameChannels (oldToNewNames);
        assert (false);
    }
    catch (...)
    {
        // expecting exception
        caught = true;
    }
    assert (caught);
}

} // namespace


void
testFlatImage (const string &tempDir)
{
    try
    {
	cout << "Testing class FlatImage" << endl;

        testScanLineImages (tempDir + "scanLines.exr");
        testTiledImages (tempDir + "tiles.exr");
        testShiftPixels();
        testCropping (tempDir + "cropped.exr");
        testRenameChannel();
        testRenameChannels();

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
