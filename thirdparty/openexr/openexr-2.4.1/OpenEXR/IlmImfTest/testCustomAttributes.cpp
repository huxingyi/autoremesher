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

#include <ImfOutputFile.h>
#include <ImfInputFile.h>
#include <ImfChannelList.h>
#include <ImfAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfOpaqueAttribute.h>
#include <ImfArray.h>
#include <half.h>

#include <stdio.h>
#include <assert.h>


namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;


//
// Definition of the custom GlorpAttribute type.
// Note that this must be in the same namespace as the definition
//

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_ENTER

struct Glorp
{
    int a, b;
    Glorp (int a = 0, int b = 0): a(a), b(b) {}
};


typedef TypedAttribute<Glorp> GlorpAttribute;
template <> const char *GlorpAttribute::staticTypeName ();
template <> void GlorpAttribute::writeValueTo (OStream &, int) const;
template <> void GlorpAttribute::readValueFrom (IStream &, int, int);

template <>
const char *
GlorpAttribute::staticTypeName ()
{
    return "glorp";
}


template <>
void
GlorpAttribute::writeValueTo (OStream &os, int version) const
{
    Xdr::write <StreamIO> (os, _value.a);
    Xdr::write <StreamIO> (os, _value.b);
}


template <>
void
GlorpAttribute::readValueFrom (IStream &is, int size, int version)
{
    Xdr::read <StreamIO> (is, _value.a);
    Xdr::read <StreamIO> (is, _value.b);
}

OPENEXR_IMF_INTERNAL_NAMESPACE_SOURCE_EXIT


namespace {


void
fillPixels (Array2D<float> &pf, int width, int height)
{
    for (int y = 0; y < height; ++y)
	for (int x = 0; x < width; ++x)
	    pf[y][x] = x % 10 + 10 * (y % 17);
}


void
writeReadCustomAttr (const Array2D<float> &pf1,
		     const char fileName[],
		     int width,
		     int height)
{
    //
    // Tell the image file library about the custom GlorpAttribute type.
    //

    GlorpAttribute::registerAttributeType();

    //
    // Write an image file with two custom attributes:
    // "custom1", of type IntAttribute, and
    // "custom2", of type GlorpAttribute.
    //

    {
	Header hdr (width, height);

	hdr.insert ("custom1", IntAttribute (25));
	hdr.insert ("custom2", GlorpAttribute (Glorp (4, 9)));

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
    // Tell the image file library to forget the GlorpAttribute type.
    //

    GlorpAttribute::unRegisterAttributeType();

    //
    // Read the header back from the file:
    // "custom1" should come back as an IntAttribute, and
    // "custom2" should come back as an OpaqueAttribute.
    //

    Header hdr;

    {
	cout << " reading" << flush;

	InputFile in (fileName);

	const IntAttribute *c1;
	const OpaqueAttribute *c2;
	
	c1 = in.header().findTypedAttribute <IntAttribute>    ("custom1");
	c2 = in.header().findTypedAttribute <OpaqueAttribute> ("custom2");

	assert (c1 != 0 && c1->value() == 25);
	assert (c2 != 0);

	hdr = in.header();
    }

    //
    // Tell the image file library to remember the custom GlorpAttribute type.
    //

    GlorpAttribute::registerAttributeType();

    //
    // Write a new image file using the header read back from
    // the previous file file.  The two custom attributes should
    // be stored in the new file.
    //

    {
	FrameBuffer fb; 

	fb.insert ("F",					// name
		   Slice (IMF::FLOAT,			// type
			  (char *) &pf1[0][0],		// base
			  sizeof (pf1[0][0]), 		// xStride
			  sizeof (pf1[0][0]) * width,	// yStride
			  1,				// xSampling
			  1)				// ySampling
		  );

	cout << " writing" << flush;

	remove (fileName);
	OutputFile out (fileName, hdr);
	out.setFrameBuffer (fb);
	out.writePixels (height);
    }

    //
    // Read the header of the new image file:
    // "custom1" should come back as an IntAttribute, and
    // "custom2" should come back as a GlorpAttribute.
    //

    {
	cout << " reading" << flush;

	InputFile in (fileName);

	const IntAttribute *c1;
	const GlorpAttribute *c2;
	
	c1 = in.header().findTypedAttribute <IntAttribute>   ("custom1");
	c2 = in.header().findTypedAttribute <GlorpAttribute> ("custom2");

	assert (c1 != 0 && c1->value() == 25);
	assert (c2 != 0 && c2->value().a == 4 && c2->value().b == 9);
    }

    remove (fileName);
    cout << endl;
}


} // namespace


void
testCustomAttributes (const std::string &tempDir)
{
    try
    {
	cout << "Testing custom attributes" << endl;

	const int W = 217;
	const int H = 197;

	Array2D<float> pf (H, W);
	fillPixels (pf, W, H);

	std::string filename = tempDir + "imf_test_custom_attr.exr";

	writeReadCustomAttr (pf, filename.c_str(), W, H);

	cout << "ok\n" << endl;
    }
    catch (const std::exception &e)
    {
	cerr << "ERROR -- caught exception: " << e.what() << endl;
	assert (false);
    }
}
