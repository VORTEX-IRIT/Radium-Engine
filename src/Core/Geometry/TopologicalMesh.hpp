#pragma once

#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/OpenMesh.hpp>
#include <Core/Geometry/StandardAttribNames.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Index.hpp>
#include <Core/Utils/Log.hpp>
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
    using Vector3 = Ra::Core::Vector3;
    using Index   = Ra::Core::Utils::Index;
    class Wedge;
    class WedgeCollection;

  public:
    class WedgeData;
    using WedgeIndex       = Ra::Core::Utils::Index;
    using WedgeAttribIndex = Ra::Core::Utils::Index;

    /**
     * Construct an empty topological mesh, only initialize mandatory properties.
     */
    explicit TopologicalMesh();

    /**
     * \brief Convenience constructor
     * \see TopologicalMesh( const Ra::Core::Geometry::TriangleMesh&, NonManifoldFaceCommand)
     */
    template <typename MeshIndex>
    explicit TopologicalMesh( [[deprecated(
        "Use MultiIndexedGeometry instead" )]] const Ra::Core::Geometry::IndexedGeometry<MeshIndex>&
                                  mesh );

    /**
     * \brief Convenience constructor
     * \see TopologicalMesh( const Ra::Core::Geometry::TriangleMesh&, NonManifoldFaceCommand)
     * \todo, when MultiIndexedGeometry will be operational, will this replace the above ?
     */
    explicit TopologicalMesh(
        const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
        const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey );

    /**
     * Construct a topological mesh from a triangle mesh.
     * This operation merges vertex with same position, but keeps vertex
     * attributes on halfedges, so that TriangleMesh vertices with the same 3D
     * position are represented only once in the topological mesh.
     * \note This is a costly operation.
     *
     * \tparam NonManifoldFaceCommand Command executed when non-manifold faces are
     * found. API and default implementation:
     * \snippet Core/Geometry/TopologicalMesh.hpp Default command implementation
     *
     */
    template <typename MeshIndex, typename NonManifoldFaceCommand>
    explicit TopologicalMesh( [[deprecated( "Use MultiIndexedGeometry instead" )]] const Ra::Core::
                                  Geometry::IndexedGeometry<MeshIndex>& mesh,
                              NonManifoldFaceCommand command );

    /**
     * Construct a topological mesh from a triangle mesh.
     * This operation merges vertex with same position, but keeps vertex
     * attributes on halfedges, so that TriangleMesh vertices with the same 3D
     * position are represented only once in the topological mesh.
     * \note This is a costly operation.
     *
     * \tparam NonManifoldFaceCommand Command executed when non-manifold faces are
     * found. API and default implementation:
     * \snippet Core/Geometry/TopologicalMesh.hpp Default command implementation
     * \todo, when MultiIndexedGeometry will be operational, will this replace the above ?
     *
     */
    template <typename NonManifoldFaceCommand>
    explicit TopologicalMesh(
        const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
        const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey,
        NonManifoldFaceCommand command );

    inline void
    initWithWedge( const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
                   const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey );

    template <typename NonManifoldFaceCommand>
    void initWithWedge( const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
                        const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey,
                        NonManifoldFaceCommand command );
    /**
     * Return a triangleMesh from the topological mesh.
     * \note This is a costly operation.
     */
    TriangleMesh toTriangleMesh();

    /**
     * Return a triangleMesh from the topological mesh.
     * \note This is a costly operation.
     * \warning It uses the attributes defined on wedges
     */
    PolyMesh toPolyMesh();

    LineMesh toLineMesh();

    /**
     * Update triangle mesh data, assuming the mesh and this topo mesh has the
     * same topology.
     */
    void updateTriangleMesh( Ra::Core::Geometry::MultiIndexedGeometry& mesh );
    void updateTriangleMeshNormals( Ra::Core::Geometry::MultiIndexedGeometry& mesh );
    void updateTriangleMeshNormals( AttribArrayGeometry::NormalAttribHandle::Container& normals );

    void update( const Ra::Core::Geometry::MultiIndexedGeometry& mesh );
    void updateNormals( const Ra::Core::Geometry::MultiIndexedGeometry& mesh );
    void updatePositions( const Ra::Core::Geometry::MultiIndexedGeometry& mesh );
    void updatePositions( const AttribArrayGeometry::PointAttribHandle::Container& vertices );
    // import other version of halfedge_handle method
    using base::halfedge_handle;

    /**
     * Return the half-edge associated with a given vertex and face.
     * \note Asserts if vh is not a member of fh.
     */
    inline HalfedgeHandle halfedge_handle( VertexHandle vh, FaceHandle fh ) const;

    /// Import Base definition of normal and set normal.
    ///\{
    using base::normal;
    using base::set_normal;
    ///\}

    /**
     * Set the normal n to all the wedges that share to vh.
     * If you work with vertex normals, please call this function on all vertex
     * handles before convertion with toTriangleMesh.
     */
    inline void propagate_normal_to_wedges( VertexHandle vh );

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
     * \name Dealing with normals
     * Utils to deal with normals when modifying the mesh topology.
     */
    ///\{

    void updateWedgeNormals();
    ///\}

    /// update wedge's position to correspond to current point position, which might not be the case
    /// if point(handle) = something; has been used.
    void copyPointsPositionToWedges();
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

    /**
     * Halfedge collapes \a he.
     * vo=from_vertex_handle(he) is deleted.
     * After collapse vo incoming halfedges points to vh = to_vertex_handle(he).
     * Wedge indices are updated to reflect the change in topology.
     * For detailed topological modifications see \ref develmeshes.
     * \param he halfedge's hangle to collapse.
     * \param keepFromVertex whether the remaining vertex is from or to vertex of \a he
     */
    void collapse( HalfedgeHandle he, bool keepFromVertex = false );
    [[deprecated( "use collapse() instead." )]] void
    collapseWedge( TopologicalMesh::HalfedgeHandle he, bool keepFromVertex = false );
    ///\}

    /**
     * Return the set of WedgeIndex incident to a given Vertex \a vh.
     * only valid non deleted wedges are present in the set.
     */
    inline std::set<WedgeIndex> getVertexWedges( OpenMesh::VertexHandle vh ) const;

    /**
     * get the wedge index associated with an halfedge
     */
    inline WedgeIndex getWedgeIndex( OpenMesh::HalfedgeHandle heh ) const;

    /**
     * Access to wedge data.
     * \param idx must be valid and correspond to a non delete wedge index.
     */
    inline const WedgeData& getWedgeData( const WedgeIndex& idx ) const;
    template <typename T>
    [[deprecated( "use getWedgeAttrib() instead." )]] const T&
    getWedgeData( const WedgeIndex& idx, const std::string& name ) const;
    template <typename T>
    const T& getWedgeAttrib( const WedgeIndex& idx, const std::string& name ) const;

    /**
     * Return the wedge refcount, for debug purpose.
     */
    inline unsigned int getWedgeRefCount( const WedgeIndex& idx ) const;

    /** set WedgeData \a wd to the wedge with index \a widx.
     * All halfedge that point to widx will get the new values.
     * \param widx index of the wedge
     * \param wd data to set to wedge that correspond to widx
     */
    inline void setWedgeData( WedgeIndex widx, const WedgeData& wd );

    /**
     * Change the WedgeData associated for \a idx, for attrib \a name to \a value.
     * The data is changed for all halfedges referencing this wedge.
     * \return true if the wedge is set, false if nothing set, i.e. if name is not an attrib of
     * type T.
     */
    template <typename T>
    [[deprecated( "use setWedgeAttrib() instead." )]] inline bool
    setWedgeData( const WedgeIndex& idx, const std::string& name, const T& value );

    template <typename T>
    inline bool setWedgeAttrib( const WedgeIndex& idx, const std::string& name, const T& value );

    /** return a WedgeData with all attrib initialized to default values */
    inline WedgeData newWedgeData() const { return m_wedges.newWedgeData(); }
    /** return a WedgeData with position (and vertex handle) initialized from the value of he's to
     * vertex. */
    inline WedgeData newWedgeData( HalfedgeHandle he ) const {
        return m_wedges.newWedgeData( to_vertex_handle( he ), point( to_vertex_handle( he ) ) );
    }

    /**
     * Replace the wedge data associated with an halfedge.
     * The old wedge is "deleted". If wedge data correspond to an already
     * present wedge, it's index is used.
     */
    inline WedgeIndex replaceWedge( OpenMesh::HalfedgeHandle he, const WedgeData& wd );

    template <typename T>
    inline WedgeAttribIndex addWedgeAttrib( const std::string& name, T value = {} ) {
        return m_wedges.addAttrib<T>( name, value );
    }

    /**
     * Replace the wedge index associated with an halfedge.
     * The old wedge is "deleted". The new wedge reference count is incremented.
     */
    inline void replaceWedgeIndex( OpenMesh::HalfedgeHandle he, const WedgeIndex& widx );

    /**
     * call mergeEquelWedges( vh ) for every vertices of the mesh.
     * \see void mergeEqualWedges( OpenMesh::VertexHandle vh );
     */
    inline void mergeEqualWedges();

    /**
     * Merge (make same index) wegdes with the same data around \a vh
     * \param vh vertex handle to process
     */
    inline void mergeEqualWedges( OpenMesh::VertexHandle vh );

    /// Remove deleted element from the mesh, including wedges.
    void garbage_collection();
    inline void clean() {
        base::clean();
        m_wedges.clean();
    }

    inline const std::vector<std::string>& getVec4AttribNames() const;
    inline const std::vector<std::string>& getVec3AttribNames() const;
    inline const std::vector<std::string>& getVec2AttribNames() const;
    inline const std::vector<std::string>& getFloatAttribNames() const;

    /// true if more than one wedge arount vertex \a vh, false if only one wedge
    inline bool isFeatureVertex( const VertexHandle& vh ) const;

    /// true if at least one of edge's vertex as two different wedge arount the
    /// edge.
    /// false if the two vertices have the same wedge for both face aside the edge.
    inline bool isFeatureEdge( const EdgeHandle& eh ) const;

    inline const OpenMesh::HPropHandleT<WedgeIndex>& getWedgeIndexPph() const;

    void delete_face( FaceHandle _fh, bool _delete_isolated_vertices = true );

    /**
     * is the vertex a "bow tie" vertex ?
     * \note Alias for OpenMesh::is_manifold
     */
    bool isManifold( VertexHandle vh ) const;

    /// Check if evrything looks right in the data structure
    /// \return true if ok, false if ko.
    bool checkIntegrity() const;

    void triangulate();

    /**
     * Inner class WedgeData represents the actual data per wedge, including position.
     *
     * At any time m_position as to be equal to the wedge's vertex point.
     * All wedges have the same set of attributes.
     * Access and management is delegated to TopologicalMesh and WedgeCollection
     */
    class WedgeData
    {
      public:
        explicit WedgeData() = default;
        inline bool operator==( const WedgeData& lhs ) const;
        inline bool operator!=( const WedgeData& lhs ) const;
        inline bool operator<( const WedgeData& lhs ) const;

        template <typename T>
        inline VectorArray<T>& getAttribArray();
        template <typename T>
        inline const VectorArray<T>& getAttribArray() const;
        friend Wedge;

        //        Index m_inputTriangleMeshIndex;
        //        Index m_outputTriangleMeshIndex;
        VertexHandle m_vertexHandle;
        Vector3 m_position {};
        VectorArray<Scalar> m_floatAttrib;
        VectorArray<Vector2> m_vector2Attrib;
        VectorArray<Vector3> m_vector3Attrib;
        VectorArray<Vector4> m_vector4Attrib;

      private:
        // return 1 : equals, 2: strict less, 3: strict greater
        template <typename T>
        static int compareVector( const T& a, const T& b );
    };

  private:
    // base on openmesh version
    void collapse_edge( HalfedgeHandle, bool );
    void collapse_loop( HalfedgeHandle );

    /**
     * This private class manage wedge data and refcount, to maintain deleted status
     *
     * \internal We need to export this class to make it accessible in .inl
     */
    class RA_CORE_API Wedge
    {
      public:
        explicit Wedge() {}
        explicit Wedge( const WedgeData& wd ) : m_wedgeData { wd }, m_refCount { 1 } {};
        const WedgeData& getWedgeData() const { return m_wedgeData; }
        void setWedgeData( const WedgeData& wedgeData ) { m_wedgeData = wedgeData; }
        void setWedgeData( WedgeData&& wedgeData ) { m_wedgeData = std::move( wedgeData ); }
        void incrementRefCount() { ++m_refCount; }
        void decrementRefCount() {
            if ( m_refCount ) --m_refCount;
        }
        /// comparison ignore refCount
        bool operator==( const Wedge& lhs ) const { return m_wedgeData == lhs.m_wedgeData; }

        bool isDeleted() const { return m_refCount == 0; }
        unsigned int getRefCount() const { return m_refCount; }

        friend WedgeCollection;
        friend TopologicalMesh;

      private:
        WedgeData& getWedgeData() { return m_wedgeData; }

        WedgeData m_wedgeData {};
        unsigned int m_refCount { 0 };
    };

    /**
     * This private class manage the wedge collection.
     * Most of the data members are public so that the enclosing class can
     * easily manage the data.
     *
     * \internal We need to export this class to make it accessible in .inl
     */
    class RA_CORE_API WedgeCollection
    {
      public:
        /**
         * Add wd to the wedge collection, and return the index.
         * If a wedge with same data is already present, it's index is returned,
         * otherwise a new wedge is added to the wedges collection.
         * \param wd Data to insert.
         * \return the index of the inserted (or found) wedge.
         */
        WedgeIndex add( const WedgeData& wd );

        /**
         * Delete the wedge \a idx from the collection.
         * These deletion actually just remove one reference from an halfedge
         * to the wedge data. If the wedge is still referenced by other
         * halfedges, it will not be removed during garbageCollection.
         */
        inline void del( const WedgeIndex& idx );
        WedgeIndex newReference( const WedgeIndex& idx );

        /**
         * Return the wedge data associated with \a idx
         */

        /// Return the wedge (not the data) for in class manipulation.
        /// client code should use getWedgeData only.
        inline const Wedge& getWedge( const WedgeIndex& idx ) const;

        inline const WedgeData& getWedgeData( const WedgeIndex& idx ) const;
        template <typename T>
        inline const T& getWedgeData( const WedgeIndex& idx, const std::string& name ) const;
        template <typename T>
        inline T& getWedgeData( const WedgeIndex& idx, int attribIndex );
        template <typename T>
        inline const T& getWedgeAttrib( const WedgeIndex& idx, const std::string& name ) const;
        template <typename T>
        inline T& getWedgeAttrib( const WedgeIndex& idx, int attribIndex );

        unsigned int getWedgeRefCount( const WedgeIndex& idx ) const;

        /// \see TopologicalMesh::setWedgeData
        inline void setWedgeData( const WedgeIndex& idx, const WedgeData& wd );

        /// change WedgeData member name to value.
        /// wd is moidified accordingly.
        /// \return false if name is not of type T
        /// \retrun true on sucess
        template <typename T>
        inline bool
        setWedgeAttrib( TopologicalMesh::WedgeData& wd, const std::string& name, const T& value );
        template <typename T>
        inline bool
        setWedgeAttrib( const WedgeIndex& idx, const std::string& name, const T& value );
        template <typename T>
        inline void setWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                    const int& attribIndex,
                                    const T& value );

        template <typename T>
        inline WedgeAttribIndex getWedgeAttribIndex( const std::string& name );

        inline bool setWedgePosition( const WedgeIndex& idx, const Vector3& value );

        /// management

        template <typename T>
        inline const std::vector<std::string>& getNameArray() const;

        // name is supposed to be unique within all attribs
        // not checks are performed
        // return the index of the the newly added attrib.
        template <typename T>
        WedgeAttribIndex addAttribName( const std::string& name );

        // add attrib to all wedges with default value value
        template <typename T>
        WedgeAttribIndex addAttrib( const std::string& name, const T& value = {} );

        /// return the offset ot apply to each wedgeindex so that
        /// after garbageCollection all indices are valid and coherent.
        std::vector<int> computeCleanupOffset() const;
        /// \todo removeDuplicateWedge
        /// merge wedges with same data
        /// return old->new index correspondance to update wedgeIndexPph
        /// inline void removeDuplicateWedge

        inline size_t size() const { return m_data.size(); }

        /// remove unreferenced wedge, halfedges need to be reindexed.
        inline void garbageCollection();

        inline void clean();

        // return a new wedgeData with uninit values.
        inline WedgeData newWedgeData() const;
        inline WedgeData newWedgeData( TopologicalMesh::VertexHandle vh,
                                       TopologicalMesh::Point p ) const;

        ///\ todo       private:
        /// attrib names associated to vertex/wedges, getted from CoreMesh, if any,
        std::vector<std::string> m_floatAttribNames;
        std::vector<std::string> m_vector2AttribNames;
        std::vector<std::string> m_vector3AttribNames;
        std::vector<std::string> m_vector4AttribNames;

        /// attrib handle from the CoreMesh given at construction, if any.
        /// used by TopologicalMesh::update()
        std::vector<AttribHandle<Scalar>> m_wedgeFloatAttribHandles;
        std::vector<AttribHandle<Vector2>> m_wedgeVector2AttribHandles;
        std::vector<AttribHandle<Vector3>> m_wedgeVector3AttribHandles;
        std::vector<AttribHandle<Vector4>> m_wedgeVector4AttribHandles;

        template <typename T>
        inline std::vector<std::string>& getNameArray();
        AlignedStdVector<Wedge> m_data;
    };

    // internal function to build Core Mesh attribs correspondance to wedge attribs.
    class InitWedgeAttribsFromMultiIndexedGeometry
    {
      public:
        InitWedgeAttribsFromMultiIndexedGeometry(
            TopologicalMesh* topo,
            const Ra::Core::Geometry::MultiIndexedGeometry& triMesh ) :
            m_topo( topo ), m_triMesh( triMesh ) {}
        void operator()( AttribBase* attr ) const;

      private:
        TopologicalMesh* m_topo;
        const Ra::Core::Geometry::MultiIndexedGeometry& m_triMesh;
    };

    /// \todo add layerKey
    //! [Default command implementation]
    struct DefaultNonManifoldFaceCommand {
        /// \brief details string is printed along with the message
        DefaultNonManifoldFaceCommand( const std::string& details = {} ) : m_details { details } {}
        /// \brief Initalize with input Ra::Core::Geometry::TriangleMesh
        inline void initialize( const Ra::Core::Geometry::MultiIndexedGeometry& ) {}
        /// \brief Process non-manifold face
        inline void process( const std::vector<TopologicalMesh::VertexHandle>& /*face_vhandles*/ ) {
            LOG( logWARNING ) << "Invalid face handle returned : face not added " + m_details;
        }
        /// \brief If needed, apply post-processing on the Ra::Core::Geometry::TopologicalMesh
        inline void postProcess( TopologicalMesh& ) {}

      private:
        std::string m_details;
    };
    //! [Default command implementation]

    WedgeData interpolateWedgeAttributes( const WedgeData&, const WedgeData&, Scalar alpha );

    /// \todo when MultiIndexView will be operational, remove the IndexedGeometry<U> version above
    template <typename T>
    inline void copyAttribToWedgeData( const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
                                       unsigned int vindex,
                                       const std::vector<AttribHandle<T>>& attrHandleVec,
                                       VectorArray<T>* to );

    /// \todo when MultiIndexView will be operational, remove the IndexedGeometry<T> version above
    inline void copyMeshToWedgeData( const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
                                     unsigned int vindex,
                                     const std::vector<AttribHandle<Scalar>>& wprop_float,
                                     const std::vector<AttribHandle<Vector2>>& wprop_vec2,
                                     const std::vector<AttribHandle<Vector3>>& wprop_vec3,
                                     const std::vector<AttribHandle<Vector4>>& wprop_vec4,
                                     TopologicalMesh::WedgeData* wd );

    template <typename T>
    using HandleAndValueVector =
        std::vector<std::pair<AttribHandle<T>, T>,
                    Eigen::aligned_allocator<std::pair<AttribHandle<T>, T>>>;

    void split_copy( EdgeHandle _eh, VertexHandle _vh );
    void split( EdgeHandle _eh, VertexHandle _vh );

    OpenMesh::HPropHandleT<WedgeIndex> m_wedgeIndexPph; /**< Halfedges' Wedge index */
    WedgeCollection m_wedges;                           /**< Wedge data management */

    ///\todo to be deleted/updated
    OpenMesh::HPropHandleT<Index> m_inputTriangleMeshIndexPph;
    OpenMesh::HPropHandleT<Index> m_outputTriangleMeshIndexPph;

    int m_normalsIndex { -1 };
    // vertex handle idx -> face handle idx -> wedge idx with the same normal
    std::vector<std::map<int, std::vector<int>>> m_vertexFaceWedgesWithSameNormals;

    friend class TMOperations;
};

