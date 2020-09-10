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


#include <OpenEXRConfig.h>
#include <ImfRgbaFile.h>
#include <ImfArray.h>

#include "namespaceAlias.h"


struct GZ
{
    half  g;
    float z;
};


void drawImage1 (IMF::Array2D<IMF::Rgba> &pixels,
		 int width,
		 int height);

void drawImage2 (IMF::Array2D<half>  &gPixels,
		 IMF::Array2D<float> &zPixels,
		 int width,
		 int height);

void drawImage3 (IMF::Array2D<IMF::Rgba> &pixels,
                 int width,
                 int height,
                 int xMin, int xMax,
                 int yMin, int yMax,
                 int xLevel = 0, int yLevel = 0);

void drawImage4 (IMF::Array2D<IMF::Rgba> &pixels,
                 int width,
                 int height,
                 int xMin, int xMax,
                 int yMin, int yMax,
                 int xLevel = 0, int yLevel = 0);

void drawImage5 (IMF::Array2D<IMF::Rgba> &pixels,
                 int width,
                 int height,
                 int xMin, int xMax,
                 int yMin, int yMax,
                 int xLevel = 0, int yLevel = 0);

void drawImage6 (IMF::Array2D<GZ> &pixels,
		 int width,
		 int height);

void drawImage7 (IMF::Array<IMF::Rgba> &pixels,
		 int width,
		 int height,
		 int y);
