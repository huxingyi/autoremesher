
////////////////////////////
// PLYOrientedPointStream //
////////////////////////////
template< class Real >
PLYOrientedPointStream< Real >::PLYOrientedPointStream( const char* fileName )
{
        _fileName = new char[ strlen( fileName )+1 ];
        strcpy( _fileName , fileName );
        _ply = NULL;
        reset();
}
template< class Real >
void PLYOrientedPointStream< Real >::reset( void )
{
        int fileType;
        float version;
        PlyProperty** plist;
        if( _ply ) _free();
        _ply = ply_open_for_reading( _fileName, &_nr_elems, &_elist, &fileType, &version );
        if( !_ply )
        {
                fprintf( stderr, "[ERROR] Failed to open ply file for reading: %s\n" , _fileName );
                exit( 0 );
        }
        bool foundVertices = false;
        for( int i=0 ; i<_nr_elems ; i++ )
        {
                int num_elems;
                int nr_props;
                char* elem_name = _elist[i];
                plist = ply_get_element_description( _ply , elem_name , &num_elems , &nr_props );
                if( !plist )
                {
                        fprintf( stderr , "[ERROR] Failed to get element description: %s\n" , elem_name );
                        exit( 0 );
                }       

                if( equal_strings( "vertex" , elem_name ) )
                {
                        foundVertices = true;
                        _pCount = num_elems , _pIdx = 0;
                        for( int i=0 ; i<PlyOrientedVertex< Real >::ReadComponents ; i++ ) 
                                if( !ply_get_property( _ply , elem_name , &(PlyOrientedVertex< Real >::ReadProperties[i]) ) )
                                {
                                        fprintf( stderr , "[ERROR] Failed to find property in ply file: %s\n" , PlyOrientedVertex< Real >::ReadProperties[i].name );
                                        exit( 0 );
                                }
                }
                for( int j=0 ; j<nr_props ; j++ )
                {
                        free( plist[j] );
                }
                free( plist );
                if( foundVertices ) break;
        }
        if( !foundVertices )
        {
                fprintf( stderr , "[ERROR] Could not find vertices in ply file\n" );
                exit( 0 );
        }
}
template< class Real >
void PLYOrientedPointStream< Real >::_free( void )
{
        if( _ply ) ply_close( _ply ) , _ply = NULL;
        if( _elist )
        {
                for( int i=0 ; i<_nr_elems ; i++ ) free( _elist[i] );
                free( _elist );
        }
}
template< class Real >
PLYOrientedPointStream< Real >::~PLYOrientedPointStream( void )
{
        _free();
        if( _fileName ) delete[] _fileName , _fileName = NULL;
}
template< class Real >
bool PLYOrientedPointStream< Real >::nextPoint( OrientedPoint3D< Real >& p )
{
        if( _pIdx<_pCount )
        {
                PlyOrientedVertex< Real > op;
                ply_get_element( _ply, (void *)&op );
                p.p = op.point;
                p.n = op.normal;
                _pIdx++;
                return true;
        }
        else return false;
}


////////////////////////////////////
// PLYOrientedPointStreamWithData //
////////////////////////////////////

template< class Real , class Data >
PLYOrientedPointStreamWithData< Real , Data >::PLYOrientedPointStreamWithData( const char* fileName , const PlyProperty* dataProperties , int dataPropertiesCount , bool (*validationFunction)( const bool* ) ) : _dataPropertiesCount( dataPropertiesCount ) , _validationFunction( validationFunction )
{
        _dataProperties = new PlyProperty[ _dataPropertiesCount ];
        memcpy( _dataProperties , dataProperties , sizeof(PlyProperty) * _dataPropertiesCount );
        for( int i=0 ; i<_dataPropertiesCount ; i++ ) _dataProperties[i].offset += sizeof( PlyOrientedVertex< Real > );
        _fileName = new char[ strlen( fileName )+1 ];
        strcpy( _fileName , fileName );
        _ply = NULL;
        reset();
}
template< class Real , class Data >
void PLYOrientedPointStreamWithData< Real , Data >::reset( void )
{
        int fileType;
        float version;
        PlyProperty** plist;
        if( _ply ) _free();
        _ply = ply_open_for_reading( _fileName, &_nr_elems, &_elist, &fileType, &version );
        if( !_ply )
        {
                fprintf( stderr, "[ERROR] Failed to open ply file for reading: %s\n" , _fileName );
                exit( 0 );
        }
        bool foundVertices = false;
        for( int i=0 ; i<_nr_elems ; i++ )
        {
                int num_elems;
                int nr_props;
                char* elem_name = _elist[i];
                plist = ply_get_element_description( _ply , elem_name , &num_elems , &nr_props );
                if( !plist )
                {
                        fprintf( stderr , "[ERROR] Failed to get element description: %s\n" , elem_name );
                        exit( 0 );
                }       

                if( equal_strings( "vertex" , elem_name ) )
                {
                        foundVertices = true;
                        _pCount = num_elems , _pIdx = 0;
                        for( int i=0 ; i<PlyOrientedVertex< Real >::ReadComponents ; i++ ) 
                                if( !ply_get_property( _ply , elem_name , &(PlyOrientedVertex< Real >::ReadProperties[i]) ) )
                                {
                                        fprintf( stderr , "[ERROR] Failed to find property in ply file: %s\n" , PlyOrientedVertex< Real >::ReadProperties[i].name );
                                        exit( 0 );
                                }
                        if( _validationFunction )
                        {
                                bool* properties = new bool[_dataPropertiesCount];
                                for( int i=0 ; i<_dataPropertiesCount ; i++ )
                                        if( !ply_get_property( _ply , elem_name , &(_dataProperties[i]) ) ) properties[i] = false;
                                        else                                                                properties[i] = true;
                                bool valid = _validationFunction( properties );
                                delete[] properties;
                                if( !valid ) fprintf( stderr , "[ERROR] Failed to validate properties in file\n" ) , exit( 0 );
                        }
                        else
                        {
                                for( int i=0 ; i<_dataPropertiesCount ; i++ )
                                        if( !ply_get_property( _ply , elem_name , &(_dataProperties[i]) ) )
                                                fprintf( stderr , "[WARNING] Failed to find property in ply file: %s\n" , _dataProperties[i].name );
                        }
                }
                for( int j=0 ; j<nr_props ; j++ )
                {
                        free( plist[j] );
                }
                free( plist );
                if( foundVertices ) break;
        }
        if( !foundVertices )
        {
                fprintf( stderr , "[ERROR] Could not find vertices in ply file\n" );
                exit( 0 );
        }
}
template< class Real , class Data >
void PLYOrientedPointStreamWithData< Real , Data >::_free( void )
{
        if( _ply ) ply_close( _ply ) , _ply = NULL;
        if( _elist )
        {
                for( int i=0 ; i<_nr_elems ; i++ ) free( _elist[i] );
                free( _elist );
        }
}
template< class Real , class Data >
PLYOrientedPointStreamWithData< Real , Data >::~PLYOrientedPointStreamWithData( void )
{
        _free();
        if( _fileName ) delete[] _fileName , _fileName = NULL;
        if( _dataProperties ) delete[] _dataProperties , _dataProperties = NULL;
}
template< class Real , class Data >
bool PLYOrientedPointStreamWithData< Real , Data >::nextPoint( OrientedPoint3D< Real >& p , Data& d )
{
        if( _pIdx<_pCount )
        {
                _PlyOrientedVertexWithData op;
                ply_get_element( _ply, (void *)&op );
                p.p = op.point;
                p.n = op.normal;
                d = op.data;
                _pIdx++;
                return true;
        }
        else return false;
}
