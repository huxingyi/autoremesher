//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifndef _PxDeepOutRow_h_
#define _PxDeepOutRow_h_

#include "PxDeepUtils.h"
#include "PxDeepOutPixel.h"

#include <ImfDeepScanLineOutputFile.h>
#include <ImfDeepFrameBuffer.h>
#include <ImfPartType.h>
#include <ImfChannelList.h>

namespace PxDeep {

//-*****************************************************************************
//-*****************************************************************************
// DEEP OUT ROW
//-*****************************************************************************
//-*****************************************************************************
template <typename RGBA_T>
class DeepOutRow
{
public:
    DeepOutRow( int i_width, bool i_doDeepBack, bool i_doRGB );

    void clear();
    
    void addHole( int i_x )
    {
        m_sampleCounts[i_x] = 0;
    }

    void addPixel( int i_x, const DeepOutPixel<RGBA_T>& i_pixel );

    void setFrameBuffer( Imf::DeepFrameBuffer& o_frameBuffer );

protected:
    // Width of the row.
    int m_width;

    // Whether or not to bother with deep back
    bool m_doDeepBack;

    // Whether or not to bother with RGB
    bool m_doRGB;
    
    // Scanline sample buffers.
    std::vector<uint> m_sampleCounts;

    // The pointers to data at each pixel
    std::vector<float*> m_deepFrontPtrs;
    std::vector<float*> m_deepBackPtrs;
    std::vector<RGBA_T*> m_redPtrs;
    std::vector<RGBA_T*> m_greenPtrs;
    std::vector<RGBA_T*> m_bluePtrs;
    std::vector<RGBA_T*> m_alphaPtrs;

