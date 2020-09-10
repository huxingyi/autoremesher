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

#ifndef INCLUDED_IMF_IMAGE_IO_H
#define INCLUDED_IMF_IMAGE_IO_H

//----------------------------------------------------------------------------
//
//      Functions to load flat or deep images from OpenEXR files
//      and to save flat or deep images in OpenEXR files.
//
//----------------------------------------------------------------------------

#include "ImfImage.h"
#include "ImfImageDataWindow.h"
#include "ImfUtilExport.h"

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER


//
// saveImage (n, h, i, d) or
// saveImage (n, i)
//
//      Saves image i in an OpenEXR file with name n.  The file will be
//      tiled if the image has more than one level, or if a header, h, is
//      given and contains a tile description attribute; otherwise the
//      file will be scan-line based.  The file will be deep if the image
//      is deep; otherwise the file will be flat.
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
//      Note: USE_HEADER_DATA_WINDOW can only be used for images with
//      level mode ONE_LEVEL.
//

IMFUTIL_EXPORT
void
saveImage
    (const std::string &fileName,
     const Header &hdr,
     const Image &img,
     DataWindowSource dws = USE_IMAGE_DATA_WINDOW);

IMFUTIL_EXPORT
void
saveImage
    (const std::string &fileName,
     const Image &img);

//
// loadImage (n, h) or
// loadImage (n)
//
//      Loads deep an image from the OpenEXR file with name n, and returns
//      a pointer to the image.  The caller owns the image and is responsible
//      for deleting it.
//
//      If header h is given, then the header of the file is copied into h.
//

IMFUTIL_EXPORT
Image *
loadImage
    (const std::string &fileName,
     Header &hdr);

IMFUTIL_EXPORT
Image *
loadImage
    (const std::string &fileName);


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

#endif
