#pragma once

#include <Core/Asset/AssetData.hpp>
#include <Core/Containers/AlignedStdVector.hpp>
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Log.hpp>

#include <map>
#include <set>
#include <string>
#include <vector>

namespace Ra {
namespace Core {
namespace Asset {

/**
 * A HandleComponentData stores the data of an animation Handle linked to an object.
 */
struct RA_CORE_API HandleComponentData {

    HandleComponentData();

    /// Handle name.
    std::string m_name;

    /// Handle transformation in model space.
    Core::Transform m_frame { Core::Transform::Identity() };

    /// Per skinned-mesh matrix from mesh space to bone space (local).
    std::map<std::string, Core::Transform> m_bindMatrices;

    /// Per skinned-mesh vertex weigths.
    std::map<std::string, std::vector<std::pair<uint, Scalar>>> m_weights;
};

/**
 * The HandleData class stores all the HandleComponentData linked to an object.
 */
class RA_CORE_API HandleData : public AssetData
{
  public:
    /**
     * The type of Handle system.
     */
    enum HandleType { UNKNOWN = 1 << 0, POINT_CLOUD = 1 << 1, SKELETON = 1 << 2, CAGE = 1 << 3 };

    HandleData( const std::string& name = "", const HandleType& type = UNKNOWN );

    HandleData( const HandleData& data ) = default;

    ~HandleData();

    /// \name HandleArray
    /// \{

    /**
     * Set the name of the Handle system.
     */
    inline void setName( const std::string& name );

    /**
     * Return the type of the Handle system.
     */
    inline HandleType getType() const;

    /**
     * Set the type of the Handle system.
     */
    inline void setType( const HandleType& type );

    /**
     * Return the transformation of the Handle system.
     */
    inline Core::Transform getFrame() const;

    /**
     * Set the transformation of the Handle system.
     */
    inline void setFrame( const Core::Transform& frame );

    /**
     * Add \p name to the list of bound mesh names.
     */
    inline void addBindMesh( const std::string& name );

    /**
     * Returns the list of bound mesh names.
     */
    inline const std::set<std::string>& getBindMeshes() const;
    /// \}

    /**
     * Return the maximal number of vertices influenced by a Handle.
     */
    inline uint getVertexSize() const;

    /**
     * Set the maximal number of vertices influenced by a Handle.
     */
    inline void setVertexSize( uint size );

    /**
     * Set the map from Handle names to Handle storage index.
     */
    inline void setNameTable( const std::map<std::string, uint>& nameTable );

    /**
     * Recompute the map from Handle names to Handle storage index.
     */
    inline void recomputeAllIndices();

    /// \name Data access
    /// \{

    /**
     * Return the number of Handles in the system.
     */
    inline uint getComponentDataSize() const;

    /**
     * Return the list of HandleComponentData.
     */
    inline const Core::AlignedStdVector<HandleComponentData>& getComponentData() const;

    /**
     * Return the list of HandleComponentData.
     */
    inline Core::AlignedStdVector<HandleComponentData>& getComponentData();

    /**
     * Return the \p i-th HandleComponentData.
     */
    inline const HandleComponentData& getComponent( const uint i ) const;

    /**
     * Return the \p i-th HandleComponentData.
     */
    inline HandleComponentData& getComponent( const uint i );

    /**
     * Set the HandleComponentData for the Handle system.
     */
    inline void setComponents( const Core::AlignedStdVector<HandleComponentData>& components );

    /**
     * Return the HandleArray hierarchy, i.e.\ bones hierarchy.
     */
    inline const Core::AlignedStdVector<Core::Vector2ui>& getEdgeData() const;

    /**
     * Return the HandleArray hierarchy, i.e.\ bones hierarchy.
     */
    inline Core::AlignedStdVector<Core::Vector2ui>& getEdgeData();

    /**
     * Set the HandleArray linear hierarchy part, i.e.\ bones hierarchy.
     */
    inline void setEdges( const Core::AlignedStdVector<Core::Vector2ui>& edgeList );

