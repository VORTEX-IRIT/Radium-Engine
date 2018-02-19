#include "TriangleMesh.hpp"

namespace Ra {
namespace Core {
inline void TriangleMesh::clear() {
    vertices().clear();
    normals().clear();
    m_triangles.clear();
}

inline void TriangleMesh::append( const TriangleMesh& other ) {
    const std::size_t verticesBefore = vertices().size();
    const std::size_t trianglesBefore = m_triangles.size();

    vertices().insert( vertices().end(), other.vertices().cbegin(), other.vertices().cend() );
    ///\todo what about other attribs ?"
    normals().insert( normals().end(), other.normals().cbegin(), other.normals().cend() );
    m_triangles.insert( m_triangles.end(), other.m_triangles.cbegin(), other.m_triangles.cend() );

    // Offset the vertex indices in the faces
    for ( uint t = trianglesBefore; t < m_triangles.size(); ++t )
    {
        for ( uint i = 0; i < 3; ++i )
        {
            m_triangles[t][i] += verticesBefore;
        }
    }
}
} // namespace Core
} // namespace Ra
