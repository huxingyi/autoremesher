#ifndef INCLUDED_LOAD_IMAGE_H
#define INCLUDED_LOAD_IMAGE_H

//////////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucasfilm
// Entertainment Company Ltd.  Portions contributed and copyright held by
// others as indicated.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above
//       copyright notice, this list of conditions and the following
//       disclaimer.
//
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided with
//       the distribution.
//
//     * Neither the name of Industrial Light & Magic nor the names of
//       any other contributors to this software may be used to endorse or
//       promote products derived from this software without specific prior
//       written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
// IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
// PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
// EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
// PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
// PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
// LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
// NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
//////////////////////////////////////////////////////////////////////////////

//----------------------------------------------------------------------------
//
//	Load an OpenEXR image into a pixel array.
//
//----------------------------------------------------------------------------
#include "namespaceAlias.h"

#include <ImfRgba.h>
#include <ImfArray.h>
#include <ImfHeader.h>


//
// Load an OpenEXR image file:
//
//	fileName	The name of the file to be loaded.
//
//	channel		If channel is 0, load the R, G and B channels,
//			otherwise channel must point to the name of the
//			image channel to be loaded; the channel is copied
//			into the R, G and B components of the frame buffer.
//
//	layer		Used only if channel is 0: if layer is 0, load
//			the R, G and B channels, otherwise load layer.R,
//			layer.G and layer.B.
//
//	preview		If preview is true load the file's preview image,
//			otherwise load the main image.
//
//	lx, ly		If lx != 0 or ly != 0 then assume that the input
//			file is tiled and load level (0, 0).
//
//	header		Output -- the header of the input file, but with
//			the dataWindow, displayWindow and pixelAspectRatio
//			attributes adjusted to match what parts of the file
//			were actually loaded.
//
//	pixels		Output -- the pixels loaded from the file.
//			loadImage() resizes the pixels array to fit
//			the dataWindow attribute of the header.
//


void loadImage (const char fileName[],
                const char channel[],
                const char layer[],
                bool preview,
                int lx,
                int ly,
                int partnum,
                int &zsize,
                IMF::Header &header,
                IMF::Array<IMF::Rgba> &pixels,
                IMF::Array<float*> &zbuffer,
                IMF::Array<unsigned int> &sampleCount,
                bool deepComp);

#endif
