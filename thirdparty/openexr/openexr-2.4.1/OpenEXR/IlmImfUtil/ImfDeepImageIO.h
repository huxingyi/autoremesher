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

#ifndef INCLUDED_IMF_DEEP_IMAGE_IO_H
#define INCLUDED_IMF_DEEP_IMAGE_IO_H

//----------------------------------------------------------------------------
//
//      Functions to load deep images from OpenEXR files
//      and to save deep images in OpenEXR files.
//
//----------------------------------------------------------------------------

#include "ImfDeepImage.h"
#include "ImfImageDataWindow.h"
#include "ImfUtilExport.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER


//
// saveDeepImage (n, h, i,d) or
// saveDeepImage (n, i)
//
//      Saves image i in an OpenEXR file with name n.  The file will be
//      tiled if the image has more than one level, or if a header, h, is
//      given and contains a tile description attribute; otherwise the
//      file will be scan-line based.
//
//      If header h is given, then the channel list in h is replaced with
//      the channel list in i, and the levelMode and the levelRounding mode
//      fields of the tile description are replaced with the level mode
//      and the levelRounding mode of i.  In addition, if the data window
//      source flag, d, is set to USE_IMAGE_DATA_WINDOW, then the data
//      window in the image is copied into the header; if d is set to
//      USE_HEADER_DATA_WINDOW, then the data window in the header is
//      replaced with the intersection of the original data window in the
//      header and the data window in the image.  The modified header then
//      becomes the header of the image file.
//

IMFUTIL_EXPORT
void
saveDeepImage
    (const std::string &fileName,
     const Header &hdr,
     const DeepImage &img,
     DataWindowSource dws = USE_IMAGE_DATA_WINDOW);

IMFUTIL_EXPORT
void
saveDeepImage
    (const std::string &fileName,
     const DeepImage &img);

//
// loadDeepImage (n, h, i) or
// loadDeepImage (n, i)
//
//      Loads deep image i from the OpenEXR file with name n.
//
//      If header h is given, then the header of the file is copied into h.
//

IMFUTIL_EXPORT
void
loadDeepImage
    (const std::string &fileName,
     Header &hdr,
     DeepImage &img);


IMFUTIL_EXPORT
void
loadDeepImage
    (const std::string &fileName,
     DeepImage &img);


//
// saveDeepScanLineImage (n, h, i, d) or
// saveDeepScanLineImage (n, i)
//
//      Saves image i in a scan-line based deep OpenEXR file with file name n.
//
//      If header h is given, then the channel list in h is replaced with
//      the channel list in i.  In addition, if the data window source flag, d,
//      is set to USE_IMAGE_DATA_WINDOW, then the data window in the image is
//      copied into the header; if d is set to USE_HEADER_DATA_WINDOW, then
//      the data window in the header is replaced with the intersection of
//      the original data window in the header and the data window in the
//      image.  The modified header then becomes the header of the image file.
//

IMFUTIL_EXPORT
void
saveDeepScanLineImage
    (const std::string &fileName,
     const Header &hdr,
     const DeepImage &img,
     DataWindowSource dws = USE_IMAGE_DATA_WINDOW);

IMFUTIL_EXPORT
void
saveDeepScanLineImage
    (const std::string &fileName,
     const DeepImage &img);


//
// loadDeepScanLineImage (n, h, i) or
// loadDeepScanLineImage (n, i)
//
//      Loads image i from a scan-line based deep OpenEXR file with file name n.
//      If header h is given, then the header of the file is copied into h.
//

IMFUTIL_EXPORT
void
loadDeepScanLineImage
    (const std::string &fileName,
     Header &hdr,
     DeepImage &img);

IMFUTIL_EXPORT
void
loadDeepScanLineImage
    (const std::string &fileName,
     DeepImage &img);

//
// saveDeepTiledImage (n, h, i, d) or
// saveDeepTiledImage (n, i)
//
//      Saves image i in a tiled deep OpenEXR file with file name n.
//
//      If header h is given, then the channel list in h is replaced with
//      the channel list i, and the levelMode and the levelRounding mode
//      fields of the tile description are replaced with the level mode
//      and the levelRounding mode of i.  In addition, if the data window
//      source flag, d, is set to USE_IMAGE_DATA_WINDOW, then the data
//      window in the image is copied into the header; if d is set to
//      USE_HEADER_DATA_WINDOW, then the data window in the header is
//      replaced with the intersection of the original data window in the
//      header and the data window in the image.  The modified header then
//      becomes the header of the image file.
//
//      Note: USE_HEADER_DATA_WINDOW can only be used for images with
//      level mode ONE_LEVEL.
//

IMFUTIL_EXPORT
void
saveDeepTiledImage
    (const std::string &fileName,
     const Header &hdr,
     const DeepImage &img,
     DataWindowSource dws = USE_IMAGE_DATA_WINDOW);

IMFUTIL_EXPORT
void
saveDeepTiledImage
    (const std::string &fileName,
     const DeepImage &img);

//
// loadDeepTiledImage (n, h, i) or
// loadDeepTiledImage (n, i)
//
//      Loads image i from a tiled deep OpenEXR file with file name n.
//      If header h is given, then the header of the file is copied into h.
//

IMFUTIL_EXPORT
void
loadDeepTiledImage
    (const std::string &fileName,
     Header &hdr,
     DeepImage &img);

IMFUTIL_EXPORT
void
loadDeepTiledImage
    (const std::string &fileName,
     DeepImage &img);

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
