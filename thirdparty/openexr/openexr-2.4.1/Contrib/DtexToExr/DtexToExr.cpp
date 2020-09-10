//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#include "PxDeepUtils.h"
#include "PxDeepOutPixel.h"
#include "PxDeepOutRow.h"
#include "PxBaseDeepHelper.h"
#include "PxOneChanDeepOpacity.h"
#include "PxOneChanDeepAlpha.h"
#include "PxFourChanDeepRgba.h"

#include <dtex.h>

#include <ImfDeepScanLineOutputFile.h>
#include <ImfPartType.h>
#include <ImfChannelList.h>
#include <ImfStandardAttributes.h>
#include <ImfHeader.h>

#include <ImathVec.h>
#include <ImathBox.h>
#include <ImathMatrix.h>

#include <half.h>

#include <assert.h>
#include <iostream>
#include <vector>
#include <utility>
#include <algorithm>
#include <math.h>
#include <float.h>

//-*****************************************************************************
// DTEX CONVERTER EXPLANATION!
//-*****************************************************************************
// There are six possible code paths through converting the dtex data. They are:
// DeepOpacity, Continuous
// DeepOpacity, Discrete
// DeepAlpha, Continuous
// DeepAlpha, Discrete
// DeepRGBA, Continuous
// DeepRGBA, Discrete
// The newer dtex usages allow for other combinations of channels, but we
// are temporarily just supporting these six paths for sake of simplicity.
// We will eventually support arbitrary outputs and multiple views.
//
// We had an earlier version of this code which condensed these six code
// pathways into a single function, with templated functors to provide
// specific differing behavior for each of the different possibilities, and
// while it may have been slightly less code, the resulting loops were very hard
// to read and understand. Because each of the different pathways
// has some specific kernel of knowledge necessary to make it work, we
// chose instead to break each one out separately, to clearly expose the
// behavior in each case.
//
// The terminology for Density, Visibility, DeepOpacity, DepthRanges,
// along with explanations of the constants are provided in
// PxDeepUtils.h
//
// Our DeepOutPixel helper class is in PxDeepOutPixel.h
//
// Our DeepOutRow helper class is in PxDeepOutRow.h
//
// The Base Helper class, which loops over rows, and then pixels of those
// rows, is in PxBaseDeepHelper.h
// 
// The DeepOpacity subclasses (discrete/continuous) of BaseDeepHelper are in
// PxOneChanDeepOpacity.h
// 
// The DeepAlpha subclasses (discrete/continuous) of BaseDeepHelper are in
// PxOneChanDeepAlpha.h
// 
// The DeepRgba subclasses (discrete/continuous) of BaseDeepHelper are in
// PxFourChanDeepAlpha.h
//
//-*****************************************************************************

