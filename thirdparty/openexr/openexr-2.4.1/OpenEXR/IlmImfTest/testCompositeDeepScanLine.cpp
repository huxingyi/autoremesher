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

#include <vector>
#include <string>
#include <Iex.h>
#include <ostream>
#include <iostream>
#include <typeinfo>
#include <stdlib.h>
#include <assert.h>
#include <sstream>

#include <ImfMultiPartOutputFile.h>
#include <ImfMultiPartInputFile.h>
#include <ImfDeepScanLineOutputPart.h>
#include <ImfDeepScanLineInputPart.h>
#include <ImfChannelList.h>
#include <ImfHeader.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfFrameBuffer.h>
#include <ImfPartType.h>
#include <ImfCompression.h>
#include <ImfInputFile.h>
#include <ImfCompositeDeepScanLine.h>
#include <ImfThreading.h>
#include <IlmThread.h>
#include <ImfNamespace.h>

namespace
{


using std::vector;
using std::string;
using std::ostream;
using std::endl;
using std::cout;
using std::ostringstream;


using OPENEXR_IMF_NAMESPACE::DeepScanLineOutputFile;
using OPENEXR_IMF_NAMESPACE::MultiPartInputFile;
using OPENEXR_IMF_NAMESPACE::DeepScanLineOutputPart;
using OPENEXR_IMF_NAMESPACE::DeepScanLineInputPart;

using OPENEXR_IMF_NAMESPACE::Header;
using OPENEXR_IMF_NAMESPACE::PixelType;
using OPENEXR_IMF_NAMESPACE::DeepFrameBuffer;
using OPENEXR_IMF_NAMESPACE::FrameBuffer;
using OPENEXR_IMF_NAMESPACE::FLOAT;
using OPENEXR_IMF_NAMESPACE::HALF;
using OPENEXR_IMF_NAMESPACE::UINT;
using OPENEXR_IMF_NAMESPACE::Slice;
using OPENEXR_IMF_NAMESPACE::DeepSlice;
using OPENEXR_IMF_NAMESPACE::MultiPartOutputFile;
using IMATH_NAMESPACE::Box2i;
using OPENEXR_IMF_NAMESPACE::DEEPSCANLINE;
using OPENEXR_IMF_NAMESPACE::ZIPS_COMPRESSION;
using OPENEXR_IMF_NAMESPACE::InputFile;
using OPENEXR_IMF_NAMESPACE::setGlobalThreadCount;
using OPENEXR_IMF_NAMESPACE::CompositeDeepScanLine;

// a marker to say we've done inserting values into a sample: do mydata << end()
struct end{};

// a marker to say we're about to send the final result, not another sample: do mydata << result()
struct result{};


//
// support class that generates deep data, along with the 'ground truth' 
// result 
//
template<class T>
class data
{
    public:
    vector<string> _channels;               // channel names - same size and order as in all other arrays,
    vector<T> _current_result;              // one value per channel: the ground truth value for the given pixel
    vector<vector <T> >_results;            // a list of result pixels
    
    
    bool _inserting_result;
    bool _started;                          // we've started to assemble the values - no more channels permitted
    vector<T> _current_sample;              // one value per channel for the sample currently being inserted
    vector< vector <T> > _current_pixel;    // a list of results for the current pixwel
    vector< vector< vector<T> > > _samples; // a list of pixels
    PixelType _type;
    
    data() : _inserting_result(false),_started(false)
    {  
        if(typeid(T).hash_code()==typeid(half).hash_code())
        {
            _type = OPENEXR_IMF_NAMESPACE::HALF;
        }
        else
        {
            _type = OPENEXR_IMF_NAMESPACE::FLOAT;
        }
    }


    // add a value to the current sample
    data & operator << (float value)
    {
        if(_inserting_result)
        {
          _current_result.push_back(value);
        }else{
          _current_sample.push_back(T(value));
        }
        _started=true;
        return *this;
    }
    
    // switch between writing samples and the result
    data & operator << (const result &)
    {
        if(_current_sample.size()!=0)
        {
            throw IEX_NAMESPACE::ArgExc("bug in test code: can't switch to inserting result: values written without 'end' statement");
        }
        if(_current_result.size()!=0)
        {
            throw IEX_NAMESPACE::ArgExc("bug in test suite: already inserting result");
        }
        _inserting_result=true;
        return *this;
    }
    
    // finalise the current sample/results
    
