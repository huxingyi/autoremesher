///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2007, Industrial Light & Magic, a division of Lucas
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


//-----------------------------------------------------------------------------
//
//	exrmultiview -- a program that combines multiple
//	single-view OpenEXR image files into a single
//	multi-view image file.
//
//-----------------------------------------------------------------------------

#include <makeMultiView.h>

#include <iostream>
#include <exception>
#include <string>
#include <string.h>
#include <stdlib.h>

#include "namespaceAlias.h"
using namespace IMF;
using namespace std;



namespace {

void
usageMessage (const char argv0[], bool verbose = false)
{
    cerr << "usage: " << argv0 << " "
	    "[options] viewname1 infile1 viewname2 infile2 ... outfile" << endl;

    if (verbose)
    {
	cerr << "\n"
		"Combines two or more single-view OpenEXR image files into\n"
		"a single multi-view image file.  On the command line,\n"
		"each single-view input image is specified together with\n"
		"a corresponding view name.  The first view on the command\n"
		"line becomes the default view.  Example:\n"
		"\n"
		"   " << argv0 << " left imgL.exr right imgR.exr imgLR.exr\n"
		"\n"
		"Here, imgL.exr and imgR.exr become the left and right\n"
		"views in output file imgLR.exr.  The left view becomes\n"
		"the default view.\n"
		"\n"
		"Options:\n"
		"\n"
		"-z x      sets the data compression method to x\n"
		"          (none/rle/zip/piz/pxr24/b44/b44a/dwaa/dwab,\n"
		"          default is piz)\n"
		"\n"
		"-v        verbose mode\n"
		"\n"
		"-h        prints this message\n";

	 cerr << endl;
    }

    exit (1);
}


Compression
getCompression (const string &str)
{
    Compression c;

    if (str == "no" || str == "none" || str == "NO" || str == "NONE")
    {
	c = NO_COMPRESSION;
    }
    else if (str == "rle" || str == "RLE")
    {
	c = RLE_COMPRESSION;
    }
    else if (str == "zip" || str == "ZIP")
    {
	c = ZIP_COMPRESSION;
    }
    else if (str == "piz" || str == "PIZ")
    {
	c = PIZ_COMPRESSION;
    }
    else if (str == "pxr24" || str == "PXR24")
    {
	c = PXR24_COMPRESSION;
    }
    else if (str == "b44" || str == "B44")
    {
	c = B44_COMPRESSION;
    }
    else if (str == "b44a" || str == "B44A")
    {
	c = B44A_COMPRESSION;
    }
    else if (str == "dwaa" || str == "DWAA")
    {
	c = DWAA_COMPRESSION;
    }
    else if (str == "dwab" || str == "DWAB")
    {
	c = DWAB_COMPRESSION;
    }
    else
    {
	cerr << "Unknown compression method \"" << str << "\"." << endl;
	exit (1);
    }

    return c;
}

} // namespace


int
main(int argc, char **argv)
{
    vector <string> views;
    vector <const char *> inFiles;
    const char *outFile = 0;
    Compression compression = PIZ_COMPRESSION;
    bool verbose = false;

    //
    // Parse the command line.
    //

    if (argc < 2)
	usageMessage (argv[0], true);

    int i = 1;

    while (i < argc)
    {
	if (!strcmp (argv[i], "-z"))
	{
	    //
	    // Set compression method
	    //

	    if (i > argc - 2)
		usageMessage (argv[0]);

	    compression = getCompression (argv[i + 1]);
	    i += 2;
	}
	else if (!strcmp (argv[i], "-v"))
	{
	    //
	    // Verbose mode
	    //

	    verbose = true;
	    i += 1;
	}
	else if (!strcmp (argv[i], "-h"))
	{
	    //
	    // Print help message
	    //

	    usageMessage (argv[0], true);
	}
	else
	{
	    //
	    // View or image file name
	    //

	    if (i > argc - 2 || argv[i + 1][0] == '-')
	    {
		//
		// Output file
		//

		if (outFile)
		    usageMessage (argv[0]);

		outFile = argv[i];
		i += 1;
	    }
	    else
	    {
	    	//
		// View plus input file
		//

		views.push_back (argv[i]);
		inFiles.push_back (argv[i + 1]);
		i += 2;
	    }
	}
    }

    if (views.size() < 2)
    {
	cerr << "Must specify at least two views." << endl;
	return 1;
    }

    if (outFile == 0)
    {
	cerr << "Must specify an output file." << endl;
	return 1;
    }

    //
    // Load inFiles, and save a combined multi-view image in outFile.
    //

    int exitStatus = 0;

    try
    {
	makeMultiView (views, inFiles, outFile, compression, verbose);
    }
    catch (const exception &e)
    {
	cerr << e.what() << endl;
	exitStatus = 1;
    }

    return exitStatus;
}
