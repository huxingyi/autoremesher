
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

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include "ImfInputFile.h"
#include "ImfInputPart.h"
#include "ImfTiledInputFile.h"
#include "ImfTiledInputPart.h"

#include "ImfDeepScanLineInputFile.h"
#include "ImfDeepScanLineInputPart.h"
#include "ImfDeepTiledInputFile.h"
#include "ImfDeepTiledInputPart.h"

#include "ImfChannelList.h"
#include "ImfMultiPartInputFile.h"
#include "ImfPartType.h"
#include <half.h>
#include <vector>

#include <assert.h>

#include "tmpDir.h"

namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;

#ifndef ILM_IMF_TEST_IMAGEDIR
    #define ILM_IMF_TEST_IMAGEDIR
#endif

namespace
{


void doFrameBuffer(vector<half>& storage, const Header & hdr,FrameBuffer& dummy)
{
    
    int chans=0;
    for( ChannelList::ConstIterator q =hdr.channels().begin() ; q!=hdr.channels().end() ; q++)
    {
        chans++;
    }
    
    Box2i dw= hdr.dataWindow();
    
    storage.resize( (dw.size().x+1)*(dw.size().y+1)*chans);
    
    int xstride = chans*sizeof(half);
    int ystride = xstride*(dw.size().x+1);
    int offset = ystride*dw.min.y + dw.min.x*xstride;
    
    int chan=0;
    for( ChannelList::ConstIterator q = hdr.channels().begin() ; q!=hdr.channels().end() ; q++)
    {
        dummy.insert(q.name(),Slice(HALF,((char*) &storage[chan])-offset,xstride,ystride));
        chan++;
    }
    
}

template<class T> void readTiledThing(T & input,bool test)
{
    vector<half> value;
    FrameBuffer dummy;
    doFrameBuffer(value,input.header(),dummy);
    input.setFrameBuffer(dummy);
    int x_levels;
    int y_levels;
         
    if(test && input.header().hasType())
    {
        if(input.header().type()!=TILEDIMAGE)
        {
            std::cerr << "tiled image/part didn't have tiledimage type\n";
            //assert(input.type()==TILEDIMAGE);
        }
    }
    
    TileDescription t = input.header().tileDescription();
    switch(t.mode)
    {
        case ONE_LEVEL :
            x_levels = 1;
            y_levels = 1;
            break;
        case MIPMAP_LEVELS :
            x_levels = input.numXLevels();
            y_levels = 1;
            break;
        case RIPMAP_LEVELS :
            x_levels = input.numXLevels();
            y_levels = input.numYLevels();
            break;
        case NUM_LEVELMODES:
        default:
            std::cerr << "Invalid tile mode " << int(t.mode) << std::endl;
            x_levels = y_levels = 0;
            break;
    }

    for(int x_level = 0 ; x_level < x_levels ; x_level++)
    {
        for(int y_level = 0 ; y_level < y_levels ;y_level++)
        {
            // unless we are RIPMapped, the y_level=x_level, not 0
            int actual_y_level = t.mode==RIPMAP_LEVELS ? y_level : x_level;
         
            input.readTiles(0,input.numXTiles(x_level)-1,0,input.numYTiles(actual_y_level)-1,x_level,actual_y_level);

        }
    } 
}


template<class T> void readScanlineThing(T& input,bool test)
{
    
    
    if(test && input.header().hasType())
    {
        if(input.header().type()!=SCANLINEIMAGE)
        {
            std::cerr << "tiled image/part didn't have tiledimage type\n";
            //assert(input.type()==TILEDIMAGE);
        }
    }
    
    vector<half> value;
    FrameBuffer dummy;
    doFrameBuffer(value,input.header(),dummy);
    input.setFrameBuffer(dummy);
    input.readPixels(input.header().dataWindow().min.x,input.header().dataWindow().max.x);
}

void checkDeepTypesFailToLoad(const char * file)
{
    bool caught = false;

    // trying to open it as a deep tiled file should fail

    try
    {
        caught = false;
        DeepTiledInputFile f(file);
        assert(false);
    }
    catch(...)
    {
        caught = true;
    }
    assert (caught);

    // trying to open it as a deep tiled part of a multipart file should fail
    try
    {
        caught = false;
        MultiPartInputFile multiin(file);
        DeepTiledInputPart p(multiin,0);
        assert(false);
    }
    catch(...)
    {
        caught = true;
    }
    assert (caught);
    
    // trying to open it as a deep scanline file should fail
    try
    {
        caught = false;
        DeepScanLineInputFile f(file);
        assert(false);
    }
    catch(...)
    {
        caught = true;
    }
    assert (caught);

    // trying to open it as a deep scanline part of a multipart file should fail
    try
    {
        caught = false;
        MultiPartInputFile multiin(file);
        DeepScanLineInputPart p(multiin,0);
        assert(false);
    }
    catch(...)
    {
        caught = true;
    }
    assert (caught);
}


void testTiledWithBadAttribute(const char* file)
{
    // it's a tiled file, so it should read as a file
    TiledInputFile in(file);
    readTiledThing(in,false);
    
    {
        // it should also read using the multipart API (and have its attribute fixed)
        MultiPartInputFile multiin(file);
        TiledInputPart tip(multiin,0);
        readTiledThing(tip,true);
            
        // it should also read using the regular file API as a scanline file
        InputFile sin(file);
        readScanlineThing(sin,false);
            
    }
    {
        // it should also read using the multipart API as a scanline file
         MultiPartInputFile multiin(file);
            
        InputPart ip(multiin,0);
        readScanlineThing(ip,false);
    }
    
    checkDeepTypesFailToLoad(file);
    
}

void testScanLineWithBadAttribute(const char * file)
{
    bool caught;

    InputFile in(file);
    readScanlineThing(in,false);
 
    MultiPartInputFile multiin(file);
    InputPart ip(multiin,0);
    readScanlineThing(ip,false);
    
    
    checkDeepTypesFailToLoad(file);
    
    // trying to open it as a tiled file should also fail
    try
    {
        caught = false;
        TiledInputFile f(file);
        assert(false);
    }
    catch(...)
    {
        caught = true;
    }
    assert (caught);
    
    // trying to open it as a tiled part of a multipart file should fail
    try
    {
        caught = false;
        MultiPartInputFile multiin(file);
        TiledInputPart p(multiin,0);
        assert(false);
    }
    catch(...)
    {
        caught = true;
    }
    assert (caught);
}

const std::string & NOTYPEATTR="";

template<class IN,class OUT> void check(const char* filename,const string& inputtype,const string &outputtype,bool add_tiledesc)
{
    Header f;
    
    if(inputtype!=NOTYPEATTR)
    {
        f.setType(inputtype);
    }
    f.compression()=ZIPS_COMPRESSION;
    if(add_tiledesc)
    {
        f.setTileDescription(TileDescription());
    }
    
    remove(filename);
    {
        OUT file(filename,f);
    }
    
    {
        IMF::MultiPartInputFile file(filename);
    
        if(outputtype!=NOTYPEATTR && file.header(0).type()!=outputtype)
        {
            cerr << "Error: expected type in header to be " << outputtype << " but got " << file.header(0).type() << " from multipart when input type was " << (inputtype==NOTYPEATTR ? "unset" : inputtype )<< std::endl;
        }
       
        assert(outputtype==NOTYPEATTR || file.header(0).type()==outputtype);
    }
    
    {
        IN file(filename);
        if(outputtype==NOTYPEATTR)
        {
            if(file.header().hasType())
            {
                cerr << " type attribute got inserted when it shouldn't have been\n";
            }
            
            assert( !file.header().hasType() );
            
        }else if(file.header().type()!=outputtype)
        {
            cerr << "Error: expected type in header to be " << outputtype << " but got " << file.header().type() << " when input type was " << (inputtype==NOTYPEATTR ? "unset" : inputtype )<< std::endl;
        }
        
    }
    remove(filename);
}

void testWriteBadTypes()
{
    static const char* tmpfile = IMF_TMP_DIR "badfile.exr";
    
    // attributes should be added automatically for deep files
    check<DeepScanLineInputFile,DeepScanLineOutputFile>(tmpfile,NOTYPEATTR,DEEPSCANLINE,false);
    check<DeepTiledInputFile,DeepTiledOutputFile>(tmpfile,NOTYPEATTR,DEEPTILE,true);

    // attributes should NOT be added automatically for normal images
    check<InputFile,OutputFile>(tmpfile,NOTYPEATTR,NOTYPEATTR,false);
    check<InputFile,TiledOutputFile>(tmpfile,NOTYPEATTR,NOTYPEATTR,true);
    check<TiledInputFile,TiledOutputFile>(tmpfile,NOTYPEATTR,NOTYPEATTR,true);
    
    
    // if an attribute is provided, it should get changed to the correct one
    check<InputFile,OutputFile>(tmpfile,SCANLINEIMAGE,SCANLINEIMAGE,false);
    check<InputFile,TiledOutputFile>(tmpfile,SCANLINEIMAGE,TILEDIMAGE,true);
    check<TiledInputFile,TiledOutputFile>(tmpfile,SCANLINEIMAGE,TILEDIMAGE,true);
    check<DeepScanLineInputFile,DeepScanLineOutputFile>(tmpfile,SCANLINEIMAGE,DEEPSCANLINE,false);
    check<DeepTiledInputFile,DeepTiledOutputFile>(tmpfile,SCANLINEIMAGE,DEEPTILE,true);
    
    check<InputFile,OutputFile>(tmpfile,TILEDIMAGE,SCANLINEIMAGE,false);
    check<InputFile,TiledOutputFile>(tmpfile,TILEDIMAGE,TILEDIMAGE,true);
    check<TiledInputFile,TiledOutputFile>(tmpfile,TILEDIMAGE,TILEDIMAGE,true);
    check<DeepScanLineInputFile, DeepScanLineOutputFile>(tmpfile,TILEDIMAGE,DEEPSCANLINE,false);
    check<DeepTiledInputFile,DeepTiledOutputFile>(tmpfile,TILEDIMAGE,DEEPTILE,true);

    check<InputFile,OutputFile>(tmpfile,DEEPSCANLINE,SCANLINEIMAGE,false);
    check<InputFile,TiledOutputFile>(tmpfile,DEEPSCANLINE,TILEDIMAGE,true);
    check<TiledInputFile,TiledOutputFile>(tmpfile,DEEPSCANLINE,TILEDIMAGE,true);
    check<DeepScanLineInputFile, DeepScanLineOutputFile>(tmpfile,DEEPSCANLINE,DEEPSCANLINE,false);
    check<DeepTiledInputFile,DeepTiledOutputFile>(tmpfile,DEEPSCANLINE,DEEPTILE,true);

    check<InputFile,OutputFile>(tmpfile,DEEPTILE,SCANLINEIMAGE,false);
    check<InputFile,TiledOutputFile>(tmpfile,DEEPTILE,TILEDIMAGE,true);
    check<TiledInputFile,TiledOutputFile>(tmpfile,DEEPTILE,TILEDIMAGE,true);
    check<DeepScanLineInputFile,DeepScanLineOutputFile>(tmpfile,DEEPTILE,DEEPSCANLINE,false);
    check<DeepTiledInputFile,DeepTiledOutputFile>(tmpfile,DEEPTILE,DEEPTILE,true);
 
}

}

void testBadTypeAttributes(const std::string & tempDir)
{
      cout << "Testing whether bad type attributes are fixed on read... " << endl;

      testTiledWithBadAttribute (ILM_IMF_TEST_IMAGEDIR "tiled_with_scanlineimage_type.exr");
      testTiledWithBadAttribute (ILM_IMF_TEST_IMAGEDIR "tiled_with_deepscanline_type.exr");
      testTiledWithBadAttribute (ILM_IMF_TEST_IMAGEDIR "tiled_with_deeptile_type.exr");
      
      testScanLineWithBadAttribute (ILM_IMF_TEST_IMAGEDIR "scanline_with_tiledimage_type.exr");
      testScanLineWithBadAttribute (ILM_IMF_TEST_IMAGEDIR "scanline_with_deeptiled_type.exr");
      testScanLineWithBadAttribute (ILM_IMF_TEST_IMAGEDIR "scanline_with_deepscanline_type.exr");
      
      cout << "Testing whether bad type attributes are fixed on write... " << endl;

      testWriteBadTypes();
      
      cout << "ok\n" << endl;
}
