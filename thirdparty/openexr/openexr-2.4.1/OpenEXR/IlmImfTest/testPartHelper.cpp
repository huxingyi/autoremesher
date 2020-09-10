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

#include "testPartHelper.h"

#include <ImfPartHelper.h>
#include <vector>
#include <iostream>
#include <algorithm>


using std::vector;
using std::cout;
using std::endl;
using std::max;
using OPENEXR_IMF_NAMESPACE::MultiViewChannelName;
using OPENEXR_IMF_NAMESPACE::SplitChannels;

namespace
{

template<class T>
void
print(const T & begin,const T & end)
{
    int parts=1;
    for(T i=begin;i!=end;i++)
    {
        parts=max(i->part_number,parts);
    }

    for(int p=0;p<parts;p++)
    {
        for(T i=begin;i!=end;i++)
        {
            if(i->part_number==p)
            {
                cout << i->part_number << ' ' << i->name << " in " << i->view
                     << ' ' << ' ' << i->internal_name << "\n";
            }
        }
    }
}

void
testSingleView()
{
    cout << "testing with single view" << endl;


    vector<MultiViewChannelName> chans(12);
    chans[0].name="R";
    chans[1].name="G";
    chans[2].name="B";
    chans[3].name="A";
    chans[4].name="bunny.foo";
    chans[5].name="velocity.X";
    chans[6].name="velocity.Y";
    chans[7].name="foo.fred";
    chans[8].name="Z";
    chans[9].name="multiple.layers.in.name";
    chans[10].name="multiple.layers.in.othername";
    chans[11].name="foo.shiela";


    cout << " one part:\n";
    SplitChannels(chans.begin(),chans.end(),false,"");

    print(chans.begin(),chans.end());

    cout << "multi part:\n";

    SplitChannels(chans.begin(),chans.end(),true,"");

    print(chans.begin(),chans.end());
    
}

void
testMultiView()
{
   MultiViewChannelName chans[20];
   // Bob layer, only in left
   chans[0].name="bob.one";
   chans[0].view="left";
   chans[1].name="bob.two";
   chans[1].view="left";
   
   chans[2].name="fred.one";
   chans[2].view="right";
   chans[3].name="fred.one";
   chans[3].view="left";
   chans[4].name="fred.two";
   chans[4].view="left";
   chans[5].name="fred.two";
   chans[5].view="right";

   chans[6].name="R";
   chans[6].view="left";
   chans[7].name="R";
   chans[7].view="right";

   chans[8].name="G";
   chans[8].view="right";
   chans[9].name="G";
   chans[9].view="left";

   chans[10].name="B";
   chans[10].view="left";
   chans[11].name="B";
   chans[11].view="right";

   chans[12].name="multiple.layers.in.name";
   chans[12].view="left";
   chans[13].name="multiple.layers.in.name";
   chans[13].view="right";
   
   chans[14].name="multiple.layers.in.othername";
   chans[14].view="left";
   chans[15].name="multiple.layers.in.othername";
   chans[15].view="right";

   chans[16].name="multiple.layers.different.name";
   chans[16].view="left";
   chans[17].name="multiple.layers.different.name";
   chans[17].view="right";

   chans[18].name="multiple.layers.different.othername";
   chans[18].view="left";
   chans[19].name="multiple.layers.different.othername";
   chans[19].view="right";

   cout << "multiview, hero left, single part:\n";
   SplitChannels(chans+0,chans+20,false,"left");
   print(chans+0,chans+20);

   cout << "multiview, hero left, multipart:\n";
   SplitChannels(chans+0,chans+20,true,"left");
   print(chans+0,chans+20);
   

    cout << "multiview, hero right, single part:\n";
   SplitChannels(chans+0,chans+20,false,"right");
   print(chans+0,chans+20);

   cout << "multiview, hero right, multipart:\n";
   SplitChannels(chans+0,chans+20,true,"right");
   print(chans+0,chans+20);

}
}

void
testPartHelper (const std::string & tempDir)
{
    
    cout << "\n\nTesting part helper\n" << endl;

    testSingleView();
    testMultiView();
    cout << " ok\n" << endl;

}
