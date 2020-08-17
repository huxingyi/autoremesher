/*
Copyright (c) 2006, Michael Kazhdan and Matthew Bolitho
All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

Redistributions of source code must retain the above copyright notice, this list of
conditions and the following disclaimer. Redistributions in binary form must reproduce
the above copyright notice, this list of conditions and the following disclaimer
in the documentation and/or other materials provided with the distribution. 

Neither the name of the Johns Hopkins University nor the names of its contributors
may be used to endorse or promote products derived from this software without specific
prior writften permission. 

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED
TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
DAMAGE.
*/

#ifndef POINT_STREAM_INCLUDED
#define POINT_STREAM_INCLUDED

// [Bruno Levy 2016]: using a replacement class for
// Ply vertices, so that we
// do not need importing all the Ply I/O code.
// Adapted from Ply.h

#include "PlyVertexMini.h"

template< class Real >
struct OrientedPoint3D
{
        Point3D< Real > p , n;
        OrientedPoint3D( Point3D< Real > pp=Point3D< Real >() , Point3D< Real > nn=Point3D< Real >() ) : p(pp) , n(nn) { ; }
};

template< class Real >
class OrientedPointStream
{
public:
        virtual ~OrientedPointStream( void ){}
        virtual void reset( void ) = 0;
        virtual bool nextPoint( OrientedPoint3D< Real >& p ) = 0;
};
template< class Real , class Data >
class OrientedPointStreamWithData : public OrientedPointStream< Real >
{
public:
        virtual ~OrientedPointStreamWithData( void ){}
        virtual void reset( void ) = 0;
        virtual bool nextPoint( OrientedPoint3D< Real >& p , Data& d ) = 0;

        virtual bool nextPoint( OrientedPoint3D< Real >& p ){ Data d ; return nextPoint( p , d ); }
};

template< class Real >
class MemoryOrientedPointStream : public OrientedPointStream< Real >
{
        const OrientedPoint3D< Real >* _points;
        size_t _pointCount;
        size_t _current;
public:
        MemoryOrientedPointStream( size_t pointCount , const OrientedPoint3D< Real >* points );
        ~MemoryOrientedPointStream( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p );
};

template< class Real , class Data >
class MemoryOrientedPointStreamWithData : public OrientedPointStreamWithData< Real , Data >
{
        const std::pair< OrientedPoint3D< Real > , Data >* _points;
        size_t _pointCount;
        size_t _current;
public:
        MemoryOrientedPointStreamWithData( size_t pointCount , const std::pair< OrientedPoint3D< Real > , Data >* points );
        ~MemoryOrientedPointStreamWithData( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p , Data& d );
};

template< class Real >
class ASCIIOrientedPointStream : public OrientedPointStream< Real >
{
        FILE* _fp;
public:
        ASCIIOrientedPointStream( const char* fileName );
        ~ASCIIOrientedPointStream( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p );
};

template< class Real , class Data >
class ASCIIOrientedPointStreamWithData : public OrientedPointStreamWithData< Real , Data >
{
        FILE* _fp;
        Data (*_readData)( FILE* );
public:
        ASCIIOrientedPointStreamWithData( const char* fileName , Data (*readData)( FILE* ) );
        ~ASCIIOrientedPointStreamWithData( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p , Data& d );
};

template< class Real >
class BinaryOrientedPointStream : public OrientedPointStream< Real >
{
        FILE* _fp;
        static const int POINT_BUFFER_SIZE=1024;
        OrientedPoint3D< Real > _pointBuffer[ POINT_BUFFER_SIZE ];
        int _pointsInBuffer , _currentPointIndex;
public:
        BinaryOrientedPointStream( const char* filename );
        ~BinaryOrientedPointStream( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p );
};

template< class Real , class Data >
class BinaryOrientedPointStreamWithData : public OrientedPointStreamWithData< Real , Data >
{
        FILE* _fp;
        static const int POINT_BUFFER_SIZE=1024;
        std::pair< OrientedPoint3D< Real > , Data > _pointBuffer[ POINT_BUFFER_SIZE ];
        int _pointsInBuffer , _currentPointIndex;
public:
        BinaryOrientedPointStreamWithData( const char* filename );
        ~BinaryOrientedPointStreamWithData( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p , Data& d );
};

#include "PointStream.inl"
#endif // POINT_STREAM_INCLUDED
