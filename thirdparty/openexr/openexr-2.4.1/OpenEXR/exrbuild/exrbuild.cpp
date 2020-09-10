///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2011, Weta Digital Ltd.
// Portions contributed and copyright held by others as indicated.  All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
//    * Redistributions of source code must retain the above
//      copyright notice, this list of conditions and the following
//      disclaimer.
//
//    * Redistributions in binary form must reproduce the above
//      copyright notice, this list of conditions and the following
//      disclaimer in the documentation and/or other materials provided with
//      the distribution.
//
//    * Neither the name of Weta Digital nor any other contributors to this software
//      may be used to endorse or promote products derived from this software without
//      specific prior written permission.
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
///////////////////////////////////////////////////////////////////////////


#include <ImfPartType.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfMultiPartInputFile.h>
#include <ImfStringAttribute.h>
#include <ImfChannelList.h>
#include <ImfTiledInputPart.h>
#include <ImfTiledOutputPart.h>
#include <ImfInputPart.h>
#include <ImfTiledInputPart.h>
#include <ImfOutputPart.h>
#include <ImfDeepScanLineInputPart.h>
#include <ImfDeepScanLineOutputPart.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfStandardAttributes.h>
#include <ImfMultiView.h>

#include <iostream>
#include <vector>
#include <stdlib.h>

#include <OpenEXRConfig.h>
using namespace OPENEXR_IMF_NAMESPACE;

//
// constructs new EXRs from parts of others
// this code is deliberately inefficient - it decompresses
// and recompresses the file
//
// 

using std::cerr;
using std::endl;
using std::vector;
using std::set;
using std::ostringstream;
using std::min;
using std::max;
using std::string;



void copy_tiledimage(MultiPartInputFile & input,MultiPartOutputFile & output,int inPart,int outPart,const std::string & inview=std::string(""))
{
  int buffer_size;
  
  FrameBuffer inFrameBuffer;
  FrameBuffer outFrameBuffer;
  TiledInputPart in(input,inPart);
  TiledOutputPart out(output,outPart);

  const Header & inhdr = input.header(inPart);
   
  
  int channels = 0;
  for(ChannelList::ConstIterator i = inhdr.channels().begin();
          i != inhdr.channels().end();++i) ++channels;
  
  int x_levels=0;
  int y_levels=0;
  
  TileDescription t = inhdr.tileDescription();
  

  StringVector views;
  if(hasMultiView(inhdr))
  {
	  views=multiView(inhdr);
  }
  
  switch(t.mode)
  {
    case ONE_LEVEL :
      x_levels = 1;
      y_levels = 1;
      break;
    case MIPMAP_LEVELS :
      x_levels = t.xSize;
      y_levels = 1;
      break;
    case RIPMAP_LEVELS :
      x_levels = t.xSize;
      y_levels = t.ySize;
  }
    
  for(int x_level = 0 ; x_level < x_levels ; x_level++)
  {
    for(int y_level = 0 ; y_level < y_levels ;y_level++)
    {
      int actual_y_level = t.mode==RIPMAP_LEVELS ? y_level : x_level;
      
      IMATH_NAMESPACE::Box2i dw=  in.dataWindowForLevel(x_level,actual_y_level);
      int width = dw.max.x-dw.min.x+1;
      int height = dw.max.y-dw.min.y+1;
   
      int y = dw.min.y;
      int x = dw.min.x;

      // allocate at least enough memory to handle the data
      // (biggest data is four bytes)
      // we store the channels separately, one after the other
    
      buffer_size = 4*width*height*channels;
    
      char * channelbuf = new char[buffer_size];
    
      // pointer to where we put the first byte
      char * bufptr = channelbuf;
  
      for(ChannelList::ConstIterator i = inhdr.channels().begin();
           i != inhdr.channels().end();++i) 
      {
	 if(inview=="" || views.size()==0 || viewFromChannelName(i.name(),views)==inview)
	 {
           inFrameBuffer.insert( i.name(),
                                 Slice( i.channel().type, bufptr-(y*width+x)*4,
                                             4,width*4));
           outFrameBuffer.insert( removeViewName(i.name(),inview),
                                  Slice( i.channel().type, bufptr-(y*width+x)*4,
                                              4,width*4));
           bufptr+=width*height*4;
	 }
       }

       in.setFrameBuffer(inFrameBuffer);
       out.setFrameBuffer(outFrameBuffer);

       //
       // copy tiles for level
       // 
       for (int tileY = 0; tileY < out.numYTiles(actual_y_level); ++tileY)
        for (int tileX = 0; tileX < out.numXTiles(x_level); ++tileX)
	{
	  memset(channelbuf,20,width*height*channels*4);
          in.readTile(tileX,tileY,x_level,actual_y_level);
          out.writeTile(tileX,tileY,x_level,actual_y_level);
	} 
      delete [] channelbuf;
    }
  }

}

