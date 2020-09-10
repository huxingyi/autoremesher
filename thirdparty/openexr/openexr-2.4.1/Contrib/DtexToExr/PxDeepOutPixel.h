//
// SPDX-License-Identifier: BSD-3-Clause
// Copyright Contributors to the OpenEXR Project.
//

#ifndef _PxDeepOutPixel_h_
#define _PxDeepOutPixel_h_

#include "PxDeepUtils.h"

namespace PxDeep {

//-*****************************************************************************
//-*****************************************************************************
// DEEP OUT PIXEL
//-*****************************************************************************
//-*****************************************************************************
// While constructing a deep out pixel from a dtex pixel, we reuse some
// temporary storage.
template <typename RGBA_T>
struct DeepOutPixel
{
    size_t size() const
    {
        return deepFront.size();
    }
    
    void clear()
    {
        deepFront.clear();
        deepBack.clear();
        red.clear();
        green.clear();
        blue.clear();
        alpha.clear();
    }

    void reserve( size_t N )
    {
        deepFront.reserve( N );
        deepBack.reserve( N );
        red.reserve( N );
        green.reserve( N );
        blue.reserve( N );
        alpha.reserve( N );
    }

    void push_back( float i_depth,
                    RGBA_T i_alpha )
    {
        deepFront.push_back( i_depth );
        deepBack.push_back( i_depth );
        red.push_back( 0.0f );
        green.push_back( 0.0f );
        blue.push_back( 0.0f );
        alpha.push_back( i_alpha );
    }

    void push_back( float i_deepFront,
                    float i_deepBack,
                    RGBA_T i_alpha )
    {
        deepFront.push_back( i_deepFront );
        deepBack.push_back( i_deepBack );
        red.push_back( 0.0f );
        green.push_back( 0.0f );
        blue.push_back( 0.0f );
        alpha.push_back( i_alpha );
    }
    
    void push_back( float i_depth,
                    RGBA_T i_red,
                    RGBA_T i_green,
                    RGBA_T i_blue,
                    RGBA_T i_alpha )
    {
        deepFront.push_back( i_depth );
        deepBack.push_back( i_depth );
        red.push_back( i_red );
        green.push_back( i_green );
        blue.push_back( i_blue );
        alpha.push_back( i_alpha );
    }

    void push_back( float i_deepFront,
                    float i_deepBack,
                    RGBA_T i_red,
                    RGBA_T i_green,
                    RGBA_T i_blue,
                    RGBA_T i_alpha )
    {
        deepFront.push_back( i_deepFront );
        deepBack.push_back( i_deepBack );
        red.push_back( i_red );
        green.push_back( i_green );
        blue.push_back( i_blue );
        alpha.push_back( i_alpha );
    }
    
    std::vector<float> deepFront;
    std::vector<float> deepBack;
    std::vector<RGBA_T> red;
    std::vector<RGBA_T> green;
    std::vector<RGBA_T> blue;
    std::vector<RGBA_T> alpha;
};

}

#endif
