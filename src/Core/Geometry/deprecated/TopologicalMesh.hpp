#pragma once

#include <Core/RaCore.hpp>

#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/OpenMesh.hpp>
#include <Core/Geometry/StandardAttribNames.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Index.hpp>
#include <Core/Utils/StdOptional.hpp>

#include <OpenMesh/Core/Mesh/PolyMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <OpenMesh/Core/Utils/Property.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

#include <Eigen/Core>
#include <Eigen/Geometry>

#include <set>
#include <typeinfo>
#include <unordered_map>

namespace Ra {
namespace Core {
namespace Geometry {
namespace deprecated {
using namespace Utils; // log, AttribXXX

/**
 * Define the Traits to be used by OpenMesh for TopologicalMesh.
 */
struct TopologicalMeshTraits : OpenMesh::DefaultTraits {
    using Point  = Ra::Core::Vector3;
    using Normal = Ra::Core::Vector3;

    VertexAttributes( OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal );
    FaceAttributes( OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal );
    EdgeAttributes( OpenMesh::Attributes::Status );
    // Add  OpenMesh::Attributes::PrevHalfedge for efficiency ?
    HalfedgeAttributes( OpenMesh::Attributes::Status | OpenMesh::Attributes::Normal );
};

/**
 * This class represents a mesh with topological information on the
 * vertex graph, using a half-edge representation.
 *
 * This integration is inspired by:
 * https://gist.github.com/Unril/03fa353d0461ed6bd41d
 *
 * \todo rename methods to respect Radium guideline (get/set/is, camelCase)
 * \todo private inheritance from OpenMesh, and import relevant methods.
 */
class RA_CORE_API TopologicalMesh : public OpenMesh::PolyMesh_ArrayKernelT<TopologicalMeshTraits>
{
  private:
    using base = OpenMesh::PolyMesh_ArrayKernelT<TopologicalMeshTraits>;
    using base::PolyMesh_ArrayKernelT;
    using Index   = Ra::Core::Utils::Index;
    using Vector3 = Ra::Core::Vector3;

  public:
    /**
     * Construct a topological mesh from a triangle mesh.
     * This operation merges vertex with same position, but keeps vertex
     * attributes on halfedges, so that TriangleMesh vertices with the same 3D
     * position are represented only once in the topological mesh.
     * \note This is a costly operation.
     *
     * \tparam NonManifoldFaceCommand Command executed when non-manifold faces are
     * found. API and default implementation:
     * \snippet Core/Geometry/deprecated/TopologicalMesh.cpp Default command implementation
     *
     */
    template <typename NonManifoldFaceCommand>
    explicit TopologicalMesh( const Ra::Core::Geometry::TriangleMesh& triMesh,
                              NonManifoldFaceCommand command );

    /**
     * \brief Convenience constructor
     * \see TopologicalMesh( const Ra::Core::Geometry::TriangleMesh&, NonManifoldFaceCommand)
     */
    explicit TopologicalMesh( const Ra::Core::Geometry::TriangleMesh& triMesh );

    /**
     * Construct an empty topological mesh, only init mandatory properties.
     */
    explicit TopologicalMesh();

    /**
     * Return a triangleMesh from the topological mesh.
     * \note This is a costly operation.
     */
    TriangleMesh toTriangleMesh();

    /**
     * Update triangle mesh data, assuming the mesh and this topo mesh has the
     * same topology.
     * \warning note implemented yet.
     */
    void updateTriangleMesh( Ra::Core::Geometry::TriangleMesh& mesh );

    // import other version of halfedge_handle method
    using base::halfedge_handle;

    /**
     * Return the half-edge associated with a given vertex and face.
     * \note Asserts if vh is not a member of fh.
     */
    inline HalfedgeHandle halfedge_handle( VertexHandle vh, FaceHandle fh ) const;

    /**
     * Get normal of the vertex vh, when member of fh.
     * \note Asserts if vh is not a member of fh.
     */
    [[deprecated]] inline const Normal& normal( VertexHandle vh, FaceHandle fh ) const;