void copy_scanlineimage(MultiPartInputFile & input, MultiPartOutputFile & output,int inPart,int outPart,const std::string & inview =std::string(""))
{
  
  
  int buffer_size;
  
  FrameBuffer inFrameBuffer;
  FrameBuffer outFrameBuffer;

  InputPart in(input,inPart);
  OutputPart out(output,outPart);

  const Header& inhdr = input.header(inPart);
  
     int channels = 0;
    for(ChannelList::ConstIterator i = inhdr.channels().begin();
          i != inhdr.channels().end();++i) ++channels;
    
    int width = inhdr.dataWindow().max.x-
       inhdr.dataWindow().min.x+1;
    int height = inhdr.dataWindow().max.y-
        inhdr.dataWindow().min.y+1;
    
    int y = inhdr.dataWindow().min.y;
    int x = inhdr.dataWindow().min.x;

  //allocate at least enough memory to handle the data
  //(biggest data is four bytes)
  // we store the channels separately, one after the other
    
  buffer_size = 4*width*height*channels;
    
  char * channelbuf = new char[buffer_size];
    
  // now iterate over each channel and hook it all up
    
  // pointer to where we put the first byte
  char * bufptr = channelbuf;
 
  StringVector views;
  if(hasMultiView(inhdr))
  {
	  views=multiView(inhdr);
  }
   
  for(ChannelList::ConstIterator i = inhdr.channels().begin();
        i != inhdr.channels().end();++i) 
  {
     if(inview=="" || views.size()==0 || viewFromChannelName(i.name(),views)==inview)
	      {
    inFrameBuffer.insert( i.name(),
                                 Slice( i.channel().type, bufptr-(y*width+x)*4,
                                             4,width*4));
    outFrameBuffer.insert( removeViewName(i.name(),inview),
                                  Slice( i.channel().type, bufptr-(y*width+x)*4,
                                              4,width*4));
     }
    bufptr+=width*height*4;
  }
  
  in.setFrameBuffer(inFrameBuffer);
  out.setFrameBuffer(outFrameBuffer);
  
  for(int row = inhdr.dataWindow().min.y ; 
      row <= inhdr.dataWindow().max.y; 
      row++)
  {
      in.readPixels(row);
      out.writePixels(1);
  }
  delete [] channelbuf;
}

void copy_scanlinedeep(MultiPartInputFile & input, MultiPartOutputFile & output,int inPart,int outPart)
{
  
  
  DeepFrameBuffer inFrameBuffer;
  DeepFrameBuffer outFrameBuffer;

  DeepScanLineInputPart in(input,inPart);
  DeepScanLineOutputPart out(output,outPart);

  const Header & header = input.header(inPart);
  
     int channels = 0;
    for(ChannelList::ConstIterator i = header.channels().begin();
          i != header.channels().end();++i) ++channels;
    
    int width = header.dataWindow().max.x-
       header.dataWindow().min.x+1;
    int height = header.dataWindow().max.y-
        header.dataWindow().min.y+1;
    
    int y = header.dataWindow().min.y;
    int x = header.dataWindow().min.x;

  //allocate enough memory to handle the sample counts for every pixel in the image
    
  int counter_size = width*height;  
  int * countbuf = new int[counter_size];

  //
  //allocate enough memory to handle the pointers for every channel of every pixel in the image
  //
  int pointer_size= width*height*channels;
  char ** pointerbuf = new char *[pointer_size];
  
  inFrameBuffer.insertSampleCountSlice(Slice (UINT,
					      (char *) (countbuf - (y*width+x)),
				       sizeof (unsigned int) * 1,// xStride
				       sizeof (unsigned int) * width));// yStride

  
  outFrameBuffer.insertSampleCountSlice(Slice (UINT,
					      (char *) (countbuf - (y*width+x)),
				       sizeof (unsigned int) * 1,// xStride
				       sizeof (unsigned int) * width));// yStride

  
  // for simplicity, allocate 4 bytes per channel per sample (even though halfs only take two)
  // pointers are interleaved -pointers for all channels of pixel 0 go first, then  pixel 1 etc
  // within the data vector, samples will be stored contiguously (i.e. channels are interleaved)
  int channel = 0;
  for(ChannelList::ConstIterator i = header.channels().begin();
        i != header.channels().end();++i) 
  {

    inFrameBuffer.insert( i.name(),
                                 DeepSlice( i.channel().type, ((char *) pointerbuf)-((y*width+x)*channels+channel)*4,
                                             4*channels,width*4*channels,4*channels));
    outFrameBuffer.insert( i.name(),
                                  DeepSlice( i.channel().type, ((char *) pointerbuf)-((y*width+x)*channels+channel)*4,
                                              4*channels,width*4*channels,4*channels));
    channel++;
  }
  
  in.setFrameBuffer(inFrameBuffer);
  out.setFrameBuffer(outFrameBuffer);
  
  //
  // read the entire sample count array
  //
  
  in.readPixelSampleCounts(header.dataWindow().min.y,header.dataWindow().max.y);
  
  
  vector<char> samples;
  
  int * count_start = countbuf; // pointer to first sample count in the image
  char ** pointer_start= pointerbuf;
  
  
  //
  // loop over each row of the image - at each row, allocate storage for this row only
  // within the sample array
  // we can reuse the array every row, so we only need as much memory as is required for the largest row
  //
  for(int row = header.dataWindow().min.y ; 
      row <= header.dataWindow().max.y; 
      row++)
  {
    // count samples on the row
    
    int count=0;
    for(int x = header.dataWindow().min.x;x<=header.dataWindow().max.x;x++) count+=count_start[x];
    
    // allocate enough data for that row
    
    samples.resize(count*channels);
    
    // set pointers for row
    count = 0;
    for(int x = header.dataWindow().min.x;x<=header.dataWindow().max.x;x++)
    {
      
      for(int i = 0 ; i < channels;i++)
      {
        pointer_start[x*channels+i]=&samples[count*channels+i];
      }
      count+=count_start[x];
    }
    //
    // set pointers for row z
    //
      in.readPixels(row);
      out.writePixels(1);
      // bump pointers to next row
      count_start+=width;
      pointer_start+=width*channels;
  }
  delete [] countbuf;
  delete [] pointerbuf;
}



