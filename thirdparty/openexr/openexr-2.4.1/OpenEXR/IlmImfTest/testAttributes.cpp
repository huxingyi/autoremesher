///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2004-2012, Industrial Light & Magic, a division of Lucas
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

#ifdef NDEBUG
#    undef NDEBUG
#endif

#include <tmpDir.h>

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfArray.h>
#include <ImfVersion.h>
#include <half.h>

#include <ImfBoxAttribute.h>
#include <ImfChannelListAttribute.h>
#include <ImfCompressionAttribute.h>
#include <ImfChromaticitiesAttribute.h>
#include <ImfFloatAttribute.h>
#include <ImfFloatVectorAttribute.h>
#include <ImfEnvmapAttribute.h>
#include <ImfDeepImageStateAttribute.h>
#include <ImfDoubleAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfLineOrderAttribute.h>
#include <ImfMatrixAttribute.h>
#include <ImfOpaqueAttribute.h>
#include <ImfStringAttribute.h>
#include <ImfStringVectorAttribute.h>
#include <ImfVecAttribute.h>

#include <stdio.h>
#include <assert.h>


namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;

using namespace std;
using namespace IMATH_NAMESPACE;


OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_ENTER

class TestOpaque 
{
   public:
    TestOpaque () : magic (666) {}
    TestOpaque (int m) : magic (m) {}
    int  magic;
};

bool
operator == (const TestOpaque& a, const TestOpaque& b)
{
    return a.magic == b.magic;
}

typedef TypedAttribute<TestOpaque> TestOpaqueAttribute;

template <>
const char *
TestOpaqueAttribute::staticTypeName ()
{
    return "testOpaque";
}
    
template <>
void
TestOpaqueAttribute::writeValueTo (OPENEXR_IMF_INTERNAL_NAMESPACE::OStream& os, int version) const
{
    Xdr::write <StreamIO> (os, _value.magic);
}
                              
template <>
void
TestOpaqueAttribute::readValueFrom (OPENEXR_IMF_INTERNAL_NAMESPACE::IStream& is, int size, int version)
{
    Xdr::read <StreamIO> (is, _value.magic);
}

OPENEXR_IMF_INTERNAL_NAMESPACE_HEADER_EXIT