// heplers
void printWedgesInfo( const TopologicalMesh& );

////////////////////////////////////////////////////////////////////////////////
///////////////////      WedgeData                //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

inline bool TopologicalMesh::WedgeData::operator==( const TopologicalMesh::WedgeData& lhs ) const {
    return
        // do not have this yet, not sure we need to test them
        // m_inputTriangleMeshIndex == lhs.m_inputTriangleMeshIndex &&
        // m_outputTriangleMeshIndex == lhs.m_outputTriangleMeshIndex &&
        m_position == lhs.m_position && m_floatAttrib == lhs.m_floatAttrib &&
        m_vector2Attrib == lhs.m_vector2Attrib && m_vector3Attrib == lhs.m_vector3Attrib &&
        m_vector4Attrib == lhs.m_vector4Attrib;
}

bool TopologicalMesh::WedgeData::operator!=( const TopologicalMesh::WedgeData& lhs ) const {
    return !( *this == lhs );
}

inline bool TopologicalMesh::WedgeData::operator<( const TopologicalMesh::WedgeData& lhs ) const {

    CORE_ASSERT( ( m_floatAttrib.size() == lhs.m_floatAttrib.size() ) &&
                     ( m_vector2Attrib.size() == lhs.m_vector2Attrib.size() ) &&
                     ( m_vector3Attrib.size() == lhs.m_vector3Attrib.size() ) &&
                     ( m_vector4Attrib.size() == lhs.m_vector4Attrib.size() ),
                 "Could only compare wedge with same number of attributes" );

    {
        int comp = compareVector( m_position, lhs.m_position );
        if ( comp == 2 ) return true;
        if ( comp == 3 ) return false;
    }
    for ( size_t i = 0; i < m_floatAttrib.size(); i++ ) {
        if ( m_floatAttrib[i] < lhs.m_floatAttrib[i] )
            return true;
        else if ( m_floatAttrib[i] > lhs.m_floatAttrib[i] )
            return false;
    }

    for ( size_t i = 0; i < m_vector2Attrib.size(); i++ ) {
        int comp = compareVector( m_vector2Attrib[i], lhs.m_vector2Attrib[i] );
        if ( comp == 2 ) return true;
        if ( comp == 3 ) return false;
    }
    for ( size_t i = 0; i < m_vector3Attrib.size(); i++ ) {
        int comp = compareVector( m_vector3Attrib[i], lhs.m_vector3Attrib[i] );
        if ( comp == 2 ) return true;
        if ( comp == 3 ) return false;
    }
    for ( size_t i = 0; i < m_vector4Attrib.size(); i++ ) {
        int comp = compareVector( m_vector4Attrib[i], lhs.m_vector4Attrib[i] );
        if ( comp == 2 ) return true;
        if ( comp == 3 ) return false;
    }
    return false;
}

