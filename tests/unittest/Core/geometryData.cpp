#include "Core/Geometry/StandardAttribNames.hpp"
#include <Core/Asset/GeometryData.hpp>
#include <catch2/catch.hpp>

TEST_CASE( "Core/Asset/GeometryData", "[Core][Core/Asset][GeometryData]" ) {

    using namespace Ra::Core::Asset;
    using namespace Ra::Core::Geometry;

    SECTION( "Normal test" ) {
        auto geom      = new GeometryData();
        auto& coreGeom = geom->getGeometry();
        auto& normal   = coreGeom.normalsWithLock();
        auto& name     = Ra::Core::Geometry::getAttribName( MeshAttrib::VERTEX_NORMAL );

        REQUIRE( normal.empty() );
        REQUIRE( coreGeom.vertexAttribs().getAttribBase( name )->isLocked() );

        normal.resize( 1, Ra::Core::Vector3::Zero() );
        normal[0] = Ra::Core::Vector3().setRandom();
        auto save = normal[0];
        coreGeom.normalsUnlock();

        REQUIRE( !normal.empty() );
        REQUIRE( !coreGeom.vertexAttribs().getAttribBase( name )->isLocked() );

        auto attriHandler = coreGeom.vertexAttribs().findAttrib<Ra::Core::Vector3>( name );
        const auto& data  = coreGeom.vertexAttribs().getData( attriHandler );
        REQUIRE( save == data[0] );

        auto geometry2  = new GeometryData();
        auto& coreGeom2 = geometry2->getGeometry();
        coreGeom2.setNormals( normal );
        auto attriHandler2 = coreGeom2.vertexAttribs().findAttrib<Ra::Core::Vector3>( name );
        const auto& d      = coreGeom2.vertexAttribs().getData( attriHandler2 );
        REQUIRE( d.size() == 1 );
        REQUIRE( d[0] == save );
    }

    SECTION( "Vertices test" ) {
        auto geom      = new GeometryData();
        auto& coreGeom = geom->getGeometry();
        auto& vertex   = coreGeom.verticesWithLock();
        auto& name     = Ra::Core::Geometry::getAttribName( MeshAttrib::VERTEX_POSITION );

        REQUIRE( vertex.empty() );
        REQUIRE( coreGeom.vertexAttribs().getAttribBase( name )->isLocked() );

        vertex.resize( 1, Ra::Core::Vector3::Zero() );
        vertex[0] = Ra::Core::Vector3().setRandom();
        auto save = vertex[0];
        coreGeom.verticesUnlock();

        REQUIRE( !vertex.empty() );
        REQUIRE( !coreGeom.vertexAttribs().getAttribBase( name )->isLocked() );

        auto attriHandle = coreGeom.vertexAttribs().findAttrib<Ra::Core::Vector3>( name );
        const auto& data = coreGeom.vertexAttribs().getData( attriHandle );
        REQUIRE( save == data[0] );

        auto geom2      = new GeometryData();
        auto& coreGeom2 = geom2->getGeometry();
        coreGeom2.setVertices( geom->getGeometry().vertices() );
        auto attriHandle2 = coreGeom2.vertexAttribs().findAttrib<Ra::Core::Vector3>( name );
        const auto& d     = coreGeom2.vertexAttribs().getData( attriHandle2 );
        REQUIRE( d.size() == 1 );
        REQUIRE( d[0] == save );
    }

    SECTION( "Tangent, BiTangent, TexCoord tests" ) {
        auto geom         = new GeometryData();
        auto& coreGeom    = geom->getGeometry();
        auto& name        = getAttribName( Ra::Core::Geometry::MeshAttrib::VERTEX_TANGENT );
        auto attribHandle = coreGeom.addAttrib<Ra::Core::Vector3>( name );
        auto& attribData  = coreGeom.vertexAttribs().getDataWithLock( attribHandle );

        REQUIRE( attribData.empty() );
        REQUIRE( coreGeom.vertexAttribs().getAttribBase( name )->isLocked() );

        attribData.resize( 1, Ra::Core::Vector3::Zero() );
        attribData[0] = Ra::Core::Vector3().setRandom();
        auto saveTan  = attribData[0];
        coreGeom.vertexAttribs().getAttribBase( name )->unlock();

        REQUIRE( !attribData.empty() );
        REQUIRE( !coreGeom.vertexAttribs().getAttribBase( name )->isLocked() );

        auto tangentAttribHandle2 = coreGeom.vertexAttribs().findAttrib<Ra::Core::Vector3>( name );
        const auto& dataTan       = coreGeom.vertexAttribs().getData( tangentAttribHandle2 );
        REQUIRE( saveTan == dataTan[0] );

        auto geom2         = new GeometryData();
        auto& coreGeom2    = geom2->getGeometry();
        auto attribHandle2 = coreGeom2.addAttrib<Ra::Core::Vector3>( name );
        coreGeom2.getAttrib<Ra::Core::Vector3>( attribHandle2 ).setData( dataTan );

        auto attribHandle3 = coreGeom2.vertexAttribs().findAttrib<Ra::Core::Vector3>( name );
        const auto& d      = coreGeom2.vertexAttribs().getData( attribHandle3 );

        REQUIRE( d.size() == 1 );
        REQUIRE( d[0] == saveTan );
    }

    SECTION( "Type test " ) {
        auto geometry = GeometryData();

        geometry.setType( GeometryData::GeometryType::LINE_MESH );
        REQUIRE( geometry.isLineMesh() );

        geometry.setType( GeometryData::GeometryType::TRI_MESH );
        REQUIRE( geometry.isTriMesh() );

        geometry.setType( GeometryData::GeometryType::QUAD_MESH );
        REQUIRE( geometry.isQuadMesh() );

        geometry.setType( GeometryData::GeometryType::POLY_MESH );
        REQUIRE( geometry.isPolyMesh() );

        geometry.setType( GeometryData::GeometryType::HEX_MESH );
        REQUIRE( geometry.isHexMesh() );

        geometry.setType( GeometryData::GeometryType::TETRA_MESH );
        REQUIRE( geometry.isTetraMesh() );

        geometry.setType( GeometryData::GeometryType::POINT_CLOUD );
        REQUIRE( geometry.isPointCloud() );

        geometry.setType( GeometryData::GeometryType::UNKNOWN );
        REQUIRE( !geometry.isLineMesh() );
        REQUIRE( !geometry.isTriMesh() );
        REQUIRE( !geometry.isQuadMesh() );
        REQUIRE( !geometry.isPolyMesh() );
        REQUIRE( !geometry.isHexMesh() );
        REQUIRE( !geometry.isTetraMesh() );
        REQUIRE( !geometry.isPointCloud() );
    }
}