namespace PxDeep {

//-*****************************************************************************
template <typename RGBA_T>
void ConvertDtexFile( const std::string& i_fileName,
                      const std::string& i_outFileName,
                      const Parameters& i_params )
{
    int dtexOpenError = DTEX_NOERR;
    DtexFile* dtexFile = NULL;
    DtexCache* dtexCache = NULL;

    if ( i_fileName.length() )
    {
        // We probably don't need 10000 tiles in the cache, but it's
        // fine for now.
        dtexCache = DtexCreateCache( 10000, NULL );
        dtexOpenError = DtexOpenFile( i_fileName.c_str(),
                                      "rb", dtexCache, &dtexFile );
    }
    
    if ( !dtexFile )
    {
        if ( i_fileName.length() == 0 )
        {
            PXDU_THROW( "no filename specified" );
        }
        else if ( dtexOpenError != DTEX_NOERR )
        {
            PXDU_THROW( "error (" << dtexOpenError
                        << " opening file: " << i_fileName );
        }
        else
        {
            PXDU_THROW( "missing file: " << i_fileName );
        }
    }

    // Just handling the first image in the Dtex file for now.
    DtexImage* image;
    DtexGetImageByIndex( dtexFile, 0, &image );

    float NP[16];
    DtexNP( image, NP );

    float Nl[16];
    DtexNl( image, Nl );

    int numDtexChans = DtexNumChan( image );

    if ( numDtexChans != 1 &&
         numDtexChans != 3 &&
         numDtexChans != 4 )
    {
        DtexClose( dtexFile );
        PXDU_THROW( "ERROR: only 1, 3 or 4 channel dtex files are supported.\n"
                    << "Dtex file " << i_fileName
                    << " contains " << numDtexChans << " channels.\n"
                    << "In the case of 3 channels, the data is assumed to be\n"
                    << "3-channel opacity, and for now, at least, only the\n"
                    << "first channel is used, rather than all three.\n" );
    }   

    int w = DtexWidth( image );
    int h = DtexHeight( image );

    // Extract the parameters so we can conditionally modify them.
    Parameters params = i_params;
    
    // If we're reading anything more than 1 channel,
    // we can't (for now) assume it's a deepOpacity signal,
    // so we turn off the deepOpacity flag.
    // We also make sure RGB is turned on when RGB data is present.
    if ( numDtexChans == 4 )
    {
        params.deepOpacity = false;
        params.doRGB = true;
    }

    // If we're discrete, we don't necessarily need to output deepBack.
    // However, from a pipeline point of view it is often preferable to have
    // all the channels actually in existence, even if they're redundant.
    // Nonetheless, if 
    if ( !params.discrete )
    {
        params.doDeepBack = true;
    }

    // Determine the output size
    int outWidth = w;
    int outHeight = h;
    if ( params.sideways )
    {
        outWidth = h;
        outHeight = w;
    }

    // Create the windows.
    Imath::Box2i dataWindow( Imath::V2i( 0, 0 ),
                             Imath::V2i( outWidth-1, outHeight-1 ) );
    Imath::Box2i displayWindow = dataWindow;

    // Create the header.
    Imf::Header header( displayWindow,
                        dataWindow,
                        1,
                        Imath::V2f( 0.0f, 0.0f ),
                        1,
                        Imf::INCREASING_Y,
                        Imf::ZIPS_COMPRESSION );

    // Add Np/Nl to the header.
    Imath::M44f NPm( NP[0], NP[1], NP[2], NP[3],
                     NP[4], NP[5], NP[6], NP[7],
                     NP[8], NP[9], NP[10], NP[11],
                     NP[12], NP[13], NP[14], NP[15] );
    addWorldToNDC( header, NPm );
    
    Imath::M44f Nlm( Nl[0], Nl[1], Nl[2], Nl[3],
                     Nl[4], Nl[5], Nl[6], Nl[7],
                     Nl[8], Nl[9], Nl[10], Nl[11],
                     Nl[12], Nl[13], Nl[14], Nl[15] );
    addWorldToCamera( header, Nlm );

    // Add channels to the header.

    // RGB
    if ( params.doRGB )
    {
        header.channels().insert( "R",
                                  Imf::Channel( ImfPixelType<RGBA_T>() ) );
        header.channels().insert( "G",
                                  Imf::Channel( ImfPixelType<RGBA_T>() ) );
        header.channels().insert( "B",
                                  Imf::Channel( ImfPixelType<RGBA_T>() ) );
    }

    // A
    header.channels().insert( "A",
                              Imf::Channel( ImfPixelType<RGBA_T>() ) );

    // Deep Front (z)
    header.channels().insert( "Z",
                              Imf::Channel( Imf::FLOAT ) );

    // Deep Back
    if ( params.doDeepBack )
    {
        header.channels().insert( "ZBack",
                                  Imf::Channel( Imf::FLOAT ) );
    }

    // Tell header to be deep!
    header.setType( Imf::DEEPSCANLINE );

    // Create output file, and fill it up!
    {
        Imf::DeepScanLineOutputFile outputFile( i_outFileName.c_str(),
                                                header );
        
        // Process deep pixels.
        if ( numDtexChans < 4 )
        {
            if ( params.discrete )
            {
                if ( params.deepOpacity )
                {
                    OneChanDeepOpacityDiscrete<RGBA_T> op( dtexFile,
                                                           numDtexChans,
                                                           params );
                    op.processDeepBox( outputFile, dataWindow );
                }
                else
                {
                    OneChanDeepAlphaDiscrete<RGBA_T> op( dtexFile,
                                                         numDtexChans,
                                                         params );
                    op.processDeepBox( outputFile, dataWindow );
                }
            }
            else
            {
                if ( params.deepOpacity )
                {
                    OneChanDeepOpacityContinuous<RGBA_T> op( dtexFile,
                                                             numDtexChans,
                                                             params );
                    op.processDeepBox( outputFile, dataWindow );
                }
                else
                {
                    OneChanDeepAlphaContinuous<RGBA_T> op( dtexFile,
                                                           numDtexChans,
                                                           params );
                    op.processDeepBox( outputFile, dataWindow );
                }
            }
        }
        else
        {
            if ( params.discrete )
            {
                FourChanDeepRgbaDiscrete<RGBA_T> op( dtexFile,
                                                     numDtexChans,
                                                     params );
                op.processDeepBox( outputFile, dataWindow );
            }
            else
            {
                FourChanDeepRgbaContinuous<RGBA_T> op( dtexFile,
                                                       numDtexChans,
                                                       params );
                op.processDeepBox( outputFile, dataWindow );
            }
        }
    } // Output file has gone out of scope, and should be closed!
    std::cout << "Wrote file: " << i_outFileName << std::endl;

    // Close it up!
    if ( dtexFile )
    {
        DtexClose( dtexFile );
    }
    
    if ( dtexCache )
    {
        DtexDestroyCache( dtexCache );
    }
}

} // End namespace PxDeep

