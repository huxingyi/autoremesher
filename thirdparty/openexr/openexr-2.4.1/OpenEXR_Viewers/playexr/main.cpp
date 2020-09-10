///////////////////////////////////////////////////////////////////////////
//
// Copyright (c) 2006, Industrial Light & Magic, a division of Lucas
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
//	playexr -- a program that plays back an
//	OpenEXR image sequence directly from disk.
//
//-----------------------------------------------------------------------------

#include "playExr.h"
#include "osDependent.h"
#include <IlmThread.h>

#include <iostream>
#include <exception>
#include <vector>
#include <string>
#include <stdlib.h>
#include <string.h>
#ifndef _WIN32
# include <unistd.h>
#endif
#include <cstring>

using namespace std;

namespace {

void
usageMessage (const char argv0[], bool verbose = false)
{
    cerr << "usage: " << argv0 << " "
	    "[options] fileName [firstFrame lastFrame]" << endl;

    if (verbose)
    {
	cerr << "\n"
		"Plays back a sequence of OpenEXR files.  All files must\n"
		"have the same data window and the same set of channels.\n"
		"The names of the files are constructed by substituting\n"
		"the first '%' in fileName with firstFrame, firstFrame+1,\n"
		"firstFrame+2, ... lastFrame.  For example,\n"
		"\n"
		"       " << argv0 << " image.%.exr 1 100\n"
		"\n"
		"plays back image.1.exr, image.2.exr ... image.100.exr.\n"
		"\n"
		"Options:\n"
		"\n"
		"-t n   read the images using n parallel threads\n"
		"\n"
		"-f n   images will be played back at a rate of n frames\n"
		"       per second (assuming that reading and displaying\n"
		"       an individual image file takes no more than 1/n\n"
		"       seconds).\n"
		"\n"
		"-S n   images will be displayed at n times their original\n"
		"       width and height.  n must be in the range from 0.1\n"
		"       to 2.0.\n"
		"\n"
	    #if HAVE_CTL_INTERPRETER
		"-C s   CTL transform s is applied to each image before it\n"
		"       is displayed.  Option -C can be specified multiple\n"
		"       times to apply a series of transforms to each image.\n"
		"       The transforms are applied in the order in which\n"
		"       they appear on the command line.\n"
		"\n"
		"-i     On machines where the graphics hardware does not\n"
		"       directly support interpolation between texture map\n"
		"       pixels images with smooth color gradients will\n"
		"       exhibit contouring artifacts.  Option -i selects\n"
		"       software-based texture pixel interpolation.  This\n"
		"       avoids contouring but may slow down image playback.\n"
		"\n"
	    #endif
		"-h     prints this message\n"
		"\n"
	    #if HAVE_CTL_INTERPRETER
		"CTL transforms:\n"
		"\n"
		"       If one or more CTL transforms are specified on\n"
		"       the command line (using the -C flag), then those\n"
		"       transforms are applied to the images.\n"
		"       If no CTL transforms are specified on the command\n"
		"       line then an optional look modification transform\n"
		"       is applied, followed by a rendering transform and\n"
		"       a display transform.\n"
		"       The name of the look modification transform is\n"
		"       taken from the lookModTransform attribute in the\n"
		"       header of the first frame of the image sequence.\n"
		"       If the header contains no such attribute, then no\n"
		"       look modification transform is applied.  The name\n"
		"       of the rendering transform is taken from the\n"
		"       renderingTransform attribute in the header of the\n"
		"       first frame of the image sequence.  If the header\n"
		"       contains no such attribute, then the name of the\n"
		"       rendering transform is \"transform_RRT.\"  The\n"
		"       name of the display transform is taken from the\n"
		"       environment variable CTL_DISPLAY_TRANSFORM.  If this\n"
		"       environment variable is not set, then the name of\n"
		"       the display transform is \"transform_display_video.\"\n"
		"       The files that contain the CTL code for the\n"
		"       transforms are located using the CTL_MODULE_PATH\n"
		"       environment variable.\n"
		"\n"
	    #endif
		"Playback frame rate:\n"
		"\n"
		"       If the frame rate is not specified on the command\n"
		"       line (using the -f flag), then the frame rate is\n"
		"       determined by the framesPerSecond attribute in the\n"
		"       header of the first frame of the image sequence.\n"
		"       If the header contains no framesPerSecond attribute\n"
		"       then the frame rate is set to 24 frames per second.\n"
		"\n"
		"Keyboard commands:\n"
		"\n"
		"       L or P       play forward / pause\n"
		"       H            play backward / pause\n"
		"       K            step one frame forward\n"
		"       J            step one frame backward\n"
		"       > or .       increase exposure\n"
		"       < or ,       decrease exposure\n"
	    #if HAVE_CTL_INTERPRETER
		"       C            CTL transforms on/off\n"
	    #endif
		"       O            text overlay on/off\n"
		"       F            full-screen mode on/off\n"
		"       Q or ESC     quit\n"
		"\n";

	 cerr << endl;
    }

    exit (1);
}


int exitStatus = 0;


void
quickexit ()
{
    //
    // Hack to avoid crashes when someone presses the close or 'X'
    // button in the title bar of our window.  Something GLUT does
    // while shutting down the program does not play well with
    // multiple threads.  Bypassing GLUT's orderly shutdown by
    // calling _exit immediately avoids crashes.
    //

    _exit (exitStatus);
}

} // namespace