    /**
     * Set normal of the vertex vh, when member of fh.
     * \note Asserts if vh is not a member of fh.
     */
    [[deprecated]] void set_normal( VertexHandle vh, FaceHandle fh, const Normal& n );

    /// Import Base definition of normal and set normal.
    ///\{
    using base::normal;
    using base::set_normal;
    ///\}

    /**
     * Set the normal n to all the halfedges that points to vh (i.e. incomming
     * halfedges) .
     * If you work with vertex normals, please call this function on all vertex
     * handles before convertion with toTriangleMesh.
     */
    [[deprecated]] void propagate_normal_to_halfedges( VertexHandle vh );

    /**
     * Return a handle to the halfedge property storing vertices indices within
     * the TriangleMesh *this has been built on.
     */
    inline const OpenMesh::HPropHandleT<Index>& getInputTriangleMeshIndexPropHandle() const;

    /**
     * Return a handle to the halfedge property storing vertices indices within
     * the TriangleMesh returned by toTriangleMesh().
     * \note This property is valid only after toTriangleMesh() has been called.
     */
    inline const OpenMesh::HPropHandleT<Index>& getOutputTriangleMeshIndexPropHandle() const;

    /**
     * \name Const access to handles of the HalfEdge properties coming from
     * the TriangleMesh attributes.
     */
    ///\{
    [[deprecated]] inline const std::vector<OpenMesh::HPropHandleT<Scalar>>&
    getFloatPropsHandles() const;
    [[deprecated]] inline const std::vector<OpenMesh::HPropHandleT<Vector2>>&
    getVector2PropsHandles() const;
    [[deprecated]] inline const std::vector<OpenMesh::HPropHandleT<Vector3>>&
    getVector3PropsHandles() const;
    [[deprecated]] inline const std::vector<OpenMesh::HPropHandleT<Vector4>>&
    getVector4PropsHandles() const;
    ///\}

    /**
     * \name Dealing with normals
     * Utils to deal with normals when modifying the mesh topology.
     */
    ///\{

    /**
     * Create a new property for normals on faces of \a mesh.
     * \note This new property will have to be propagated onto the newly created
     * halfedges with copyNormal().
     */
    inline void createNormalPropOnFaces( OpenMesh::FPropHandleT<Normal>& fProp );

    /**
     * Remove face property \a prop from \a mesh.
     * \note Invalidates the property handle.
     */
    inline void clearProp( OpenMesh::FPropHandleT<Normal>& fProp );

    /**
     * Copy the normal property from \a input_heh to \a copy_heh.
     */
    inline void copyNormal( HalfedgeHandle input_heh, HalfedgeHandle copy_heh );

    /** Copy the face normal property \a fProp from \a fh to \a heh.
     * \note \a fProp must have been previously created through createNormalPropOnFaces().
     */
    inline void
    copyNormalFromFace( FaceHandle fh, HalfedgeHandle heh, OpenMesh::FPropHandleT<Normal> fProp );

    /**
     * Interpolate normal property on edge center (after edge split).
     */
    inline void
    interpolateNormal( HalfedgeHandle in_a, HalfedgeHandle in_b, HalfedgeHandle out, Scalar f );

    /** Interpolate normal property on face center.
     * \note \a fProp must have been previously created through createNormalPropOnFaces().
     */
    inline void interpolateNormalOnFaces( FaceHandle fh, OpenMesh::FPropHandleT<Normal> fProp );
    ///\}

    /**
     * \name Dealing with custom properties
     * Utils to deal with custom properties of any type when modifying the mesh topology.
     */
    ///\{

    /**
     * Create a new property for each \a input properties of \a mesh on faces.
     * \note This new property will have to be propagated onto the newly created
     * halfedges with copyProps().
     */
    template <typename T>
    void createPropsOnFaces( const std::vector<OpenMesh::HPropHandleT<T>>& input,
                             std::vector<OpenMesh::FPropHandleT<T>>& output );

