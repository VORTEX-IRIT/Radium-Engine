#pragma once

#include <Core/Geometry/IndexedGeometry.hpp>
#include <Core/Geometry/TopologicalMesh.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <Core/Math/LinearAlgebra.hpp>
#include <Core/RaCore.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/StdOptional.hpp>

#include <random>

namespace Ra {
namespace Core {
namespace Geometry {
//
// Primitive construction
//

/// Create a 2D grid mesh with given number of row and columns
/// \param rows: number of rows
/// \param cols: number of columns
/// \param halfExts: from center of the grid to each corner, in 2D, before applying T
/// \param T: apply T to the generated vertices
/// \param color: if set, colorize vertices
/// \param generateTexCoord: create uv tex coordinates on the grid, -halfExts have uv = (0,0),
/// +halfExts have uv=(1,1)
RA_CORE_API TriangleMesh makePlaneGrid( const uint rows         = 1,
                                        const uint cols         = 1,
                                        const Vector2& halfExts = Vector2( .5_ra, .5_ra ),
                                        const Transform& T      = Transform::Identity(),
                                        const Utils::optional<Utils::Color>& color = {},
                                        bool generateTexCoord                      = false );

/// Create a 2D quad mesh given half extents, centered on the origin with x axis as normal
/// \see makePlaneGrid
RA_CORE_API TriangleMesh makeXNormalQuad( const Vector2& halfExts = Vector2( .5_ra, .5_ra ),
                                          const Utils::optional<Utils::Color>& color = {},
                                          bool generateTexCoord                      = false );

/// Create a 2D quad mesh given half extents, centered on the origin with y axis as normal
/// \see makePlaneGrid
RA_CORE_API TriangleMesh makeYNormalQuad( const Vector2& halfExts = Vector2( .5_ra, .5_ra ),
                                          const Utils::optional<Utils::Color>& color = {},
                                          bool generateTexCoord                      = false );

/// Create a 2D quad mesh given half extents, centered on the origin with z axis as normal
/// \see makePlaneGrid
RA_CORE_API TriangleMesh makeZNormalQuad( const Vector2& halfExts = Vector2( .5_ra, .5_ra ),
                                          const Utils::optional<Utils::Color>& color = {},
                                          bool generateTexCoord                      = false );

/// Create an axis-aligned cubic mesh with the given half extents, centered on the origin.
RA_CORE_API TriangleMesh makeBox( const Vector3& halfExts = Vector3( .5_ra, .5_ra, .5_ra ),
                                  const Utils::optional<Utils::Color>& color = {} );

/// Create an axis-aligned cubic mesh
RA_CORE_API TriangleMesh makeBox( const Aabb& aabb,
                                  const Utils::optional<Utils::Color>& color = {} );

/// Create an axis-aligned cubic mesh with the given half extents, centered on the origin.
RA_CORE_API TriangleMesh makeSharpBox( const Vector3& halfExts = Vector3( .5_ra, .5_ra, .5_ra ),
                                       const Utils::optional<Utils::Color>& color = {},
                                       bool generateTexCoord                      = false );

/// Create an axis-aligned cubic mesh
RA_CORE_API TriangleMesh makeSharpBox( const Aabb& aabb,
                                       const Utils::optional<Utils::Color>& color = {},
                                       bool generateTexCoord                      = false );

/// Create an axis-aligned cubic mesh with the given half extents, centered on the origin.
RA_CORE_API MultiIndexedGeometry makeSharpBox2( const Vector3& halfExts = Vector3( .5_ra,
                                                                                   .5_ra,
                                                                                   .5_ra ),
                                                const Utils::optional<Utils::Color>& color = {},
                                                bool generateTexCoord = false );

/// Create an axis-aligned cubic mesh
RA_CORE_API MultiIndexedGeometry makeSharpBox2( const Aabb& aabb,
                                                const Utils::optional<Utils::Color>& color = {},
                                                bool generateTexCoord = false );

/// Create a parametric spherical mesh of given radius. Template parameters set the resolution.
/// \param generateTexCoord: maps parametric (u,v) to texture corrdinates [0,1]^2
template <uint U = 16, uint V = U>
TriangleMesh makeParametricSphere( Scalar radius                              = 1_ra,
                                   const Utils::optional<Utils::Color>& color = {},
                                   bool generateTexCoord                      = false );

/// Create a parametric torus mesh. The minor radius is the radius of the inside of the tube and the
/// major radius is the radius of the whole torus. The torus will be centered at the origin and have
/// Z as rotation axis. Template parameters set the resolution of the mesh.
/// \param generateTexCoord: maps parametric (u,v) to texture corrdinates [0,1]^2
template <uint U = 16, uint V = U>
TriangleMesh makeParametricTorus( Scalar majorRadius,
                                  Scalar minorRadius,
                                  const Utils::optional<Utils::Color>& color = {},
                                  bool generateTexCoord                      = false );

/// Create a spherical mesh by subdivision of an icosahedron.
RA_CORE_API TriangleMesh makeGeodesicSphere( Scalar radius                              = 1_ra,
                                             uint numSubdiv                             = 3,
                                             const Utils::optional<Utils::Color>& color = {} );

/// Create a cylinder approximation (sideSegments-faced prism) with base faces centered on a and b
/// with given radius.
/// Fill (the tube part) is split into equally sapced fill segments.
/// Side and fill make a sharp edge.
RA_CORE_API TriangleMesh makeCylinder( const Vector3& a,
                                       const Vector3& b,
                                       Scalar radius,
                                       uint sideSegments                          = 32,
                                       uint fillSegments                          = 2,
                                       const Utils::optional<Utils::Color>& color = {} );

/// Create a capsule with given cylinder length and radius.
/// Total length is length + 2*radius
/// The capsule is along z axis
RA_CORE_API TriangleMesh makeCapsule( Scalar length,
                                      Scalar radius,
                                      uint nFaces                                = 32,
                                      const Utils::optional<Utils::Color>& color = {} );

/// Create a tube (empty cylinder) delimited by two radii, with bases centered on A and B.
/// Outer radius must be larger than inner radius.
RA_CORE_API TriangleMesh makeTube( const Vector3& a,
                                   const Vector3& b,
                                   Scalar outerRadius,
                                   Scalar InnerRadius,
                                   uint nFaces                                = 32,
                                   const Utils::optional<Utils::Color>& color = {} );

/// Create a cone approximation (n-faced pyramid) with base face centered on base, pointing
/// towards tip with given base radius.
RA_CORE_API TriangleMesh makeCone( const Vector3& base,
                                   const Vector3& tip,
                                   Scalar radius,
                                   uint nFaces                                = 32,
                                   const Utils::optional<Utils::Color>& color = {} );

template <uint SLICES, uint STACKS>
TriangleMesh
makeParametricSphere( Scalar radius, const Utils::optional<Utils::Color>& color, bool gtc ) {

    const Scalar du = 1_ra / SLICES;
    const Scalar dv = 1_ra / STACKS;
    using WAI       = TopologicalMesh::WedgeAttribIndex;
    TopologicalMesh topoMesh;
    auto whNormal = topoMesh.addWedgeAttrib<Vector3>( getAttribName( MeshAttrib::VERTEX_NORMAL ) );

    WAI whColor =
        color ? topoMesh.addWedgeAttrib<Vector4>( getAttribName( MeshAttrib::VERTEX_COLOR ) )
              : WAI::Invalid();
    WAI whTexCoord =
        gtc ? topoMesh.addWedgeAttrib<Vector3>( getAttribName( MeshAttrib::VERTEX_TEXCOORD ) )
            : WAI::Invalid();

    TopologicalMesh::VertexHandle vhandles[( STACKS - 1 ) * SLICES + 2];
    Vector3Array topoTexCoords;
    topoTexCoords.reserve( ( STACKS - 1 ) * SLICES + 2 + 2 * SLICES );
    Vector3Array topoNormals;
    topoNormals.reserve( ( STACKS - 1 ) * SLICES + 2 );

    uint index = 0;
    // check https://en.wikipedia.org/wiki/Spherical_coordinate_system
    // theta \in [0, pi]
    // phi \in [0, 2pi]
    const Scalar uFactor = 2_ra / Scalar { SLICES } * Core::Math::Pi;
    const Scalar vFactor = Core::Math::Pi / STACKS;
    for ( uint u = 0; u < SLICES; ++u ) {
        const Scalar phi = u * uFactor;
        for ( uint v = 1; v < STACKS; ++v ) {
            // Regular vertices on the sphere.
            const Scalar theta = v * vFactor;
            Vector3 p          = Vector3( radius * std::cos( phi ) * std::sin( theta ),
                                 radius * std::sin( phi ) * std::sin( theta ),
                                 radius * std::cos( theta ) );
            vhandles[index++]  = topoMesh.add_vertex( p );
            topoNormals.push_back( p.normalized() );
            topoTexCoords.emplace_back( 1_ra - u * du, v * dv, 0_ra );
        }
    }

    // Add the pole vertices.
    auto topoNorthPoleIdx      = ( STACKS - 1 ) * SLICES;
    auto topoSouthPoleIdx      = topoNorthPoleIdx + 1;
    vhandles[topoNorthPoleIdx] = topoMesh.add_vertex( Vector3( 0, 0, radius ) );
    vhandles[topoSouthPoleIdx] = topoMesh.add_vertex( Vector3( 0, 0, -radius ) );

    // Set wedge for fh, vhIndex to n and t.
    auto wedgeSetter = [&topoMesh, &vhandles, color, whNormal, whTexCoord, whColor, gtc](
                           int vhIndex, TopologicalMesh::FaceHandle fh, Vector3 n, Vector3 t ) {
        auto heh                     = topoMesh.halfedge_handle( vhandles[vhIndex], fh );
        auto wd                      = topoMesh.newWedgeData( heh );
        wd.m_vector3Attrib[whNormal] = n;
        if ( gtc ) wd.m_vector3Attrib[whTexCoord] = t;
        if ( color ) wd.m_vector4Attrib[whColor] = *color;
        topoMesh.replaceWedge( heh, wd );
    };

    // For general vertices retrieve normals and texCoords from vhIndex
    auto wedgeSetterGeneric = [wedgeSetter, &topoNormals, &topoTexCoords, whNormal, whTexCoord](
                                  int vhIndex, TopologicalMesh::FaceHandle fh ) {
        wedgeSetter( vhIndex, fh, topoNormals[vhIndex], topoTexCoords[vhIndex] );
    };

    // take seams into account when u =1
    auto wedgeSetterSeam = [wedgeSetter, &topoTexCoords, &topoNormals, whNormal, whTexCoord](
                               int u, int vhIndex, TopologicalMesh::FaceHandle fh ) {
        Vector3 t = topoTexCoords[vhIndex];
        if ( u == SLICES - 1 ) t[0] = 0_ra;
        wedgeSetter( vhIndex, fh, topoNormals[vhIndex], t );
    };

    // special for poles
    auto wedgeSetterPole =
        [wedgeSetter, &topoTexCoords, whNormal, whTexCoord]( bool north,
                                                             int id,
                                                             int baseSlice,
                                                             int nextSlice,
                                                             int u,
                                                             TopologicalMesh::FaceHandle fh ) {
            // pole vertex use "midpoint" texCoord
            Scalar bu = topoTexCoords[baseSlice][0];
            Scalar nu = ( u == SLICES - 1 ) ? 0_ra : topoTexCoords[nextSlice][0];
            Scalar tu = ( bu + nu ) * .5_ra;
            wedgeSetter( id, fh, Vector3( 0, 0, north ? 1 : -1 ), Vector3( tu, north ? 0 : 1, 0 ) );
        };

    for ( uint u = 0; u < SLICES; ++u ) {
        for ( uint v = 2; v < STACKS; ++v ) {
            const uint nextSlice = ( ( u + 1 ) % SLICES ) * ( STACKS - 1 );
            const uint baseSlice = u * ( STACKS - 1 );
            std::vector vindices = { baseSlice + v - 2,
                                     baseSlice + v - 1,
                                     nextSlice + v - 1,
                                     baseSlice + v - 2,
                                     nextSlice + v - 1,
                                     nextSlice + v - 2 };

            auto fh1 = topoMesh.add_face(
                { vhandles[vindices[0]], vhandles[vindices[1]], vhandles[vindices[2]] } );
            auto fh2 = topoMesh.add_face(
                { vhandles[vindices[3]], vhandles[vindices[4]], vhandles[vindices[5]] } );

            wedgeSetterGeneric( baseSlice + v - 2, fh1 );
            wedgeSetterGeneric( baseSlice + v - 1, fh1 );
            wedgeSetterGeneric( baseSlice + v - 2, fh2 );

            wedgeSetterSeam( u, nextSlice + v - 1, fh1 );
            wedgeSetterSeam( u, nextSlice + v - 1, fh2 );
            wedgeSetterSeam( u, nextSlice + v - 2, fh2 );
        }
    }
    // caps faces
    for ( uint u = 0; u < SLICES; ++u ) {
        const uint nextSlice = ( ( u + 1 ) % SLICES ) * ( STACKS - 1 );
        const uint baseSlice = u * ( STACKS - 1 );

        auto fh1 = topoMesh.add_face(
            { vhandles[topoNorthPoleIdx], vhandles[baseSlice], vhandles[nextSlice] } );
        auto fh2 = topoMesh.add_face( { vhandles[topoSouthPoleIdx],
                                        vhandles[nextSlice + STACKS - 2],
                                        vhandles[baseSlice + STACKS - 2] } );

        wedgeSetterGeneric( baseSlice, fh1 );
        wedgeSetterGeneric( baseSlice + STACKS - 2, fh2 );
        wedgeSetterSeam( u, nextSlice, fh1 );
        wedgeSetterSeam( u, nextSlice + STACKS - 2, fh2 );

        // pole vertex use "midpoint" texCoord
        wedgeSetterPole( true, topoNorthPoleIdx, baseSlice, nextSlice, u, fh1 );
        wedgeSetterPole( false, topoSouthPoleIdx, baseSlice, nextSlice, u, fh2 );
    }

    topoMesh.mergeEqualWedges();
    topoMesh.garbage_collection();
    TriangleMesh result = topoMesh.toTriangleMesh();
    result.checkConsistency();
    return result;
}

template <uint U, uint V>
TriangleMesh makeParametricTorus( Scalar majorRadius,
                                  Scalar minorRadius,
                                  const Utils::optional<Utils::Color>& color,
                                  bool generateTexCoord ) {
    TriangleMesh result;
    TriangleMesh::PointAttribHandle::Container vertices;
    TriangleMesh::NormalAttribHandle::Container normals;
    TriangleMesh::IndexContainerType indices;
    Ra::Core::Vector3Array texCoords;

    vertices.reserve( ( U + 1 ) * ( V + 1 ) );
    normals.reserve( ( U + 1 ) * ( V + 1 ) );
    indices.reserve( 2 * U * V );
    texCoords.reserve( ( U + 1 ) * ( V + 1 ) );

    const Scalar du = 1_ra / U;
    const Scalar dv = 1_ra / V;

    for ( uint iu = 0; iu <= U; ++iu ) {
        Scalar u = Scalar( iu ) * Core::Math::PiMul2 / Scalar( U );
        Core::Vector3 circleCenter( majorRadius * std::cos( u ), majorRadius * std::sin( u ), 0.f );

        for ( uint iv = 0; iv <= V; ++iv ) {
            Scalar v = Scalar( iv ) * Core::Math::PiMul2 / Scalar( V );

            Core::Vector3 vertex( ( majorRadius + minorRadius * std::cos( v ) ) * std::cos( u ),
                                  ( majorRadius + minorRadius * std::cos( v ) ) * std::sin( u ),
                                  minorRadius * std::sin( v ) );

            vertices.push_back( vertex );
            normals.push_back( ( vertex - circleCenter ).normalized() );
            texCoords.emplace_back( iu * du, iv * dv, 0_ra );

            if ( iu != U && iv != V ) {
                indices.push_back( Vector3ui( iu * ( V + 1 ) + iv,
                                              ( iu + 1 ) * ( V + 1 ) + iv,
                                              iu * ( V + 1 ) + ( iv + 1 ) ) );
                indices.push_back( Vector3ui( ( iu + 1 ) * ( V + 1 ) + iv,
                                              ( iu + 1 ) * ( V + 1 ) + ( iv + 1 ),
                                              iu * ( V + 1 ) + ( iv + 1 ) ) );
            }
        }
    }

    result.setVertices( std::move( vertices ) );
    result.setNormals( std::move( normals ) );
    result.setIndices( std::move( indices ) );
    if ( generateTexCoord )
        result.addAttrib( getAttribName( MeshAttrib::VERTEX_TEXCOORD ), std::move( texCoords ) );
    if ( color ) result.colorize( *color );
    result.checkConsistency();

    return result;
}
} // namespace Geometry
} // namespace Core
} // namespace Ra