//-*****************************************************************************
// Print Usage.
void PrintUsage( const std::string& i_cmd, std::ostream& ostr )
{
    ostr << "DtexToExr: USAGE: "
         << i_cmd
         << std::endl << std::endl
         << "\t <inFileName.dtex>"
         << std::endl << std::endl
         << "\t <outFileName.exr>"
         << std::endl << std::endl
         << "\t --deepOpacity (DEFAULT) "
         << "\n\t\t (corresponds to output channels \'deepopacity\')"
         << std::endl << std::endl
         << "\t --deepAlpha "
         << "\n\t\t (corresponds to output channels \'a\' or \'rgba\')"
         << std::endl << std::endl
         << "\t --discrete (DEFAULT) "
         << "\n\t\t (corresponds to \'volumeinterpretation discrete\')"
         << std::endl << std::endl
         << "\t --continuous "
         << "\n\t\t (corresponds to \'volumeinterpretation continuous\')"
         << std::endl << std::endl
         << "\t --full "
         << "\n\t\t (use full 32-bit precision for non-depth (RGBA) data)"
         << std::endl << std::endl
         << "\t --half (DEFAULT) "
         << "\n\t\t (use half 16-bit precision for non-depth (RGBA) data)"
         << std::endl << std::endl
         << "\t --multRgb "
         << "\n\t\t (multiply RGB data by Alpha, "
         << "implying that source data is unpremultiplied)"
         << std::endl << std::endl
         << "\t --sideways [true|false]"
         << "\n\t\t (rotate image 90 deg clockwise)"
         << std::endl << std::endl
         << "\t --compressionError <float> (DEFAULT: 0.0f) "
         << "\n\t\t (compress dtex data before converting to deep exr)"
         << std::endl << std::endl
         << "\t --keepZeroAlpha "
         << "\n\t\t (don\'t discard samples with zero alpha)"
         << std::endl << std::endl
         << "\t --discardZeroAlpha (DEFAULT) "
         << "\n\t\t (discard samples with zero alpha)"
         << std::endl << std::endl
         << "\t -h,--h,--help "
         << "\n\t\t (print this message and exit)"
         << std::endl << std::endl;

}

//-*****************************************************************************
inline bool GoodFileName( const std::string& i_fileName )
{
    return !( ( i_fileName.length() == 0 ||
                i_fileName == "" ||
                i_fileName[0] == '-' ) );
}