    /**
     * Remove \a props from \a mesh.
     * \note Clears \a props.
     */
    template <typename T>
    void clearProps( std::vector<OpenMesh::FPropHandleT<T>>& props );

    /**
     * Copy \a props properties from \a input_heh to \a copy_heh.
     */
    template <typename T>
    void copyProps( HalfedgeHandle input_heh,
                    HalfedgeHandle copy_heh,
                    const std::vector<OpenMesh::HPropHandleT<T>>& props );

    /**
     * Copy face properties \a props from \a fh to \a heh.
     * \note \a fProps must have been previously created through createPropsOnFaces().
     */
    template <typename T>
    void copyPropsFromFace( FaceHandle fh,
                            HalfedgeHandle heh,
                            const std::vector<OpenMesh::FPropHandleT<T>>& fProps,
                            const std::vector<OpenMesh::HPropHandleT<T>>& hProps );

    /**
     * Interpolate \a props on edge center (after edge split).
     */
    template <typename T>
    void interpolateProps( HalfedgeHandle in_a,
                           HalfedgeHandle in_b,
                           HalfedgeHandle out,
                           Scalar f,
                           const std::vector<OpenMesh::HPropHandleT<T>>& props );

    /**
     * Interpolate \a hprops on face center.
     * \note \a fProps must have been previously created through createPropsOnFaces().
     */
    template <typename T>
    void interpolatePropsOnFaces( FaceHandle fh,
                                  const std::vector<OpenMesh::HPropHandleT<T>>& hProps,
                                  const std::vector<OpenMesh::FPropHandleT<T>>& fProps );
    ///\}
    /**
        * \name Deal with all attributes* Utils to deal with the normal and
        custom properties when modifying the mesh topology.*/

    ///\{

    /**
     * Create a new property for each property of \a mesh on faces.
     * Outputs the new face properties handles in the corresponding output parameters.
     * \note These new properties will have to be propagated onto the newly created
     * halfedges with copyAllProps().
     */
    inline void createAllPropsOnFaces( OpenMesh::FPropHandleT<Normal>& normalProp,
                                       std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                                       std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                                       std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                                       std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props );

    /**
     * Remove all the given properties from \a mesh.
     * \note Invalidates \a normalProp and clears the given property containers.
     */
    inline void clearAllProps( OpenMesh::FPropHandleT<Normal>& normalProp,
                               std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                               std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                               std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                               std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props );

    /**
     * Copy all properties from \a input_heh to \a copy_heh.
     */
    inline void copyAllProps( HalfedgeHandle input_heh, HalfedgeHandle copy_heh );

    /**
     * Copy all given face properties from \a fh to \a heh.
     * \note Each property must have been previously created either all at once
     * through createAllPropsOnFaces(), or individually through
     * createNormalPropOnFaces() and createPropsOnFaces().
     */
    inline void copyAllPropsFromFace( FaceHandle fh,
                                      HalfedgeHandle heh,
                                      OpenMesh::FPropHandleT<Normal> normalProp,
                                      std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                                      std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                                      std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                                      std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props );

    /**
     * Interpolate all properties on edge center (after edge split).
     */
    inline void
    interpolateAllProps( HalfedgeHandle in_a, HalfedgeHandle in_b, HalfedgeHandle out, Scalar f );

    /**
     * Interpolate \a hprops on face center.
     * \note Each property must have been previously created either all at once
     * through createAllPropsOnFaces(), or individually through
     * createNormalPropOnFaces() and createPropsOnFaces().
     */
    inline void
    interpolateAllPropsOnFaces( FaceHandle fh,
                                OpenMesh::FPropHandleT<Normal> normalProp,
                                std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                                std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                                std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                                std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props );
    ///\}