int
main(int argc, char **argv)
{
    glutInit (&argc, argv);

    const char *fileNameTemplate = 0;
    int firstFrame = 1;
    int lastFrame = 1;
    int numThreads = 0;
    float fps = -1;
    float xyScale = 1;
    vector<string> transformNames;
    bool useHwTexInterpolation = true;

    //
    // Parse the command line.
    //

    if (argc < 2)
	usageMessage (argv[0], true);

    int i = 1;
    int j = 0;

    while (i < argc)
    {
	if (!strcmp (argv[i], "-t"))
	{
	    //
	    // Set number of threads
	    //

	    if (i > argc - 2)
		usageMessage (argv[0]);

	    numThreads = strtol (argv[i + 1], 0, 0);

	    if (numThreads < 0)
	    {
		cerr << "Number of threads cannot be negative." << endl;
		return 1;
	    }

	    i += 2;
	}
	else if (!strcmp (argv[i], "-f"))
	{
	    //
	    // Set frame rate
	    //

	    if (i > argc - 2)
		usageMessage (argv[0]);

	    fps = strtod (argv[i + 1], 0);

	    if (fps < 1 || fps > 1000)
	    {
		cerr << "Playback speed must be between "
			"1 and 1000 frames per second." << endl;
		return 1;
	    }

	    i += 2;
	}
	else if (!strcmp (argv[i], "-S"))
	{
	    //
	    // Set image scale factor
	    //

	    if (i > argc - 2)
		usageMessage (argv[0]);

	    xyScale = strtod (argv[i + 1], 0);

	    if (xyScale < 0.1 || xyScale > 2.0)
	    {
		cerr << "Scale factor must be between 0.1 and 2.0." << endl;
		return 1;
	    }

	    i += 2;
	}
	else if (!strcmp (argv[i], "-C"))
	{
	    //
	    // Apply a CTL transform
	    //

	    if (i > argc - 2)
		usageMessage (argv[0]);

	    transformNames.push_back (argv[i + 1]);
	    i += 2;
	}
	else if (!strcmp (argv[i], "-i"))
	{
	    //
	    // Use software-based texture map interpolation
	    //

	    useHwTexInterpolation = false;
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
	    // Image file name or frame number
	    //

	    switch (j)
	    {
	      case 0:
		fileNameTemplate = argv[i];
		break;

	      case 1:
		firstFrame = strtol (argv[i], 0, 0);
		break;

	      case 2:
		lastFrame = strtol (argv[i], 0, 0);
		break;

	      default:
		break;
	    }

	    i += 1;
	    j += 1;
	}
    }

    if (j != 1 && j != 3)
	usageMessage (argv[0]);

    if (firstFrame > lastFrame)
    {
	cerr << "Frame number of first frame is greater than "
		"frame number of last frame." << endl;
	return 1;
    }

    //
    // Make sure that we have threading support.
    //

    if (!ILMTHREAD_NAMESPACE::supportsThreads())
    {
	cerr << "This program requires multi-threading support.\n" << endl;
	return 1;
    }

    //
    // Play the image sequence.
    //

    atexit (quickexit);

    try
    {
	playExr (fileNameTemplate,
	         firstFrame,
		 lastFrame,
		 numThreads,
		 fps,
		 xyScale,
		 transformNames,
		 useHwTexInterpolation);
    }
    catch (const exception &e)
    {
	cerr << e.what() << endl;
	exitStatus = 1;
    }

    return exitStatus;
}
