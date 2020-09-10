//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifndef _PxDeepBaseHelper_h_
#define _PxDeepBaseHelper_h_

#include "PxDeepUtils.h"
#include "PxDeepOutPixel.h"
#include "PxDeepOutRow.h"

#include <dtex.h>

#include <ImfDeepScanLineOutputFile.h>
#include <ImfPartType.h>
#include <ImfChannelList.h>

#include <ImathVec.h>
#include <ImathBox.h>

namespace PxDeep {

//-*****************************************************************************
// PARAMETERS STRUCT
//-*****************************************************************************
// This allows us to keep function signatures from changing around too much
// as the parameter set grows & changes, which it always does.
struct Parameters
{
    Parameters()
      : deepOpacity( true )
      , discrete( true )
      , multiplyColorByAlpha( false )
      , sideways( false )
      , discardZeroAlphaSamples( true )
      , doDeepBack( true )
      , doRGB( true )
      , compressionError( 0.0f ) {}

    bool deepOpacity;
    bool discrete;
    bool multiplyColorByAlpha;
    bool sideways;
    bool discardZeroAlphaSamples;
    bool doDeepBack;
    bool doRGB;
    float compressionError;
};

//-*****************************************************************************
// BASE DEEP HELPER CLASS
//-*****************************************************************************
// The intention of this templated base class is to provide consistent
// storage vectors for spans and deep pixels across multiple pixel reads,
// so we don't slow down constantly creating and destroying std::vectors.
// This class has a "processDeepBox" function which does the work.
template <typename RGBA_T, typename DERIVED, typename SPAN>
class BaseDeepHelper
{
public:
    typedef DERIVED sub_type;
    typedef SPAN span_type;

    BaseDeepHelper( DtexFile* i_dtexFile,
                    int i_numDtexChans,
                    const Parameters& i_params )
      : m_dtexFile( i_dtexFile )
      , m_numDtexChans( i_numDtexChans )
      , m_params( i_params )
    {
        m_image = NULL;
        DtexGetImageByIndex( m_dtexFile, 0, &m_image );
        
        m_fileWidth = DtexWidth( m_image );
        m_fileHeight = DtexHeight( m_image );

        m_pixel = DtexMakePixel( m_numDtexChans );
        m_rawPixel = DtexMakePixel( m_numDtexChans );
    }

    ~BaseDeepHelper()
    {
        DtexDestroyPixel( m_pixel );
        DtexDestroyPixel( m_rawPixel );
    }

    void processDeepBox( Imf::DeepScanLineOutputFile& o_file,
                         const Imath::Box2i& i_box );

protected:
    DtexFile* m_dtexFile;
    int m_numDtexChans;
    Parameters m_params;

    DtexImage* m_image;
    int m_fileWidth;
    int m_fileHeight;
    DtexPixel* m_pixel;
    DtexPixel* m_rawPixel;
    
    std::vector<span_type> m_spans;
    DeepOutPixel<RGBA_T> m_deepOutPixel;
};

//-*****************************************************************************
//-*****************************************************************************
// SPAN CLASSES
//-*****************************************************************************
//-*****************************************************************************

//-*****************************************************************************
// These span objects are used by the helper classes below to keep track
// of the information read out of the DTEX file, so it can be processed.
// They have an ordering operator which sorts them by depth and then index.
// We use double precision for 'viz', for reasons described in the 'VIZ'
// section of the explanatory comments in the PxDeepUtils library.
struct Span
{
    Span() : in( 0.0f ), out( 0.0f ), viz( 0.0 ), index( 0 ) {}
    
    float in;
    float out;
    double viz;
    int index;

    void clear()
    {
        in = 0.0f;
        out = 0.0f;
        viz = 0.0;
        index = 0;
    }

    bool operator<( const Span& i_other ) const
    {
        if ( in < i_other.in )
        {
            return true;
        }
        else if ( in == i_other.in )
        {
            return index < i_other.index;
        }
        else
        {
            return false;
        }
    }
};

//-*****************************************************************************
// Because the RGB values here are unpremultiplied, we use double precision
// to avoid precision loss when going (RGB/A)*A.
struct SpanRgba : public Span
{
    SpanRgba() : Span() { rgb[0] = 0.0; rgb[1] = 0.0; rgb[2] = 0.0; }
    
    double rgb[3];

    void clear()
    {
        Span::clear();
        rgb[0] = 0.0;
        rgb[1] = 0.0;
        rgb[2] = 0.0;
    }
};

//-*****************************************************************************
// As above, we use double precision for viz.
struct SpanOpac : public Span
{
    SpanOpac() : Span(), deepViz( 0.0 ) {}
    
    double deepViz;

    void clear()
    {
        Span::clear();
        deepViz = 0.0;
    }
};


//-*****************************************************************************
// The box processing simply loops over the rows, compresses each pixel, then
// converts from dtex representation to deep exr representation, and finally
// writes the rows to the file.
template <typename RGBA_T, typename DERIVED, typename SPAN>
void BaseDeepHelper<RGBA_T,DERIVED,SPAN>::processDeepBox
(
    Imf::DeepScanLineOutputFile& o_file,
    const Imath::Box2i& i_box )
{
    int width = ( i_box.max.x - i_box.min.x ) + 1;

    DeepOutRow<RGBA_T> outRow( width, m_params.doDeepBack, m_params.doRGB );
    
    for ( int y = i_box.max.y; y >= i_box.min.y; --y )
    {
        outRow.clear();
        
        for ( int x = i_box.min.x; x <= i_box.max.x; ++x )
        {
            if ( m_params.sideways )
            {
                DtexGetPixel( m_image, m_fileWidth - 1 - y,
                              m_fileHeight - 1 - x, m_rawPixel );
            }
            else
            {
                DtexGetPixel( m_image, x,
                              m_fileHeight - 1 - y, m_rawPixel );
            }

            // Get the number of input points.
            int numPointsIn = DtexPixelGetNumPoints( m_rawPixel );
            if ( numPointsIn < 0 )
            {
                PXDU_THROW( "Negative num points returned at dtex pixel: "
                            << x << ", " << y );
            }

            // Compress the pixel.
            if ( numPointsIn > 1 && m_params.compressionError > 0.0f )
            {
                DtexCompressPixel( m_rawPixel, m_pixel,
                                   m_params.compressionError );
                DtexFinishPixel( m_pixel );
            }
            else
            {
                DtexCopyPixel( m_pixel, m_rawPixel );
                DtexFinishPixel( m_pixel );
            }

            // Get num points of compressed pixel.
            int numPts = DtexPixelGetNumPoints( m_pixel );

            // If no samples here, continue on.
            if ( numPts < 1 )
            {
                outRow.addHole( x - i_box.min.x );
                continue;
            }

            m_deepOutPixel.clear();
            m_deepOutPixel.reserve( numPts * m_numDtexChans );
            
            // Process the deep pixels.
            DERIVED* derivedThis = static_cast<DERIVED*>( this );
            derivedThis->processDeepPixel( numPts );

            // Add the pixel to the row.
            outRow.addPixel( x - i_box.min.x, m_deepOutPixel );
        }

        // Create the frame buffer.
        // I'm not sure if this can be reused from row to row.
        Imf::DeepFrameBuffer frameBuffer;

        // Set the deep out row into the framebuffer.
        outRow.setFrameBuffer( frameBuffer );

        // Write row.
        o_file.setFrameBuffer( frameBuffer );
        o_file.writePixels( 1 );
    }
}

} // End namespace PxDeep

#endif