    /**
     * \name Topological operations
     */
    ///\{
    /**
     * Apply a 2-4 edge split.
     * \param eh The handle to the edge to split.
     * \param f The interpolation factor to place the new point on the edge.
     *          Must be in [0,1].
     * \return True if the edge has been split, false otherwise.
     * \note Only applies on edges between 2 triangles, and if \a f is in [0,1].
     * \note Mesh attributes are linearly interpolated on the newly created
     * halfedge.
     * \note f=0 correspond to halfedge_handle( eh, 0 ) (i.e. first vertex of
     * the edge)
     */
    bool splitEdge( TopologicalMesh::EdgeHandle eh, Scalar f );
    bool splitEdgeWedge( TopologicalMesh::EdgeHandle eh, Scalar f );

    ///\}

  private:
    template <typename T>
    using HandleAndValueVector =
        std::vector<std::pair<AttribHandle<T>, T>,
                    Eigen::aligned_allocator<std::pair<AttribHandle<T>, T>>>;

    template <typename T>
    using PropPair = std::pair<AttribHandle<T>, OpenMesh::HPropHandleT<T>>;

    template <typename T>
    inline void copyAttribToTopo( const TriangleMesh& triMesh,
                                  const std::vector<PropPair<T>>& vprop,
                                  TopologicalMesh::HalfedgeHandle heh,
                                  unsigned int vindex );

    template <typename T>
    inline void addAttribPairToTopo( const TriangleMesh& triMesh,
                                     AttribManager::pointer_type attr,
                                     std::vector<PropPair<T>>& vprop,
                                     std::vector<OpenMesh::HPropHandleT<T>>& pph );

    void split_copy( EdgeHandle _eh, VertexHandle _vh );
    void split( EdgeHandle _eh, VertexHandle _vh );

    ///\todo to be deleted/updated
    OpenMesh::HPropHandleT<Index> m_inputTriangleMeshIndexPph;
    OpenMesh::HPropHandleT<Index> m_outputTriangleMeshIndexPph;
    [[deprecated]] std::vector<OpenMesh::HPropHandleT<Scalar>> m_floatPph;
    [[deprecated]] std::vector<OpenMesh::HPropHandleT<Vector2>> m_vec2Pph;
    [[deprecated]] std::vector<OpenMesh::HPropHandleT<Vector3>> m_vec3Pph;
    [[deprecated]] std::vector<OpenMesh::HPropHandleT<Vector4>> m_vec4Pph;

