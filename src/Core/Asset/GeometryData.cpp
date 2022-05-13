#include <Core/Asset/GeometryData.hpp>

#include <Core/Utils/Log.hpp>

namespace Ra {
namespace Core {
namespace Asset {

GeometryData::GeometryData( const std::string& name, const GeometryType& type ) :
    AssetData( name ),
    m_frame( Transform::Identity() ),
    m_type( type ),
    m_multiIndexedGeometry(),
    m_material() {}

GeometryData::~GeometryData() {}

void GeometryData::displayInfo() const {
    using namespace Core::Utils; // log
    std::string type;
    switch ( m_type ) {
    case UNKNOWN:
        type = "UNKNOWN";
        break;
    case POINT_CLOUD:
        type = "POINT CLOUD";
        break;
    case LINE_MESH:
        type = "LINE MESH";
        break;
    case TRI_MESH:
        type = "TRIANGLE MESH";
        break;
    case QUAD_MESH:
        type = "QUAD MESH";
        break;
    case POLY_MESH:
        type = "POLY MESH";
        break;
    case TETRA_MESH:
        type = "TETRA MESH";
        break;
    case HEX_MESH:
        type = "HEX MESH";
        break;
    }

    LOG( logINFO ) << "======== MESH INFO ========";
    LOG( logINFO ) << " Name           : " << m_name;
    LOG( logINFO ) << " Type           : " << type;
    LOG( logINFO ) << " Vertex #       : " << getVerticesSize();
    LOG( logINFO ) << " Edge #         : " << hasEdges();
    LOG( logINFO ) << " Face #         : " << hasFaces();
    LOG( logINFO ) << " Normal ?       : " << ( ( !hasNormals() ) ? "NO" : "YES" );
    LOG( logINFO ) << " Tangent ?      : " << ( ( !hasTangents() ) ? "NO" : "YES" );
    LOG( logINFO ) << " Bitangent ?    : " << ( ( !hasBiTangents() ) ? "NO" : "YES" );
    LOG( logINFO ) << " Tex.Coord. ?   : " << ( ( !hasTextureCoordinates() ) ? "NO" : "YES" );
    LOG( logINFO ) << " Material ?     : " << ( ( !hasMaterial() ) ? "NO" : "YES" );

    if ( hasMaterial() ) { m_material->displayInfo(); }
}
} // namespace Asset
} // namespace Core
} // namespace Ra