namespace {

void
fillPixels (Array2D<float> &pf, int width, int height)
{
    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	    pf[y][x] = x % 10 + 10 * (y % 17);
}


void
writeReadAttr (const Array2D<float> &pf1,
	       const char fileName[],
	       int width,
	       int height)
{
    TestOpaqueAttribute::registerAttributeType();
    
    //
    // We don't test ChannelList, LineOrder, Compression and opaque
    // attributes here; those types are covered by other tests.
    //

    Box2i  a1  (V2i (1, 2), V2i (3, 4));
    Box2f  a2  (V2f (1.5, 2.5), V2f (3.5, 4.5));
    float  a3  (3.14159);
    int    a4  (17);
    M33f   a5  (11, 12, 13, 14, 15, 16, 17, 18, 19);
    M44f   a6  (1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16);
    string a7  ("extensive rebuilding by Nebuchadrezzar has left");
    V2i    a8  (27, 28);
    V2f    a9  (27.5, 28.5);
    V3i    a10 (37, 38, 39);
    V3f    a11 (37.5, 38.5, 39.5);
    double a12 (7.12342341419);
    Chromaticities a13 (V2f (1, 2), V2f (3, 4), V2f (5, 6), V2f (7, 8));
    Envmap a14 (ENVMAP_CUBE);

    StringVector a15;
    a15.push_back ("who can spin");
    a15.push_back ("");
    a15.push_back ("straw into");
    a15.push_back ("gold");
    M33d   a16 (12, 13, 14, 15, 16, 17, 18, 19, 20);
    M44d   a17 (2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17);
    V2d    a18 (27.51, 28.51);
    V3d    a19 (37.51, 38.51, 39.51);
    
    StringVector a20;

    DeepImageState a21 (DIS_TIDY);

    FloatVector a22;
    FloatVector a23;
    a23.push_back (1.5f);
    a23.push_back (-1.5f);
    a23.push_back (15.0f);
    a23.push_back (150.0f);

    TestOpaque  a24 (42);

    //
    // Write an image file with extra attributes in the header
    //

    {
	Header hdr (width, height);

	hdr.insert ("a1",  Box2iAttribute          (a1));
	hdr.insert ("a2",  Box2fAttribute          (a2));
	hdr.insert ("a3",  FloatAttribute          (a3));
	hdr.insert ("a4",  IntAttribute            (a4));
	hdr.insert ("a5",  M33fAttribute           (a5));
	hdr.insert ("a6",  M44fAttribute           (a6));
	hdr.insert ("a7",  StringAttribute         (a7));
	hdr.insert ("a8",  V2iAttribute            (a8));
	hdr.insert ("a9",  V2fAttribute            (a9));
	hdr.insert ("a10", V3iAttribute            (a10));
	hdr.insert ("a11", V3fAttribute            (a11));
	hdr.insert ("a12", DoubleAttribute         (a12));
	hdr.insert ("a13", ChromaticitiesAttribute (a13));
	hdr.insert ("a14", EnvmapAttribute         (a14));
	hdr.insert ("a15", StringVectorAttribute   (a15));
	hdr.insert ("a16", M33dAttribute           (a16));
	hdr.insert ("a17", M44dAttribute           (a17));
	hdr.insert ("a18", V2dAttribute            (a18));
	hdr.insert ("a19", V3dAttribute            (a19));
	hdr.insert ("a20", StringVectorAttribute   (a20));
	hdr.insert ("a21", DeepImageStateAttribute (a21));
	hdr.insert ("a22", FloatVectorAttribute    (a22));
	hdr.insert ("a23", FloatVectorAttribute    (a23));
        hdr.insert ("a24", TestOpaqueAttribute          (a24));
        
	hdr.channels().insert ("F",			// name
			       Channel (IMF::FLOAT,	// type
					1,		// xSampling
					1)		// ySampling
			      );

	FrameBuffer fb; 

	fb.insert ("F",					// name
		   Slice (IMF::FLOAT,			// type
			  (char *) &pf1[0][0],		// base
			  sizeof (pf1[0][0]), 		// xStride
			  sizeof (pf1[0][0]) * width,	// yStride
			  1,				// xSampling
			  1)				// ySampling
		  );

	cout << "writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    //
    // Test the handling of "opaque" attributes that preserve
    // attributes whose type is not recognized; remove the definition
    // of the "testOpaque" type now that the file has been written.
    //

    TestOpaqueAttribute::unRegisterAttributeType();
    
    //
    // Read the header back from the file, and see if the
    // values of the extra attributes come back correctly.
    //

    {
	cout << " reading" << flush;

	InputFile in (fileName);

	cout << " (version " << in.version() << ")" << flush;

	const Header &hdr = in.header();

	assert (hdr.typedAttribute <Box2iAttribute>  ("a1").value()  == a1);
	assert (hdr.typedAttribute <Box2fAttribute>  ("a2").value()  == a2);
	assert (hdr.typedAttribute <FloatAttribute>  ("a3").value()  == a3);
	assert (hdr.typedAttribute <IntAttribute>    ("a4").value()  == a4);
	assert (hdr.typedAttribute <M33fAttribute>   ("a5").value()  == a5);
	assert (hdr.typedAttribute <M44fAttribute>   ("a6").value()  == a6);
	assert (hdr.typedAttribute <StringAttribute> ("a7").value()  == a7);
	assert (hdr.typedAttribute <V2iAttribute>    ("a8").value()  == a8);
	assert (hdr.typedAttribute <V2fAttribute>    ("a9").value()  == a9);
	assert (hdr.typedAttribute <V3iAttribute>    ("a10").value() == a10);
	assert (hdr.typedAttribute <V3fAttribute>    ("a11").value() == a11);
	assert (hdr.typedAttribute <DoubleAttribute> ("a12").value() == a12);

	assert (hdr.typedAttribute <ChromaticitiesAttribute>
					("a13").value().red == a13.red);

	assert (hdr.typedAttribute <ChromaticitiesAttribute>
					("a13").value().green == a13.green);

	assert (hdr.typedAttribute <ChromaticitiesAttribute>
					("a13").value().blue == a13.blue);

	assert (hdr.typedAttribute <ChromaticitiesAttribute>
					("a13").value().white == a13.white);

	assert (hdr.typedAttribute <EnvmapAttribute> ("a14").value() == a14);

	assert (hdr.typedAttribute <StringVectorAttribute>
					("a15").value().size() == 4);

	assert (hdr.typedAttribute <StringVectorAttribute>
					("a15").value()[0] == "who can spin");

	assert (hdr.typedAttribute <StringVectorAttribute>
					("a15").value()[1] == "");

	assert (hdr.typedAttribute <StringVectorAttribute>
					("a15").value()[2] == "straw into");

	assert (hdr.typedAttribute <StringVectorAttribute>
					("a15").value()[3] == "gold");

	assert (hdr.typedAttribute <M33dAttribute>   ("a16").value()  == a16);
	assert (hdr.typedAttribute <M44dAttribute>   ("a17").value()  == a17);
	assert (hdr.typedAttribute <V2dAttribute>    ("a18").value()  == a18);
	assert (hdr.typedAttribute <V3dAttribute>    ("a19").value()  == a19);

        assert (hdr.typedAttribute <StringVectorAttribute>
                                        ("a20").value() == a20);

	assert (hdr.typedAttribute <DeepImageStateAttribute>
					("a21").value() == a21);

        assert (hdr.typedAttribute <FloatVectorAttribute>
                                        ("a22").value() == a22);

        assert (hdr.typedAttribute <FloatVectorAttribute>
                                        ("a23").value() == a23);

        //
        // a24 should be an opaque attribute with type "testOpaque".
        //

        const Attribute& a = hdr["a24"];
        const OpaqueAttribute* oa = dynamic_cast <const OpaqueAttribute*> (&a);
        assert (oa);
        assert (!strcmp (a.typeName(), "testOpaque"));

        // test the copy constructor
        OpaqueAttribute b (*oa);
        assert (!strcmp (b.typeName(), a.typeName()));
        assert (b.dataSize() == oa->dataSize());
        for (int i=0; i<b.dataSize(); i++)
            assert (b.data()[i] == oa->data()[i]);
    }

    remove (fileName);
    cout << endl;
}


void
channelList ()
{
    cout << "channel list" << endl;

    {
	// test channelsWithPrefix()

	ChannelList channels;

	channels.insert ("b0", Channel (IMF::HALF, 1, 1));
	channels.insert ("b1", Channel (IMF::HALF, 1, 1));
	channels.insert ("b2", Channel (IMF::HALF, 1, 1));
	channels.insert ("d3", Channel (IMF::HALF, 1, 1));
	channels.insert ("e4", Channel (IMF::HALF, 1, 1));

	ChannelList::Iterator first;
	ChannelList::Iterator last;

	channels.channelsWithPrefix ("a", first, last);
	assert (first != channels.end());
	assert (first == last);

	channels.channelsWithPrefix ("b", first, last);
	assert (first != channels.end());
	assert (first != last);
	assert (first++.name() == Name ("b0"));
	assert (first++.name() == Name ("b1"));
	assert (first++.name() == Name ("b2"));
	assert (first == last);

	channels.channelsWithPrefix ("b1", first, last);
	assert (first != channels.end());
	assert (first != last);
	assert (first++.name() == Name ("b1"));
	assert (first == last);

	channels.channelsWithPrefix ("b11", first, last);
	assert (first != channels.end());
	assert (first == last);

	channels.channelsWithPrefix ("c", first, last);
	assert (first != channels.end());
	assert (first == last);

	channels.channelsWithPrefix ("d", first, last);
	assert (first != channels.end());
	assert (first != last);
	assert (first++.name() == Name ("d3"));
	assert (first == last);

	channels.channelsWithPrefix ("e", first, last);
	assert (first != channels.end());
	assert (first != last);
	assert (first++.name() == Name ("e4"));
	assert (first == last);

	channels.channelsWithPrefix ("f", first, last);
	assert (first == channels.end());
	assert (first == last);
    }

    {
	// Test support for layers
	
	ChannelList channels;

	channels.insert ("a",   Channel (IMF::HALF, 1, 1));
	channels.insert (".a",  Channel (IMF::HALF, 1, 1));
	channels.insert ("a.",  Channel (IMF::HALF, 1, 1));

	channels.insert ("layer1.R", Channel (IMF::HALF, 1, 1));
	channels.insert ("layer1.G", Channel (IMF::HALF, 1, 1));
	channels.insert ("layer1.B", Channel (IMF::HALF, 1, 1));

	channels.insert ("layer1.sublayer1.AA", Channel (IMF::HALF, 1, 1));
	channels.insert ("layer1.sublayer1.R", Channel  (IMF::HALF, 1, 1));
	channels.insert ("layer1.sublayer1.G", Channel  (IMF::HALF, 1, 1));
	channels.insert ("layer1.sublayer1.B", Channel  (IMF::HALF, 1, 1));

	channels.insert ("layer1.sublayer2.R", Channel (IMF::HALF, 1, 1));

	channels.insert ("layer2.R", Channel (IMF::HALF, 1, 1));
	channels.insert ("layer2.G", Channel (IMF::HALF, 1, 1));
	channels.insert ("layer2.B", Channel (IMF::HALF, 1, 1));

	set <string> layerNames;
	channels.layers (layerNames);

	set<string>::iterator i = layerNames.begin();
	assert (*i++ == "layer1");
	assert (*i++ == "layer1.sublayer1");
	assert (*i++ == "layer1.sublayer2");
	assert (*i++ == "layer2");
	assert (i == layerNames.end());

	ChannelList::ConstIterator first, last;

	channels.channelsInLayer ("layer1.sublayer1", first, last);
	assert (first != channels.end());
	assert (first != last);
	assert (first++.name() == Name ("layer1.sublayer1.AA"));
	assert (first++.name() == Name ("layer1.sublayer1.B"));
	assert (first++.name() == Name ("layer1.sublayer1.G"));
	assert (first++.name() == Name ("layer1.sublayer1.R"));
	assert (first == last);

	channels.channelsInLayer ("layer2", first, last);
	assert (first != channels.end());
	assert (first != last);
	assert (first++.name() == Name ("layer2.B"));
	assert (first++.name() == Name ("layer2.G"));
	assert (first++.name() == Name ("layer2.R"));
	assert (first == last);
    }
}


void
longNames (const Array2D<float> &pf1,
           const char fileName[],
           int width,
           int height)
{
    //
    // Verify that long attibute or channel names in the header
    // set the LONG_NAMES_FLAG in the file version number.
    //

    FrameBuffer fb; 

    fb.insert ("F",					// name
               Slice (IMF::FLOAT,			// type
                      (char *) &pf1[0][0],		// base
                      sizeof (pf1[0][0]), 		// xStride
                      sizeof (pf1[0][0]) * width,	// yStride
                      1,				// xSampling
                      1)				// ySampling
              );

    cout << "only short names" << endl;

    {
	Header hdr (width, height);

	hdr.channels().insert ("F",			// name
			       Channel (IMF::FLOAT,	// type
					1,		// xSampling
					1)		// ySampling
			      );


	cout << "writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    {
	cout << " reading" << endl;

	InputFile in (fileName);
        assert (!(in.version() & LONG_NAMES_FLAG));
    }

    static const char longName[] = "x2345678901234567890123456789012";

    cout << "long attribute name" << endl;

    {
	Header hdr (width, height);
	hdr.insert (longName, StringAttribute ("y"));

	hdr.channels().insert ("F",			// name
			       Channel (IMF::FLOAT,	// type
					1,		// xSampling
					1)		// ySampling
			      );

	cout << "writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    {
	cout << " reading" << endl;

	InputFile in (fileName);
        assert (in.version() & LONG_NAMES_FLAG);

	const Header &hdr = in.header();
	assert (hdr.typedAttribute <StringAttribute> (longName).value() == "y");
    }

    cout << "long channel name" << endl;

    {
	Header hdr (width, height);

	hdr.channels().insert (longName,		// name
			       Channel (IMF::FLOAT,	// type
					1,		// xSampling
					1)		// ySampling
			      );

	cout << "writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    {
	cout << " reading" << endl;

	InputFile in (fileName);
        assert (in.version() & LONG_NAMES_FLAG);

	const Header &hdr = in.header();
	assert (hdr.channels().findChannel (longName));
    }

    remove (fileName);
    cout << endl;
}


} // namespace


template<class T> void 
print_type(const OPENEXR_IMF_NAMESPACE::TypedAttribute<T> & object)
{
    cout << object.typeName() << endl;
}

void
testAttributes (const std::string &tempDir)
{
    try
    {
	cout << "Testing built-in attributes" << endl;

	const int W = 217;
	const int H = 197;

	Array2D<float> pf (H, W);
	fillPixels (pf, W, H);

	std::string filename = tempDir + "imf_test_attr.exr";

	writeReadAttr (pf, filename.c_str(), W, H);
	channelList();
        longNames(pf, filename.c_str(), W, H);

        print_type(OPENEXR_IMF_NAMESPACE::TypedAttribute<int>());

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