    friend class TMOperations;
};

template <typename NonManifoldFaceCommand>
inline TopologicalMesh::TopologicalMesh( const TriangleMesh& triMesh,
                                         NonManifoldFaceCommand command ) :
    TopologicalMesh() {

    LOG( logINFO ) << "TopologicalMesh: load triMesh with " << triMesh.getIndices().size()
                   << " faces and " << triMesh.vertices().size() << " vertices.";

    struct hash_vec {
        size_t operator()( const Vector3& lvalue ) const {
            size_t hx = std::hash<Scalar>()( lvalue[0] );
            size_t hy = std::hash<Scalar>()( lvalue[1] );
            size_t hz = std::hash<Scalar>()( lvalue[2] );
            return ( hx ^ ( hy << 1 ) ) ^ hz;
        }
    };
    // use a hashmap for fast search of existing vertex position
    using VertexMap = std::unordered_map<Vector3, TopologicalMesh::VertexHandle, hash_vec>;
    VertexMap vertexHandles;

    std::vector<PropPair<Scalar>> vprop_float;
    std::vector<std::pair<AttribHandle<Vector2>, OpenMesh::HPropHandleT<Vector2>>> vprop_vec2;
    std::vector<std::pair<AttribHandle<Vector3>, OpenMesh::HPropHandleT<Vector3>>> vprop_vec3;
    std::vector<std::pair<AttribHandle<Vector4>, OpenMesh::HPropHandleT<Vector4>>> vprop_vec4;

    // loop over all attribs and build correspondance pair
    triMesh.vertexAttribs().for_each_attrib(
        [&triMesh, this, &vprop_float, &vprop_vec2, &vprop_vec3, &vprop_vec4]( const auto& attr ) {
            // skip builtin attribs
            if ( attr->getName() != std::string( getAttribName( MeshAttrib::VERTEX_POSITION ) ) &&
                 attr->getName() != std::string( getAttribName( MeshAttrib::VERTEX_NORMAL ) ) ) {
                if ( attr->isFloat() )
                    addAttribPairToTopo( triMesh, attr, vprop_float, m_floatPph );
                else if ( attr->isVector2() )
                    addAttribPairToTopo( triMesh, attr, vprop_vec2, m_vec2Pph );
                else if ( attr->isVector3() )
                    addAttribPairToTopo( triMesh, attr, vprop_vec3, m_vec3Pph );
                else if ( attr->isVector4() )
                    addAttribPairToTopo( triMesh, attr, vprop_vec4, m_vec4Pph );
                else
                    LOG( logWARNING )
                        << "Warning, mesh attribute " << attr->getName()
                        << " type is not supported (only float, vec2, vec3 nor vec4 are supported)";
            }
        } );

    size_t num_triangles = triMesh.getIndices().size();

    command.initialize( triMesh );

    const bool hasNormals = !triMesh.normals().empty();
    if ( !hasNormals ) {
        release_face_normals();
        release_vertex_normals();
        release_halfedge_normals();
    }
    for ( unsigned int i = 0; i < num_triangles; i++ ) {
        std::vector<TopologicalMesh::VertexHandle> face_vhandles( 3 );
        std::vector<TopologicalMesh::Normal> face_normals( 3 );
        std::vector<unsigned int> face_vertexIndex( 3 );
        const auto& triangle = triMesh.getIndices()[i];
        for ( size_t j = 0; j < 3; ++j ) {
            unsigned int inMeshVertexIndex = triangle[j];
            const Vector3& p               = triMesh.vertices()[inMeshVertexIndex];

            typename VertexMap::iterator vtr = vertexHandles.find( p );

            TopologicalMesh::VertexHandle vh;
            if ( vtr == vertexHandles.end() ) {
                vh = add_vertex( p );
                vertexHandles.insert( vtr, typename VertexMap::value_type( p, vh ) );
            }
            else { vh = vtr->second; }

            face_vhandles[j]    = vh;
            face_vertexIndex[j] = inMeshVertexIndex;
            if ( hasNormals ) face_normals[j] = triMesh.normals()[inMeshVertexIndex];
        }

        TopologicalMesh::FaceHandle fh;

        // skip 2 vertex face
        if ( face_vhandles.size() > 2 ) fh = add_face( face_vhandles );
        // x-----------------------------------------------------------------------------------x

        if ( fh.is_valid() ) {
            for ( size_t vindex = 0; vindex < face_vhandles.size(); vindex++ ) {
                TopologicalMesh::HalfedgeHandle heh = halfedge_handle( face_vhandles[vindex], fh );
                if ( hasNormals ) set_normal( heh, face_normals[vindex] );
                property( m_inputTriangleMeshIndexPph, heh ) = face_vertexIndex[vindex];
                copyAttribToTopo( triMesh, vprop_float, heh, face_vertexIndex[vindex] );
                copyAttribToTopo( triMesh, vprop_vec2, heh, face_vertexIndex[vindex] );
                copyAttribToTopo( triMesh, vprop_vec3, heh, face_vertexIndex[vindex] );
                copyAttribToTopo( triMesh, vprop_vec4, heh, face_vertexIndex[vindex] );
            }
        }
        else { command.process( face_vhandles ); }
        face_vhandles.clear();
        face_normals.clear();
        face_vertexIndex.clear();
    }
    command.postProcess( *this );

    // grabage collect since some wedge might already be deleted
    garbage_collection();
}

template <typename T>
void TopologicalMesh::addAttribPairToTopo( const TriangleMesh& triMesh,
                                           AttribManager::pointer_type attr,
                                           std::vector<TopologicalMesh::PropPair<T>>& vprop,
                                           std::vector<OpenMesh::HPropHandleT<T>>& pph ) {
    AttribHandle<T> h = triMesh.getAttribHandle<T>( attr->getName() );
    if ( attr->getSize() == triMesh.vertices().size() ) {
        OpenMesh::HPropHandleT<T> oh;
        this->add_property( oh, attr->getName() );
        vprop.push_back( std::make_pair( h, oh ) );
        pph.push_back( oh );
    }
    else {
        LOG( logWARNING ) << "[TopologicalMesh] Skip badly sized attribute " << attr->getName()
                          << ".";
    }
}

template <typename T>
void TopologicalMesh::copyAttribToTopo( const TriangleMesh& triMesh,
                                        const std::vector<PropPair<T>>& vprop,
                                        TopologicalMesh::HalfedgeHandle heh,
                                        unsigned int vindex ) {
    for ( auto pp : vprop ) {
        this->property( pp.second, heh ) = triMesh.getAttrib( pp.first ).data()[vindex];
    }
}

inline const TopologicalMesh::Normal& TopologicalMesh::normal( VertexHandle vh,
                                                               FaceHandle fh ) const {
    // find halfedge that point to vh and member of fh
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, return dummy ref to  (0,0,0)";
        static TopologicalMesh::Normal dummy { 0_ra, 0_ra, 0_ra };
        return dummy;
    }
    return normal( halfedge_handle( vh, fh ) );
}

inline void TopologicalMesh::set_normal( VertexHandle vh, FaceHandle fh, const Normal& n ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }

    set_normal( halfedge_handle( vh, fh ), n );
}

inline void TopologicalMesh::propagate_normal_to_halfedges( VertexHandle vh ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }
    for ( VertexIHalfedgeIter vih_it = vih_iter( vh ); vih_it.is_valid(); ++vih_it ) {
        set_normal( *vih_it, normal( vh ) );
    }
}

inline TopologicalMesh::HalfedgeHandle TopologicalMesh::halfedge_handle( VertexHandle vh,
                                                                         FaceHandle fh ) const {
    for ( ConstVertexIHalfedgeIter vih_it = cvih_iter( vh ); vih_it.is_valid(); ++vih_it ) {
        if ( face_handle( *vih_it ) == fh ) { return *vih_it; }
    }
    CORE_ASSERT( false, "vh is not a vertex of face fh" );
    return HalfedgeHandle();
}

inline const OpenMesh::HPropHandleT<TopologicalMesh::Index>&
TopologicalMesh::getInputTriangleMeshIndexPropHandle() const {
    return m_inputTriangleMeshIndexPph;
}

inline const OpenMesh::HPropHandleT<TopologicalMesh::Index>&
TopologicalMesh::getOutputTriangleMeshIndexPropHandle() const {
    return m_outputTriangleMeshIndexPph;
}

inline const std::vector<OpenMesh::HPropHandleT<Scalar>>&
TopologicalMesh::getFloatPropsHandles() const {
    return m_floatPph;
}

inline const std::vector<OpenMesh::HPropHandleT<Vector2>>&
TopologicalMesh::getVector2PropsHandles() const {
    return m_vec2Pph;
}

inline const std::vector<OpenMesh::HPropHandleT<Vector3>>&
TopologicalMesh::getVector3PropsHandles() const {
    return m_vec3Pph;
}

inline const std::vector<OpenMesh::HPropHandleT<Vector4>>&
TopologicalMesh::getVector4PropsHandles() const {
    return m_vec4Pph;
}

inline void TopologicalMesh::createNormalPropOnFaces( OpenMesh::FPropHandleT<Normal>& fProp ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }
    auto nph = halfedge_normals_pph();
    add_property( fProp, property( nph ).name() + "_subdiv_copy_F" );
}

inline void TopologicalMesh::clearProp( OpenMesh::FPropHandleT<Normal>& fProp ) {
    remove_property( fProp );
}

inline void TopologicalMesh::copyNormal( HalfedgeHandle input_heh, HalfedgeHandle copy_heh ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }
    auto nph                  = halfedge_normals_pph();
    property( nph, copy_heh ) = property( nph, input_heh );
}

