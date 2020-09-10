///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Weta Digital Ltd
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
// *       Neither the name of Weta Digital nor the names of
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

#include "testCompositeDeepScanLine.h"
#include "random.h"

#include <ImfDeepScanLineOutputFile.h>
#include <ImfDeepScanLineInputFile.h>
#include <ImfChannelList.h>
#include <ImfPartType.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfHeader.h>
#include <ImfNamespace.h>

#include <vector>

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include "tmpDir.h"

namespace{

using std::cout;
using std::endl;
using std::flush;
using std::vector;

using OPENEXR_IMF_NAMESPACE::Header;
using OPENEXR_IMF_NAMESPACE::Channel;
using OPENEXR_IMF_NAMESPACE::UINT;
using OPENEXR_IMF_NAMESPACE::FLOAT;
using OPENEXR_IMF_NAMESPACE::DEEPSCANLINE;
using OPENEXR_IMF_NAMESPACE::ZIPS_COMPRESSION;
using OPENEXR_IMF_NAMESPACE::DeepScanLineOutputFile;
using OPENEXR_IMF_NAMESPACE::DeepScanLineInputFile;
using OPENEXR_IMF_NAMESPACE::DeepFrameBuffer;
using OPENEXR_IMF_NAMESPACE::Slice;
using OPENEXR_IMF_NAMESPACE::DeepSlice;
using IMATH_NAMESPACE::Box2i;

namespace IMF = OPENEXR_IMF_NAMESPACE;

static void 
make_file(const char * filename)
{
    
    int width=4;
    int height=48;
    
    //
    // create a deep output file of widthxheight, where each pixel has 'y' samples,
    // each with value 'x'
    //
    
    Header header( width,height);
    header.channels().insert("Z", Channel(IMF::FLOAT));
    header.compression()=ZIPS_COMPRESSION;
    header.setType(DEEPSCANLINE);
        
    remove (filename);
    DeepScanLineOutputFile file(filename, header);
    
    unsigned int sample_count; 
    float sample;
    float * sample_ptr = &sample; 
    
    DeepFrameBuffer fb;
    
    fb.insertSampleCountSlice(Slice(IMF::UINT,(char *)&sample_count));
    fb.insert("Z",DeepSlice(IMF::FLOAT,(char *) &sample_ptr));
    
    
    file.setFrameBuffer(fb);
    
    for( int y=0 ; y < height ; y++ )
    {
        //
        // ensure each scanline contains a different number of samples,
        // with different values. We don't care that each sample has the same
        // value, or that each pixel on the scanline is identical
        //
        sample_count = y;
        sample = y+100.0;
        
        file.writePixels(1);
        
    }
    
}

static void read_file(const char * filename)
{
    DeepScanLineInputFile file(filename);
    
    Box2i datawin = file.header().dataWindow();
    int width = datawin.size().x+1;
    int height = datawin.size().y+1;
    int x_offset = datawin.min.x;
    int y_offset = datawin.min.y;
    const char * channel = file.header().channels().begin().name();
    
    vector<unsigned int> samplecounts(width);
    vector<float *> sample_pointers(width);
    vector<float> samples;
    
    DeepFrameBuffer fb;
    
    fb.insertSampleCountSlice(Slice(IMF::UINT,(char *) (&samplecounts[0]-x_offset) , sizeof(unsigned int)));
    
    fb.insert( channel,  DeepSlice(IMF::FLOAT,(char *) (&sample_pointers[0]-x_offset) , sizeof(float *),0,sizeof(float)) );
    
    file.setFrameBuffer(fb);
    
    for(int count=0;count<4000;count++)
    {
        int row = random_int(height) + y_offset;
        
        //
        // read row y (at random)
        //
        
        file.readPixelSampleCounts(row,row);
        //
        // check that's correct, and also resize samples array
        //
        
        int total_samples = 0;
        for(int i=0;i<width;i++)
        {
            
            if( samplecounts[i]!= static_cast<unsigned int>(row))
            {
              cout << i << ", " << row << " error, sample counts hould be "
              << row  << ", is " << samplecounts[i]
              << endl << flush;
            }
            
            assert (samplecounts[i]== static_cast<unsigned int>(row));
            
            total_samples+=samplecounts[i];
        }
        
        samples.resize(total_samples);
        //
        // set pointers to point to the correct place
        //
        int total=0;
        for(int i=0 ; i<width && total < total_samples ; i++)
        {
            sample_pointers[i] = &samples[total];
            total+=samplecounts[i];
        }
        
        //
        // read channel
        //
        
        file.readPixels(row,row);
        
        //
        // check
        //
        
        for(int i=0;i<total_samples;i++)
        {
           if(samples[i]!=row+100.f)
           {
               cout << " sample " << i << " on row " << row << " error, shuold be " 
                    << 100.f+row << " got " << samples[i] << endl;
               cout << flush;
           }
           assert(samples[i]==row+100.f);
        }
        
    }
    
    
}
}

void
testDeepScanLineMultipleRead(const std::string & tempDir)
{
    
    cout << "\n\nTesting random re-reads from deep scanline file:\n" << endl;
    
    std::string source_filename = tempDir + "imf_test_multiple_read";
    random_reseed(1);
    
    make_file(source_filename.c_str());
    read_file(source_filename.c_str());
    remove(source_filename.c_str());
    
    cout << " ok\n" << endl;
    
}
