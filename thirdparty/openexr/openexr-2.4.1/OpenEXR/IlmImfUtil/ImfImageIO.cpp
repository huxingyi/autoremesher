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

//----------------------------------------------------------------------------
//
//      OpenEXR file I/O for deep images.
//
//----------------------------------------------------------------------------

#include "ImfImageIO.h"
#include "ImfFlatImageIO.h"
#include "ImfDeepImageIO.h"
#include <ImfMultiPartInputFile.h>
#include <ImfHeader.h>
#include <ImfTestFile.h>
#include <ImfPartType.h>
#include <Iex.h>

using namespace IMATH_NAMESPACE;
using namespace IEX_NAMESPACE;
using namespace std;

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER


void
saveImage
    (const string &fileName,
     const Header &hdr,
     const Image &img,
     DataWindowSource dws)
{
    if (const FlatImage *fimg = dynamic_cast <const FlatImage *> (&img))
    {
        if (fimg->levelMode() != ONE_LEVEL || hdr.hasTileDescription())
            saveFlatTiledImage (fileName, hdr, *fimg, dws);
        else
            saveFlatScanLineImage (fileName, hdr, *fimg, dws);
    }

    if (const DeepImage *dimg = dynamic_cast <const DeepImage *> (&img))
    {
        if (dimg->levelMode() != ONE_LEVEL || hdr.hasTileDescription())
            saveDeepTiledImage (fileName, hdr, *dimg, dws);
        else
            saveDeepScanLineImage (fileName, hdr, *dimg, dws);
    }
}


void
saveImage (const string &fileName, const Image &img)
{
    Header hdr;
    hdr.displayWindow() = img.dataWindow();
    saveImage (fileName, hdr, img);
}


Image *
loadImage (const string &fileName, Header &hdr)
{
    bool tiled, deep, multiPart;

    if (!isOpenExrFile (fileName.c_str(), tiled, deep, multiPart))
    {
        THROW (ArgExc, "Cannot load image file " << fileName << ".  "
                       "The file is not an OpenEXR file.");
    }

    if (multiPart)
    {
        THROW (ArgExc, "Cannot load image file " << fileName << ".  "
                       "Multi-part file loading is not supported.");
    }

    //XXX TODO: the tiled flag obtained above is unreliable;
    // open the file as a multi-part file and inspect the header.
    // Can the IlmImf library be fixed?

    {
        MultiPartInputFile mpi (fileName.c_str());

        tiled = (mpi.parts() > 0 &&
                 mpi.header(0).hasType() &&
                 isTiled (mpi.header(0).type()));
    }

    Image *img = 0;

    try
    {
        if (deep)
        {
            DeepImage *dimg = new DeepImage;
            img = dimg;

            if (tiled)
                loadDeepTiledImage (fileName, hdr, *dimg);
            else
                loadDeepScanLineImage (fileName, hdr, *dimg);
        }
        else
        {
            FlatImage *fimg = new FlatImage;
            img = fimg;

            if (tiled)
                loadFlatTiledImage (fileName, hdr, *fimg);
            else
                loadFlatScanLineImage (fileName, hdr, *fimg);
        }
    }
    catch (...)
    {
        delete img;
        throw;
    }

    return img;
}


Image *
loadImage (const string &fileName)
{
    Header hdr;
    return loadImage (fileName, hdr);
}


OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT
