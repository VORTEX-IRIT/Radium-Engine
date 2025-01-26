#include <IO/AsciiPCLoader/asciipcloader.hpp>

#include <Core/Asset/FileData.hpp>
#include <Core/Utils/Attribs.hpp> //logs
#include <fstream>
#include <iostream>

using std::ifstream;
using std::make_unique;
using std::runtime_error;
using std::to_string;

const string asciiExt( "txt" );

namespace Ra {
namespace IO {

vector<string> AsciiPointCloudLoader::getFileExtensions() const {
    return vector<string>( { "*." + asciiExt } );
}

bool AsciiPointCloudLoader::handleFileExtension( const string& extension ) const {
    return extension.compare( asciiExt ) == 0;
}

Ra::Core::Asset::FileData* AsciiPointCloudLoader::loadFile( const string& filename ) {
    using Ra::Core::Asset::FileData;
    using Ra::Core::Asset::GeometryData;
    using Ra::Core::Utils::logERROR;
    using Ra::Core::Utils::logINFO;

    FileData* fileData = new Ra::Core::Asset::FileData( filename );

    if ( !fileData->isInitialized() ) {
        delete fileData;
        LOG( logINFO ) << "Filedata could not be initialized";
        return nullptr;
    }

    auto startTime = clock();
    // a unique name is required by the component messaging system
    static int id = 0;
    auto geometry =
        make_unique<GeometryData>( "TRAJ_PC_" + to_string( ++id ), GeometryData::POINT_CLOUD );
    if ( geometry == nullptr ) {
        LOG( logERROR ) << "could not create geometry";
        return nullptr;
    }

    geometry->setFrame( Ra::Core::Transform::Identity() );

    ifstream stream( filename );
    if ( !stream.is_open() ) {
        throw runtime_error( "Could not open ifstream to path " + filename );
        return nullptr;
    }
    else {
        LOG( logINFO ) << "---Beginning file loading---";
    }

    /*reading how data is arranged*/
    string line;

    if ( !std::getline( stream, line ) ) {
        delete fileData;
        LOG( logINFO ) << "file does not contain ascii data format. aborting";
        return nullptr;
    }

    // retrieving all properties from header line
    std::stringstream words( line );
    vector<string> tokens;
    string temp;
    while ( words >> temp ) {
        tokens.emplace_back( temp );
    };

    // making sure X, Y, Z and Time properties are found
    int xpos = -1, ypos = -1, zpos = -1;
    for ( size_t i = 0; i < tokens.size(); ++i ) {
        string token = tokens[i];
        if ( xpos == -1 ) {
            if ( token.compare( "X" ) == 0 ) { xpos = i; }
        }
        if ( ypos == -1 ) {
            if ( token.compare( "Y" ) == 0 ) { ypos = i; }
        }
        if ( zpos == -1 ) {
            if ( token.compare( "Z" ) == 0 ) { zpos = i; }
        }
    }

    if ( ( xpos == -1 ) || ( ypos == -1 ) || ( zpos == -1 ) ) {
        delete fileData;
        LOG( logERROR ) << "file does not contain mandatory properties (X, Y, Z, Time). aborting";
        return nullptr;
    }

    Ra::Core::VectorArray<Ra::Core::Vector3>& vertices = geometry->getGeometry().verticesWithLock();

    // creating custom attribs for all other properties
    vector<Ra::Core::Utils::AttribHandle<Scalar>> attribs;
    attribs.reserve( tokens.size() - 3 );

    for ( size_t i = 0; i < tokens.size(); ++i ) {
        // skipping mandatory attribs
        if ( ( (int)i == xpos ) || ( (int)i == ypos ) || ( (int)i == zpos ) ) { continue; }
        attribs.emplace_back(
            geometry->getGeometry().vertexAttribs().addAttrib<Scalar>( tokens[i] ) );
    }

    /*recording data*/
    vector<double> pointRecord;
    int i = 0;
    double num;
    while ( stream >> num ) {
        pointRecord.emplace_back( num );

        if ( i == (int)( tokens.size() - 1 ) ) {
            vertices.emplace_back( Scalar( pointRecord[xpos] ),
                                   Scalar( pointRecord[ypos] ),
                                   Scalar( pointRecord[zpos] ) );
            size_t k = 0;
            for ( size_t j = 0; j < tokens.size(); ++j ) {
                if ( ( (int)j == xpos ) || ( (int)j == ypos ) || ( (int)j == zpos ) ) { continue; }
                geometry->getGeometry()
                    .vertexAttribs()
                    .getDataWithLock( attribs[k] )
                    .emplace_back( Scalar( pointRecord[j] ) );
                geometry->getGeometry().vertexAttribs().unlock( attribs[k] );
                ++k;
            }
            pointRecord.clear();
        }
        i = ( i + 1 ) % tokens.size();
    }
    stream.close();

    geometry->getGeometry().verticesUnlock();

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

string AsciiPointCloudLoader::name() const {
    return "Plain Text (ASCII)";
}

} // namespace IO
} // namespace Ra
