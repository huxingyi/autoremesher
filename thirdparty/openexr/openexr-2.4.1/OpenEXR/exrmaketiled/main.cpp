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


//-----------------------------------------------------------------------------
//
//	exrmaketiled -- program that produces tiled
//	multiresolution versions of OpenEXR images.
//
//-----------------------------------------------------------------------------

#include "makeTiled.h"

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
    cerr << "usage: " << argv0 << " [options] infile outfile" << endl;

    if (verbose)
    {
        cerr << "\n"
        "Reads an OpenEXR image from infile, produces a tiled\n"
        "version of the image, and saves the result in outfile.\n"
        "\n"
        "Options:\n"
        "\n"
        "-o        produces a ONE_LEVEL image (default)\n"
        "\n"
        "-m        produces a MIPMAP_LEVELS multiresolution image\n"
        "\n"
        "-r        produces a RIPMAP_LEVELS multiresolution image\n"
        "\n"
        "-f c      when a MIPMAP_LEVELS or RIPMAP_LEVELS image\n"
        "          is produced, image channel c will be resampled\n"
        "          without low-pass filtering.  This option can\n"
        "          be specified multiple times to disable low-pass\n"
        "          filtering for mutiple channels.\n"
        "\n"
        "-e x y    when a MIPMAP_LEVELS or RIPMAP_LEVELS image\n"
        "          is produced, low-pass filtering takes samples\n"
        "          outside the image's data window.  This requires\n"
        "          extrapolating the image.  Option -e specifies\n"
        "          how the image is extrapolated horizontally and\n"
        "          vertically (black/clamp/periodic/mirror, default\n"
        "          is clamp).\n"
        "\n"
        "-t x y    sets the tile size in the output image to\n"
        "          x by y pixels (default is 64 by 64)\n"
        "\n"
        "-d        sets level size rounding to ROUND_DOWN (default)\n"
        "\n"
        "-u        sets level size rounding to ROUND_UP\n"
        "\n"
        "-z x      sets the data compression method to x\n"
        "          (none/rle/zip/piz/pxr24/b44/b44a/dwaa/dwab,\n"
        "          default is zip)\n"
        "\n"
        "-v        verbose mode\n"
        "\n"
        "-h        prints this message\n"
        "\n"
        "Multipart Options:\n"
        "\n"
        "-p i      part number, default is 0\n";

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


Extrapolation
getExtrapolation (const string &str)
{
    Extrapolation e;

    if (str == "black" || str == "BLACK")
    {
        e = BLACK;
    }
    else if (str == "clamp" || str == "CLAMP")
    {
        e = CLAMP;
    }
    else if (str == "periodic" || str == "PERIODIC")
    {
        e = PERIODIC;
    }
    else if (str == "mirror" || str == "MIRROR")
    {
        e = MIRROR;
    }
    else
    {
        cerr << "Unknown extrapolation method \"" << str << "\"." << endl;
        exit (1);
    }

    return e;
}

void
getPartNum (int argc,
            char **argv,
            int &i,
            int *j)
{
    if (i > argc - 2)
        usageMessage (argv[0]);

    *j = strtol (argv[i + 1], 0, 0);
    cout << "part number: "<< *j << endl;
    i += 2;
}

} // namespace


int
main(int argc, char **argv)
{
    const char *inFile = 0;
    const char *outFile = 0;
    LevelMode mode = ONE_LEVEL;
    LevelRoundingMode roundingMode = ROUND_DOWN;
    Compression compression = ZIP_COMPRESSION;
    int tileSizeX = 64;
    int tileSizeY = 64;
    set<string> doNotFilter;
    Extrapolation extX = CLAMP;
    Extrapolation extY = CLAMP;
    bool verbose = false;

    //
    // Parse the command line.
    //

    if (argc < 2)
        usageMessage (argv[0], true);

    int i = 1;
    int partnum = 0;

    while (i < argc)
    {
        if (!strcmp (argv[i], "-o"))
        {
            //
            // generate a ONE_LEVEL image
            //

            mode = ONE_LEVEL;
            i += 1;
        }
        else if (!strcmp (argv[i], "-m"))
        {
            //
            // Generate a MIPMAP_LEVELS image
            //

            mode = MIPMAP_LEVELS;
            i += 1;
        }
        else if (!strcmp (argv[i], "-r"))
        {
            //
            // Generate a RIPMAP_LEVELS image
            //

            mode = RIPMAP_LEVELS;
            i += 1;
        }
        else if (!strcmp (argv[i], "-f"))
        {
            //
            // Don't low-pass filter the specified image channel
            //

            if (i > argc - 2)
                usageMessage (argv[0]);

            doNotFilter.insert (argv[i + 1]);
            i += 2;
        }
        else if (!strcmp (argv[i], "-e"))
        {
            //
            // Set x and y extrapolation method
            //

            if (i > argc - 3)
                usageMessage (argv[0]);

            extX = getExtrapolation (argv[i + 1]);
            extY = getExtrapolation (argv[i + 2]);
            i += 3;
        }
        else if (!strcmp (argv[i], "-t"))
        {
            //
            // Set tile size
            //

            if (i > argc - 3)
                usageMessage (argv[0]);

            tileSizeX = strtol (argv[i + 1], 0, 0);
            tileSizeY = strtol (argv[i + 2], 0, 0);

            if (tileSizeX <= 0 || tileSizeY <= 0)
            {
                cerr << "Tile size must be greater than zero." << endl;
                return 1;
            }

            i += 3;
        }
        else if (!strcmp (argv[i], "-d"))
        {
            //
            // Round down
            //

            roundingMode = ROUND_DOWN;
            i += 1;
        }
        else if (!strcmp (argv[i], "-u"))
        {
            //
            // Round down
            //

            roundingMode = ROUND_UP;
            i += 1;
        }
        else if (!strcmp (argv[i], "-z"))
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
        else if (!strcmp (argv[i], "-p"))
        {
            getPartNum (argc, argv, i, &partnum);
        }
        else
        {
            //
            // Image file name
            //

            if (inFile == 0)
                inFile = argv[i];
            else
                outFile = argv[i];

            i += 1;
        }
    }

    if (inFile == 0 || outFile == 0)
        usageMessage (argv[0]);

    if (!strcmp (inFile, outFile))
    {
        cerr << "Input and output cannot be the same file." << endl;
        return 1;
    }

    //
    // Load inFile, and save a tiled version in outFile.
    //

    int exitStatus = 0;

    //
    // check input
    //
    {
        MultiPartInputFile input (inFile);
        int parts = input.parts();

        if (partnum < 0 || partnum >= parts){
            cerr << "ERROR: you asked for part " << partnum << " in " << inFile;
            cerr << ", which only has " << parts << " parts\n";
            exit(1);
        }

        Header h = input.header (partnum);
        if (h.type() == DEEPTILE || h.type() == DEEPSCANLINE)
        {
            cerr << "Cannot make tile for deep data" << endl;
            exit(1);
        }

    }


    try
    {
        makeTiled (inFile, outFile, partnum,
                   mode, roundingMode, compression,
                   tileSizeX, tileSizeY,
                   doNotFilter,
                   extX, extY,
                   verbose);
    }
    catch (const exception &e)
    {
        cerr << e.what() << endl;
        exitStatus = 1;
    }

    return exitStatus;
}