void make_unique_names(vector<Header> & headers)
{
   set<string> names;
   for( size_t i = 0 ; i < headers.size() ; i++ )
   {
	std::string base_name; 
	// if no name at all, set it to <type><partnum> (first part is part 1)
	if(!headers[i].hasName())
        {	
           ostringstream s;
           s << headers[i].type() << (i+1);
	   base_name = s.str();
	   headers[i].setName(base_name);
	}else{
           base_name = headers[i].name();
	}
        // check name hasn't already been used, if so add a _<number> to it
        if(names.find(base_name)!=names.end())
        {
            
            ostringstream s;
	    size_t backup=1;
	    do{
	       s.clear();
               s << headers[i].type() << i << "_" << backup;
	       backup++;
	    }while(names.find(s.str())!=names.end());
	   headers[i].setName(s.str());
	}
   }
}

int main(int argc,char * argv[])
{

  if(argc <3 )
  {
    cerr << argv[0] << " takes a collection of EXR images and outputs them as a single multipart EXR\n";
    cerr << std::string(strlen(argv[0]),' ') << " syntax: " << argv[0] << "  [input.exr[:partnum[.view]]] [input2.exr[:partnum[.view]]] ... output.exr\n";
    exit(1);
  }
  
  int numInputs = argc-2;
  vector<int> partnums(numInputs);
  vector<MultiPartInputFile *> inputs(numInputs);
  vector<Header> headers(numInputs);
  vector<string> views(numInputs);

  //
  // parse all inputs
  //
  //
  for(int i = 0 ; i < numInputs; i++)
  {

     // if input is <file>:<partnum>[.view] then extract part number (and also view if preset), else get part 0	  
     string filename(argv[i+1]);
     size_t colon = filename.rfind(':');
     if(colon==string::npos)
     {
	partnums[i]=0;
     }else{
        string num=filename.substr(colon+1);
	partnums[i]=atoi(num.c_str());
	filename=filename.substr(0,colon);
        size_t dot = num.rfind('.');
        if(dot!=string::npos)
        {
           views[i]=num.substr(dot+1);
	}
      }
      // open and check
      inputs[i] = new MultiPartInputFile(filename.c_str());
      if(partnums[i] >= inputs[i]->parts())
      {
       std::cerr << "oops: you asked for part " << partnums[i] << " in " << argv[1+i]
         << ", which only has " << inputs[i]->parts() << " parts\n";
       exit(1);
      }
      //copy header from required part of input to our header array
      headers[i] = inputs[i]->header(partnums[i]);
      if(views[i]!="")
      {
          if(hasMultiView(headers[i]))
	  {
		  StringVector multiview = multiView(headers[i]);
		  ChannelList newList;
		  //
		  // also clean up channel names
		  //
		  for(ChannelList::Iterator c=headers[i].channels().begin();c!=headers[i].channels().end();++c)
		  {
			  if(viewFromChannelName(c.name(),multiview)==views[i])
			  {
				  newList.insert(removeViewName(c.name(),views[i]),c.channel());
			  }
		  }
		  headers[i].channels()=newList;
		  headers[i].erase("multiView");
	  }
	  headers[i].setView(views[i]);
      }
   }


   // sort out names - make unique

   if(numInputs>1)
   {
       make_unique_names(headers);
   }	       

  MultiPartOutputFile out(argv[argc-1],&headers[0],headers.size(),false, 4);
 
  for(int p = 0 ; p <numInputs;p++)
  { 

	  //todo - this only works for regular images - make one of these
	  //for every type of data 
    Header header = headers[p];
    std::string type = header.type();
    if(type==SCANLINEIMAGE)
    {
      copy_scanlineimage(*inputs[p],out,partnums[p],p,views[p]);
    }else if(type==TILEDIMAGE)
    {
      copy_tiledimage(*inputs[p],out,partnums[p],p,views[p]);
    }else if(type==DEEPSCANLINE)
    {
      copy_scanlinedeep(*inputs[p],out,partnums[p],p);
    }
   }//next part
}