    data & operator << (const end &)
    {
        if(_inserting_result)
        {
            if(_current_result.size()!=_channels.size())
            {
                throw IEX_NAMESPACE::ArgExc("bug in test suite: cannot end result: wrong number of values written");
            }
            _results.push_back(_current_result);
            _current_result.resize(0);
            
            //
            // also cause the current_samples to be written as the given number of pixels
            //
            _samples.push_back(_current_pixel);
            _current_pixel.resize(0);
            _inserting_result=false;
        }
        else
        {
            if(_current_sample.size()!=_channels.size())
            {
                throw IEX_NAMESPACE::ArgExc("bug in test suite: cannot end sample: wrong number of values written");
            }
            _current_pixel.push_back(_current_sample);
            _current_sample.resize(0);
        }
        return *this;
    }
    
    // add a new channel
    
    data & operator << (const string & s) 
    {
        if(_started)
        {
            throw IEX_NAMESPACE::ArgExc("bug in test suite: cannot insert new channels here");
        }
        _channels.push_back(s);
        return *this;
    }
    
    
    // total number of samples - storage for one copy of everything is sizeof(T)*channels.size()*totalSamples
    size_t totalSamples() const
    {
        size_t answer=0;
        for(size_t i=0;i<_samples.size();i++)
        {
            answer+=_samples[i].size();
        }
        return answer;
    }
    
    //copy the channels into the header list
    void 
    setHeader(Header & hdr) const
    {
      for(size_t i=0;i<_channels.size();i++)
      {
         hdr.channels().insert(_channels[i],_type);
      }
    }
    
    
    void frak(vector< data<T> > & parts) const
    {
        for(size_t i=0;i<parts.size();i++)
        {
            parts[i]._channels = _channels;
            parts[i]._results = _results;
            parts[i]._type = _type;
            parts[i]._samples.resize(_samples.size());
        }
        
        //
        // loop over each pixel, pushing its values to a random part
        //
        for(size_t i=0;i<_samples.size();i++)
        {
            // copy sample to a random part
            
            for(size_t s=0;s<_samples[i].size();s++)
            {
                int part = random_int(parts.size());
                parts[part]._samples[i].push_back(_samples[i][s]);
            }
        }
    }
    
    void 
    writeData(DeepScanLineOutputPart & part) const
    {
        Box2i dw=part.header().dataWindow();
        size_t output_pixels = (dw.size().x+1)*(dw.size().y+1);
        
        // how many times we'll write the same pattern
        size_t repeats = 1+(output_pixels/_results.size());
        
        size_t sample_buffer_size = totalSamples()*repeats;
        
        // buffer for sample counts
        vector<unsigned int> counts(output_pixels);
        
        // buffers for sample pointers
        vector< vector<T *> > sample_pointers(_channels.size());
        
        // buffer for actual sample data
        vector< vector<T> > sample_buffers(_channels.size());
        
        for(size_t i=0;i<sample_buffers.size();i++)
        {
            sample_pointers[i].resize(output_pixels);
            sample_buffers[i].resize(sample_buffer_size);
        }
        
        
        size_t pixel=0; // which pixel we are currently writing
        size_t sample=0; // which sample we are currently writing into
        
        for(size_t p=0;p<output_pixels;p++)
        {
            size_t count = _samples[pixel].size();
            counts[p]=count;
            if( count>0 )
            {
                for(size_t c=0 ; c<_channels.size() ; c++)
                {
                    for(size_t s=0 ; s < count ; s++ )
                    {
                        sample_buffers[c][sample+s]=_samples[pixel][s][c];
                    }
                    sample_pointers[c][p]=&sample_buffers[c][sample];
                }
                sample+=count;
            }
            pixel++;
            if(pixel==_samples.size()) pixel=0;
        }
        cout << " wrote " << sample << " samples  into " << output_pixels << " pixels\n";
        
        DeepFrameBuffer fb;
        fb.insertSampleCountSlice(Slice(UINT,
                                        (char *)(&counts[0]-dw.min.x-(dw.size().x+1)*dw.min.y),
                                        sizeof(unsigned int),
                                        sizeof(unsigned int)*(dw.size().x+1)
                                        )
                                  );
        for(size_t c=0;c<_channels.size();c++)
        {
            fb.insert(_channels[c],
                      DeepSlice(_type,(char *)(&sample_pointers[c][0]-dw.min.x-(dw.size().x+1)*dw.min.y),
                            sizeof(T *),
                            sizeof(T *)*(dw.size().x+1),
                            sizeof(T)
                            )
                     );
        }
        part.setFrameBuffer(fb);
        part.writePixels(dw.size().y+1);
        
    }
    