inline void TopologicalMesh::copyNormalFromFace( FaceHandle fh,
                                                 HalfedgeHandle heh,
                                                 OpenMesh::FPropHandleT<Normal> fProp ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }
    auto nph             = halfedge_normals_pph();
    property( nph, heh ) = property( fProp, fh );
}

inline void TopologicalMesh::interpolateNormal( HalfedgeHandle in_a,
                                                HalfedgeHandle in_b,
                                                HalfedgeHandle out,
                                                Scalar f ) {
    auto nph = halfedge_normals_pph();
    property( nph, out ) =
        ( ( 1 - f ) * property( nph, in_a ) + f * property( nph, in_b ) ).normalized();
}

inline void TopologicalMesh::interpolateNormalOnFaces( FaceHandle fh,
                                                       OpenMesh::FPropHandleT<Normal> fProp ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }
    auto nph = halfedge_normals_pph();

    // init sum to first
    auto heh              = halfedge_handle( fh );
    property( fProp, fh ) = property( nph, heh );
    heh                   = next_halfedge_handle( heh );

    // sum others
    for ( size_t i = 1; i < valence( fh ); ++i ) {
        property( fProp, fh ) += property( nph, heh );
        heh = next_halfedge_handle( heh );
    }

    // normalize
    property( fProp, fh ) = property( fProp, fh ).normalized();
}

template <typename T>
void TopologicalMesh::createPropsOnFaces( const std::vector<OpenMesh::HPropHandleT<T>>& input,
                                          std::vector<OpenMesh::FPropHandleT<T>>& output ) {
    output.reserve( input.size() );
    for ( const auto& oh : input ) {
        OpenMesh::FPropHandleT<T> oh_;
        add_property( oh_, property( oh ).name() + "_subdiv_copy_F" );
        output.push_back( oh_ );
    }
}

template <typename T>
void TopologicalMesh::clearProps( std::vector<OpenMesh::FPropHandleT<T>>& props ) {
    for ( auto& oh : props ) {
        remove_property( oh );
    }
    props.clear();
}

template <typename T>
void TopologicalMesh::copyProps( HalfedgeHandle input_heh,
                                 HalfedgeHandle copy_heh,
                                 const std::vector<OpenMesh::HPropHandleT<T>>& props ) {
    for ( const auto& oh : props ) {
        property( oh, copy_heh ) = property( oh, input_heh );
    }
}

template <typename T>
void TopologicalMesh::copyPropsFromFace( FaceHandle fh,
                                         HalfedgeHandle heh,
                                         const std::vector<OpenMesh::FPropHandleT<T>>& fProps,
                                         const std::vector<OpenMesh::HPropHandleT<T>>& hProps ) {
    for ( uint i = 0; i < fProps.size(); ++i ) {
        auto hp             = hProps[i];
        auto fp             = fProps[i];
        property( hp, heh ) = property( fp, fh );
    }
}

template <typename T>
void TopologicalMesh::interpolateProps( HalfedgeHandle in_a,
                                        HalfedgeHandle in_b,
                                        HalfedgeHandle out,
                                        Scalar f,
                                        const std::vector<OpenMesh::HPropHandleT<T>>& props ) {
    // interpolate properties
    for ( const auto& oh : props ) {
        property( oh, out ) = ( 1 - f ) * property( oh, in_a ) + f * property( oh, in_b );
    }
}

template <typename T>
void TopologicalMesh::interpolatePropsOnFaces(
    FaceHandle fh,
    const std::vector<OpenMesh::HPropHandleT<T>>& hProps,
    const std::vector<OpenMesh::FPropHandleT<T>>& fProps ) {
    auto heh       = halfedge_handle( fh );
    const size_t v = valence( fh );

    // init sum to first
    for ( size_t j = 0; j < fProps.size(); ++j ) {
        auto hp            = hProps[j];
        auto fp            = fProps[j];
        property( fp, fh ) = property( hp, heh );
    }
    heh = next_halfedge_handle( heh );
    // sum others
    for ( size_t i = 1; i < v; ++i ) {
        for ( size_t j = 0; j < fProps.size(); ++j ) {
            auto hp = hProps[j];
            auto fp = fProps[j];
            property( fp, fh ) += property( hp, heh );
        }
        heh = next_halfedge_handle( heh );
    }
    // normalize
    for ( size_t j = 0; j < fProps.size(); ++j ) {
        auto fp            = fProps[j];
        property( fp, fh ) = property( fp, fh ) / v;
    }
}

