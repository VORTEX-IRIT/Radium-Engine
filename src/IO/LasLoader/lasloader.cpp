#include <IO/LasLoader/lasloader.hpp>

#include <Core/Asset/FileData.hpp>
#include <fstream>
#include <iostream>

using std::ifstream;
using std::ios;
using std::make_unique;
using std::runtime_error;
using std::to_string;

const string lasExt( "las" );

namespace Ra {
namespace IO {

vector<string> LasLoader::getFileExtensions() const {
    return vector<string>( { "*." + lasExt } );
}

bool LasLoader::handleFileExtension( const string& extension ) const {
    return extension.compare( lasExt ) == 0;
}

double readDouble( const char* buffer ) {
    double res      = 0;
    char* converter = reinterpret_cast<char*>( &res );
    std::copy( buffer, buffer + sizeof( double ), converter );
    return res;
}

Ra::Core::Asset::FileData* LasLoader::loadFile( const string& filename ) {
    using Ra::Core::VectorArray;
    using Ra::Core::Asset::FileData;
    using Ra::Core::Asset::GeometryData;
    using Ra::Core::Utils::logERROR;
    using Ra::Core::Utils::logINFO;
    using Ra::Core::Utils::logWARNING;

    FileData* fileData = new Ra::Core::Asset::FileData( filename );

    if ( !fileData->isInitialized() ) {
        delete fileData;
        LOG( logWARNING ) << "Filedata could not be initialized";
        return nullptr;
    }

    auto startTime = clock();
    // a unique name is required by the component messaging system
    static int nameId = 0;
    auto geometry =
        make_unique<GeometryData>( "LAS_PC_" + to_string( ++nameId ), GeometryData::POINT_CLOUD );
    if ( geometry == nullptr ) {
        LOG( logERROR ) << "could not create geometry";
        return nullptr;
    }
    geometry->setFrame( Ra::Core::Transform::Identity() );

    ifstream stream( filename, ios::out | ios::binary );

    if ( !stream.is_open() ) {
        throw runtime_error( "Could not open binary ifstream to path " + filename );
        return nullptr;
    }
    else {
        LOG( logINFO ) << "---Beginning file loading---";
    }

    /*header part*/
    char buffer[48];

    // reading minor version (needed to check data format)
    stream.seekg( 0 );
    stream.seekg( 25 );
    if ( !stream.read( buffer, 1 ) ) {
        delete fileData;
        throw runtime_error( "Could not read minor version" );
        return nullptr;
    }
    unsigned char minor = *(unsigned char*)buffer;

    if ( ( minor < 1 ) || ( minor > 4 ) ) {
        delete fileData;
        LOG( logERROR ) << "LAS version 1." << (int)minor << " unsupported";
        return nullptr;
    }
    else {
        LOG( logINFO ) << "LAS version 1." << (int)minor;
    }

    // reading distance from file start to first point record
    stream.seekg( 0 );
    stream.seekg( 96 );
    if ( !stream.read( buffer, 4 ) ) {
        delete fileData;
        throw runtime_error( "Could not read offset to point records" );
        return nullptr;
    }
    unsigned int offset = *(unsigned int*)buffer;

    // reading point data format
    stream.seekg( 0 );
    stream.seekg( 104 );
    if ( !stream.read( buffer, 1 ) ) {
        delete fileData;
        throw runtime_error( "Could not read data format" );
        return nullptr;
    }
    unsigned char data_format = *(unsigned char*)buffer;

    if ( ( data_format > 3 && minor < 3 ) || ( data_format > 5 && minor == 3 ) ||
         ( data_format > 6 && minor == 4 ) ) {
        delete fileData;
        throw runtime_error( "Corrupted file. Unvalid data format" );
        return nullptr;
    }
    else {
        LOG( logINFO ) << "Data format " << (int)data_format;
        LOG( logINFO ) << "Loading properties \"x\", \"y\", \"z\",";
    }

    // reading data record length (bytes)
    if ( !stream.read( buffer, 2 ) ) {
        delete fileData;
        throw runtime_error( "Could not read point record length" );
        return nullptr;
    }
    unsigned short data_len = *(unsigned short*)buffer;

    // reading total number of data records
    if ( !stream.read( buffer, 4 ) ) {
        delete fileData;
        throw runtime_error( "Could not read number of point records" );
        return nullptr;
    }
    unsigned int nb_data = *(unsigned int*)buffer;

    // reading scales and offsets (used for computing coordinates)
    stream.seekg( 0 );
    stream.seekg( 131 );
    if ( !stream.read( buffer, 48 ) ) {
        delete fileData;
        throw runtime_error( "Could not read scale and offset fields" );
        return nullptr;
    }

    double scale_x  = readDouble( buffer );
    double scale_y  = readDouble( buffer + 8 );
    double scale_z  = readDouble( buffer + 16 );
    double offset_x = readDouble( buffer + 24 );
    double offset_y = readDouble( buffer + 32 );
    double offset_z = readDouble( buffer + 40 );

    /*loading properties*/
    vector<char> point( data_len );

    VectorArray<Ra::Core::Vector3>& vertices = geometry->getGeometry().verticesWithLock();
    vertices.reserve( nb_data );
    // checking for colors
    Ra::Core::Utils::AttribHandle<Ra::Core::Vector4> handle_color;
    Ra::Core::VectorArray<Ra::Core::Vector4> colors;
    if ( data_format == 2 || data_format == 3 || ( minor >= 3 && data_format == 5 ) ) {
        handle_color = geometry->getGeometry().vertexAttribs().addAttrib<Ra::Core::Vector4>(
            Ra::Core::Geometry::getAttribName( Ra::Core::Geometry::MeshAttrib::VERTEX_COLOR ) );
        LOG( logINFO ) << "\"red\", \"green\", \"blue\".";
    }

    // checking for GPS Time
    Ra::Core::Utils::AttribHandle<Scalar> handle_time;
    Ra::Core::VectorArray<Scalar> gps_time;
    if ( ( data_format == 1 ) || ( data_format == 3 ) ||
         ( ( minor > 2 ) && ( data_format == 4 ) ) || ( ( minor > 2 ) && ( data_format == 5 ) ) ||
         ( ( minor == 4 ) && ( data_format == 6 ) ) ) {
        handle_time = geometry->getGeometry().vertexAttribs().addAttrib<Scalar>( "gps_time" );
        LOG( logINFO ) << "\"GPS time\".";
    }

    for ( unsigned int i = 0; i < nb_data; ++i ) {
        // positioning at point data location in file
        int pos = offset + i * data_len;
        stream.seekg( 0 );
        stream.seekg( pos );

        // reading point data
        if ( !stream.read( point.data(), data_len ) ) {
            delete fileData;
            throw runtime_error( "Could not read point record " + to_string( i ) );
            return nullptr;
        }

        // extracting initial x y z coordinates
        int ix;
        int iy;
        int iz;

        ix = *(int*)( point.data() );
        iy = *(int*)( point.data() + 4 );
        iz = *(int*)( point.data() + 8 );

        // scaling to obtain actual coordinates
        double x = (double)ix * scale_x + offset_x;
        double y = (double)iy * scale_y + offset_y;
        double z = (double)iz * scale_z + offset_z;

        // storing coordinates in vertices object
        vertices.emplace_back( Scalar( x ), Scalar( y ), Scalar( z ) );

        // loading colors if found
        if ( handle_color.idx().isValid() ) {
            unsigned short red, green, blue;

            if ( data_format == 5 || data_format == 3 ) {
                red   = *(unsigned short*)( point.data() + 28 );
                green = *(unsigned short*)( point.data() + 30 );
                blue  = *(unsigned short*)( point.data() + 32 );
            }
            else {
                red   = *(unsigned short*)( point.data() + 20 );
                green = *(unsigned short*)( point.data() + 22 );
                blue  = *(unsigned short*)( point.data() + 24 );
            }

            colors.emplace_back( Scalar( red ) / 65536_ra,
                                 Scalar( green ) / 65536_ra,
                                 Scalar( blue ) / 65536_ra,
                                 1_ra );
        }

        if ( handle_time.idx().isValid() ) {
            // loading GPS time if found
            if ( data_format == 6 ) {
                gps_time.emplace_back( Scalar( readDouble( point.data() + 22 ) ) );
            }
            else {
                gps_time.emplace_back( Scalar( readDouble( point.data() + 20 ) ) );
            }
        }
    }
    stream.close();
    point.clear();

    geometry->getGeometry().verticesUnlock();

    if ( handle_color.idx().isValid() ) {
        geometry->getGeometry().vertexAttribs().getAttrib( handle_color ).setData( colors );
    }
    if ( handle_time.idx().isValid() ) {
        geometry->getGeometry().vertexAttribs().getAttrib( handle_time ).setData( gps_time );
    }

    // finalizing
    fileData->m_geometryData.clear();
    fileData->m_geometryData.reserve( 1 );
    fileData->m_geometryData.push_back( move( geometry ) );

    fileData->m_loadingTime = ( clock() - startTime ) / Scalar( CLOCKS_PER_SEC );

    LOG( logINFO ) << "---File loading ended---";

    fileData->displayInfo();

    fileData->m_processed = true;

    return fileData;
}

string LasLoader::name() const {
    return "LASer (.las)";
}

} // namespace IO
} // namespace Ra