    void 
    setUpFrameBuffer(vector<T> & data,FrameBuffer & framebuf,const Box2i & dw,bool dontbotherloadingdepth) const
    {
        
        // allocate enough space for all channels (even the depth channel)
        data.resize(_channels.size()*(dw.size().x+1)*(dw.size().y+1));
        for(size_t i=0;i<_channels.size();i++)
        {
            if(!dontbotherloadingdepth || (_channels[i]!="Z" && _channels[i]!="ZBack") )
            {
                framebuf.insert(_channels[i].c_str(),
                                Slice(_type,(char *) (&data[i] - (dw.min.x + dw.min.y*(dw.size().x+1))*_channels.size() ),
                                      sizeof(T)*_channels.size(),
                                      sizeof(T)*(dw.size().x+1)*_channels.size())
                                      );
            }
        }
    }
    
    
    //
    // check values are within a suitable tolerance of the expected value (expect some errors due to half float storage etc)
    //
    void 
    checkValues(const vector<T> & data,const Box2i & dw,bool dontbothercheckingdepth)
    {
        size_t size = _channels.size()+(dw.size().x+1)*(dw.size().y+1);
        size_t pel=0;
        size_t channel=0;
        if(dontbothercheckingdepth)
        {
            for(size_t i=0;i<size;i++)
            {
                if(_channels[channel]!="Z" && _channels[channel]!="ZBack")
                {
                    if(fabs(_results[pel][channel] - data[i])>0.005)
                    {
                      cout << "sample " << i << " (channel " << _channels[channel] << " of pixel " << i % _channels.size() << ") ";
                      cout << "doesn't match expected value (channel " << channel << " of pixel " << pel << ") : ";
                      cout << "got " << data[i] << " expected " << _results[pel][channel] << endl;
                    }
                    assert(fabs(_results[pel][channel]- data[i])<=0.005);
                }
                channel++;
                if(channel==_channels.size())
                {
                    channel=0;
                    pel++;
                    if(pel==_results.size())
                    {
                        pel=0;
                    }
                }
                
            }
        }else{
            for(size_t i=0;i<size;i++)
            {
                if(fabs(_results[pel][channel] - data[i])>0.005)
                {
                    cout << "sample " << i << " (channel " << _channels[channel] << " of pixel " << i % _channels.size() << ") ";
                    cout << "doesn't match expected value (channel " << channel << " of pixel " << pel << ") : ";
                    cout << "got " << data[i] << " expected " << _results[pel][channel] << endl;
                }
                assert(fabs(_results[pel][channel] - data[i])<=0.005);
                channel++;
                if(channel==_channels.size())
                {
                    channel=0;
                    pel++;
                    if(pel==_results.size())
                    {
                        pel=0;
                    }
                }
            }
        }
        
    }
    
};

template<class T>
ostream & operator << (ostream & o,data<T> & d)
{
    o << "channels: [ ";
    for(size_t i=0;i<d._channels.size();i++)
    {
        o << d._channels[i] << " ";
    }
    o << "]" << endl;
    
    for(size_t i=0;i<d._samples.size();i++)
    {
        o << "pixel: " << d._samples[i].size() << " samples" << endl;
        
        for(size_t j=0;j<d._samples[i].size();j++)
        {
          o << "     " << j << ": [ ";
          for(size_t k = 0; k < d._samples[i][j].size();k++)
          {
              o << d._samples[i][j][k] << ' ';
          }
          o << "]" << endl;
        }
        o << "result: [ ";
        for(size_t k=0;k<d._results[i].size();k++)
        {
            o << d._results[i][k] << ' ' ;
        }
        o << "]\n" << endl;
    }
    
    return o;
}

template<class DATA>
void
make_pattern(data<DATA> & bob,int pattern_number)
{
    
    if (pattern_number==0)
    {
        // set channels
        
        bob << string("Z") << string("ZBack") << string("A") << string("R");

        // regular two-pixel composite
        bob << 1.0 << 2.0 << 0.0 << 1.0 << end();
        bob << 2.1 << 2.3 << 0.5 << 0.4 << end();
        bob << result();
        bob << 3.1 << 4.3 << 0.5 << 1.4 << end();
        
        bob << 10 << 20 << 1.0 << 1.0 << end();
        bob << 20 << 30 << 1.0 << 2.0 << end();
        bob << result();
        bob << 10 << 20 << 1.0 << 1.0 << end();
        
        bob << result();
        bob << 0.0 << 0.0 << 0.0 << 0.0 << end();
    }
    else if (pattern_number==1)
    {
        //
        // out of order channels, no zback - should-re-order them for us
        //
        bob << string("Z") << string("R") << string("G") << string("B") << string("A");
        
        
        // write this four times, so we get various patterns for splitting the blocks
        for(int pass=0;pass<4;pass++)
        {
          // regular four-pixel composite
          bob << 1.0 << 0.4 << 1.25 << -0.1 << 0.7 << end();
          bob << 2.2 << 0.2 << -0.1 << 0.0 << 0.24 << end();
          bob << 2.3 << 0.9 << 0.56 << 2.26 << 0.9 << end();
          bob << 5.0 << 1.0 << 0.5  << 0.60 << 0.2 << end();
          bob << result();
        
          // eight-pixel composite
          bob << 2.2984 << 0.68800 <<  1.35908 << 0.42896 << 0.9817 <<  end();
          bob << 1.0  << 0.4 << 1.25 << -0.1 << 0.7 << end();
          bob << 2.2  << 0.2 << -0.1 << 0.0 << 0.24 << end();
          bob << 2.3  << 0.9 << 0.56 << 2.26 << 0.9 << end();
          bob << 5.0  << 1.0 << 0.5  << 0.60 << 0.2 << end();
          bob << 11.0 << 0.4 << 1.25 << -0.1 << 0.7 << end();
          bob << 12.2 << 0.2 << -0.1 << 0.0 << 0.24 << end();
          bob << 12.3 << 0.9 << 0.56 << 2.26 << 0.9 << end();
          bob << 15.0 << 1.0 << 0.5  << 0.60 << 0.2 << end();
          bob << result();
          bob << 2.62319 << 0.7005 <<  1.38387 << 0.43678 << 0.99967 <<  end();
        
          // one-pixel composite
         
          bob << 27.0 << 1.0 << -1.0 << 42.0 << 14 << end(); // alpha>1 should still work
          bob << result();
          bob << 27.0 << 1.0 << -1.0 << 42.0 << 14 << end();
        }
        
        
    }
}

template<class T>
void
write_file(const char * filename, const data<T> & master, int number_of_parts)
{
    vector<Header> headers(number_of_parts);
    
    // all headers are the same in this test
    headers[0].displayWindow().max.x=164;
    headers[0].displayWindow().max.y=216;
    headers[0].dataWindow().min.x=random_int(400) - 200;
    headers[0].dataWindow().max.x=headers[0].dataWindow().min.x+40+random_int(400);
    headers[0].dataWindow().min.y=random_int(400) - 200;
    headers[0].dataWindow().max.y=headers[0].dataWindow().min.y+40+random_int(400);
    cout << "data window: " << headers[0].dataWindow().min.x << ',' << headers[0].dataWindow().min.y << ' ' << 
    headers[0].dataWindow().max.x << ',' << headers[0].dataWindow().max.y << endl;
    headers[0].setType(DEEPSCANLINE);
    headers[0].compression()=ZIPS_COMPRESSION;
    headers[0].setName("Part0");
    
    for(int i=1;i<number_of_parts;i++)
    {
        headers[i]=headers[0];
        ostringstream s;
        s << "Part" << i;
        headers[i].setName(s.str());
    }
    
    vector< data<T> > sub_parts(number_of_parts);
    
    if(number_of_parts>1)
    {
      master.frak(sub_parts);
    }  
    
    if(number_of_parts==1)
    {
        master.setHeader(headers[0]);
    }else{
        
        for(int i=0;i<number_of_parts;i++)
        {
            sub_parts[i].setHeader(headers[i]);
        }
    }
    
    MultiPartOutputFile f(filename,&headers[0],headers.size());
    for(int i=0;i<number_of_parts;i++)
    {
        DeepScanLineOutputPart p(f,i);
        if(number_of_parts==1)
        {
            master.writeData(p);
        }else{
            sub_parts[i].writeData(p);
        }
    }
}

template<class T>
void
test_parts (int pattern_number,
            int number_of_parts,
            bool load_depths,
            bool entire_buffer,
            const std::string &tempDir)
{
     std::string fn = tempDir + "imf_test_composite_deep_scanline_source.exr";

     data<T> master;
     make_pattern (master, pattern_number);
     write_file (fn.c_str(), master,number_of_parts);
     

     {
        vector<T> data;         
        CompositeDeepScanLine comp;
        FrameBuffer testbuf;
        MultiPartInputFile input(fn.c_str());
        vector<DeepScanLineInputPart *> parts(number_of_parts);
     
     
        // use 'part' interface TODO test file interface too
        for(int i=0;i<number_of_parts;i++)
        {
            parts[i] = new DeepScanLineInputPart(input,i);
            comp.addSource(parts[i]);
        }
     
     
        master.setUpFrameBuffer(data,testbuf,comp.dataWindow(),load_depths);
     
        comp.setFrameBuffer(testbuf);
     
         //
         // try loading the whole buffer
         //
         if(entire_buffer)
         {
            comp.readPixels(comp.dataWindow().min.y,comp.dataWindow().max.y);
         }else{
           int low = comp.dataWindow().min.y;
           while(low<comp.dataWindow().max.y)
           {
               int high = low + random_int(64);
               if(high>comp.dataWindow().max.y) 
                   high = comp.dataWindow().max.y;
               comp.readPixels(low,high);
               low = high;
           }
         }
        
         master.checkValues(data,comp.dataWindow(),load_depths);
  
         for(int i=0;i<number_of_parts;i++)
         {
            delete parts[i];
         }
     }
     if(number_of_parts==1)
     {
         // also test InputFile interface
         InputFile file(fn.c_str());
         vector<T> data;
         FrameBuffer testbuf;
         const Box2i & dataWindow = file.header().dataWindow();
         master.setUpFrameBuffer(data,testbuf,dataWindow,load_depths);
         file.setFrameBuffer(testbuf);
         if(entire_buffer)
         {
             file.readPixels(dataWindow.min.y,dataWindow.max.y);
         }else{
             int low = dataWindow.min.y;
             while(low<dataWindow.max.y)
             {
                 int high = low + random_int(64);
                 if(high>dataWindow.max.y) 
                     high = dataWindow.max.y;
                 file.readPixels(low,high);
                 low = high;
             }
         }
         
         master.checkValues (data, dataWindow, load_depths);
         
     }
     remove (fn.c_str());
}

}

