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

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImfVersion.h>
#include "half.h"

#include <ImfBoxAttribute.h>
#include <ImfChannelListAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfChromaticitiesAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfEnvmapAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfOpaqueAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfStringVectorAttribute.h>
#include <ImfVecAttribute.h>


#include <stdio.h>
#include <assert.h>
#include <string.h>


using namespace std;
namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;

namespace IMATH = IMATH_NAMESPACE;
using namespace IMATH;


namespace
{

static const int IMAGE_2K_HEIGHT = 1152;
static const int IMAGE_2K_WIDTH = 2048;

static const char* CHANNEL_NAMES[] = {"R", "G", "B", "A"};
static const char* CHANNEL_NAMES_LEFT[] = {"left.R", "left.G", "left.B", "left.A"};

static const half ALPHA_DEFAULT_VALUE(1.0f);

#define RGB_FILENAME "imf_optimized_aces_rgb.exr"
#define RGBA_FILENAME "imf_optimized_aces_rgba.exr"
#define RGB_STEREO_FILENAME "imf_optimized_aces_rgb_stereo.exr"
#define RGBA_STEREO_FILENAME "imf_optimized_aces_rgba_stereo.exr"

typedef enum EImageType
{
    IMAGE_TYPE_RGB         = 1,
    IMAGE_TYPE_RGBA        = 2,
    IMAGE_TYPE_OTHER        =3
} EImageType;

int
getNbChannels(EImageType pImageType)
{
    int retVal = 0;

    switch(pImageType)
    {
        case IMAGE_TYPE_RGB:
            retVal = 3;
            break;
        case IMAGE_TYPE_RGBA:
            retVal = 4;
            break;
        case IMAGE_TYPE_OTHER:
            retVal = 2;
            break;
        default:
            retVal = 0;
            break;
    }

    return retVal;
}

//
//
//
void
generatePixel (int i, int j, half* rgbaValue, bool pIsLeft)
{
    float redValue = 0.0f;
    float greenValue = 0.0f;
    float blueValue = 0.0f;
    float alphaValue = 0.0f;

    // These formulas are arbitrary but generate results that vary
    // depending on pixel position.  They are used to validate that
    // pixels are read/written correctly, because if we had only one
    // value for the whole image, the tests would still pass if we read
    // only one pixel and copied it all across the buffer.
    if(pIsLeft)
    {
        redValue = ((i + j) % 10 + 0.004f * j) / 10.0f;
        greenValue = ((j + j) % 10 + 0.006f * i) / 10.0f;
        blueValue = ((j * j + i) % 10 + 0.003f * j) / 10.0f;
        alphaValue = ALPHA_DEFAULT_VALUE;
    }
    else
    {
        redValue = ((i * j) % 10 + 0.005f * j) / 10.0f;
        greenValue = ((i + i) % 10 + 0.007f * i) / 10.0f;
        blueValue = ((i * i + j) % 10 + 0.006f * j) / 10.0f;
        alphaValue = ALPHA_DEFAULT_VALUE;
    }


    rgbaValue[0] = redValue;
    rgbaValue[1] = greenValue;
    rgbaValue[2] = blueValue;
    rgbaValue[3] = alphaValue;
}

//
// Given a buffer, fill the pixels with arbitrary but deterministic values.
// Used to fill the pixels in a buffer before writing them to a file.
//
void
fillPixels (const int& pImageHeight,
            const int& pImageWidth,
            Array2D<half>& pPixels,
            int pNbChannels,
            bool pIsLeft)
{
    for(int i = 0; i < pImageHeight; ++i)
    {
        for(int j = 0; j < pImageWidth; ++j)
        {
            half rgbaValue[4];

            generatePixel(i, j, &rgbaValue[0], pIsLeft);
            memcpy( (void*)&pPixels[i][j * pNbChannels],
                    &rgbaValue[0],
                    pNbChannels * sizeof(half));
        }
    }
}

//
// Validate that the pixels value are the same as what we used to fill them.
// Used after reading the pixels from the file to validate that it was read
// properly.
//
void
validatePixels (const int& pImageHeight,
                const int& pImageWidth,
                Array2D<half>& pPixels,
                int pNbChannels,
                bool pIsLeft)
{
    for(int i = 0; i < pImageHeight; ++i)
    {
        for(int j = 0; j < pImageWidth; ++j)
        {
            int retVal = -1;
            half rgbaValue[4];
            generatePixel(i, j, &rgbaValue[0], pIsLeft);

            retVal = memcmp ((void*)&pPixels[i][j * pNbChannels],
                             (void*)&rgbaValue[0],
                             pNbChannels * sizeof(half));

            if(retVal != 0)
            {
                cout << "ERROR at pixel [" << i << ";" << j << "]" << endl;
                cout << "\tExpected [" << rgbaValue[0] << ", "
                        << rgbaValue[1] << ", "
                        << rgbaValue[2] << "] " << endl;

                cout << "\tReceived ["      << pPixels[i][j * pNbChannels] << ", "
                        << pPixels[i][j * pNbChannels + 1] << ", "
                        << pPixels[i][j * pNbChannels + 2] << "]" << endl;
                assert(retVal == 0);
            }

        }
    }
}

//
//  Write pixels to a file (mono version)
//
void
writePixels (const char pFilename[],
             const int& pImageHeight,
             const int& pImageWidth,
             Array2D<half>& pPixels,
             int pNbChannels,
             Compression pCompression)
{
    Header header (pImageWidth,
                   pImageHeight,
                   1.0f,
                   V2f(0.0f,0.0f),
                   1.0f,
                   INCREASING_Y,
                   pCompression);
    for(int i = 0; i < pNbChannels; ++i)
    {
        header.channels().insert (CHANNEL_NAMES[i], Channel(HALF));
    }

    OutputFile lFile(pFilename, header);
    FrameBuffer lOutputFrameBuffer;

    for(int i = 0; i < pNbChannels; ++i)
    {
        lOutputFrameBuffer.insert (CHANNEL_NAMES[i],
                Slice (HALF,
                        (char *) &pPixels[0][i],
                        sizeof (pPixels[0][0]) * pNbChannels,
                        sizeof (pPixels[0][0]) * pNbChannels * pImageWidth));
    }

    lFile.setFrameBuffer (lOutputFrameBuffer);
    lFile.writePixels (pImageHeight);
}

//
//  Write pixels to a file (stereo version)
//
void
writePixels (const char pFilename[],
             const int& pImageHeight,
             const int& pImageWidth,
             Array2D<half>& pPixels,
             Array2D<half>& pPixelsLeft,
             int pNbChannels,
             Compression pCompression)
{
    Header header (pImageWidth,
                   pImageHeight,
                   1.0f,
                   V2f(0.0f,0.0f),
                   1.0f,
                   INCREASING_Y,
                   pCompression);
    for(int i = 0; i < pNbChannels; ++i)
    {
        header.channels().insert (CHANNEL_NAMES[i], Channel(HALF));
        header.channels().insert (CHANNEL_NAMES_LEFT[i], Channel(HALF));
    }

    StringVector multiView;
    multiView.push_back ("right");
    multiView.push_back ("left");
    header.insert("multiView", IMF::TypedAttribute<IMF::StringVector>(multiView));

    OutputFile lFile(pFilename, header);
    FrameBuffer lOutputFrameBuffer;

    for(int i = 0; i < pNbChannels; ++i)
    {
        lOutputFrameBuffer.insert (CHANNEL_NAMES[i],
                Slice (HALF,
                        (char *) &pPixels[0][i],
                        sizeof (pPixels[0][0]) * pNbChannels,
                        sizeof (pPixels[0][0]) * pNbChannels * pImageWidth));

        lOutputFrameBuffer.insert (CHANNEL_NAMES_LEFT[i],
                Slice (HALF,
                        (char *) &pPixelsLeft[0][i],
                        sizeof (pPixelsLeft[0][0]) * pNbChannels,
                        sizeof (pPixelsLeft[0][0]) * pNbChannels * pImageWidth));
    }

    lFile.setFrameBuffer (lOutputFrameBuffer);
    lFile.writePixels (pImageHeight);
}

//
//  Read pixels from a file (mono version).
//
void
readPixels (const char pFilename[], int pNbChannels, Array2D<half>& pPixels)
{
    InputFile lFile(pFilename);
    IMATH::Box2i lDataWindow = lFile.header().dataWindow();

    int lWidth = lDataWindow.max.x - lDataWindow.min.x + 1;

    FrameBuffer lInputFrameBuffer;

    for(int i = 0; i < pNbChannels; ++i)
    {
        lInputFrameBuffer.insert (CHANNEL_NAMES[i],
                Slice (HALF,
                        (char *) &pPixels[0][i],
                        sizeof (pPixels[0][0]) * pNbChannels,
                        sizeof (pPixels[0][0]) * pNbChannels * lWidth,
                        1,
                        1,
                        ALPHA_DEFAULT_VALUE));
    }

    lFile.setFrameBuffer (lInputFrameBuffer);
    
    bool is_optimized = lFile.isOptimizationEnabled();
    if(is_optimized)
    {
        cout << " optimization enabled\n";
        
        if(pNbChannels==2)
        {
            cerr << " error: isOptimizationEnabled returned TRUE, but "
            "optimization not known to work for two channel images\n";
            assert(pNbChannels!=2);
        }
            
    }else{
        cout << " optimization disabled\n";
#ifdef IMF_HAVE_SSE2
        if(pNbChannels!=2)
        {
            cerr << " error: isOptimizationEnabled returned FALSE, but "
            "should work for " << pNbChannels << "channel images\n";
            assert(pNbChannels==2);
        }
        
#endif
    }
    
    lFile.readPixels (lDataWindow.min.y, lDataWindow.max.y);
}

//
//  Read pixels from a file (stereo version).
//
void
readPixels (const char pFilename[],
            int pNbChannels,
            Array2D<half>& pPixels,
            Array2D<half>& pPixelsLeft)
{
    InputFile lFile(pFilename);
    IMATH::Box2i lDataWindow = lFile.header().dataWindow();

    int lWidth = lDataWindow.max.x - lDataWindow.min.x + 1;

    FrameBuffer lInputFrameBuffer;

    for(int i = 0; i < pNbChannels; ++i)
    {
        lInputFrameBuffer.insert (CHANNEL_NAMES[i],
                Slice (HALF,
                        (char *) &pPixels[0][i],
                        sizeof (pPixels[0][0]) * pNbChannels,
                        sizeof (pPixels[0][0]) * pNbChannels * lWidth,
                        1,
                        1,
                        ALPHA_DEFAULT_VALUE));

        lInputFrameBuffer.insert (CHANNEL_NAMES_LEFT[i],
                Slice (HALF,
                        (char *) &pPixelsLeft[0][i],
                        sizeof (pPixelsLeft[0][0]) * pNbChannels,
                        sizeof (pPixelsLeft[0][0]) * pNbChannels * lWidth,
                        1,
                        1,
                        ALPHA_DEFAULT_VALUE));
    }

    lFile.setFrameBuffer (lInputFrameBuffer);
    lFile.readPixels (lDataWindow.min.y, lDataWindow.max.y);
}

//
// Allocate an array of pixels, fill them with values and then write
// them to a file.
void
writeFile (const char pFilename[],
           int pHeight,
           int pWidth,
           EImageType pImageType,
           bool pIsStereo,
           Compression pCompression)
{
    const int lNbChannels = getNbChannels (pImageType);
    Array2D<half> lPixels;

    lPixels.resizeErase (pHeight, pWidth * lNbChannels);
    fillPixels (pHeight, pWidth, lPixels, lNbChannels, false);

    if(pIsStereo)
    {
        Array2D<half> lPixelsLeft;

        lPixelsLeft.resizeErase (pHeight, pWidth * lNbChannels);
        fillPixels (pHeight,
                    pWidth,
                    lPixelsLeft,
                    lNbChannels,
                    true);

        writePixels (pFilename,
                     pHeight,
                     pWidth,
                     lPixels,
                     lPixelsLeft,
                     lNbChannels,
                     pCompression);
    }
    else
    {
        writePixels (pFilename,
                    pHeight,
                    pWidth,
                    lPixels,
                    lNbChannels,
                    pCompression);
    }
}

//
// Read pixels from a file and then validate that the values are the
// same as what was used to fill them before writing.
//
void
readValidateFile (const char pFilename[],
                  int pHeight,
                  int pWidth,
                  EImageType
                  pImageType,
                  bool pIsStereo)
{
    const int lNbChannels = getNbChannels(pImageType);

    Array2D<half> lPixels;
    lPixels.resizeErase(pHeight, pWidth * lNbChannels);
    //readPixels(pFilename, lNbChannels, lPixels);
    //writePixels("pkTest.exr", pHeight, pWidth, lPixels, lNbChannels, NO_COMPRESSION);


    if(pIsStereo)
    {
        Array2D<half> lPixelsLeft;
        lPixelsLeft.resizeErase (pHeight, pWidth * lNbChannels);
        readPixels (pFilename, lNbChannels, lPixels, lPixelsLeft);
        validatePixels (pHeight, pWidth, lPixels, lNbChannels, false);
        validatePixels (pHeight, pWidth, lPixelsLeft, lNbChannels, true);
    }
    else
    {
        readPixels (pFilename, lNbChannels, lPixels);
        validatePixels (pHeight, pWidth, lPixels, lNbChannels, false);
    }
}

//
// confirm the optimization flag returns false for non-RGB files
//
void
testNonOptimized (const std::string & tempDir)
{
    const int pHeight = IMAGE_2K_HEIGHT - 1;
    const int pWidth  =  IMAGE_2K_WIDTH - 1;
    std::string fn = tempDir +  RGB_FILENAME;
    const char* filename  = fn.c_str();

    remove(filename);
    writeFile (filename,  pHeight, pWidth, IMAGE_TYPE_OTHER,  false, NO_COMPRESSION);
    readValidateFile(filename,pHeight,pWidth,IMAGE_TYPE_OTHER,false);
    remove(filename);
}

//
// Test all combinations of file/framebuffer
//  RGB  file to RGB  framebuffer
//  RGB  file to RGBA framebuffer
//  RGBA file to RGB  framebuffer
//  RGBA file to RGBA framebuffer
// Given a switch that determines whether the pixels will be SSE-aligned,
// whether the file is mono or stereo, and the compression algorithm used
// to write the file.
//
void
testAllCombinations (bool isAligned,
                     bool isStereo,
                     Compression pCompression,
                     const std::string & tempDir)
{
    std::string pRgb  = isStereo ?  RGB_STEREO_FILENAME :
                                    RGB_FILENAME;
    pRgb = tempDir + pRgb;
    std::string pRgba = isStereo ?  RGBA_STEREO_FILENAME :
                                    RGBA_FILENAME;
    pRgba = tempDir + pRgba;

    const char * pRgbFilename  = pRgb.c_str();
    const char * pRgbaFilename = pRgba.c_str();

    const int pHeight = isAligned ? IMAGE_2K_HEIGHT : IMAGE_2K_HEIGHT - 1;
    const int pWidth  = isAligned ? IMAGE_2K_WIDTH  : IMAGE_2K_WIDTH  - 1;

    remove(pRgbFilename);
    remove(pRgbaFilename);

    writeFile (pRgbFilename,  pHeight, pWidth, IMAGE_TYPE_RGB,  isStereo, pCompression);
    writeFile (pRgbaFilename, pHeight, pWidth, IMAGE_TYPE_RGBA, isStereo, pCompression);

    cout << "\t\tRGB file to RGB framebuffer" << endl;
    readValidateFile (pRgbFilename,  pHeight, pWidth, IMAGE_TYPE_RGB,  isStereo);
    
    
    cout << "\t\tRGB file to RGB framebuffer" << endl;
    readValidateFile (pRgbFilename,  pHeight, pWidth, IMAGE_TYPE_RGB,  isStereo);

    cout << "\t\tRGB file to RGBA framebuffer" << endl;
    readValidateFile (pRgbFilename,  pHeight, pWidth, IMAGE_TYPE_RGBA, isStereo);

    cout << "\t\tRGBA file to RGB framebuffer" << endl;
    readValidateFile (pRgbaFilename, pHeight, pWidth, IMAGE_TYPE_RGB,  isStereo);

    cout << "\t\tRGBA file to RGBA framebuffer" << endl;
    readValidateFile (pRgbaFilename, pHeight, pWidth, IMAGE_TYPE_RGBA, isStereo);

    remove(pRgbFilename);
    remove(pRgbaFilename);

}

} // anonymous namespace