    /**
     * Return the HandleArray N-Dimensional parts, i.e.\ cage polyhedra.
     */
    inline const Core::AlignedStdVector<Core::VectorNui>& getFaceData() const;

    /**
     * Return the HandleArray N-Dimensional parts, i.e.\ cage polyhedra.
     */
    inline Core::AlignedStdVector<Core::VectorNui>& getFaceData();

    /**
     * Set the HandleArray N-Dimensional parts, i.e.\ cage polyhedra.
     */
    inline void setFaces( const Core::AlignedStdVector<Core::VectorNui>& faceList );

    /**
     * Set whether the Handle system needs end bones.
     */
    inline void needEndNodes( bool need );
    /// \}

    /// \name Status querries
    /// \{

    /**
     * Return true if the Handle system is a Point Cloud.
     */
    inline bool isPointCloud() const;

    /**
     * Return true if the Handle system is a Skeleton.
     */
    inline bool isSkeleton() const;

    /**
     * Return true if the Handle system is a Cage.
     */
    inline bool isCage() const;

    /**
     * Return true if the Handle system has Handles.
     */
    inline bool hasComponents() const;

    /**
     * Return true if the Handle system has a hierarchy.
     */
    inline bool hasEdges() const;

    /**
     * Return true if the Handle system has N-Dimensional parts.
     */
    inline bool hasFaces() const;

    /**
     * Return true if the Handle system needs end bones.
     */
    inline bool needsEndNodes() const;

    /**
     * Return the storage index of the Handle with the given name
     * if it exists, -1 otherwise.
     */
    inline int getIndexOf( const std::string& name ) const;
    /// \}

    /**
     * Print stat info to the Debug output.
     */
    inline void displayInfo() const;

  private:
    /// The type of the Handle system.
    HandleType m_type;

    /// The transformation of the Handle system.
    Core::Transform m_frame { Core::Transform::Identity() };

    /// Whether the Handle System needs end bones.
    bool m_endNode { false };

    /// The maximal number of vertices influenced by a Handle of the system.
    uint m_vertexSize { 0 };

    /// The map from Handle name to storage index.
    std::map<std::string, uint> m_nameTable;

    /// The list of bound mesh names.
    std::set<std::string> m_bindMeshes;

    /// The list of HandleComponentData.
    Core::AlignedStdVector<HandleComponentData> m_component;

    /// The HandleArray hierarchy, i.e.\ bones hierarchy.
    Core::AlignedStdVector<Core::Vector2ui> m_edge;