    // The data itself.
    std::vector<float> m_deepFrontSamples;
    std::vector<float> m_deepBackSamples;
    std::vector<RGBA_T> m_redSamples;
    std::vector<RGBA_T> m_greenSamples;
    std::vector<RGBA_T> m_blueSamples;
    std::vector<RGBA_T> m_alphaSamples;
};

//-*****************************************************************************
template <typename T>
inline void VecAppend( T& i_dst, const T& i_src )
{
    i_dst.insert( i_dst.end(), i_src.begin(), i_src.end() );
}

//-*****************************************************************************
template <typename RGBA_T>
DeepOutRow<RGBA_T>::DeepOutRow( int i_width, bool i_doDeepBack, bool i_doRGB )
  : m_width( i_width )
  , m_doDeepBack( i_doDeepBack )
  , m_doRGB( i_doRGB )
{
    m_sampleCounts.resize( ( size_t )m_width );
    m_deepFrontPtrs.resize( ( size_t )m_width );
    if ( m_doDeepBack )
    {
        m_deepBackPtrs.resize( ( size_t )m_width );
    }
    if ( m_doRGB )
    {
        m_redPtrs.resize( ( size_t )m_width );
        m_greenPtrs.resize( ( size_t )m_width );
        m_bluePtrs.resize( ( size_t )m_width );
    }
    m_alphaPtrs.resize( ( size_t )m_width );
}

//-*****************************************************************************
template <typename RGBA_T>
void DeepOutRow<RGBA_T>::clear()
{
    std::fill( m_sampleCounts.begin(),
               m_sampleCounts.end(),
               ( uint )0 );
    m_deepFrontSamples.clear();
    m_deepBackSamples.clear();
    m_redSamples.clear();
    m_greenSamples.clear();
    m_blueSamples.clear();
    m_alphaSamples.clear();
}

//-*****************************************************************************
template <typename RGBA_T>
void DeepOutRow<RGBA_T>::addPixel( int i_x,
                                   const DeepOutPixel<RGBA_T>& i_pixel )
{
    int npoints = i_pixel.size();
    m_sampleCounts[i_x] = npoints;
    if ( npoints > 0 )
    {
        VecAppend( m_deepFrontSamples, i_pixel.deepFront );
        if ( m_doDeepBack )
        {
            VecAppend( m_deepBackSamples, i_pixel.deepBack );
        }
        if ( m_doRGB )
        {
            VecAppend( m_redSamples, i_pixel.red );
            VecAppend( m_greenSamples, i_pixel.green );
            VecAppend( m_blueSamples, i_pixel.blue );
        }
        VecAppend( m_alphaSamples, i_pixel.alpha );
    }
}

//-*****************************************************************************
template <typename RGBA_T>
void DeepOutRow<RGBA_T>::setFrameBuffer( Imf::DeepFrameBuffer& o_frameBuffer )
{
    // Set up the pointers.
    float *deepFrontLastPtr = m_deepFrontSamples.data();
    float *deepBackLastPtr = m_deepBackSamples.data();
    RGBA_T *redLastPtr = m_redSamples.data();
    RGBA_T *greenLastPtr = m_greenSamples.data();
    RGBA_T *blueLastPtr = m_blueSamples.data();
    RGBA_T *alphaLastPtr = m_alphaSamples.data();
    for ( int x = 0; x < m_width; ++x )
    {
        m_deepFrontPtrs[x] = deepFrontLastPtr;
        if ( m_doDeepBack )
        {    
            m_deepBackPtrs[x] = deepBackLastPtr;
        }
        if ( m_doRGB )
        {
            m_redPtrs[x] = redLastPtr;
            m_greenPtrs[x] = greenLastPtr;
            m_bluePtrs[x] = blueLastPtr;
        }
        m_alphaPtrs[x] = alphaLastPtr;

        int c = m_sampleCounts[x];
            
        deepFrontLastPtr += c;
        deepBackLastPtr += c;
        redLastPtr += c;
        greenLastPtr += c;
        blueLastPtr += c;
        alphaLastPtr += c;
    }
        
    // Sample counts
    o_frameBuffer.insertSampleCountSlice(
        Imf::Slice( Imf::UINT,
                    ( char * )m_sampleCounts.data(),
                    sizeof( uint ),    // x stride
                    0 ) );             // y stride

    // RGB
    if ( m_doRGB )
    {
        o_frameBuffer.insert(
            "R",
            Imf::DeepSlice( ImfPixelType<RGBA_T>(),
                            ( char * )m_redPtrs.data(),
                            sizeof( RGBA_T* ),     // xstride
                            0,                     // ystride
                            sizeof( RGBA_T ) ) );  // sample stride
    
        o_frameBuffer.insert(
            "G",
            Imf::DeepSlice( ImfPixelType<RGBA_T>(),
                            ( char * )m_greenPtrs.data(),
                            sizeof( RGBA_T* ),     // xstride
                            0,                     // ystride
                            sizeof( RGBA_T ) ) );  // sample stride
            
        o_frameBuffer.insert(
            "B",
            Imf::DeepSlice( ImfPixelType<RGBA_T>(),
                            ( char * )m_bluePtrs.data(),
                            sizeof( RGBA_T* ),     // xstride
                            0,                     // ystride
                            sizeof( RGBA_T ) ) );  // sample stride
    }

    // ALPHA
    o_frameBuffer.insert(
        "A",
        Imf::DeepSlice( ImfPixelType<RGBA_T>(),
                        ( char * )m_alphaPtrs.data(),
                        sizeof( RGBA_T* ),     // xstride
                        0,                     // ystride
                        sizeof( RGBA_T ) ) );  // sample stride
        
    // DEEP FRONT
    o_frameBuffer.insert(
        "Z",
        Imf::DeepSlice( Imf::FLOAT,
                        ( char * )m_deepFrontPtrs.data(),
                        sizeof( float* ),      // xstride
                        0,                     // ystride
                        sizeof( float ) ) );   // sample stride

    // DEEP BACK
    if ( m_doDeepBack )
    {
        o_frameBuffer.insert(
            "ZBack",
            Imf::DeepSlice( Imf::FLOAT,
                            ( char * )m_deepBackPtrs.data(),
                            sizeof( float* ),      // xstride
                            0,                     // ystride
                            sizeof( float ) ) );   // sample stride
    }
}

} // End namespace PxDeep

#endif