void
testOptimized (const std::string & tempDir)
{    
    try
    {
        // Test all combinations
        // Aligned file with no compression
        // Unaligned file with no compression
        // Aligned file with zip compression 
        // Unaligned file with zip compression
        //
        // Other algorithms are not necessary because we are not testing
        // compression but whetherthe optimized readPixels() code works
        // with a compressed file.
        // MONO
        cout << "\nTesting optimized code path for rgb(a) images-- "
                "2048x1152 (alignment respected) UNCOMPRESSED" << endl;

                         
        cout << "\tNON-OPTIMIZABLE file" << endl;
        testNonOptimized(tempDir);
                
        cout << "\tALIGNED -- MONO -- NO COMPRESSION" << endl;
        testAllCombinations (true, false, NO_COMPRESSION, tempDir);

        cout << "\tUNALIGNED -- MONO -- NO COMPRESSION" << endl;
        testAllCombinations (false, false, NO_COMPRESSION, tempDir);

        cout << "\tALIGNED -- MONO -- ZIP COMPRESSION" << endl;
        testAllCombinations (true, false, ZIP_COMPRESSION, tempDir);

        cout << "\tUNALIGNED -- MONO -- ZIP COMPRESSION" << endl;
        testAllCombinations (false, false, ZIP_COMPRESSION, tempDir);


        //// STEREO
        cout << "\tALIGNED -- STEREO -- NO COMPRESSION" << endl;
        testAllCombinations (true, true, NO_COMPRESSION, tempDir);

        cout << "\tUNALIGNED -- STEREO -- NO COMPRESSION" << endl;
        testAllCombinations (false, true, NO_COMPRESSION, tempDir);

        cout << "\tALIGNED -- STEREO -- ZIP COMPRESSION" << endl;
        testAllCombinations (true, true, ZIP_COMPRESSION, tempDir);

        cout << "\tUNALIGNED -- STEREO -- ZIP COMPRESSION" << endl;
        testAllCombinations (false, true, ZIP_COMPRESSION, tempDir);

        cout << "RGB(A) files validation complete \n" << endl;
    }
    catch (const std::exception &e)
    {
	    cerr << "ERROR -- caught exception: " << e.what() << endl;
	    assert (false);
    }

}