void testCompositeDeepScanLine (const std::string & tempDir)
{
    
    cout << "\n\nTesting deep compositing interface basic functionality:\n" << endl;

    int passes=2;
    if (!ILMTHREAD_NAMESPACE::supportsThreads ())
    {
            passes=1;
    }
  
    random_reseed(1);
    
    for(int pass=0;pass<2;pass++)
    {

        test_parts<float>(0, 1, true,  true,  tempDir);
        test_parts<float>(0, 1, false, false, tempDir);
        test_parts<half> (0, 1, true,  false, tempDir);
        test_parts<half> (0, 1, false, true,  tempDir);

        //
        // test pattern 1: tested by confirming data is written correctly and
        // then reading correct results in Nuke
        //
        test_parts<float>(1, 1, true,  false, tempDir);
        test_parts<float>(1, 1, false, true,  tempDir);
        test_parts<half> (1, 1, true,  true,  tempDir);
        test_parts<half> (1, 1, false, false, tempDir);


        cout << "Testing deep compositing across multiple parts:\n" << endl;

        test_parts<float>(0, 5, true,  false, tempDir);
        test_parts<float>(0, 5, false, true,  tempDir);
        test_parts<half> (0, 5, true,  false, tempDir);
        test_parts<half> (0, 5, false, true,  tempDir);

        test_parts<float>(1, 3, true,  true,  tempDir);
        test_parts<float>(1, 3, false, false, tempDir);
        test_parts<half> (1, 3, true,  true,  tempDir);
        test_parts<half> (1, 3, false, false, tempDir);

        test_parts<float>(1, 4, true,  true,  tempDir);
        test_parts<float>(1, 4, false, false, tempDir);
        test_parts<half> (1, 4, true,  false, tempDir);
        test_parts<half> (1, 4, false, true,  tempDir);

        if(passes==2 && pass==0)
        {
            cout << " testing with multithreading...\n";
            setGlobalThreadCount(64);
        }
    }
    cout << " ok\n" << endl;
}
