///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2013, Weta Digital Ltd
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

#include "ImfInputFile.h"
#include <stdlib.h>
#include <vector>
#include "ImfChannelList.h"
#include "ImfOutputFile.h"
#include "ImfCompression.h"
#include "ImfStandardAttributes.h"
#include <algorithm>
#include <iostream>
#include <assert.h>
#include <IlmThread.h>
#include <ImathBox.h>

#include "tmpDir.h"
#include "random.h"

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;
using namespace ILMTHREAD_NAMESPACE;


namespace
{

using OPENEXR_IMF_NAMESPACE::UINT;
using OPENEXR_IMF_NAMESPACE::FLOAT;

std::string filename;

vector<char> writingBuffer; // buffer as file was written
vector<char> readingBuffer; // buffer containing new image

static const long long maxMem = 16ll*1024ll*1024ll; // max memory usage for pixel storage should be 16 MiB
static const long long maxMemoryPerBuffer = maxMem/2ll; // read and write buffers are separate
static const long long pixelCount = maxMemoryPerBuffer / (long long) (10*sizeof(float));

bool compare(const FrameBuffer& asRead,
             const FrameBuffer& asWritten,
             const Box2i& dataWindow
            )
{
    for (FrameBuffer::ConstIterator i =asRead.begin();i!=asRead.end();i++)
    {
        FrameBuffer::ConstIterator p = asWritten.find(i.name());
        for (int y=dataWindow.min.y; y<= dataWindow.max.y; y++)
        {
            for (int x = dataWindow.min.x; x <= dataWindow.max.x; x++)
                 
            {
                char * ptr = (i.slice().base+i.slice().yStride*intptr_t(y) +i.slice().xStride*intptr_t(x));
                half readHalf;
                switch (i.slice().type)
                {
                    case IMF::FLOAT :
                        readHalf =  half(*(float*) ptr);
                        break;
                    case IMF::HALF :
                        readHalf = half(*(half*) ptr);
                        break;
                    case IMF::UINT :
                        continue; // can't very well check this
                    default :
                        cout << "don't know about that\n";
                        exit(1);
                }
                
                half writtenHalf;

                if (p!=asWritten.end())
                {
                    char * ptr = p.slice().base+p.slice().yStride*intptr_t(y) +
                                 p.slice().xStride*intptr_t(x);
                    switch (p.slice().type)
                    {
                    case IMF::FLOAT :
                        writtenHalf = half(*(float*) ptr);
                        break;
                    case IMF::HALF :
                        writtenHalf = half(*(half*) ptr);
                        break;
                    case IMF::UINT :
                        continue;
                    default :
                        cout << "don't know about that\n";
                        exit(1);
                    }
                }
                else
                {
                    writtenHalf=half(i.slice().fillValue);
                }

                if (writtenHalf.bits()!=readHalf.bits())
                {
                        cout << "\n\nerror reading back channel " << i.name() << " pixel " << x << ',' << y << " got " << readHalf << " expected " << writtenHalf << endl;
                        assert(writtenHalf.bits()==readHalf.bits());
                        exit(1);
                    
                }             
            }

        }
    }
    return true;
}

//
// allocate readingBuffer or writingBuffer, setting up a framebuffer to point to the right thing
//
ChannelList
setupBuffer (const Header& hdr,       // header to grab datawindow from
             const char * const *channels, // NULL terminated list of channels to write
             const PixelType *pt,
             FrameBuffer& buf,        // buffer to fill with pointers to channel
             bool writing                  // true if should allocate
            )
{
    Box2i dw = hdr.dataWindow();

    //
    // how many channels in total
    //
    size_t activechans = 0;
    int bytes_per_pixel =0;
    
    while (channels[activechans]!=NULL)
    {
        if (pt==NULL)
        {
            bytes_per_pixel+=2;
        }
        else
        {
            switch (pt[activechans])
            {
                case IMF::HALF : bytes_per_pixel+=2;break;
                case IMF::FLOAT : case IMF::UINT : bytes_per_pixel+=4;break;
                default :
                    cout << "Unexpected PixelType?\n";
                    exit(1);
            }
        }
        activechans++;
    }

  
    
    size_t samples = size_t(hdr.dataWindow().max.x+1-hdr.dataWindow().min.x)*
                  size_t(hdr.dataWindow().max.y+1-hdr.dataWindow().min.y)*activechans;

    size_t size = size_t(hdr.dataWindow().max.x+1-hdr.dataWindow().min.x)*
                  size_t(hdr.dataWindow().max.y+1-hdr.dataWindow().min.y)*bytes_per_pixel;

    
    if (writing)
    {
        writingBuffer.resize(size);
    }
    else
    {
        readingBuffer.resize(size);
    }
   
     const char * write_ptr = writing ? &writingBuffer[0] : &readingBuffer[0];
     // fill with random halfs, casting to floats for float channels
     size_t chan=0;
     for (size_t i=0;i<samples;i++)
     {
         unsigned short int values = random_int(std::numeric_limits<unsigned short>::max());
         half v;
         v.setBits(values);
         if (pt==NULL || pt[chan]==IMF::HALF)
         {
             *(half*)write_ptr = half(v);
             write_ptr+=2;
         }
         else
         {
             *(float*)write_ptr = float(v);
             write_ptr+=4;
         }
         chan++;
         if (chan==activechans)
         {
             chan=0;
         }
     
     }

     

    ChannelList chanlist;

    int64_t width = (dw.max.x+1-dw.min.x);
    int64_t bytes_per_row = bytes_per_pixel*width;
   
    const char* offset = ( writing ? writingBuffer.data() : readingBuffer.data() ); 
    for (size_t i=0;i<activechans;i++)
    {
        PixelType type = pt==NULL ? IMF::HALF : pt[i];

        chanlist.insert(channels[i],type);
        buf.insert (channels[i],
                        Slice::Make (type,
			       offset,
			       dw.min,
			       width,1,
                               bytes_per_pixel,
                               bytes_per_row,
                               1,
			       1,
			       100.+i,false,false
			       ));
        switch (type)
        {
            case IMF::HALF :
                offset+=2;
                break;
            case IMF::FLOAT :
                offset+=4;
                break;
            default :
                cout << "Unexpected Pixel Type\n";
                exit(1);
        
        }
    }

    return chanlist;
}



Header writefile(FrameBuffer& buf, const char * const *channels, // NULL terminated list of channels to write
             const PixelType *pt)
{
    
    const int height = random_int()/2; 
    const int width  = random_int()/2;

    Header hdr(width,height,1);
    
    
    //
    // set min origin to be anything up to half INT_MAX
    //
    hdr.dataWindow().min.x = random_int() / 4;
    hdr.dataWindow().min.y = random_int() / 4;
    if(random_int(2)) 
    {
        hdr.dataWindow().min.x = -hdr.dataWindow().min.x;
    }
    if(random_int(2)) 
    {
        hdr.dataWindow().min.y = -hdr.dataWindow().min.y;
    }
    
    //
    // up to 512 scanlines
    //
    hdr.dataWindow().max.y = hdr.dataWindow().min.y + random_int (512) + 1;

    //
    // compute image width to give us at most 'pixelCount' pixels in the image
    //
    hdr.dataWindow().max.x = hdr.dataWindow().min.x+ pixelCount /
        ((long long) (hdr.dataWindow().max.y) - (long long) (hdr.dataWindow().min.y));
    
    hdr.compression() = Compression(random_int(static_cast<int>(NUM_COMPRESSION_METHODS)));
    hdr.channels() = setupBuffer (hdr,
                                  channels,
                                  pt,
                                  buf,
                                  true);
    
    
    remove (filename.c_str());
    OutputFile f(filename.c_str(), hdr);
    f.setFrameBuffer(buf);
    f.writePixels(hdr.dataWindow().max.y-hdr.dataWindow().min.y+1);

    return hdr;
}

void
readfile (FrameBuffer & buf,      ///< list of channels to read: index to readingBuffer,
          const char * const *channels, // NULL terminated list of channels to write
             const PixelType *pt
        )
{
    InputFile infile (filename.c_str());
    setupBuffer(infile.header(),
                channels,pt,
                buf,
                false);
    infile.setFrameBuffer(buf);
    
    infile.readPixels (infile.header().dataWindow().min.y,
                       infile.header().dataWindow().max.y);

   
}

static const char* rgb[] = {"R","G","B",NULL};
static const char* rgba[] = {"R","G","B","A",NULL};
static const char* rgbaz[] = {"R","G","B","A","Z",NULL};
static const char* lots[] = {"1","2","3","4","5","6","7","8","9","10",NULL};

static const PixelType allFloats[] = {FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT,FLOAT};
static const PixelType halfFloat[] = {HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT};
static const PixelType floatHalf[] = {FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT,HALF,FLOAT};

void
test (int testCount)
{
   
    for(int i = 0 ; i < testCount ; ++i )
    {
        FrameBuffer writeFrameBuf;
        const char** channels=NULL;
        switch( random_int(4)
)        {
            case 0 : channels = rgb; break;
            case 1 : channels = rgba; break;
            case 2 : channels = rgbaz; break;
            case 3 : channels = lots; break;
        }
        const PixelType* writetypes=NULL;
        switch( random_int(4))
        {
            case 0 : writetypes = NULL; break;
            case 1 : writetypes = allFloats; break;
            case 2 : writetypes = halfFloat; break;
            case 3 : writetypes = floatHalf; break;
        }
        const PixelType* readTypes=NULL;
        switch( random_int(4))
        {
            case 0 : readTypes = NULL; break;
            case 1 : readTypes = allFloats; break;
            case 2 : readTypes = halfFloat; break;
            case 3 : readTypes = floatHalf; break;
        }
        
        Header hdr = writefile(writeFrameBuf,channels,writetypes);
        Box2i dw = hdr.dataWindow();
        cout << "dataWindow: " << dw.min << ' ' << dw.max << ' ';
        cout.flush();
        FrameBuffer readFrameBuf;
        readfile (readFrameBuf,channels,readTypes);
        
        //
        // only the first 5 compression methods are guaranteed lossless on both half and float.
        // skip comparison for other types
        //
        if(hdr.compression() < 5)
        {
            if (compare(readFrameBuf, writeFrameBuf, dw))
            {
                cout <<  " OK ";
            }
            else
            {
                cout <<  " FAIL" << endl;        
                
            }
        }
        cout << "\n";        
    }
    remove (filename.c_str());
}


} // namespace anon


void 
testLargeDataWindowOffsets (const std::string & tempDir)
{
    filename = tempDir + "imf_test_interleave_patterns.exr";

    random_reseed(1);

    cout << "Testing dataWindows with large offsets ... " << endl;

    test (100);
    
    cout << "ok\n" << endl;
}