inline void
TopologicalMesh::createAllPropsOnFaces( OpenMesh::FPropHandleT<Normal>& normalProp,
                                        std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                                        std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                                        std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                                        std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props ) {
    createNormalPropOnFaces( normalProp );
    createPropsOnFaces( getFloatPropsHandles(), floatProps );
    createPropsOnFaces( getVector2PropsHandles(), vec2Props );
    createPropsOnFaces( getVector3PropsHandles(), vec3Props );
    createPropsOnFaces( getVector4PropsHandles(), vec4Props );
}

inline void
TopologicalMesh::clearAllProps( OpenMesh::FPropHandleT<Normal>& normalProp,
                                std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                                std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                                std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                                std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props ) {
    clearProp( normalProp );
    clearProps( floatProps );
    clearProps( vec2Props );
    clearProps( vec3Props );
    clearProps( vec4Props );
}

inline void TopologicalMesh::copyAllProps( HalfedgeHandle input_heh, HalfedgeHandle copy_heh ) {
    copyNormal( input_heh, copy_heh );
    copyProps( input_heh, copy_heh, getFloatPropsHandles() );
    copyProps( input_heh, copy_heh, getVector2PropsHandles() );
    copyProps( input_heh, copy_heh, getVector3PropsHandles() );
    copyProps( input_heh, copy_heh, getVector4PropsHandles() );
}

inline void
TopologicalMesh::copyAllPropsFromFace( FaceHandle fh,
                                       HalfedgeHandle heh,
                                       OpenMesh::FPropHandleT<Normal> normalProp,
                                       std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
                                       std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
                                       std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
                                       std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props ) {
    copyNormalFromFace( fh, heh, normalProp );
    copyPropsFromFace( fh, heh, floatProps, getFloatPropsHandles() );
    copyPropsFromFace( fh, heh, vec2Props, getVector2PropsHandles() );
    copyPropsFromFace( fh, heh, vec3Props, getVector3PropsHandles() );
    copyPropsFromFace( fh, heh, vec4Props, getVector4PropsHandles() );
}

inline void TopologicalMesh::interpolateAllProps( HalfedgeHandle in_a,
                                                  HalfedgeHandle in_b,
                                                  HalfedgeHandle out,
                                                  Scalar f ) {
    interpolateNormal( in_a, in_b, out, f );
    interpolateProps( in_a, in_b, out, f, getFloatPropsHandles() );
    interpolateProps( in_a, in_b, out, f, getVector2PropsHandles() );
    interpolateProps( in_a, in_b, out, f, getVector3PropsHandles() );
    interpolateProps( in_a, in_b, out, f, getVector4PropsHandles() );
}

inline void TopologicalMesh::interpolateAllPropsOnFaces(
    FaceHandle fh,
    OpenMesh::FPropHandleT<Normal> normalProp,
    std::vector<OpenMesh::FPropHandleT<Scalar>>& floatProps,
    std::vector<OpenMesh::FPropHandleT<Vector2>>& vec2Props,
    std::vector<OpenMesh::FPropHandleT<Vector3>>& vec3Props,
    std::vector<OpenMesh::FPropHandleT<Vector4>>& vec4Props ) {
    interpolateNormalOnFaces( fh, normalProp );
    interpolatePropsOnFaces( fh, getFloatPropsHandles(), floatProps );
    interpolatePropsOnFaces( fh, getVector2PropsHandles(), vec2Props );
    interpolatePropsOnFaces( fh, getVector3PropsHandles(), vec3Props );
    interpolatePropsOnFaces( fh, getVector4PropsHandles(), vec4Props );
}

} // namespace deprecated
} // namespace Geometry
} // namespace Core
} // namespace Ra