//-*****************************************************************************
// Argument parsing. So inelegant, but libarg is not widely supported, and
// boost::program_options is, well, boost. Also - not particularly awesome.
void ParseArguments( int argc, char* argv[],
                     bool& o_full,
                     std::string& o_dtexFileName,
                     std::string& o_exrFileName,
                     PxDeep::Parameters& o_params )
{
    if ( argc < 3 )
    {
        PrintUsage( argv[0], std::cerr );
        exit( -1 );
    }

    // Make our params match what the usage string says by default.
    o_full = false;
    o_dtexFileName = "";
    o_exrFileName = "";
    o_params.deepOpacity = true;
    o_params.discrete = true;
    o_params.multiplyColorByAlpha = false;
    o_params.sideways = false;
    o_params.discardZeroAlphaSamples = true;
    o_params.doDeepBack = true;
    o_params.doRGB = false;
    o_params.compressionError = 0.0f;

    // Eat up the args!
    for ( int argi = 1; argi < argc; ++argi )
    {
        std::string arg = argv[argi];

        if ( arg == "-h" || arg == "-help" || arg == "--help" )
        {
            PrintUsage( argv[0], std::cerr );
            exit( -1 );
        }
        else if ( argi == 1 )
        {
            if ( !GoodFileName( arg ) )
            {
                PrintUsage( argv[0], std::cerr );
                PXDU_THROW( "Bad file name: " << arg );
            }
            o_dtexFileName = arg;
        }
        else if ( argi == 2 )
        {
            if ( !GoodFileName( arg ) )
            {
                PrintUsage( argv[0], std::cerr );
                PXDU_THROW( "Bad file name: " << arg );
            }
            o_exrFileName = arg;
        }
        else if ( arg == "--deepOpacity" )
        {
            o_params.deepOpacity = true;
        }
        else if ( arg == "--deepAlpha" )
        {
            o_params.deepOpacity = false;
        }
        else if ( arg == "--discrete" )
        {
            o_params.discrete = true;
        }
        else if ( arg == "--continuous" )
        {
            o_params.discrete = false;
        }
        else if ( arg == "--full" )
        {
            o_full = true;
        }
        else if ( arg == "--half" )
        {
            o_full = false;
        }
        else if ( arg == "--multRgb" )
        {
            o_params.multiplyColorByAlpha = true;
        }
        else if ( arg == "--sideways" )
        {
            if ( argi <= argc-1 )
            {
                std::cout << argv[argi+1] << std::endl;
                if ( strcmp(argv[argi+1], "true") == 0 )
                {
                    o_params.sideways = true;
                    ++argi;
                }
                else if ( strcmp(argv[argi+1], "false") == 0 )
                {
                    o_params.sideways = false;
                    ++argi;
                }
                else if ( argv[argi+1][0] == '-' )
                {
                    // found another arg. sideways has no parameter, which means sideways is on.
                    o_params.sideways = true;
                }
                else
                {
                    // sideways must be followed by "true", "false", another arg, or nothing.
                    PXDU_THROW( "Invalid parameter for --sideways: " << argv[argi+1] );
                }
            }
            else
            {
                o_params.sideways = true;
            }
        }
        else if ( arg == "--compressionError" )
        {
            if ( argi >= argc-1 )
            {
                PrintUsage( argv[0], std::cerr );
                PXDU_THROW( "Unspecified compression error." );
            }
            
            o_params.compressionError = atof( argv[argi+1] );
            ++argi;
        }
        else if ( arg == "--keepZeroAlpha" )
        {
            o_params.discardZeroAlphaSamples = false;
        }
        else if ( arg == "--discardZeroAlpha" )
        {
            o_params.discardZeroAlphaSamples = true;
        }
        else
        {
            PrintUsage( argv[0], std::cerr );
            PXDU_THROW( "Unknown command line argument: " << arg );
        }
    }
}

//-*****************************************************************************
// MAIN FUNCTION
//-*****************************************************************************
int main( int argc, char *argv[] )
{
    try
    {
        PxDeep::Parameters params;
        std::string dtexFileName;
        std::string exrFileName;
        bool full;

        ParseArguments( argc, argv, full,
                        dtexFileName, exrFileName,
                        params );

        if ( full )
        {
            PxDeep::ConvertDtexFile<float>( dtexFileName,
                                            exrFileName,
                                            params );
        }
        else
        {
            PxDeep::ConvertDtexFile<half>( dtexFileName,
                                           exrFileName,
                                           params );
        }
    }
    catch ( std::exception& exc )
    {
        std::cerr << "ERROR EXCEPTION: " << exc.what() << std::endl;
        exit( -1 );
    }
    catch ( ... )
    {
        std::cerr << "UNKNOWN EXCEPTION." << std::endl;
        exit( -1 );
    }

    return 0;
}