#define GET_ATTRIB_ARRAY_HELPER( TYPE, NAME )                                                  \
    template <>                                                                                \
    inline VectorArray<TYPE>& TopologicalMesh::WedgeData::getAttribArray<TYPE>() {             \
        return m_##NAME##Attrib;                                                               \
    }                                                                                          \
    template <>                                                                                \
    inline const VectorArray<TYPE>& TopologicalMesh::WedgeData::getAttribArray<TYPE>() const { \
        return m_##NAME##Attrib;                                                               \
    }

GET_ATTRIB_ARRAY_HELPER( Scalar, float )
GET_ATTRIB_ARRAY_HELPER( Vector2, vector2 )
GET_ATTRIB_ARRAY_HELPER( Vector3, vector3 )
GET_ATTRIB_ARRAY_HELPER( Vector4, vector4 )
#undef GET_ATTRIB_ARRAY_HELPER

template <typename T>
inline VectorArray<T>& TopologicalMesh::WedgeData::getAttribArray() {
    static_assert( sizeof( T ) == -1, "this type is not supported" );
}

// return 1 : equals, 2: strict less, 3: strict greater
template <typename T>
int TopologicalMesh::WedgeData::compareVector( const T& a, const T& b ) {
    for ( int i = 0; i < T::RowsAtCompileTime; i++ ) {
        if ( a[i] < b[i] ) return 2;
        if ( a[i] > b[i] ) return 3;
    }
    // (a == b)
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////      Wedge                    //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// all in class for the moment

////////////////////////////////////////////////////////////////////////////////
///////////////////      WedgeCollection          //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

inline void TopologicalMesh::WedgeCollection::del( const TopologicalMesh::WedgeIndex& idx ) {
    if ( idx.isValid() ) m_data[idx].decrementRefCount();
}

inline TopologicalMesh::WedgeIndex
TopologicalMesh::WedgeCollection::newReference( const TopologicalMesh::WedgeIndex& idx ) {
    if ( idx.isValid() ) m_data[idx].incrementRefCount();
    return idx;
}

inline const TopologicalMesh::Wedge&
TopologicalMesh::WedgeCollection::getWedge( const TopologicalMesh::WedgeIndex& idx ) const {
    return m_data[idx];
}

inline const TopologicalMesh::WedgeData&
TopologicalMesh::WedgeCollection::getWedgeData( const WedgeIndex& idx ) const {
    CORE_ASSERT( idx.isValid() && !m_data[idx].isDeleted(),
                 "access to invalid or deleted wedge is prohibited" );

    return m_data[idx].getWedgeData();
}

template <typename T>
inline const T&
TopologicalMesh::WedgeCollection::getWedgeData( const TopologicalMesh::WedgeIndex& idx,
                                                const std::string& name ) const {
    return getWedgeAttrib<T>( idx, name );
}

template <typename T>
inline const T&
TopologicalMesh::WedgeCollection::getWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                                  const std::string& name ) const {
    if ( idx.isValid() ) {
        auto nameArray = getNameArray<T>();
        auto itr       = std::find( nameArray.begin(), nameArray.end(), name );
        if ( itr != nameArray.end() ) {
            auto attrIndex = std::distance( nameArray.begin(), itr );
            return m_data[idx].getWedgeData().getAttribArray<T>()[attrIndex];
        }
        else {
            LOG( logERROR ) << "Warning, set wedge: no wedge attrib named " << name << " of type "
                            << typeid( T ).name();
        }
    }
    static T dummy;
    return dummy;
}

