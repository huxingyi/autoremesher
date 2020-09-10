///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2012, Industrial Light & Magic, a division of Lucas
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

#include "testMultiPartSharedAttributes.h"
#include "random.h"

#include <ImfMultiPartInputFile.h>
#include <ImfMultiPartOutputFile.h>
#include <ImfOutputFile.h>
#include <ImfTiledOutputFile.h>
#include <ImfGenericOutputFile.h>
#include <ImfArray.h>
#include <ImfChannelList.h>
#include <ImfOutputPart.h>
#include <ImfInputPart.h>
#include <ImfTiledOutputPart.h>
#include <ImfTiledInputPart.h>
#include <ImfBoxAttribute.h>
#include <ImfChromaticitiesAttribute.h>
#include <ImfTimeCodeAttribute.h>
#include <ImfIntAttribute.h>
#include <ImfPartType.h>
#include <IexBaseExc.h>

#include <iostream>
#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


namespace IMF = OPENEXR_IMF_NAMESPACE;
using namespace IMF;
using namespace std;
using namespace IMATH_NAMESPACE;

#ifndef ILM_IMF_TEST_IMAGEDIR
    #define ILM_IMF_TEST_IMAGEDIR
#endif

namespace
{

const int height = 263;
const int width = 197;


void
generateRandomHeaders (int partCount, vector<Header> & headers)
{
    headers.clear();

    for (int i = 0; i < partCount; i++)
    {
        Header header(width, height);
        int pixelType = random_int(3);
        int partType = random_int(2);

        stringstream ss;
        ss << i;
        header.setName(ss.str());

        switch (pixelType)
        {
            case 0:
                header.channels().insert("UINT", Channel(IMF::UINT));
                break;
            case 1:
                header.channels().insert("FLOAT", Channel(IMF::FLOAT));
                break;
            case 2:
                header.channels().insert("HALF", Channel(IMF::HALF));
                break;
        }

        switch (partType)
        {
            case 0:
                header.setType(IMF::SCANLINEIMAGE);
                break;
            case 1:
                header.setType(IMF::TILEDIMAGE);
                break;
        }

        int tileX;
        int tileY;
        int levelMode;
        if (partType == 1)
        {
            tileX = random_int(width) + 1;
            tileY = random_int(height) + 1;
            levelMode = random_int(3);
            LevelMode lm = NUM_LEVELMODES;
            switch (levelMode)
            {
                case 0:
                    lm = ONE_LEVEL;
                    break;
                case 1:
                    lm = MIPMAP_LEVELS;
                    break;
                case 2:
                    lm = RIPMAP_LEVELS;
                    break;
            }
            header.setTileDescription(TileDescription(tileX, tileY, lm));
        }

        headers.push_back(header);
    }
}

void
testMultiPartOutputFileForExpectedFailure (const vector<Header> & headers,
                                           const std::string & fn,
                                           const string & failMessage="")
{
    bool caught = false;

    try
    {
        remove(fn.c_str());
        MultiPartOutputFile file(fn.c_str(), headers.data() , headers.size() );
        cerr << "ERROR -- " << failMessage << endl;
        assert (false);
    }
    catch (const IEX_NAMESPACE::ArgExc & e)
    {
        // expected behaviour
        caught = true;
    }
    assert (caught);
}


void
testDisplayWindow (const vector<Header> & hs, const std::string & fn)
{
    vector<Header> headers(hs);
    IMATH_NAMESPACE::Box2i newDisplayWindow = headers[0].displayWindow();
    Header newHeader (newDisplayWindow.size().x+10, newDisplayWindow.size().y+10);
    newHeader.setType (headers[0].type());
    newHeader.setName (headers[0].name() + string("_newHeader"));
    headers.push_back (newHeader);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Shared Attributes : displayWindow : should fail for !=values");

    return;
}

void
testPixelAspectRatio (const vector<Header> & hs, const std::string & fn)
{
    vector<Header> headers(hs);

    Header newHeader (headers[0].displayWindow().size().x+1,
                      headers[0].displayWindow().size().y+1,
                      headers[0].pixelAspectRatio() + 1.f);
    newHeader.setType (headers[0].type());
    newHeader.setName (headers[0].name() + string("_newHeader"));
    headers.push_back (newHeader);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Shared Attributes : pixelAspecRatio : should fail for !=values");

    return;
}

void
testTimeCode (const vector<Header> & hs, const std::string & fn)
{
    vector<Header> headers(hs);

    Header newHeader (headers[0]);
    newHeader.setName (headers[0].name() + string("_newHeader"));


    //
    // Test against a vector of headers that has no attributes of this type
    //
    TimeCode t(1234567);
    TimeCodeAttribute ta(t);
    newHeader.insert(TimeCodeAttribute::staticTypeName(), ta);
    headers.push_back (newHeader);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Shared Attributes : timecode : should fail for !presence");


    //
    // Test against a vector of headers that has chromaticities attribute
    // but of differing value
    //
    for (size_t i=0; i<headers.size(); i++)
        headers[i].insert (TimeCodeAttribute::staticTypeName(), ta);

    t.setTimeAndFlags (t.timeAndFlags()+1);
    TimeCodeAttribute tta(t);
    newHeader.insert (TimeCodeAttribute::staticTypeName(), tta);
    newHeader.setName (newHeader.name() + string("_+1"));
    headers.push_back (newHeader);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Shared Attributes : timecode : should fail for != values");

    return;
}

void
testChromaticities (const vector<Header> & hs, const std::string & fn)
{
    vector<Header> headers(hs);

    Header newHeader (headers[0]);
    newHeader.setName (headers[0].name() + string("_newHeader"));

    Chromaticities c;
    ChromaticitiesAttribute ca(c);
    newHeader.insert (ChromaticitiesAttribute::staticTypeName(), ca);

    //
    // Test against a vector of headers that has no attributes of this type
    //
    headers.push_back (newHeader);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Shared Attributes : chromaticities : should fail for !present");


    //
    // Test against a vector of headers that has chromaticities attribute
    // but of differing value
    //
    for (size_t i=0; i<headers.size(); i++)
        headers[i].insert (ChromaticitiesAttribute::staticTypeName(), ca);

    c.red += IMATH_NAMESPACE::V2f (1.0f, 1.0f);
    ChromaticitiesAttribute cca(c);
    newHeader.insert (ChromaticitiesAttribute::staticTypeName(), cca);
    headers.push_back (newHeader);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Shared Attributes : chromaticities : should fail for != values");

    return;
}


void
testSharedAttributes (const std::string & fn)
{
    //
    // The Shared Attributes are currently:
    // Display Window
    // Pixel Aspect Ratio
    // TimeCode
    // Chromaticities
    //

    int partCount = 3;
    vector<Header> headers;


    // This will generate headers that are valid for all parts
    generateRandomHeaders (partCount, headers);

    // expect this to be successful
    {
        remove(fn.c_str());
        MultiPartOutputFile file(fn.c_str(), &headers[0],headers.size());
    }

    // Adding a header a that has non-complient standard attributes will throw
    // an exception.

    // Run the tests
    testDisplayWindow    (headers, fn);
    testPixelAspectRatio (headers, fn);
    testTimeCode         (headers, fn);
    testChromaticities   (headers, fn);
}


template <class T>
void
testDiskAttrValue (const Header & diskHeader, const T & testAttribute)
{
    const string & attrName = testAttribute.typeName();
    const T & diskAttr = dynamic_cast <const T &> (diskHeader[attrName]);
    if (diskAttr.value() != testAttribute.value())
    {
        throw IEX_NAMESPACE::InputExc ("incorrect value from disk");
    }

    return;
}


void
testHeaders (const std::string & fn)
{
    //
    // In a multipart context the headers must be subject to the following
    // constraints
    // * type must be set and valid
    // * unique names
    //


    vector<Header> headers;

    //
    // expect this to fail - empty header list
    //
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Header : empty header list passed");


    //
    // expect this to fail - header has no image attribute type
    //
    Header h;
    headers.push_back (h);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Header : empty image type passed");


    //
    // expect this to fail - header name duplication
    //
    headers[0].setType (IMF::SCANLINEIMAGE);
    Header hh(headers[0]);
    headers.push_back(hh);
    testMultiPartOutputFileForExpectedFailure (headers,
                                               fn,
                                               "Header: duplicate header names passed");


    //
    // expect this to fail - header has incorrect image attribute type
    //
    bool caught = false;
    try
    {
        headers[0].setType ("invalid image type");
        cerr << "Header : unsupported image type passed" << endl;
        assert (false);
    }
    catch (const IEX_NAMESPACE::ArgExc & e)
    {
        // expected behaviour
        caught = true;
    }
    assert (caught);


    //
    // Write and Read the data off disk and check values
    //
    TimeCode t(1234567);
    TimeCodeAttribute ta(t);
    Chromaticities c;
    ChromaticitiesAttribute ca(c);
    vector<IntAttribute> ia;
    for (size_t i=0; i<headers.size(); i++)
    {
        stringstream ss;
        ss << i;
        headers[i].setName (ss.str());
        headers[i].setType (IMF::SCANLINEIMAGE);
        headers[i].insert(TimeCodeAttribute::staticTypeName(), ta);
        headers[i].insert(ChromaticitiesAttribute::staticTypeName(), ca);

        IntAttribute ta(i);
        ia.push_back(ta);
        headers[i].insert(IntAttribute::staticTypeName(), ta);
    }


    // write out the file
    remove(fn.c_str());
    {
        MultiPartOutputFile file(fn.c_str(), &headers[0],headers.size());
    }


    // read in the file and look at the attribute data
    MultiPartInputFile file (fn.c_str());

    assert (file.parts() == 2);

    for (int i=0; i<file.parts(); i++)
    {
        const Header & diskHeader = file.header(i);
        //
        // Test Display Window
        //
        const IMATH_NAMESPACE::Box2i & diskDispWin =     diskHeader.displayWindow();
        const IMATH_NAMESPACE::Box2i & testDispWin =     headers[i].displayWindow();
        assert (diskDispWin == testDispWin);

        //
        // Test Pixel Aspect Ratio
        //
        float diskPAR = diskHeader.pixelAspectRatio();
        float testPAR =     headers[i].pixelAspectRatio();
        assert (diskPAR == testPAR);

        //
        // Test TimeCode
        //
        try
        {
            testDiskAttrValue<TimeCodeAttribute> (diskHeader, ta);
        }
        catch (const IEX_NAMESPACE::InputExc &e)
        {
            cerr << "Shared Attributes : TimeCode : " << e.what() << endl;
            assert (false);
        }

        //
        // Test Chromaticities
        //
        try
        {
            testDiskAttrValue<ChromaticitiesAttribute> (diskHeader, ca);
        }
        catch (const IEX_NAMESPACE::InputExc &e)
        {
            cerr << "Shared Attributes : Chromaticities : " << e.what() << endl;
            assert (false);
        }

        //
        // Test for non-shared attribute that can have differing values across
        // multiple parts
        //
        try
        {
            testDiskAttrValue<IntAttribute> (diskHeader, ia[i]);
        }
        catch (const IEX_NAMESPACE::InputExc &e)
        {
            cerr <<  "Shared Attributes : IntAttribute : " << e.what() << endl;
            assert (false);
        }
    }


    //
    // Test the code against an incorrectly constructed file
    //
    try
    {
        caught = false;
        MultiPartInputFile file (ILM_IMF_TEST_IMAGEDIR "invalid_shared_attrs_multipart.exr");
        cerr << "Shared Attributes : InputFile : incorrect input file passed\n";
        assert (false);
    }
    catch (const IEX_NAMESPACE::InputExc &e)
    {
        // expectected behaviour
        caught = true;
    }
    assert (caught);
}


} // namespace

void
testMultiPartSharedAttributes (const std::string & tempDir)
{
    try
    {
        cout << "Testing multi part APIs : shared attributes, header... " << endl;

        random_reseed(1);

        std::string fn = tempDir +  "imf_test_multipart_shared_attrs.exr";

        testSharedAttributes (fn);
        testHeaders (fn);

        cout << " ... ok\n" << endl;
    }
    catch (const IEX_NAMESPACE::BaseExc & e)
    {
        cerr << "ERROR -- caught IEX_NAMESPACE::BaseExc exception: " << e.what() << endl;
        assert (false);
    }
    catch (const std::exception & e)
    {
        cerr << "ERROR -- caught std::exception exception: " << e.what() << endl;
        assert (false);
    }
}