    /// The HandleArray N-Dimensional parts, i.e.\ cage polyhedra.
    Core::AlignedStdVector<Core::VectorNui> m_face;
};

inline void HandleData::setName( const std::string& name ) {
    m_name = name;
}

inline HandleData::HandleType HandleData::getType() const {
    return m_type;
}

inline void HandleData::setType( const HandleType& type ) {
    m_type = type;
}

inline Core::Transform HandleData::getFrame() const {
    return m_frame;
}

inline void HandleData::setFrame( const Core::Transform& frame ) {
    m_frame = frame;
}

inline uint HandleData::getVertexSize() const {
    return m_vertexSize;
}
inline void HandleData::setVertexSize( uint size ) {
    m_vertexSize = size;
}

inline void HandleData::setNameTable( const std::map<std::string, uint>& nameTable ) {
    m_nameTable = nameTable;
}

inline uint HandleData::getComponentDataSize() const {
    return m_component.size();
}

inline const Core::AlignedStdVector<HandleComponentData>& HandleData::getComponentData() const {
    return m_component;
}

inline Core::AlignedStdVector<HandleComponentData>& HandleData::getComponentData() {
    return m_component;
}

inline void
HandleData::setComponents( const Core::AlignedStdVector<HandleComponentData>& components ) {
    const uint size = components.size();
    m_component.resize( size );
#pragma omp parallel for
    for ( int i = 0; i < int( size ); ++i ) {
        m_component[i] = components[i];
    }
}

inline const HandleComponentData& HandleData::getComponent( const uint i ) const {
    CORE_ASSERT( ( i < m_component.size() ), "Index i out of bound" );
    return m_component[i];
}

inline HandleComponentData& HandleData::getComponent( const uint i ) {
    CORE_ASSERT( ( i < m_component.size() ), "Index i out of bound" );
    return m_component[i];
}

inline const Core::AlignedStdVector<Core::Vector2ui>& HandleData::getEdgeData() const {
    return m_edge;
}

inline Core::AlignedStdVector<Core::Vector2ui>& HandleData::getEdgeData() {
    return m_edge;
}

inline void HandleData::setEdges( const Core::AlignedStdVector<Core::Vector2ui>& edgeList ) {
    const uint size = edgeList.size();
    m_edge.resize( size );
#pragma omp parallel for
    for ( int i = 0; i < int( size ); ++i ) {
        m_edge[i] = edgeList[i];
    }
}

inline const Core::AlignedStdVector<Core::VectorNui>& HandleData::getFaceData() const {
    return m_face;
}

inline Core::AlignedStdVector<Core::VectorNui>& HandleData::getFaceData() {
    return m_face;
}

inline void HandleData::setFaces( const Core::AlignedStdVector<Core::VectorNui>& faceList ) {
    const uint size = faceList.size();
    m_face.resize( size );
#pragma omp parallel for
    for ( int i = 0; i < int( size ); ++i ) {
        m_face[i] = faceList[i];
    }
}

inline void HandleData::recomputeAllIndices() {
    m_nameTable.clear();
    for ( uint i = 0; i < getComponentDataSize(); ++i ) {
        m_nameTable[m_component[i].m_name] = i;
    }
}

inline bool HandleData::isPointCloud() const {
    return ( m_type == POINT_CLOUD );
}

inline bool HandleData::isSkeleton() const {
    return ( m_type == SKELETON );
}

inline bool HandleData::isCage() const {
    return ( m_type == CAGE );
}

inline bool HandleData::hasComponents() const {
    return !m_component.empty();
}

inline bool HandleData::hasEdges() const {
    return !m_edge.empty();
}

inline bool HandleData::hasFaces() const {
    return !m_face.empty();
}

inline bool HandleData::needsEndNodes() const {
    return m_endNode;
}

inline int HandleData::getIndexOf( const std::string& name ) const {
    auto it = m_nameTable.find( name );
    if ( it == m_nameTable.end() ) { return -1; }
    return it->second;
}

inline void HandleData::needEndNodes( bool need ) {
    m_endNode = need;
}

inline void HandleData::addBindMesh( const std::string& name ) {
    m_bindMeshes.insert( name );
}

inline const std::set<std::string>& HandleData::getBindMeshes() const {
    return m_bindMeshes;
}

inline void HandleData::displayInfo() const {
    using namespace Core::Utils; // log
    std::string type;
    switch ( m_type ) {
    case POINT_CLOUD:
        type = "POINT CLOUD";
        break;
    case SKELETON:
        type = "SKELETON";
        break;
    case CAGE:
        type = "CAGE";
        break;
    case UNKNOWN:
    default:
        type = "UNKNOWN";
        break;
    }
    LOG( logINFO ) << "======== HANDLE INFO ========";
    LOG( logINFO ) << " Name            : " << m_name;
    LOG( logINFO ) << " Type            : " << type;
    LOG( logINFO ) << " Element #       : " << m_component.size();
    LOG( logINFO ) << " Edge #          : " << m_edge.size();
    LOG( logINFO ) << " Face #          : " << m_face.size();
    LOG( logINFO ) << " Need EndNodes ? : " << ( ( m_endNode ) ? "YES" : "NO" );
}

} // namespace Asset
} // namespace Core
} // namespace Ra