template <typename T>
inline T& TopologicalMesh::WedgeCollection::getWedgeData( const TopologicalMesh::WedgeIndex& idx,
                                                          int attribIndex ) {
    return getWedgeAttrib<T>( idx, attribIndex );
}

template <typename T>
inline T& TopologicalMesh::WedgeCollection::getWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                                            int attribIndex ) {
    return m_data[idx].getWedgeData().getAttribArray<T>()[attribIndex];
}

inline unsigned int
TopologicalMesh::WedgeCollection::getWedgeRefCount( const WedgeIndex& idx ) const {
    CORE_ASSERT( idx.isValid(), "access to invalid or deleted wedge is prohibited" );
    return m_data[idx].getRefCount();
}

inline void TopologicalMesh::WedgeCollection::setWedgeData( const TopologicalMesh::WedgeIndex& idx,
                                                            const TopologicalMesh::WedgeData& wd ) {
    if ( !( wd.m_floatAttrib.size() == m_floatAttribNames.size() &&
            wd.m_vector2Attrib.size() == m_vector2AttribNames.size() &&
            wd.m_vector3Attrib.size() == m_vector3AttribNames.size() &&
            wd.m_vector4Attrib.size() == m_vector4AttribNames.size() ) ) {
        LOG( logWARNING ) << "Warning, topological mesh set wedge: number of attribs inconsistency";
    }
    if ( idx.isValid() ) m_data[idx].setWedgeData( wd );
}

