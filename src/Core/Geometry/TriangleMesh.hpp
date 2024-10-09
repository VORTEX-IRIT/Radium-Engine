#pragma once

#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/AbstractGeometry.hpp>
#include <Core/Geometry/StandardAttribNames.hpp>
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Attribs.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/Observable.hpp>

namespace Ra {
namespace Core {
namespace Geometry {

/// \brief This class represents vertex + attributes per vertex. Topology is handled in
/// MultiIndexedGeometry subclass.
///
/// Attributes are unique per vertex, so that same position with different
/// normals are two vertices.
/// Points and Normals, defining the geometry, are always present.
/// They can be accessed through vertices() and normals().
/// Other attribs could be added with addAttrib() and accesssed with getAttrib().
/// \note Mesh attribute VERTEX_POSITION and VERTEX_NORMAL are reserved and pre-allocated.
class RA_CORE_API AttribArrayGeometry : public AbstractGeometry
{
  public:
    using Point  = Vector3;
    using Normal = Vector3;

    using PointAttribHandle  = Utils::AttribHandle<Point>;
    using NormalAttribHandle = Utils::AttribHandle<Normal>;
    using FloatAttribHandle  = Utils::AttribHandle<Scalar>;
    using Vec2AttribHandle   = Utils::AttribHandle<Vector2>;
    using Vec3AttribHandle   = Utils::AttribHandle<Vector3>;
    using Vec4AttribHandle   = Utils::AttribHandle<Vector4>;

    /// Create an empty geometry.
    inline AttribArrayGeometry() : AbstractGeometry() { initDefaultAttribs(); }

    /// Copy constructor, copy all the data (geometry, attributes).
    /// \note Handles on \p other are not valid for *this.
    inline explicit AttribArrayGeometry( const AttribArrayGeometry& other );

    /// Move constructor, copy all the data (geometry, attributes).
    /// \note Handles on \p other are also valid for *this.
    inline explicit AttribArrayGeometry( AttribArrayGeometry&& other );

    /// Assignment operator, copy all the data (geometry, attributes).
    /// \warning Handles on \p other are not valid for *this.
    inline AttribArrayGeometry& operator=( const AttribArrayGeometry& other );

    /// Move assignment, copy all the data (geometry, attributes).
    /// \note Handles on \p other are also valid for *this.
    inline AttribArrayGeometry& operator=( AttribArrayGeometry&& other );

    ~AttribArrayGeometry() = default;

    /// Appends another AttribArrayGeometry to this one, but only if they have the same attributes.
    /// Return True if \p other has been successfully appended.
    /// \warning There is no error check on the handles attribute type.
    bool append( const AttribArrayGeometry& other );

    /// Erases all data, making the AttribArrayGeometry empty.
    void clear() override;

    /// Set vertices
    inline void setVertices( PointAttribHandle::Container&& vertices );
    /// Set vertices
    inline void setVertices( const PointAttribHandle::Container& vertices );

    /// Access the vertices positions.
    inline const PointAttribHandle::Container& vertices() const;

    /// Set normals
    inline void setNormals( PointAttribHandle::Container&& normals );
    /// Set normals
    inline void setNormals( const PointAttribHandle::Container& normals );

    /// Access the vertices normals.
    inline const NormalAttribHandle::Container& normals() const;

    /// Return the Handle to the attribute with the given name if it exists, an
    /// invalid handle otherwise.
    /// \see AttribManager::getAttribHandle() for more info.
    template <typename T>
    inline Utils::AttribHandle<T> getAttribHandle( const std::string& name ) const;

    /// Return true if \p h refers to an existing attribute in *this.
    template <typename T>
    inline bool isValid( const Utils::AttribHandle<T>& h ) const;

    /// \name Wrappers to Utils::AttribManager.
    /// \{

    /// \see Utils::AttribManager::getAttrib
    template <typename T>
    inline Utils::Attrib<T>& getAttrib( const Utils::AttribHandle<T>& h );
    ///  \see Utils::AttribManager::getAttrib
    template <typename T>
    inline const Utils::Attrib<T>& getAttrib( const Utils::AttribHandle<T>& h ) const;

    /// \see Utils::AttribManager::getAttrib
    template <typename T>
    inline Utils::Attrib<T>& getAttrib( const std::string& name );
    /// \see Utils::AttribManager::getAttrib
    template <typename T>
    inline const Utils::Attrib<T>& getAttrib( const std::string& name ) const;

