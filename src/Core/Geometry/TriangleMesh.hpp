#ifndef RADIUMENGINE_TRIANGLEMESH_HPP
#define RADIUMENGINE_TRIANGLEMESH_HPP

#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/AbstractGeometry.hpp>
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Attribs.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/Observable.hpp>

namespace Ra {
namespace Core {
namespace Geometry {

/// Simple Mesh structure that handles indexed polygonal mesh with vertex
/// attributes.
/// Attributes are unique per vertex, so that same position with different
/// normals are two vertices.
/// Points and Normals, defining the mesh geometry, are always present.
/// They can be accessed through vertices() and normals().
/// Other attribs could be added with addAttrib() and accesssed with getAttrib().
/// \note Attribute names "in_position" "in_normal" are reserved and pre-allocated.
/// \note Attribute name "in_color" is not reserved, but automatically binded to
/// colors by Ra::Engine::Mesh when it exists (type must be Vec4AttribHandle)
class RA_CORE_API TriangleMesh : public AbstractGeometry
{
  public:
    EIGEN_MAKE_ALIGNED_OPERATOR_NEW

    using Point  = Vector3;
    using Normal = Vector3;

    using PointAttribHandle  = Utils::AttribHandle<Point>;
    using NormalAttribHandle = Utils::AttribHandle<Normal>;
    using FloatAttribHandle  = Utils::AttribHandle<float>;
    using Vec2AttribHandle   = Utils::AttribHandle<Vector2>;
    using Vec3AttribHandle   = Utils::AttribHandle<Vector3>;
    using Vec4AttribHandle   = Utils::AttribHandle<Vector4>;
    using Face               = VectorNui;

    /// Create an empty mesh.
    inline TriangleMesh() : AbstractGeometry() { initDefaultAttribs(); }

    /// Copy constructor, copy all the mesh data (faces, geometry, attributes).
    /// \note Handles on \p other are not valid for *this.
    inline TriangleMesh( const TriangleMesh& other );

    /// Move constructor, copy all the mesh data (faces, geometry, attributes).
    /// \note Handles on \p other are also valid for *this.
    inline TriangleMesh( TriangleMesh&& other );

    /// Assignment operator, copy all the mesh data (faces, geometry, attributes).
    /// \warning Handles on \p other are not valid for *this.
    inline TriangleMesh& operator=( const TriangleMesh& other );

    /// Move assignment, copy all the mesh data (faces, geometry, attributes).
    /// \note Handles on \p other are also valid for *this.
    inline TriangleMesh& operator=( TriangleMesh&& other );

    ~TriangleMesh() = default;

    /// Appends another mesh to this one, but only if they have the same attributes.
    /// Return True if the mesh has been successfully appended.
    /// \warning There is no error check on the handles attribute type.
    bool append( const TriangleMesh& other );

    /// Erases all data, making the mesh empty.
    void clear() override;

    /// Set vertices
    inline void setVertices( PointAttribHandle::Container&& vertices );
    inline void setVertices( const PointAttribHandle::Container& vertices );

    /// Access the vertices positions.
    inline const PointAttribHandle::Container& vertices() const;

    /// Set normals
    inline void setNormals( PointAttribHandle::Container&& normals );
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

    /// Get attribute by handle.
    /// \see AttribManager::getAttrib() for more info.
    template <typename T>
    inline Utils::Attrib<T>& getAttrib( const Utils::AttribHandle<T>& h );

    template <typename T>
    inline Utils::Attrib<T>* getAttribPtr( const Utils::AttribHandle<T>& h );

    /// Get attribute by handle (const).
    /// \see AttribManager::getAttrib() for more info.
    template <typename T>
    inline const Utils::Attrib<T>& getAttrib( const Utils::AttribHandle<T>& h ) const;

    inline Utils::AttribBase* getAttribBase( const std::string& name );

    /// Check if an attribute exists with the given name.
    /// \see AttribManager::contains for more info.
    inline bool hasAttrib( const std::string& name );

    /// Add attribute with the given name.
    /// \see AttribManager::addAttrib() for more info.
    template <typename T>
    inline Utils::AttribHandle<T> addAttrib( const std::string& name );

    /// Remove attribute by handle.
    /// \see AttribManager::removeAttrib() for more info.
    template <typename T>
    inline void removeAttrib( Utils::AttribHandle<T>& h );

    /// Erases all attributes, leaving the mesh with faces and geometry only.
    inline void clearAttributes();

    /// Copy only the mesh faces and geometry.
    /// The needed attributes can be copied through copyAttributes().
    /// \warning Deletes all attributes of *this.
    inline void copyBaseGeometry( const TriangleMesh& other );

    /// Copy only the required attributes from \p input. Existing attributes are
    /// kept untouched, except if overwritten by attributes copied from \p other.
    /// \return True if the attributes have been sucessfully copied, false otherwise.
    /// \note *this and \p input must have the same number of vertices.
    /// \warning The original handles are not valid for the mesh copy.
    template <typename... Handles>
    bool copyAttributes( const TriangleMesh& input, Handles... attribs );

    /// Copy all the attributes from \p input. Existing attributes are
    /// kept untouched, except if overwritten by attributes copied from \p other.
    /// \return True if the attributes have been sucessfully copied, false otherwise.
    /// \note *this and \p input must have the same number of vertices.
    /// \warning The original handles are not valid for the mesh copy.
    inline bool copyAllAttributes( const TriangleMesh& input );

    inline Aabb computeAabb() const override;

    /// Check that the mesh is well built, asserting when it is not.
    /// only compiles to something when in debug mode.
    void checkConsistency() const;

    /// Utility function colorzing the mesh with a given color. Add the color attribute if needed.
    void colorize( const Utils::Color& c );

    inline Utils::AttribManager& vertexAttribs();

    /// Access the vertices positions.
    inline PointAttribHandle::Container& verticesWithLock();
    /// Access the vertices positions.
    inline void verticesUnlock();

    /// Access the vertices positions.
    inline NormalAttribHandle::Container& normalsWithLock();

    /// Access the vertices positions.
    inline void normalsUnlock();

  public:
    /// The list of triangles.
    VectorArray<Vector3ui> m_triangles;

    /// A list of non-triangular polygons.
    VectorArray<Face> m_faces;

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

    // Ease wrapper
    friend class TopologicalMesh;
};

} // namespace Geometry
} // namespace Core
} // namespace Ra

#include <Core/Geometry/TriangleMesh.inl>

#endif // RADIUMENGINE_TRIANGLEMESH_HPP