template <typename T>
inline bool TopologicalMesh::WedgeCollection::setWedgeAttrib( TopologicalMesh::WedgeData& wd,
                                                              const std::string& name,
                                                              const T& value ) {
    auto nameArray = getNameArray<T>();
    auto itr       = std::find( nameArray.begin(), nameArray.end(), name );
    if ( itr != nameArray.end() ) {
        auto attrIndex                    = std::distance( nameArray.begin(), itr );
        wd.getAttribArray<T>()[attrIndex] = value;
        return true;
    }
    else {
        LOG( logERROR ) << "Warning, set wedge: no wedge attrib named " << name << " of type "
                        << typeid( T ).name();
    }
    return false;
}

template <typename T>
inline bool
TopologicalMesh::WedgeCollection::setWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                                  const std::string& name,
                                                  const T& value ) {
    if ( idx.isValid() ) {
        auto nameArray = getNameArray<T>();
        auto itr       = std::find( nameArray.begin(), nameArray.end(), name );
        if ( itr != nameArray.end() ) {
            auto attrIndex = std::distance( nameArray.begin(), itr );
            m_data[idx].getWedgeData().getAttribArray<T>()[attrIndex] = value;
            return true;
        }
        else {
            LOG( logERROR ) << "Warning, set wedge: no wedge attrib named " << name << " of type "
                            << typeid( T ).name();
        }
    }
    return false;
}

template <typename T>
inline void
TopologicalMesh::WedgeCollection::setWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                                  const int& attrIndex,
                                                  const T& value ) {
    m_data[idx].getWedgeData().getAttribArray<T>()[attrIndex] = value;
}

template <typename T>
inline TopologicalMesh::WedgeAttribIndex
TopologicalMesh::WedgeCollection::getWedgeAttribIndex( const std::string& name ) {
    auto nameArray = getNameArray<T>();
    auto itr       = std::find( nameArray.begin(), nameArray.end(), name );
    if ( itr != nameArray.end() ) { return std::distance( nameArray.begin(), itr ); }
    return 0;
}

inline bool
TopologicalMesh::WedgeCollection::setWedgePosition( const TopologicalMesh::WedgeIndex& idx,
                                                    const Vector3& value ) {
    if ( idx.isValid() ) {
        m_data[idx].getWedgeData().m_position = value;
        return true;
    }
    return false;
}

#define GET_NAME_ARRAY_HELPER( TYPE, NAME )                                                       \
    template <>                                                                                   \
    inline const std::vector<std::string>& TopologicalMesh::WedgeCollection::getNameArray<TYPE>() \
        const {                                                                                   \
        return m_##NAME##AttribNames;                                                             \
    }                                                                                             \
    template <>                                                                                   \
    inline std::vector<std::string>& TopologicalMesh::WedgeCollection::getNameArray<TYPE>() {     \
        return m_##NAME##AttribNames;                                                             \
    }

GET_NAME_ARRAY_HELPER( Scalar, float )
GET_NAME_ARRAY_HELPER( Vector2, vector2 )
GET_NAME_ARRAY_HELPER( Vector3, vector3 )
GET_NAME_ARRAY_HELPER( Vector4, vector4 )

#undef GET_NAME_ARRAY_HELPER
// These template functions are defined above for supported types.
// For unsupported types they simply generate a compile error.
template <typename T>
inline const std::vector<std::string>& TopologicalMesh::WedgeCollection::getNameArray() const {

    LOG( logWARNING ) << "Warning, mesh attribute " << typeid( T ).name()
                      << " is not supported (only float, vec2, vec3 nor vec4 are supported)";
    static_assert( sizeof( T ) == -1, "this type is not supported" );
    return m_floatAttribNames;
}

template <typename T>
inline std::vector<std::string>& TopologicalMesh::WedgeCollection::getNameArray() {

    LOG( logWARNING ) << "Warning, mesh attribute " << typeid( T ).name()
                      << " is not supported (only float, vec2, vec3 nor vec4 are supported)";
    static_assert( sizeof( T ) == -1, "this type is not supported" );
    return m_floatAttribNames;
}
template <typename T>
TopologicalMesh::WedgeAttribIndex
TopologicalMesh::WedgeCollection::addAttribName( const std::string& name ) {
    if ( name != getAttribName( MeshAttrib::VERTEX_POSITION ) ) {
        getNameArray<T>().push_back( name );
    }
    return getNameArray<T>().size() - 1;
}

template <typename T>
TopologicalMesh::WedgeAttribIndex
TopologicalMesh::WedgeCollection::addAttrib( const std::string& name, const T& value ) {

    auto index = addAttribName<T>( name );
    for ( auto& w : m_data ) {
        CORE_ASSERT( index = w.getWedgeData().getAttribArray<T>().size(),
                     "inconsistent wedge attrib" );
        w.getWedgeData().getAttribArray<T>().push_back( value );
    }
    return index;
}

inline void TopologicalMesh::WedgeCollection::garbageCollection() {
    m_data.erase( std::remove_if( m_data.begin(),
                                  m_data.end(),
                                  []( const Wedge& w ) { return w.isDeleted(); } ),
                  m_data.end() );
}

inline void TopologicalMesh::WedgeCollection::clean() {
    m_data.clear();
    m_floatAttribNames.clear();
    m_vector2AttribNames.clear();
    m_vector3AttribNames.clear();
    m_vector4AttribNames.clear();
    m_wedgeFloatAttribHandles.clear();
    m_wedgeVector2AttribHandles.clear();
    m_wedgeVector3AttribHandles.clear();
    m_wedgeVector4AttribHandles.clear();
}

template <typename T>
void init( VectorArray<T>& vec, const std::vector<std::string> names ) {
    for ( size_t i = 0; i < names.size(); ++i ) {
        vec.emplace_back();
    }
}
// return a new wedgeData with uninit values.
inline TopologicalMesh::WedgeData TopologicalMesh::WedgeCollection::newWedgeData() const {
    WedgeData ret;
    init<Scalar>( ret.getAttribArray<Scalar>(), m_floatAttribNames );
    init<Vector2>( ret.getAttribArray<Vector2>(), m_vector2AttribNames );
    init<Vector3>( ret.getAttribArray<Vector3>(), m_vector3AttribNames );
    init<Vector4>( ret.getAttribArray<Vector4>(), m_vector4AttribNames );
    return ret;
}

inline TopologicalMesh::WedgeData
TopologicalMesh::WedgeCollection::newWedgeData( TopologicalMesh::VertexHandle vh,
                                                TopologicalMesh::Point p ) const {
    WedgeData ret      = newWedgeData();
    ret.m_vertexHandle = vh;
    ret.m_position     = p;
    return ret;
}

////////////////////////////////////////////////////////////////////////////////
///////////////////      InitWedgeProps           //////////////////////////////
////////////////////////////////////////////////////////////////////////////////
inline void
TopologicalMesh::InitWedgeAttribsFromMultiIndexedGeometry::operator()( AttribBase* attr ) const {
    if ( attr->getSize() != m_triMesh.vertices().size() ) {
        LOG( logWARNING ) << "[TopologicalMesh] Skip badly sized attribute " << attr->getName();
    }
    else if ( attr->getName() != getAttribName( MeshAttrib::VERTEX_POSITION ) ) {
        if ( attr->isFloat() ) {
            m_topo->m_wedges.m_wedgeFloatAttribHandles.push_back(
                m_triMesh.template getAttribHandle<Scalar>( attr->getName() ) );
            m_topo->m_wedges.addAttribName<Scalar>( attr->getName() );
        }
        else if ( attr->isVector2() ) {
            m_topo->m_wedges.m_wedgeVector2AttribHandles.push_back(
                m_triMesh.template getAttribHandle<Vector2>( attr->getName() ) );
            m_topo->m_wedges.addAttribName<Vector2>( attr->getName() );
        }
        else if ( attr->isVector3() ) {
            m_topo->m_wedges.m_wedgeVector3AttribHandles.push_back(
                m_triMesh.template getAttribHandle<Vector3>( attr->getName() ) );
            m_topo->m_wedges.addAttribName<Vector3>( attr->getName() );
        }
        else if ( attr->isVector4() ) {
            m_topo->m_wedges.m_wedgeVector4AttribHandles.push_back(
                m_triMesh.template getAttribHandle<Vector4>( attr->getName() ) );
            m_topo->m_wedges.addAttribName<Vector4>( attr->getName() );
        }
        else
            LOG( logWARNING )
                << "Warning, mesh attribute " << attr->getName()
                << " type is not supported (only float, vec2, vec3 nor vec4 are supported)";
    }
}

////////////////////////////////////////////////////////////////////////////////
///////////////////      TopologicalMesh          //////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct hash_vec {
    std::size_t operator()( const Vector3& lvalue ) const {
        size_t hx = std::hash<Scalar>()( lvalue[0] );
        size_t hy = std::hash<Scalar>()( lvalue[1] );
        size_t hz = std::hash<Scalar>()( lvalue[2] );
        return ( hx ^ ( hy << 1 ) ) ^ hz;
    }
};

template <typename MeshIndex>
TopologicalMesh::TopologicalMesh( const Ra::Core::Geometry::IndexedGeometry<MeshIndex>& mesh ) :
    TopologicalMesh( mesh, DefaultNonManifoldFaceCommand( "[default ctor]" ) ) {}

template <typename MeshIndex, typename NonManifoldFaceCommand>
TopologicalMesh::TopologicalMesh( const IndexedGeometry<MeshIndex>& mesh,
                                  NonManifoldFaceCommand command ) :
    TopologicalMesh() {
    initWithWedge( mesh, mesh.getLayerKey(), command );
}

inline TopologicalMesh::TopologicalMesh(
    const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
    const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey ) :
    TopologicalMesh( mesh, layerKey, DefaultNonManifoldFaceCommand( "[default ctor]" ) ) {}

template <typename NonManifoldFaceCommand>
TopologicalMesh::TopologicalMesh(
    const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
    const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey,
    NonManifoldFaceCommand command ) :
    TopologicalMesh() {
    initWithWedge( mesh, layerKey, command );
}

inline void TopologicalMesh::initWithWedge(
    const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
    const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey ) {
    initWithWedge( mesh,
                   layerKey,
                   DefaultNonManifoldFaceCommand( "[initWithWedges (MultiIndexedGeometry)]" ) );
}