    /// \see Utils::AttribManager::getAttribPtr
    template <typename T>
    inline Utils::Attrib<T>* getAttribPtr( const Utils::AttribHandle<T>& h );
    /// \see Utils::AttribManager::getAttribPtr
    template <typename T>
    inline const Utils::Attrib<T>* getAttribPtr( const Utils::AttribHandle<T>& h ) const;

    /// \see Utils::AttribManager::getAttribBase
    inline Utils::AttribBase* getAttribBase( const std::string& name );
    /// \see Utils::AttribManager::getAttribBase
    inline const Utils::AttribBase* getAttribBase( const std::string& name ) const;
    ///\}

    /// Check if an attribute exists with the given name.
    /// \see AttribManager::contains for more info.
    inline bool hasAttrib( const std::string& name ) const;

    ///\{
    /// Add attribute with the given name.
    /// \see AttribManager::addAttrib() for more info.
    /// \param name: attrib name, uniquely identify the attrib
    /// \return handle to set the attrib data with getAttrib(handle).setData()
    template <typename T>
    inline Utils::AttribHandle<T> addAttrib( const std::string& name );

    /// Add attribute with the given name.
    /// \see AttribManager::addAttrib() for more info.
    /// \param name: attrib name, uniquely identify the attrib
    /// \param data: an array defining the attrib data
    template <typename T>
    inline Utils::AttribHandle<T> addAttrib( const std::string& name,
                                             const typename Core::VectorArray<T>& data );

    /// Add attribute with the given name.
    /// \see AttribManager::addAttrib() for more info.
    /// \param name: attrib name, uniquely identify the attrib
    /// \param data: an array defining the attrib data, the data is moved to the attrib
    template <typename T>
    inline Utils::AttribHandle<T> addAttrib( const std::string& name,
                                             const typename Utils::Attrib<T>::Container&& data );
    ///\}

    /// Remove attribute by handle.
    /// \see AttribManager::removeAttrib() for more info.
    template <typename T>
    inline void removeAttrib( Utils::AttribHandle<T>& h );

    /// Erases all attributes, leaving the AttribArrayGeometry with geometry only.
    inline void clearAttributes();

    /// Copy only the geometry.
    /// The needed attributes can be copied through copyAttributes().
    /// \warning Deletes all attributes of *this.
    inline virtual void copyBaseGeometry( const AttribArrayGeometry& other );

    /// Copy only the required attributes from \p input. Existing attributes are
    /// kept untouched, except if overwritten by attributes copied from \p other.
    /// \return True if the attributes have been sucessfully copied, false otherwise.
    /// \note *this and \p input must have the same number of vertices.
    /// \warning The original handles are not valid for the AttribArrayGeometry copy.
    template <typename... Handles>
    bool copyAttributes( const AttribArrayGeometry& input, Handles... attribs );

    /// Copy all the attributes from \p input. Existing attributes are
    /// kept untouched, except if overwritten by attributes copied from \p other.
    /// \return True if the attributes have been sucessfully copied, false otherwise.
    /// \note *this and \p input must have the same number of vertices.
    /// \warning The original handles are not valid for the AttribArrayGeometry copy.
    inline bool copyAllAttributes( const AttribArrayGeometry& input );

    inline Aabb computeAabb() const override;

    /// Utility function colorzing the AttribArrayGeometry with a given color.
    /// \note Add the color attribute if needed.
    void colorize( const Utils::Color& c );

    /// Return the vertexAttribs manager. In case you want to have direct access
    /// to Utils::AttribManager functionality.
    /// \todo rename to getAttribManager ?
    inline Utils::AttribManager& vertexAttribs();

    /// Return the vertexAttribs manager. In case you want to have direct access
    /// to Utils::AttribManager functionality.
    inline const Utils::AttribManager& vertexAttribs() const;

    /// Read/write access to the vertices positions.
    /// \warning need to call verticesUnlock when job done.
    inline PointAttribHandle::Container& verticesWithLock();

    /// Release lock on vertices positions
    inline void verticesUnlock();

    /// Read/write access the vertices normals.
    /// \warning need to call normalsUnlock when job done.
    inline NormalAttribHandle::Container& normalsWithLock();

    /// Release lock on vertices normals
    inline void normalsUnlock();

  private:
    /// Sets the default attribs.
    inline void initDefaultAttribs();

    /// Append the data of \p attr to the attribute with the same name.
    /// \warning There is no check on the existence of *this's attribute.
    /// \warning There is no error check on the handles attribute type.
    template <typename T>
    void append_attrib( Utils::AttribBase* attr );

    /// The attrib manager.
    Utils::AttribManager m_vertexAttribs;

    /// The handle for positions, making request faster.
    PointAttribHandle m_verticesHandle;

    /// The handle for normals, making request faster.
    NormalAttribHandle m_normalsHandle;
};

class RA_CORE_API PointCloud : public AttribArrayGeometry
{};

class RA_CORE_API LineStrip : public AttribArrayGeometry
{};

inline AttribArrayGeometry ::AttribArrayGeometry( const AttribArrayGeometry& other ) :
    AbstractGeometry( other ) {
    m_vertexAttribs.copyAllAttributes( other.m_vertexAttribs );
    m_verticesHandle = other.m_verticesHandle;
    m_normalsHandle  = other.m_normalsHandle;
}

inline AttribArrayGeometry::AttribArrayGeometry( AttribArrayGeometry&& other ) :
    m_vertexAttribs( std::move( other.m_vertexAttribs ) ),
    m_verticesHandle( std::move( other.m_verticesHandle ) ),
    m_normalsHandle( std::move( other.m_normalsHandle ) ) {}

inline AttribArrayGeometry& AttribArrayGeometry::operator=( const AttribArrayGeometry& other ) {
    if ( this != &other ) {
        m_vertexAttribs.clear();
        m_vertexAttribs.copyAllAttributes( other.m_vertexAttribs );
        m_verticesHandle = other.m_verticesHandle;
        m_normalsHandle  = other.m_normalsHandle;

        invalidateAabb();
    }
    return *this;
}

inline AttribArrayGeometry& AttribArrayGeometry::operator=( AttribArrayGeometry&& other ) {
    if ( this != &other ) {
        m_vertexAttribs  = std::move( other.m_vertexAttribs );
        m_verticesHandle = std::move( other.m_verticesHandle );
        m_normalsHandle  = std::move( other.m_normalsHandle );

        invalidateAabb();
    }
    return *this;
}

inline void AttribArrayGeometry::clear() {
    m_vertexAttribs.clear();
    // restore the default attribs (empty though)
    initDefaultAttribs();
    invalidateAabb();
}

inline void AttribArrayGeometry::copyBaseGeometry( const AttribArrayGeometry& other ) {
    clear();
    m_vertexAttribs.copyAttributes(
        other.m_vertexAttribs, other.m_verticesHandle, other.m_normalsHandle );
    invalidateAabb();
}

template <typename... Handles>
inline bool AttribArrayGeometry::copyAttributes( const AttribArrayGeometry& input,
                                                 Handles... attribs ) {
    if ( vertices().size() != input.vertices().size() ) return false;
    // copy attribs
    m_vertexAttribs.copyAttributes( input.m_vertexAttribs, attribs... );
    invalidateAabb();
    return true;
}

inline bool AttribArrayGeometry::copyAllAttributes( const AttribArrayGeometry& input ) {
    if ( vertices().size() != input.vertices().size() ) return false;
    // copy attribs
    m_vertexAttribs.copyAllAttributes( input.m_vertexAttribs );
    invalidateAabb();
    return true;
}

inline Aabb AttribArrayGeometry::computeAabb() const {
    if ( !isAabbValid() ) {
        Aabb aabb;
        for ( const auto& v : vertices() ) {
            aabb.extend( v );
        }
        setAabb( aabb );
    }

    return getAabb();
}

inline void AttribArrayGeometry::setVertices( PointAttribHandle::Container&& vertices ) {
    m_vertexAttribs.setAttrib( m_verticesHandle, std::move( vertices ) );
    invalidateAabb();
}

inline void AttribArrayGeometry::setVertices( const PointAttribHandle::Container& vertices ) {
    m_vertexAttribs.setAttrib<PointAttribHandle::value_type>( m_verticesHandle, vertices );
    invalidateAabb();
}

inline const AttribArrayGeometry::PointAttribHandle::Container&
AttribArrayGeometry::vertices() const {
    return m_vertexAttribs.getAttrib( m_verticesHandle ).data();
}

inline void AttribArrayGeometry::setNormals( PointAttribHandle::Container&& normals ) {
    m_vertexAttribs.setAttrib( m_normalsHandle, std::move( normals ) );
}
inline void AttribArrayGeometry::setNormals( const PointAttribHandle::Container& normals ) {
    m_vertexAttribs.setAttrib( m_normalsHandle, normals );
}

inline const AttribArrayGeometry::NormalAttribHandle::Container&
AttribArrayGeometry::normals() const {
    return m_vertexAttribs.getAttrib( m_normalsHandle ).data();
}

template <typename T>
inline Utils::AttribHandle<T>
AttribArrayGeometry::getAttribHandle( const std::string& name ) const {
    return m_vertexAttribs.findAttrib<T>( name );
}

template <typename T>
inline bool AttribArrayGeometry::isValid( const Utils::AttribHandle<T>& h ) const {
    return m_vertexAttribs.isValid( h );
}

template <typename T>
inline Utils::Attrib<T>& AttribArrayGeometry::getAttrib( const Utils::AttribHandle<T>& h ) {
    return m_vertexAttribs.getAttrib( h );
}

template <typename T>
const Utils::Attrib<T>& AttribArrayGeometry::getAttrib( const Utils::AttribHandle<T>& h ) const {
    return m_vertexAttribs.getAttrib( h );
}

inline Utils::AttribBase* AttribArrayGeometry::getAttribBase( const std::string& name ) {
    return m_vertexAttribs.getAttribBase( name );
}

inline const Utils::AttribBase*
AttribArrayGeometry::getAttribBase( const std::string& name ) const {
    return m_vertexAttribs.getAttribBase( name );
}

template <typename T>
inline const Utils::Attrib<T>& AttribArrayGeometry::getAttrib( const std::string& name ) const {
    return m_vertexAttribs.getAttrib<T>( name );
}

template <typename T>
inline Utils::Attrib<T>& AttribArrayGeometry::getAttrib( const std::string& name ) {
    return m_vertexAttribs.getAttrib<T>( name );
}

inline bool AttribArrayGeometry::hasAttrib( const std::string& name ) const {
    return m_vertexAttribs.contains( name );
}

template <typename T>
inline Utils::AttribHandle<T> AttribArrayGeometry::addAttrib( const std::string& name ) {
    invalidateAabb();
    return m_vertexAttribs.addAttrib<T>( name );
}

template <typename T>
inline Utils::AttribHandle<T>
AttribArrayGeometry::addAttrib( const std::string& name,
                                const typename Core::VectorArray<T>& data ) {
    auto handle = addAttrib<T>( name );
    getAttrib( handle ).setData( data );
    invalidateAabb();
    return handle;
}

template <typename T>
inline Utils::AttribHandle<T>
AttribArrayGeometry::addAttrib( const std::string& name,
                                const typename Utils::Attrib<T>::Container&& data ) {
    auto handle = addAttrib<T>( name );
    getAttrib( handle ).setData( std::move( data ) );
    invalidateAabb();
    return handle;
}

template <typename T>
inline void AttribArrayGeometry::removeAttrib( Utils::AttribHandle<T>& h ) {
    m_vertexAttribs.removeAttrib( h );
    invalidateAabb();
}

inline Utils::AttribManager& AttribArrayGeometry::vertexAttribs() {
    return m_vertexAttribs;
}

inline const Utils::AttribManager& AttribArrayGeometry::vertexAttribs() const {
    return m_vertexAttribs;
}

inline AttribArrayGeometry::PointAttribHandle::Container& AttribArrayGeometry::verticesWithLock() {
    return m_vertexAttribs.getAttrib( m_verticesHandle ).getDataWithLock();
}

inline void AttribArrayGeometry::verticesUnlock() {
    return m_vertexAttribs.getAttrib( m_verticesHandle ).unlock();
}

inline AttribArrayGeometry::NormalAttribHandle::Container& AttribArrayGeometry::normalsWithLock() {
    return m_vertexAttribs.getAttrib( m_normalsHandle ).getDataWithLock();
}

inline void AttribArrayGeometry::normalsUnlock() {
    return m_vertexAttribs.getAttrib( m_normalsHandle ).unlock();
}

inline void AttribArrayGeometry::initDefaultAttribs() {
    m_verticesHandle = m_vertexAttribs.addAttrib<PointAttribHandle::value_type>(
        getAttribName( MeshAttrib::VERTEX_POSITION ) );
    m_normalsHandle = m_vertexAttribs.addAttrib<NormalAttribHandle::value_type>(
        getAttribName( MeshAttrib::VERTEX_NORMAL ) );
    invalidateAabb();
}

template <typename T>
inline void AttribArrayGeometry::append_attrib( Utils::AttribBase* attr ) {
    auto h         = m_vertexAttribs.findAttrib<T>( attr->getName() );
    auto& v0       = m_vertexAttribs.getAttrib( h ).getDataWithLock();
    const auto& v1 = attr->cast<T>().data();
    v0.insert( v0.end(), v1.cbegin(), v1.cend() );
    m_vertexAttribs.getAttrib( h ).unlock();
    invalidateAabb();
}

} // namespace Geometry
} // namespace Core
} // namespace Ra

#include "IndexedGeometry.hpp"
