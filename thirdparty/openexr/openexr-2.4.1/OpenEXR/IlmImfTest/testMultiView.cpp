///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007 Weta Digital Ltd
// Copyright (c) 2012 Industrial Light & Magic, a division of Lucasfilm
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

#include <ImfMultiView.h>

#include <typeinfo>
#include <sstream>
#include <string.h>
#include <assert.h>
#include <stdarg.h>


using namespace OPENEXR_IMF_NAMESPACE;
using namespace std;


namespace {

ChannelList
buildList (const char *name, ...)
{
    //
    // nice function to build channel lists
    //

    ChannelList list;
    const char *channelName = name;

    va_list ap;
    va_start (ap, name);

    while (channelName != 0)
    {
	list.insert (channelName, Channel());
	channelName = va_arg (ap, char *);
    }

    va_end (ap);
    return list;
}


void
testMultiViewFunctions ()
{
    StringVector multiView;

    multiView.push_back ("right");
    multiView.push_back ("left");
    multiView.push_back ("centre");

    //
    // Test viewFromChannelName()
    //

    // default view

    assert (viewFromChannelName ("R", multiView) == "right");

    // explicitly specified default view

    assert (viewFromChannelName ("right.balween", multiView) == "right");

    // non-default view: two sections

    assert (viewFromChannelName ("left.gritstone", multiView) == "left");

    // non-default view: two sections

    assert (viewFromChannelName ("centre.ronaldsay", multiView) == "centre");

    // non-default view: three sections

    assert (viewFromChannelName ("swaledale.left.lonk", multiView) == "left");

    // explicitly specified default view: four sections

    assert (viewFromChannelName ("manx.loghtan.right.shetland",
				 multiView) == "right");

    // non-default view: five sections

    assert (viewFromChannelName ("dorset.down.hebridean.centre.r",
                                 multiView) == "centre");

    // shouldn't happen that we have null channel names

    assert (viewFromChannelName ("", multiView) == "");

    // single section with no view name: default view

    assert (viewFromChannelName ("dartmoor", multiView) == "right");

    // two sections with no view name: no view

    assert (viewFromChannelName ("scottish.blackface", multiView) == "");

    // three sections with no view name: no view

    assert (viewFromChannelName ("beulah.speckled.face", multiView) == "");

    // four sections with no view name: no view

    assert (viewFromChannelName ("devon.and.cornwall.longwool",
				 multiView) == "");

    //
    // Test areCounterparts()
    //

    // two non default channel names in list

    assert (areCounterparts ("right.R",
			     "centre.R",
			     multiView) == true);

    // two channel names, both explicit and in list,
    // even though one is default channel

    assert (areCounterparts ("left.R",
			     "right.R",
			     multiView) == true);

    // default view with non-default view

    assert (areCounterparts ("R",
			     "left.R",
			     multiView) == true);

    // as above, but other way round

    assert (areCounterparts ("left.R",
			     "R",
			     multiView) == true);

    // same channel name specified in two different ways

    assert (areCounterparts ("right.R",
			     "R",
			     multiView) == false);

    // as above, but other way round

    assert (areCounterparts ("R",
			     "right.R",
			     multiView) == false);

    // none.R is not in a view

    assert (areCounterparts ("none.R",
			     "left.R",
			     multiView) == false);

    // as above, but other way round

    assert (areCounterparts ("left.R",
			     "none.R",
			     multiView) == false);

    // as above, but with default channel

    assert (areCounterparts ("X",
			     "none.X",
			     multiView) == false);

    // as above, but other way round

    assert (areCounterparts ("none.B",
			     "B",
			     multiView) == false);

    // both not in a view

    assert (areCounterparts ("southdown.none.G",
			     "wiltshire.horn.G",
			     multiView) == false);

    // as above, but different lengths of names

    assert (areCounterparts ("wiltshire.horn.G",
			     "cotswold.G",
			     multiView) == false);

    // three section pairs

    assert (areCounterparts ("wensleydale.left.baa",
                             "wensleydale.right.baa",
			     multiView) == true);

    // different in first section

    assert (areCounterparts ("wensleydal.left.baa",
                             "wensleydale.right.baa",
			     multiView) == false);

    // different in last section

    assert (areCounterparts ("wensleydale.left.bah",
                             "wensleydale.right.baa",
			     multiView) == false);

    // same channel

    assert (areCounterparts ("wensleydale.left.baa",
                             "wensleydale.left.baa",
			     multiView) == false);

    // second is in no view

    assert (areCounterparts ("wensleydale.right.fell",
			     "wensleydale.rough.fell",
			     multiView) == false);

    // first is in no view

    assert (areCounterparts ("wensleydale.rough.fell",
			     "wensleydale.left.fell",
			     multiView) == false);

    // four sectons

    assert (areCounterparts ("lincoln.longwool.right.A",
			     "lincoln.longwool.left.A",
			     multiView) == true);

    // different in final section

    assert (areCounterparts ("lincoln.longwool.right.B",
			     "lincoln.longwool.left.A",
			     multiView) == false);

    // different in second section

    assert (areCounterparts ("lincoln.shortwool.right.A",
			     "lincoln.longwool.left.A",
			     multiView) == false);

    // different in first section

    assert (areCounterparts ("cumbria.longwool.right.A",
			     "lincoln.longwool.left.A",
			     multiView) == false);

    // enough said

    assert (areCounterparts ("baa.baa.black.sheep",
			     "lincoln.longwool.left.A",
			     multiView) == false);


    // three sections with default - only last is same

    assert (areCounterparts ("portland.left.baa",
			     "baa",
			     multiView) == false);

    // four sections with default

    assert (areCounterparts ("dorset.down.left.baa",
			     "baa",
			     multiView) == false);

    //
    // Channel list tests
    //

    // list of channels in some multiview image

    ChannelList a = buildList
	("A",
	 "B",
	 "C",
	 "right.jacob",
	 "shropshire.right.D",
	 "castlemilk.moorit.right.A",
	 "black.welsh.mountain.right.A",
	 "left.A",
	 "left.B",
	 "left.C",
	 "left.jacob",
	 "shropshire.left.D",
	 "castlemilk.moorit.left.A",
	 "black.welsh.mountain.left.A",
	 "centre.A",
	 "centre.B",
	 "centre.C",
	 "shropshire.centre.D",
	 "castlemilk.moorit.centre.A",
	 "none.A",
	 "none.B",
	 "none.C",
	 "none.D",
	 "none.jacob",
	 "shropshire.none.D",
	 "rough.fell",
	 (char *) 0);

    //
    // List of channels in each view
    //

    // all left channels

    ChannelList realLeft = buildList
	("left.A",
	 "left.B",
	 "left.C",
	 "left.jacob",
	 "shropshire.left.D",
	 "castlemilk.moorit.left.A",
	 "black.welsh.mountain.left.A",
	 (char *) 0);

    ChannelList realRight = buildList
	("A",
	 "B",
	 "C",
	 "right.jacob",
	 "shropshire.right.D",
	 "castlemilk.moorit.right.A",
	 "black.welsh.mountain.right.A",
	 (char *) 0);

    // all the right channels including the default channels

    ChannelList realCentre = buildList
	("centre.A",
	 "centre.B",
	 "centre.C",
	 "shropshire.centre.D",
	 "castlemilk.moorit.centre.A",
	 (char *) 0);

    // no jacob channel
    // there IS a jacob channel but it has no counterparts because
    // this is in "no view"

    ChannelList realNone = buildList
	("none.A",
	 "none.B",
	 "none.D",
	 "none.C",
	 "none.jacob",
	 "shropshire.none.D",
	 "rough.fell",
	 (char *) 0);

    // have a dummy name just to throw a wolf amongst the sheep

    multiView.push_back ("wolf");

    // no channels

    ChannelList realNull = buildList ((char *) 0);

    //
    // Test channelsInView()
    //

    // default view channel extraction

    assert (channelsInView ("right", a, multiView) == realRight);

    // non-default view channel extraction

    assert (channelsInView ("left", a, multiView) == realLeft);

    // missing 'centre.jacob'

    assert (channelsInView ("centre", a, multiView) == realCentre);

    // "none" isn't a view name, no channels returned

    assert (channelsInView ("none", a, multiView) == realNull);

    // "wolf" has no channels, no channels returned

    assert (channelsInView ("wolf", a, multiView) == realNull);

    // all no view channels

    assert (channelsInNoView (a, multiView)  == realNone);


    //
    // Test channelInAllViews()
    //

    ChannelList realA = buildList
	("left.A",
	 "A",
	 "centre.A",
	 (char *) 0);

    ChannelList realB = buildList
	("left.B",
	 "B",
	 "centre.B",
	 (char *) 0);

    ChannelList realJacob = buildList
	("left.jacob",
	 "right.jacob",
	 (char *) 0);

    ChannelList realCm = buildList
	("castlemilk.moorit.left.A",
	 "castlemillk.moorit.right.A",
	 "castlemilk.moorit.centre.A",
	 (char *) 0);

    ChannelList realBwm = buildList
	("black.welsh.mountain.left.A",
	 "black.welsh.mountain.right.A",
	 (char *) 0);

    assert (channelInAllViews ("left.A", a, multiView) == realA);

    assert (channelInAllViews ("A", a, multiView) == realA);

    assert (channelInAllViews ("centre.B", a, multiView) == realB);

    assert (channelInAllViews ("right.jacob", a, multiView) == realJacob);

    assert (channelInAllViews ("castlemilk.moorit.centre.A",
			       a, multiView) == realCm);

    assert (channelInAllViews ("black.welsh.mountain.right.A",
			       a, multiView) == realBwm);

    //
    // Test insertViewName()
    //

    assert (insertViewName ("A", multiView, 0) ==
			    "A");

    assert (insertViewName ("mountain.A", multiView, 0) ==
			    "mountain.right.A");

    assert (insertViewName ("welsh.mountain.A", multiView, 0) ==
			    "welsh.mountain.right.A");

    assert (insertViewName ("black.welsh.mountain.A", multiView, 0) ==
			    "black.welsh.mountain.right.A");

    assert (insertViewName ("A", multiView, 1) ==
			    "left.A");

    assert (insertViewName ("mountain.A", multiView, 1) ==
			    "mountain.left.A");

    assert (insertViewName ("welsh.mountain.A", multiView, 1) ==
			    "welsh.mountain.left.A");

    assert (insertViewName ("black.welsh.mountain.A", multiView, 1) ==
			    "black.welsh.mountain.left.A");
}

} // namespace 


void
testMultiView (const std::string&)
{
    try
    {
	cout << "Testing multi-view channel list functions" << endl;
	testMultiViewFunctions();
	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
