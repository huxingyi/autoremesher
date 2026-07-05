
template< class Real >
class PLYOrientedPointStream : public OrientedPointStream< Real >
{
        char* _fileName;
        PlyFile* _ply;
        int _nr_elems;
        char **_elist;

        int _pCount , _pIdx;
        void _free( void );
public:
        PLYOrientedPointStream( const char* fileName );
        ~PLYOrientedPointStream( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p );
};

template< class Real , class Data >
class PLYOrientedPointStreamWithData : public OrientedPointStreamWithData< Real , Data >
{
        struct _PlyOrientedVertexWithData : public PlyOrientedVertex< Real > { Data data; };
        char* _fileName;
        PlyFile* _ply;
        int _nr_elems;
        char **_elist;
        PlyProperty* _dataProperties;
        int _dataPropertiesCount;
        bool (*_validationFunction)( const bool* );

        int _pCount , _pIdx;
        void _free( void );
public:
        PLYOrientedPointStreamWithData( const char* fileName , const PlyProperty* dataProperties , int dataPropertiesCount , bool (*validationFunction)( const bool* )=NULL );
        ~PLYOrientedPointStreamWithData( void );
        void reset( void );
        bool nextPoint( OrientedPoint3D< Real >& p , Data& d );
};


#include "PlyPointStream.inl"