template <typename NonManifoldFaceCommand>
void TopologicalMesh::initWithWedge(
    const Ra::Core::Geometry::MultiIndexedGeometry& mesh,
    const Ra::Core::Geometry::MultiIndexedGeometry::LayerKeyType& layerKey,
    NonManifoldFaceCommand command ) {

    const auto& abstractLayer = mesh.getLayer( layerKey );

    if ( !abstractLayer.hasSemantic( TriangleIndexLayer::staticSemanticName ) &&
         !abstractLayer.hasSemantic( QuadIndexLayer::staticSemanticName ) &&
         !abstractLayer.hasSemantic( PolyIndexLayer::staticSemanticName ) ) {
        LOG( logWARNING ) << "TopologicalMesh: mesh does not contains faces. Aborting conversion";
        return;
    }

    clean();

    LOG( logDEBUG ) << "TopologicalMesh: load mesh with " << abstractLayer.getSize()
                    << " faces and " << mesh.vertices().size() << " vertices.";
    // use a hashmap for fast search of existing vertex position
    using VertexMap = std::unordered_map<Vector3, TopologicalMesh::VertexHandle, hash_vec>;
    VertexMap vertexHandles;

    // loop over all attribs and build correspondance pair
    mesh.vertexAttribs().for_each_attrib( InitWedgeAttribsFromMultiIndexedGeometry { this, mesh } );

    for ( size_t i = 0; i < mesh.vertices().size(); ++i ) {
        // create an empty wedge, with 0 ref
        Wedge w;

        WedgeData wd;
        wd.m_position = mesh.vertices()[i];
        copyMeshToWedgeData( mesh,
                             i,
                             m_wedges.m_wedgeFloatAttribHandles,
                             m_wedges.m_wedgeVector2AttribHandles,
                             m_wedges.m_wedgeVector3AttribHandles,
                             m_wedges.m_wedgeVector4AttribHandles,
                             &wd );
        // here ref is not incremented
        w.setWedgeData( std::move( wd ) );
        // the newly added wedge is not referenced yet, will be done with `newReference` when
        // creating faces just below
        m_wedges.m_data.push_back( w );
    }

    LOG( logDEBUG ) << "TopologicalMesh: have  " << m_wedges.size() << " wedges ";

    const bool hasNormals = !mesh.normals().empty();
    if ( !hasNormals ) {
        release_face_normals();
        release_vertex_normals();
        release_halfedge_normals();
    }

    command.initialize( mesh );

    auto processFaces = [&mesh, &vertexHandles, this, hasNormals, &command]( const auto& faces ) {
        size_t num_triangles = faces.size();
        for ( unsigned int i = 0; i < num_triangles; i++ ) {
            const auto& face      = faces[i];
            const size_t num_vert = face.size();
            std::vector<TopologicalMesh::VertexHandle> face_vhandles( num_vert );
            std::vector<TopologicalMesh::Normal> face_normals( num_vert );
            std::vector<WedgeIndex> face_wedges( num_vert );

            for ( size_t j = 0; j < num_vert; ++j ) {
                unsigned int inMeshVertexIndex = face[j];
                const Vector3& p               = mesh.vertices()[inMeshVertexIndex];

                typename VertexMap::iterator vtr = vertexHandles.find( p );
                TopologicalMesh::VertexHandle vh;
                if ( vtr == vertexHandles.end() ) {
                    vh = add_vertex( p );
                    vertexHandles.insert( vtr, typename VertexMap::value_type( p, vh ) );
                }
                else { vh = vtr->second; }

                face_vhandles[j] = vh;
                if ( hasNormals ) face_normals[j] = mesh.normals()[inMeshVertexIndex];
                face_wedges[j] = WedgeIndex { inMeshVertexIndex };
                m_wedges.m_data[inMeshVertexIndex].getWedgeData().m_vertexHandle = vh;
            }

            // remove consecutive equal vertex
            // first take care of "loop" if begin == *end-1
            // apply the same modifications on wedges and normals
            // e.g. 1 2 1 becomes 1 2
            {
                auto begin = face_vhandles.begin();
                if ( face_vhandles.size() > 2 ) {
                    auto end       = face_vhandles.end() - 1;
                    auto wedgeEnd  = face_wedges.end() - 1;
                    auto normalEnd = face_normals.end() - 1;

                    while ( begin != end && *begin == *end ) {
                        end--;
                        wedgeEnd--;
                        normalEnd--;
                    }
                    face_vhandles.erase( end + 1, face_vhandles.end() );
                    face_wedges.erase( wedgeEnd + 1, face_wedges.end() );
                    face_normals.erase( normalEnd + 1, face_normals.end() );
                }
            }
            // then remove duplicates
            // e.g. 1 2 2 becomes 1 2
            // equiv of
            // face_vhandles.erase( std::unique( face_vhandles.begin(), face_vhandles.end() ),
            //                     face_vhandles.end() );
            // but handles wedges and normals
            // see (https://en.cppreference.com/w/cpp/algorithm/unique)
            {
                auto first       = face_vhandles.begin();
                auto wedgeFirst  = face_wedges.begin();
                auto normalFirst = face_normals.begin();
                auto last        = face_vhandles.end();

                if ( first != last ) {
                    auto result       = first;
                    auto wedgeResult  = wedgeFirst;
                    auto normalResult = normalFirst;
                    while ( ++first != last ) {
                        if ( !( *result == *first ) ) {
                            ++result;
                            ++wedgeResult;
                            ++normalResult;
                            if ( result != first ) {
                                *result       = std::move( *first );
                                *wedgeResult  = std::move( *wedgeFirst );
                                *normalResult = std::move( *normalFirst );
                            }
                        }
                    }
                    face_vhandles.erase( result + 1, face_vhandles.end() );
                    face_wedges.erase( wedgeResult + 1, face_wedges.end() );
                    face_normals.erase( normalResult + 1, face_normals.end() );
                }
            }

            ///\todo and "cross face ?"
            // unique sort size == vhandles size, if not split ...

            TopologicalMesh::FaceHandle fh;
            // skip 2 vertex face
            if ( face_vhandles.size() > 2 ) fh = add_face( face_vhandles );

            // In case of topological inconsistancy, face will be invalid (or uninitialized <>
            // invalid)
            if ( fh.is_valid() ) {
                for ( size_t vindex = 0; vindex < face_vhandles.size(); vindex++ ) {
                    TopologicalMesh::HalfedgeHandle heh =
                        halfedge_handle( face_vhandles[vindex], fh );
                    if ( hasNormals ) set_normal( heh, face_normals[vindex] );
                    property( m_wedgeIndexPph, heh ) = m_wedges.newReference( face_wedges[vindex] );
                }
            }
            else { command.process( face_vhandles ); }
            face_vhandles.clear();
            face_normals.clear();
        }
    };

    if ( abstractLayer.hasSemantic( TriangleIndexLayer::staticSemanticName ) ) {
        const auto& faces = static_cast<const TriangleIndexLayer&>( abstractLayer ).collection();
        LOG( logDEBUG ) << "TopologicalMesh: process " << faces.size() << " triangular faces ";
        processFaces( faces );
    }
    else if ( abstractLayer.hasSemantic( PolyIndexLayer::staticSemanticName ) ) {
        const auto& faces = static_cast<const PolyIndexLayer&>( abstractLayer ).collection();
        LOG( logDEBUG ) << "TopologicalMesh: process " << faces.size() << " polygonal faces ";
        processFaces( faces );
    }

    command.postProcess( *this );
    if ( hasNormals ) {
        m_normalsIndex =
            m_wedges.getWedgeAttribIndex<Normal>( getAttribName( MeshAttrib::VERTEX_NORMAL ) );

        m_vertexFaceWedgesWithSameNormals.clear();
        m_vertexFaceWedgesWithSameNormals.resize( n_vertices() );

        for ( auto itr = vertices_begin(), stop = vertices_end(); itr != stop; ++itr ) {
            std::unordered_map<TopologicalMesh::Normal,
                               std::pair<std::set<FaceHandle>, std::set<WedgeIndex>>,
                               hash_vec>
                normalSharedByWedges;

            auto vh = *itr;

            for ( ConstVertexIHalfedgeIter vh_it = cvih_iter( vh ); vh_it.is_valid(); ++vh_it ) {
                const auto& widx = property( m_wedgeIndexPph, *vh_it );
                if ( widx.isValid() && !m_wedges.getWedge( widx ).isDeleted() ) {
                    auto oldNormal = m_wedges.getWedgeData<Normal>( widx, m_normalsIndex );
                    normalSharedByWedges[oldNormal].first.insert( face_handle( *vh_it ) );
                    normalSharedByWedges[oldNormal].second.insert( widx );
                }
            }

            for ( const auto& pair : normalSharedByWedges ) {
                for ( const auto& fh : pair.second.first ) {
                    auto& v = m_vertexFaceWedgesWithSameNormals[vh.idx()][fh.idx()];
                    v.insert( v.end(), pair.second.second.begin(), pair.second.second.end() );
                }
            }
        }
    }
    LOG( logDEBUG ) << "TopologicalMesh: load end with  " << m_wedges.size() << " wedges ";
}

