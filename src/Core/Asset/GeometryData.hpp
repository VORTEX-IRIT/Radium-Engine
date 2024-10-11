#pragma once

#include <Core/Asset/AssetData.hpp>
#include <Core/Asset/MaterialData.hpp>
#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/StandardAttribNames.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Attribs.hpp>
#include <Core/Utils/Index.hpp>

#include <memory>
#include <string>
#include <vector>

#include <algorithm> //std::transform

namespace Ra {
namespace Core {
namespace Asset {

class MaterialData;

/**
 * The GeometryData class stores all the geometry related data of a loaded object.
 */
class RA_CORE_API GeometryData : public AssetData
{
  public:
    using ColorArray = Vector4Array;

  public:
    /**
     * The type of geometry.
     */
    enum GeometryType {
        UNKNOWN     = 1 << 0,
        POINT_CLOUD = 1 << 1,
        LINE_MESH   = 1 << 2,
        TRI_MESH    = 1 << 3,
        QUAD_MESH   = 1 << 4,
        POLY_MESH   = 1 << 5,
        TETRA_MESH  = 1 << 6,
        HEX_MESH    = 1 << 7
    };

    GeometryData( const std::string& name = "", const GeometryType& type = UNKNOWN );

    GeometryData( const GeometryData& data ) = delete;

    ~GeometryData();

    /// \name Data access
    /// \{

    /// Return the name of the object.
    inline void setName( const std::string& name );

    /// Return the type of geometry.
    inline GeometryType getType() const;

    /// Set the type of geometry.
    inline void setType( const GeometryType& type );

    /// Return the Transform of the object.
    inline Transform getFrame() const;

    /// Set the Transform of the object.
    inline void setFrame( const Transform& frame );

    /// Return the MaterialData associated to the objet.
    inline const MaterialData& getMaterial() const;

    /// Set the MaterialData for the object.
    inline void setMaterial( MaterialData* material );

    /// Read/write access to the multiIndexedGeometry;
    inline Geometry::MultiIndexedGeometry& getGeometry();

    /// Read only access to the multiIndexedGeometry;
    inline const Geometry::MultiIndexedGeometry& getGeometry() const;

    /// \}

    /// \name Status queries
    /// \{

    /// Return true if the object is a Point Cloud.
    inline bool isPointCloud() const;

    /// Return true if the object is a Line Mesh.
    inline bool isLineMesh() const;

    /// Return true if the object is a Triangle Mesh.
    inline bool isTriMesh() const;

    /// Return true if the object is a Quadrangle Mesh.
    inline bool isQuadMesh() const;

    /// Return true if the object is a Polygon Mesh.
    /// \note Return false for Triangle and Quadrangle meshes.
    inline bool isPolyMesh() const;

    /// Return true if the object is a Tetrahedron Mesh.
    inline bool isTetraMesh() const;

    /// Return true if the object is a Hexahedron Mesh.
    inline bool isHexMesh() const;

    /// Return true if the object has lines.
    inline bool hasEdges() const;

    /// Return true if the object has faces.
    inline bool hasFaces() const;

    /// Return true if the object has polyhedra.
    inline bool hasPolyhedra() const;

    /// Return true if the object has MaterialData.
    inline bool hasMaterial() const;

    /// Used to track easily the number of primitives in the geometry data
    inline void setPrimitiveCount( int n );

    /// Return the number of primitives in the geometry data
    inline int getPrimitiveCount() const;

    /// \}

    /// Print stast info to the Debug output.
    void displayInfo() const;

  protected:
    /// The transformation of the object.
    Transform m_frame;

    /// The type of geometry for the object.
    GeometryType m_type;

    /// Named attributes
    Core::Geometry::MultiIndexedGeometry m_geometry;

    /// Simple tracking of geometric primitive number
    int m_primitiveCount { -1 };

    /// The MaterialData for the object.
    std::shared_ptr<MaterialData> m_material;
};

inline void GeometryData::setName( const std::string& name ) {
    m_name = name;
}

inline GeometryData::GeometryType GeometryData::getType() const {
    return m_type;
}

inline void GeometryData::setType( const GeometryType& type ) {
    m_type = type;
}

inline Transform GeometryData::getFrame() const {
    return m_frame;
}

inline void GeometryData::setFrame( const Transform& frame ) {
    m_frame = frame;
}

inline const MaterialData& GeometryData::getMaterial() const {
    return *( m_material.get() );
}

inline void GeometryData::setMaterial( MaterialData* material ) {
    m_material.reset( material );
}

inline bool GeometryData::isPointCloud() const {
    return ( m_type == POINT_CLOUD );
}

inline bool GeometryData::isLineMesh() const {
    return ( m_type == LINE_MESH );
}

inline bool GeometryData::isTriMesh() const {
    return ( m_type == TRI_MESH );
}

inline bool GeometryData::isQuadMesh() const {
    return ( m_type == QUAD_MESH );
}

inline bool GeometryData::isPolyMesh() const {
    return ( m_type == POLY_MESH );
}

inline bool GeometryData::isTetraMesh() const {
    return ( m_type == TETRA_MESH );
}

inline bool GeometryData::isHexMesh() const {
    return ( m_type == HEX_MESH );
}

inline bool GeometryData::hasEdges() const {
    return m_geometry.containsLayer( { Geometry::LineIndexLayer::staticSemanticName }, "indices" );
}

inline bool GeometryData::hasFaces() const {
    std::string layerSemanticName;
    switch ( m_type ) {
    case TRI_MESH:
        layerSemanticName = std::string( Geometry::TriangleIndexLayer::staticSemanticName );
        break;
    case QUAD_MESH:
        layerSemanticName = std::string( Geometry::QuadIndexLayer::staticSemanticName );
        break;
    case POLY_MESH:
        layerSemanticName = std::string( Geometry::PolyIndexLayer::staticSemanticName );
        break;
    default:
        return false;
    }
    return m_geometry.containsLayer( { layerSemanticName }, "indices" );
}

inline bool GeometryData::hasPolyhedra() const {
    return m_geometry.containsLayer( { Geometry::PolyIndexLayer::staticSemanticName }, "indices" );
}

inline bool GeometryData::hasMaterial() const {
    return m_material != nullptr;
}

const Geometry::MultiIndexedGeometry& GeometryData::getGeometry() const {
    return m_geometry;
}

Geometry::MultiIndexedGeometry& GeometryData::getGeometry() {
    return m_geometry;
}

inline void GeometryData::setPrimitiveCount( int n ) {
    m_primitiveCount = n;
}
inline int GeometryData::getPrimitiveCount() const {
    return m_primitiveCount;
}

} // namespace Asset
} // namespace Core
} // namespace Ra