template <typename T>
void TopologicalMesh::copyAttribToWedgeData( const MultiIndexedGeometry& mesh,
                                             unsigned int vindex,
                                             const std::vector<AttribHandle<T>>& attrHandleVec,
                                             VectorArray<T>* to ) {
    for ( auto handle : attrHandleVec ) {
        auto& attr = mesh.template getAttrib<T>( handle );
        to->push_back( attr.data()[vindex] );
    }
}

void TopologicalMesh::copyMeshToWedgeData( const MultiIndexedGeometry& mesh,
                                           unsigned int vindex,
                                           const std::vector<AttribHandle<Scalar>>& wprop_float,
                                           const std::vector<AttribHandle<Vector2>>& wprop_vec2,
                                           const std::vector<AttribHandle<Vector3>>& wprop_vec3,
                                           const std::vector<AttribHandle<Vector4>>& wprop_vec4,
                                           TopologicalMesh::WedgeData* wd ) {
    copyAttribToWedgeData( mesh, vindex, wprop_float, &wd->m_floatAttrib );
    copyAttribToWedgeData( mesh, vindex, wprop_vec2, &wd->m_vector2Attrib );
    copyAttribToWedgeData( mesh, vindex, wprop_vec3, &wd->m_vector3Attrib );
    copyAttribToWedgeData( mesh, vindex, wprop_vec4, &wd->m_vector4Attrib );
}

inline void TopologicalMesh::propagate_normal_to_wedges( VertexHandle vh ) {
    if ( !has_halfedge_normals() ) {
        LOG( logERROR ) << "TopologicalMesh has no normals, nothing set";
        return;
    }
    for ( VertexIHalfedgeIter vih_it = vih_iter( vh ); vih_it.is_valid(); ++vih_it ) {
        auto wd = getWedgeData( property( getWedgeIndexPph(), *vih_it ) );

        m_wedges.setWedgeAttrib( wd, getAttribName( MeshAttrib::VERTEX_NORMAL ), normal( vh ) );

        replaceWedge( *vih_it, wd );
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

inline std::set<TopologicalMesh::WedgeIndex>
TopologicalMesh::getVertexWedges( OpenMesh::VertexHandle vh ) const {
    std::set<TopologicalMesh::WedgeIndex> ret;

    for ( ConstVertexIHalfedgeIter vh_it = cvih_iter( vh ); vh_it.is_valid(); ++vh_it ) {
        auto widx = property( m_wedgeIndexPph, *vh_it );
        if ( widx.isValid() && !m_wedges.getWedge( widx ).isDeleted() ) ret.insert( widx );
    }
    return ret;
}

inline TopologicalMesh::WedgeIndex
TopologicalMesh::getWedgeIndex( OpenMesh::HalfedgeHandle heh ) const {
    return property( getWedgeIndexPph(), heh );
}

inline unsigned int TopologicalMesh::getWedgeRefCount( const WedgeIndex& idx ) const {
    return m_wedges.getWedgeRefCount( idx );
}

template <typename T>
inline bool TopologicalMesh::setWedgeData( const TopologicalMesh::WedgeIndex& idx,
                                           const std::string& name,
                                           const T& value ) {
    return setWedgeAttrib( idx, name, value );
}

template <typename T>
inline bool TopologicalMesh::setWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                             const std::string& name,
                                             const T& value ) {
    return m_wedges.setWedgeAttrib( idx, name, value );
}

inline void TopologicalMesh::setWedgeData( TopologicalMesh::WedgeIndex widx,
                                           const TopologicalMesh::WedgeData& wedge ) {
    m_wedges.setWedgeData( widx, wedge );
}

inline const TopologicalMesh::WedgeData&
TopologicalMesh::getWedgeData( const WedgeIndex& idx ) const {
    return m_wedges.getWedgeData( idx );
}

template <typename T>
inline const T& TopologicalMesh::getWedgeData( const TopologicalMesh::WedgeIndex& idx,
                                               const std::string& name ) const {
    return getWedgeAttrib<T>( idx, name );
}

template <typename T>
inline const T& TopologicalMesh::getWedgeAttrib( const TopologicalMesh::WedgeIndex& idx,
                                                 const std::string& name ) const {
    return m_wedges.getWedgeData<T>( idx, name );
}

inline TopologicalMesh::WedgeIndex TopologicalMesh::replaceWedge( OpenMesh::HalfedgeHandle he,
                                                                  const WedgeData& wd ) {
    m_wedges.del( property( getWedgeIndexPph(), he ) );
    auto index                         = m_wedges.add( wd );
    property( getWedgeIndexPph(), he ) = index;
    return index;
}

inline void TopologicalMesh::replaceWedgeIndex( OpenMesh::HalfedgeHandle he,
                                                const WedgeIndex& widx ) {
    m_wedges.del( property( getWedgeIndexPph(), he ) );
    property( getWedgeIndexPph(), he ) = m_wedges.newReference( widx );
}

inline void TopologicalMesh::mergeEqualWedges() {
    for ( auto itr = vertices_begin(), stop = vertices_end(); itr != stop; ++itr ) {
        mergeEqualWedges( *itr );
    }
}

inline void TopologicalMesh::mergeEqualWedges( OpenMesh::VertexHandle vh ) {
    for ( auto itr = vih_iter( vh ); itr.is_valid(); ++itr ) {
        // replace will search if wedge already present and use it, so merge occurs.
        if ( !is_boundary( *itr ) )
            replaceWedge( *itr, getWedgeData( property( getWedgeIndexPph(), *itr ) ) );
    }
}

inline const std::vector<std::string>& TopologicalMesh::getVec4AttribNames() const {
    return m_wedges.m_vector4AttribNames;
}
inline const std::vector<std::string>& TopologicalMesh::getVec3AttribNames() const {
    return m_wedges.m_vector3AttribNames;
}
inline const std::vector<std::string>& TopologicalMesh::getVec2AttribNames() const {
    return m_wedges.m_vector2AttribNames;
}
inline const std::vector<std::string>& TopologicalMesh::getFloatAttribNames() const {
    return m_wedges.m_floatAttribNames;
}

inline bool TopologicalMesh::isFeatureVertex( const VertexHandle& vh ) const {
    return getVertexWedges( vh ).size() != 1;
}

inline bool TopologicalMesh::isFeatureEdge( const EdgeHandle& eh ) const {
    auto heh0 = halfedge_handle( eh, 0 );
    auto heh1 = halfedge_handle( eh, 1 );

    return property( m_wedgeIndexPph, heh0 ) !=
               property( m_wedgeIndexPph,
                         prev_halfedge_handle( opposite_halfedge_handle( heh0 ) ) ) ||
           property( m_wedgeIndexPph, heh1 ) !=
               property( m_wedgeIndexPph,
                         prev_halfedge_handle( opposite_halfedge_handle( heh1 ) ) );
}

inline const OpenMesh::HPropHandleT<TopologicalMesh::WedgeIndex>&
TopologicalMesh::getWedgeIndexPph() const {
    return m_wedgeIndexPph;
}

} // namespace Geometry
} // namespace Core
} // namespace Ra
