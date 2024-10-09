#pragma once

#include <Core/Asset/GeometryData.hpp>
#include <Core/Containers/VectorArray.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Core/Geometry/StandardAttribNames.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <Core/Utils/BijectiveAssociation.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/Log.hpp>
#include <Core/Utils/ObjectWithSemantic.hpp>
#include <Engine/Data/DisplayableObject.hpp>
#include <Engine/Data/ShaderProgram.hpp>
#include <Engine/RaEngine.hpp>

#include <globjects/Buffer.h>
#include <globjects/Program.h>
#include <globjects/VertexArray.h>
#include <globjects/VertexAttributeBinding.h>

#include <array>
#include <iterator>
#include <map>
#include <vector>

namespace Ra {
namespace Engine {
namespace Data {
class ShaderProgram;

using namespace Ra::Core::Utils;

/// VAO + VBO attributes management,
/// also manage draw calls
class RA_ENGINE_API Vao
{
    /// \todo not used for now ... but may be if we allow multiple vao per mesh
    std::unique_ptr<globjects::VertexArray> m_vao;
    std::vector<std::unique_ptr<globjects::Buffer>> m_vbos;
    std::vector<bool> m_dataDirty;
    std::map<std::string, int> m_handleToBuffer;
};

/**
 * A class representing an openGL general mesh to be displayed.
 * It stores the vertex attributes, indices, and can be rendered
 * with a specific render mode (e.g. GL_TRIANGLES or GL_LINES).
 * It maintains the attributes and keeps them in sync with the GPU.
 * \note Attribute names are used to automatic location binding when using shaders.
 *
 * \TODO Remove rendermode attribute
 */
class RA_ENGINE_API AttribArrayDisplayable : public Displayable
{
  public:
    /// Render mode enum used when render()/
    /// values taken from OpenGL specification
    /// \see https://www.khronos.org/registry/OpenGL/api/GL/glcorearb.h
    enum MeshRenderMode : uint {
        RM_POINTS                   = 0x0000,
        RM_LINES                    = 0x0001, // decimal value: 1
        RM_LINE_LOOP                = 0x0002, // decimal value: 2
        RM_LINE_STRIP               = 0x0003, // decimal value: 3
        RM_TRIANGLES                = 0x0004, // decimal value: 4
        RM_TRIANGLE_STRIP           = 0x0005, // decimal value: 5
        RM_TRIANGLE_FAN             = 0x0006, // decimal value: 6
        RM_QUADS                    = 0x0007, // decimal value: 7
        RM_QUAD_STRIP               = 0x0008, // decimal value: 8
        RM_POLYGON                  = 0x0009, // decimal value: 9
        RM_LINES_ADJACENCY          = 0x000A, // decimal value: 10
        RM_LINE_STRIP_ADJACENCY     = 0x000B, // decimal value: 11
        RM_TRIANGLES_ADJACENCY      = 0x000C, // decimal value: 12
        RM_TRIANGLE_STRIP_ADJACENCY = 0x000D, // decimal value: 13
        RM_PATCHES                  = 0x000E, // decimal value: 14
    };

  public:
    explicit AttribArrayDisplayable( const std::string& name,
                                     MeshRenderMode renderMode = RM_TRIANGLES );

    AttribArrayDisplayable( const AttribArrayDisplayable& rhs ) = delete;
    void operator=( const AttribArrayDisplayable& rhs )         = delete;

    // no need to detach listener since TriangleMesh is owned by Mesh.
    ~AttribArrayDisplayable() {}

    using Displayable::getName;

    /// Set the render mode.
    inline void setRenderMode( MeshRenderMode mode );
    /// Get the render mode.
    inline MeshRenderMode getRenderMode() const;

    /// \name
    /// Mark attrib data as dirty, forcing an update of the OpenGL buffer.
    ///\{

    /// Use g_attribName to find the matching name and call setDirty(const std::string& name).
    /// \param type: the data to set to MeshAttrib
    void setDirty( const Core::Geometry::MeshAttrib& type );

    /// \param name: data buffer name to set to dirty
    void setDirty( const std::string& name );

    /// If index is greater than then number of buffer, this function as no effect.
    /// \param index: the data buffer index to set to dirty.
    void setDirty( unsigned int index );
    ///\}

    /// This function is called at the start of the rendering.
    /// It will update the necessary openGL buffers.
    void updateGL() override = 0;

    /// \name
    /// Core::Geometry getters.
    ///\{
    virtual const Core::Geometry::AttribArrayGeometry& getAttribArrayGeometry() const = 0;
    virtual Core::Geometry::AttribArrayGeometry& getAttribArrayGeometry()             = 0;
    ///\}

    /// \brief Get opengl's vbo handle (uint) matching to attrib \b name.
    ///
    /// If vbo is not initialized or name do not correponds to an actual attrib name, the returned
    /// optional is empty
    Ra::Core::Utils::optional<gl::GLuint> getVboHandle( const std::string& name );

    /// \brief Get opengl's vao handle (uint).
    ///
    /// If vao is not initialized, the returned optional is empty
    Ra::Core::Utils::optional<gl::GLuint> getVaoHandle();

  protected:
    /// Update the picking render mode according to the object render mode
    void updatePickingRenderMode();

    class AttribObserver
    {
      public:
        explicit AttribObserver( AttribArrayDisplayable* displayable, int idx ) :
            m_displayable( displayable ), m_idx( idx ) {}
        void operator()() {
            if ( m_idx < int( m_displayable->m_dataDirty.size() ) ) {
                m_displayable->m_dataDirty[m_idx] = true;
                m_displayable->m_isDirty          = true;
            }
            else {
                /// \todo Should never be here
                LOG( logDEBUG ) << "Invalid dirty bit notified on " << m_displayable->getName();
            }
        }

      private:
        AttribArrayDisplayable* m_displayable;
        int m_idx;
    };

  protected:
    std::unique_ptr<globjects::VertexArray> m_vao;

    MeshRenderMode m_renderMode { MeshRenderMode::RM_TRIANGLES };

    // m_vbos and m_dataDirty have the same size and are indexed thru m_handleToBuffer[attribName]
    std::vector<std::unique_ptr<globjects::Buffer>> m_vbos;
    std::vector<bool> m_dataDirty;

    // Geometry attrib name (std::string) to buffer id (int)
    // buffer id are indices in m_vbos and m_dataDirty
    std::map<std::string, unsigned int> m_handleToBuffer;

    /// \brief General dirty bit of the mesh.
    ///
    /// Must be equivalent of the "or" of the other dirty flags. An empty mesh is not dirty
    bool m_isDirty { false };
};

/// Concept class to ensure consistent naming of VaoIndices accross derived classes.
class RA_ENGINE_API VaoIndices
{
  public:
    /// Tag the indices as dirty, asking for a update to gpu.
    inline void setIndicesDirty();

    ///\todo Add test for Indices observer
    class IndicesObserver
    {
      public:
        /// not tested
        explicit IndicesObserver( VaoIndices* displayable ) : m_displayable { displayable } {}
        /// not tested
        void operator()() { m_displayable->m_indicesDirty = true; }

      private:
        VaoIndices* m_displayable { nullptr };
    };

  protected:
    // vbo
    std::unique_ptr<globjects::Buffer> m_indices { nullptr };
    bool m_indicesDirty { true };
    /// number of elements to draw (i.e number of indices to use)
    /// automatically set by updateGL(), not meaningfull if m_indicesDirty.
    size_t m_numElements { 0 };
};

/// Template class to manage the Displayable aspect of a Core Geomertry, such as TriangleMesh.
template <typename T>
class CoreGeometryDisplayable : public AttribArrayDisplayable
{
  public:
    using base         = AttribArrayDisplayable;
    using CoreGeometry = T;

    explicit CoreGeometryDisplayable( const std::string& name,
                                      MeshRenderMode renderMode = RM_TRIANGLES );

    // no need to detach observer in dtor since CoreGeometry is owned by this, and CoreGeometry dtor
    // will detachAll observers.

    /// \name
    /// Core::Geometry getters
    ///\{
    inline const Core::Geometry::AbstractGeometry& getAbstractGeometry() const override;
    inline Core::Geometry::AbstractGeometry& getAbstractGeometry() override;

    inline const Core::Geometry::AttribArrayGeometry& getAttribArrayGeometry() const override;
    inline Core::Geometry::AttribArrayGeometry& getAttribArrayGeometry() override;

    inline const CoreGeometry& getCoreGeometry() const;
    inline CoreGeometry& getCoreGeometry();
    ///\}

    /// Helper function that calls Ra::Core::CoreGeometry::addAttrib()
    template <typename A>
    inline Ra::Core::Utils::AttribHandle<A> addAttrib( const std::string& name,
                                                       const typename Core::VectorArray<A>& data );
    inline size_t getNumVertices() const override;

    /// Use the given geometry as base for a display mesh.
    /// This will move \p mesh and *this will take the ownership
    /// of the data.
    /// The currently owned mesh is deleted, if any.
    /// This method should be called to set or replace the CoreGeometry, if you
    /// want to update attributes or indices, use getCoreGeometry and
    /// Core::Geometry::AttribArrayGeometry setters instead.
    /// \warning For indices, you must call setIndicesDirty after modification.
    /// \todo add observer mecanism for indices.
    virtual void loadGeometry( CoreGeometry&& mesh );

    /// Update (i.e. send to GPU) the buffers marked as dirty
    void updateGL() override;

    /// Bind meshAttribName to shaderAttribName.
    /// meshAttribName is a vertex attrib added to the underlying CoreGeometry
    /// shaderAttribName is the name of the input paramter of the shader.
    /// By default the same name is used, but this mecanism allows to override
    /// this behavior.
    /// Only one shaderAttribName can be bound to a meshAttribName and the other
    /// way round.
    /// \param meshAttribName: name of the attribute on the CoreGeomtry side
    /// \param shaderAttribName: name of the input vertex attribute on the
    /// shader side.
    void setAttribNameMatching( const std::string& meshAttribName,
                                const std::string& shaderAttribName );

  protected:
    virtual void updateGL_specific_impl() {}

    void loadGeometry_common( CoreGeometry&& mesh );
    void setupCoreMeshObservers();

    /// assume m_vao is bound.
    void autoVertexAttribPointer( const ShaderProgram* prog );

    /// m_mesh Observer method, called whenever an attrib is added or removed from
    /// m_mesh.
    /// it adds an observer to the new attrib.
    void addAttribObserver( const std::string& name );

    void addToTranslationTable( const std::string& name );

    /// Core::Mesh attrib name to Render::Mesh attrib name
    /// key: core mesh name, value: shader name
    BijectiveAssociation<std::string, std::string> m_translationTable {};

    CoreGeometry m_mesh;
};

/// A PointCloud without indices
class RA_ENGINE_API PointCloud : public CoreGeometryDisplayable<Core::Geometry::PointCloud>
{
    using base = CoreGeometryDisplayable<Core::Geometry::PointCloud>;

  public:
    using base::CoreGeometryDisplayable;
    inline explicit PointCloud(
        const std::string& name,
        typename base::CoreGeometry&& geom,
        typename base::MeshRenderMode renderMode = base::MeshRenderMode::RM_POINTS );

    inline explicit PointCloud( const std::string& name, MeshRenderMode renderMode = RM_POINTS );

    /// use glDrawArrays to draw all the points in the point cloud
    void render( const ShaderProgram* prog ) override;

    void loadGeometry( Core::Geometry::PointCloud&& mesh ) override;

  protected:
    void updateGL_specific_impl() override;
};

/// An engine mesh owning CoreGeometry, with indices
template <typename T>
class IndexedGeometry : public CoreGeometryDisplayable<T>, public VaoIndices
{
  public:
    using base = CoreGeometryDisplayable<T>;
    using CoreGeometryDisplayable<T>::CoreGeometryDisplayable;
    explicit IndexedGeometry(
        const std::string& name,
        typename base::CoreGeometry&& geom,
        typename base::MeshRenderMode renderMode = base::MeshRenderMode::RM_TRIANGLES );

    void render( const ShaderProgram* prog ) override;

    void loadGeometry( T&& mesh ) override;

  protected:
    void updateGL_specific_impl() override;
};

/// An engine mesh owning a MultiIndexedCoreGeometry, with multiple indices layer.
class RA_ENGINE_API GeometryDisplayable : public AttribArrayDisplayable
{
  public:
    using base = AttribArrayDisplayable;

    using LayerSemanticCollection =
        typename Core::Geometry::MultiIndexedGeometry::LayerSemanticCollection;
    using LayerSemantic = typename Core::Geometry::MultiIndexedGeometry::LayerSemantic;
    using LayerKeyType  = typename Core::Geometry::MultiIndexedGeometry::LayerKeyType;
    using LayerKeyHash  = Core::Geometry::MultiIndexedGeometry::LayerKeyHash;

    explicit GeometryDisplayable( const std::string& name );
    explicit GeometryDisplayable( const std::string& name,
                                  typename Core::Geometry::MultiIndexedGeometry&& geom );
    virtual ~GeometryDisplayable();
    void render( const ShaderProgram* prog ) override;
    void render( const ShaderProgram* prog, const LayerKeyType& key );

    ///@{
    /**  Returns the underlying CoreGeometry as an Core::Geometry::AbstractGeometry */
    inline const Core::Geometry::AbstractGeometry& getAbstractGeometry() const override {
        return m_geom;
    }
    inline Core::Geometry::AbstractGeometry& getAbstractGeometry() override { return m_geom; }
    ///@}
    inline const Core::Geometry::AttribArrayGeometry& getAttribArrayGeometry() const override {
        return m_geom;
    }
    inline Core::Geometry::AttribArrayGeometry& getAttribArrayGeometry() override { return m_geom; }

    inline Core::Geometry::MultiIndexedGeometry& getCoreGeometry() { return m_geom; }
    inline const Core::Geometry::MultiIndexedGeometry& getCoreGeometry() const { return m_geom; }

    /// Bind meshAttribName to shaderAttribName.
    /// meshAttribName is a vertex attrib added to the underlying CoreGeometry
    /// shaderAttribName is the name of the input paramter of the shader.
    /// By default the same name is used, but this mecanism allows to override
    /// this behavior.
    /// Only one shaderAttribName can be bound to a meshAttribName and the other
    /// way round.
    /// \param meshAttribName: name of the attribute on the CoreGeometry side
    /// \param shaderAttribName: name of the input vertex attribute on the
    /// shader side.
    void setAttribNameMatching( const std::string& meshAttribName,
                                const std::string& shaderAttribName );

    void loadGeometry( Core::Geometry::MultiIndexedGeometry&& mesh );
    inline void loadGeometry( Core::Geometry::MultiIndexedGeometry&& mesh,
                              LayerKeyType key,
                              base::MeshRenderMode renderMode ) {
        loadGeometry( std::move( mesh ) );
        addRenderLayer( key, renderMode );
    }

    /// \param r is a collection of keys and renderMode, e.g. { {key1, RM_TRIANGLES}, {key2,
    /// RM_LINES} }
    template <typename RangeOfLayerKeys>
    inline void loadGeometry( Core::Geometry::MultiIndexedGeometry&& mesh,
                              const RangeOfLayerKeys& r ) {
        loadGeometry( std::move( mesh ) );
        for ( const auto& k : r )
            addRenderLayer( k.first, k.second );
    }
    bool addRenderLayer( LayerKeyType key, base::MeshRenderMode renderMode );
    bool removeRenderLayer( LayerKeyType key );
    // bool setRenderMode( LayerKeyType key, RenderMode );
    // RenderMode getRenderMode( LayerKeyType key );

    /// Update (i.e. send to GPU) the buffers marked as dirty
    void updateGL() override;

    inline size_t getNumVertices() const override { return m_geom.vertices().size(); }

  protected:
    void setupCoreMeshObservers();

    /// assume m_vao is bound.
    void autoVertexAttribPointer( const ShaderProgram* prog, const LayerKeyType& key );

    /// m_mesh Observer method, called whenever an attrib is added or removed from
    /// m_mesh.
    /// it adds an observer to the new attrib.
    void addAttribObserver( const std::string& name );

    void addToTranslationTable( const std::string& name );

  private:
    Core::Geometry::MultiIndexedGeometry m_geom;

    // for vertex attribs, with dirty
    struct VBOEntryType {
        bool dirty { false };
        std::unique_ptr<globjects::Buffer> buffer { nullptr };
    };

    // for indices, with dirty and  num elements
    struct IndicesVBO {
        bool dirty { false };
        std::unique_ptr<globjects::Buffer> buffer { nullptr };
        size_t numElements { 0 };
    };

    /// LayerKey with its corresponding indices.
    struct LayerEntryType {
        int observerId { -1 };
        std::unique_ptr<globjects::VertexArray> vao { nullptr };
        IndicesVBO indices;
        base::MeshRenderMode renderMode { RM_TRIANGLES };

        inline LayerEntryType() = default;
    };

    /// The collection of indices layer we can use for rendering
    std::unordered_map<LayerKeyType, LayerEntryType, LayerKeyHash> m_geomLayers;

    /// "main" triangle layer
    LayerKeyType m_activeLayerKey;

    /// \todo use this in place of m_vbos
    //    using VBOCollection = std::vector<VBOEntryType>;
    /// Collection of VBOs for per-vertex attributes
    //    VBOCollection m_attribVBOs;

    /// Core::Mesh attrib name to Render::Mesh attrib name
    /// key: core mesh name, value: shader name
    BijectiveAssociation<std::string, std::string> m_translationTable {};
};

/// LineMesh, own a Core::Geometry::LineMesh
class RA_ENGINE_API LineMesh : public IndexedGeometry<Core::Geometry::LineMesh>
{
    using base = IndexedGeometry<Core::Geometry::LineMesh>;

  public:
    using base::IndexedGeometry;
    inline explicit LineMesh(
        const std::string& name,
        typename base::CoreGeometry&& geom,
        typename base::MeshRenderMode renderMode = base::MeshRenderMode::RM_LINES );
    inline explicit LineMesh(
        const std::string& name,
        typename base::MeshRenderMode renderMode = base::MeshRenderMode::RM_LINES );

  protected:
  private:
};

/// Mesh, own a Core::Geometry::TriangleMesh
class RA_ENGINE_API Mesh : public IndexedGeometry<Core::Geometry::TriangleMesh>
{
    using base = IndexedGeometry<Core::Geometry::TriangleMesh>;

  public:
    using base::IndexedGeometry;
    size_t getNumFaces() const override;

    /**
     * Use the given vertices and indices to build a display mesh according to
     * the MeshRenderMode.
     * \note This has to be used for non RM_TRIANGLES meshes only.
     * \note Also removes all vertex attributes.
     * \warning This will disappear as soon as old code will be removed.
     */
    using base::loadGeometry;
    [[deprecated]] void loadGeometry( const Core::Vector3Array& vertices,
                                      const std::vector<uint>& indices );

  protected:
  private:
};

/// GeneralMesh, own a Mesh of type T ( e.g. Core::Geometry::PolyMesh or Core::Geometry::QuadMesh)
/// This class handle the GPU representation of a polyhedron mesh.
/// Each face of the polyhedron (typically quads) are assume to be planar and convex.
/// Simple triangulation is performed on the fly before sending data to the GPU.
template <typename T>
class GeneralMesh : public IndexedGeometry<T>
{
    using base      = IndexedGeometry<T>;
    using IndexType = Core::Vector3ui;

  public:
    using base::IndexedGeometry;
    inline size_t getNumFaces() const override;

  protected:
    inline void updateGL_specific_impl() override;

  private:
    Core::VectorArray<IndexType> m_triangleIndices;
};

using PolyMesh = GeneralMesh<Core::Geometry::PolyMesh>;
using QuadMesh = GeneralMesh<Core::Geometry::QuadMesh>;

/// create a TriangleMesh, PolyMesh or other Core::*Mesh from GeometryData
/// \todo replace the copy of all geometry data by reference to original data.
template <typename CoreMeshType>
CoreMeshType createCoreMeshFromGeometryData( const Ra::Core::Asset::GeometryData* data ) {
    CoreMeshType mesh;
    typename CoreMeshType::IndexContainerType indices;

    if ( !data->isLineMesh() ) {
        auto& geo = data->getGeometry();
        const auto& [layerKeyType, layerBase] =
            geo.getFirstLayerOccurrence( mesh.getLayerKey().first );
        const auto& layer = static_cast<
            const Core::Geometry::GeometryIndexLayer<typename CoreMeshType::IndexType>&>(
            layerBase );
        const auto& faces = layer.collection();
        indices.reserve( faces.size() );
        std::copy( faces.begin(), faces.end(), std::back_inserter( indices ) );
    }
#if 0
    // TODO manage line meshes in a "usual" way, i.e. as an indexed geometry with specific
    //  rendering properties (i.e. shader, as it is the case for point clouds)
    // Create a degenerated triangle to handle edges case.
    else {
        const auto& edges = ... access the LineIndexLayer
        indices.reserve( edges.size() );
        std::transform(
            edges.begin(), edges.end(), std::back_inserter( indices ), []( Ra::Core::Vector2ui v ) {
                return ( Ra::Core::Vector3ui { v( 0 ), v( 1 ), v( 1 ) } );
            } );
    }
#endif

    mesh.setIndices( std::move( indices ) );

    // This copy only "usual" attributes. See Core::Geometry::AttribManager::copyAllAttributes
    mesh.vertexAttribs().copyAllAttributes( data->getGeometry().vertexAttribs() );

    return mesh;
}

/// Helpers to get RenderMesh type from CoreMesh Type
namespace RenderMeshType {
template <class CoreMeshT>
struct getType {};

template <>
struct getType<Ra::Core::Geometry::LineMesh> {
    using Type = Ra::Engine::Data::LineMesh;
};

template <>
struct getType<Ra::Core::Geometry::TriangleMesh> {
    using Type = Ra::Engine::Data::Mesh;
};

template <>
struct getType<Ra::Core::Geometry::QuadMesh> {
    using Type = Ra::Engine::Data::QuadMesh;
};

template <>
struct getType<Ra::Core::Geometry::PolyMesh> {
    using Type = Ra::Engine::Data::PolyMesh;
};

template <>
struct getType<Ra::Core::Geometry::MultiIndexedGeometry> {
    using Type = Ra::Engine::Data::GeometryDisplayable;
};

} // namespace RenderMeshType

/// create Mesh, PolyMesh Engine::Data::*Mesh * from GeometryData
template <typename CoreMeshType>
typename RenderMeshType::getType<CoreMeshType>::Type*
createMeshFromGeometryData( const std::string& name, const Ra::Core::Asset::GeometryData* data ) {
    using MeshType = typename RenderMeshType::getType<CoreMeshType>::Type;

    auto mesh = createCoreMeshFromGeometryData<CoreMeshType>( data );

    MeshType* ret = new MeshType { name };
    ret->loadGeometry( std::move( mesh ) );

    return ret;
}

////////////////  AttribArrayDisplayable ///////////////////////////////

void AttribArrayDisplayable::setRenderMode( MeshRenderMode mode ) {
    m_renderMode = mode;
    updatePickingRenderMode();
}

AttribArrayDisplayable::MeshRenderMode AttribArrayDisplayable::getRenderMode() const {
    return m_renderMode;
}

///////////////// VaoIndices  ///////////////////////

void VaoIndices::setIndicesDirty() {
    m_indicesDirty = true;
}

////////////////  CoreGeometryDisplayable ///////////////////////////////

template <typename CoreGeometry>
CoreGeometryDisplayable<CoreGeometry>::CoreGeometryDisplayable( const std::string& name,
                                                                MeshRenderMode renderMode ) :
    base( name, renderMode ) {
    setupCoreMeshObservers();
}

template <typename CoreGeometry>
const Ra::Core::Geometry::AbstractGeometry&
CoreGeometryDisplayable<CoreGeometry>::getAbstractGeometry() const {
    return m_mesh;
}

template <typename CoreGeometry>
Ra::Core::Geometry::AbstractGeometry& CoreGeometryDisplayable<CoreGeometry>::getAbstractGeometry() {
    return m_mesh;
}

template <typename CoreGeometry>
const Ra::Core::Geometry::AttribArrayGeometry&
CoreGeometryDisplayable<CoreGeometry>::getAttribArrayGeometry() const {
    return m_mesh;
}

template <typename CoreGeometry>
Ra::Core::Geometry::AttribArrayGeometry&
CoreGeometryDisplayable<CoreGeometry>::getAttribArrayGeometry() {
    return m_mesh;
}

template <typename CoreGeometry>
const CoreGeometry& CoreGeometryDisplayable<CoreGeometry>::getCoreGeometry() const {
    return m_mesh;
}

template <typename CoreGeometry>
CoreGeometry& CoreGeometryDisplayable<CoreGeometry>::getCoreGeometry() {
    return m_mesh;
}

template <typename CoreGeometry>
void CoreGeometryDisplayable<CoreGeometry>::addToTranslationTable( const std::string& name ) {
    m_translationTable.insert( name, name );
}

template <typename CoreGeometry>
void CoreGeometryDisplayable<CoreGeometry>::addAttribObserver( const std::string& name ) {
    // this observer is called each time an attrib is added or removed from m_mesh
    auto attrib = m_mesh.getAttribBase( name );
    // if attrib not nullptr, then it's an attrib add, so attach an observer to it

    if ( attrib ) {
        auto itr = m_handleToBuffer.find( name );
        if ( itr == m_handleToBuffer.end() ) {
            m_handleToBuffer[name] = m_dataDirty.size();

            addToTranslationTable( name );

            m_dataDirty.push_back( true );
            m_vbos.emplace_back( nullptr );
        }
        auto idx = m_handleToBuffer[name];
        attrib->attach( AttribObserver( this, idx ) );
    }
    // else it's an attrib remove, do nothing, cleanup will be done in updateGL()
    else {}
}

template <typename CoreGeometry>
void CoreGeometryDisplayable<CoreGeometry>::autoVertexAttribPointer( const ShaderProgram* prog ) {

    auto glprog           = prog->getProgramObject();
    gl::GLint attribCount = glprog->get( GL_ACTIVE_ATTRIBUTES );

    for ( GLint idx = 0; idx < attribCount; ++idx ) {
        const gl::GLsizei bufSize = 256;
        gl::GLchar name[bufSize];
        gl::GLsizei length;
        gl::GLint size;
        gl::GLenum type;
        glprog->getActiveAttrib( idx, bufSize, &length, &size, &type, name );
        auto loc = glprog->getAttributeLocation( name );

        auto attribNameOpt = m_translationTable.keyIfExists( name );
        if ( attribNameOpt ) {
            auto attribName = *attribNameOpt;
            auto attrib     = m_mesh.getAttribBase( attribName );
            if ( attrib && attrib->getSize() > 0 ) {
                m_vao->enable( loc );
                auto binding = m_vao->binding( idx );
                binding->setAttribute( loc );
                CORE_ASSERT( m_vbos[m_handleToBuffer[attribName]].get(), "vbo is nullptr" );
#ifdef CORE_USE_DOUBLE
                binding->setBuffer( m_vbos[m_handleToBuffer[attribName]].get(),
                                    0,
                                    attrib->getNumberOfComponents() * sizeof( float ) );
#else

                binding->setBuffer(
                    m_vbos[m_handleToBuffer[attribName]].get(), 0, attrib->getStride() );
#endif
                binding->setFormat( attrib->getNumberOfComponents(), GL_SCALAR );
            }
            else { m_vao->disable( loc ); }
        }
        else { m_vao->disable( loc ); }
    }
}

template <typename T>
void CoreGeometryDisplayable<T>::loadGeometry_common( T&& mesh ) {
    m_mesh = std::move( mesh );
    setupCoreMeshObservers();
}

template <typename T>

void CoreGeometryDisplayable<T>::setupCoreMeshObservers() {
    int idx = 0;
    m_dataDirty.resize( m_mesh.vertexAttribs().getNumAttribs() );
    m_vbos.resize( m_mesh.vertexAttribs().getNumAttribs() );
    // here capture ref to idx to propagate idx incrementation
    m_mesh.vertexAttribs().for_each_attrib( [&idx, this]( Ra::Core::Utils::AttribBase* b ) {
        auto name              = b->getName();
        m_handleToBuffer[name] = idx;
        m_dataDirty[idx]       = true;

        // create a identity translation if name is not already translated.
        addToTranslationTable( name );

        b->attach( AttribObserver( this, idx ) );
        ++idx;
    } );

    // add an observer on attrib manipulation.
    m_mesh.vertexAttribs().attachMember(
        this, &CoreGeometryDisplayable<CoreGeometry>::addAttribObserver );
    m_isDirty = true;
}

/// Helper function that calls Ra::Core::CoreGeometry::addAttrib()
template <typename CoreGeometry>
template <typename A>
Ra::Core::Utils::AttribHandle<A>
CoreGeometryDisplayable<CoreGeometry>::addAttrib( const std::string& name,
                                                  const typename Core::VectorArray<A>& data ) {
    return m_mesh.addAttrib( name, data );
}

template <typename CoreGeometry>
size_t CoreGeometryDisplayable<CoreGeometry>::getNumVertices() const {
    return m_mesh.vertices().size();
}

template <typename CoreGeometry>
void CoreGeometryDisplayable<CoreGeometry>::loadGeometry( CoreGeometry&& /*mesh*/ ) {
    CORE_ASSERT( false, "must be specialized" );
}

template <typename CoreGeometry>
void CoreGeometryDisplayable<CoreGeometry>::updateGL() {
    if ( m_isDirty ) {
        // Check that our dirty bits are consistent.
        ON_ASSERT( bool dirtyTest = false; for ( auto d
                                                 : m_dataDirty ) { dirtyTest = dirtyTest || d; } );
        CORE_ASSERT( dirtyTest == m_isDirty, "Dirty flags inconsistency" );
        CORE_ASSERT( !( m_mesh.vertices().empty() ), "No vertex." );

        updateGL_specific_impl();
#ifdef CORE_USE_DOUBLE
        // need convserion
        auto func = [this]( Ra::Core::Utils::AttribBase* b ) {
            auto idx = m_handleToBuffer[b->getName()];

            if ( m_dataDirty[idx] ) {
                if ( !m_vbos[idx] ) { m_vbos[idx] = globjects::Buffer::create(); }

                auto stride      = b->getStride();
                auto eltSize     = b->getNumberOfComponents();
                auto size        = b->getSize();
                auto data        = std::make_unique<float[]>( size * eltSize );
                const void* ptr  = b->dataPtr();
                const char* cptr = reinterpret_cast<const char*>( ptr );

                for ( size_t i = 0; i < size; i++ ) {
                    auto tptr = reinterpret_cast<const Scalar*>( cptr + i * stride );
                    for ( size_t j = 0; j < eltSize; ++j ) {
                        data[i * eltSize + j] = tptr[j];
                    }
                }

                m_vbos[idx]->setData(
                    size * eltSize * sizeof( float ), data.get(), GL_DYNAMIC_DRAW );

                m_dataDirty[idx] = false;
            }
        };

#else
        auto func = [this]( Ra::Core::Utils::AttribBase* b ) {
            auto idx = m_handleToBuffer[b->getName()];

            if ( m_dataDirty[idx] ) {
                if ( !m_vbos[idx] ) { m_vbos[idx] = globjects::Buffer::create(); }
                m_vbos[idx]->setData( b->getBufferSize(), b->dataPtr(), GL_DYNAMIC_DRAW );
                m_dataDirty[idx] = false;
            }
        };
#endif
        m_mesh.vertexAttribs().for_each_attrib( func );

        // cleanup removed attrib
        for ( auto buffer : m_handleToBuffer ) {
            // do not remove name from handleToBuffer to keep index ...
            // we could also update handleToBuffer, m_vbos, m_dataDirty
            if ( !m_mesh.hasAttrib( buffer.first ) && m_vbos[buffer.second] ) {
                m_vbos[buffer.second].reset( nullptr );
                m_dataDirty[buffer.second] = false;
            }
        }

        GL_CHECK_ERROR;
        m_isDirty = false;
    }
}

template <typename CoreGeometry>
void CoreGeometryDisplayable<CoreGeometry>::setAttribNameMatching(
    const std::string& meshAttribName,
    const std::string& shaderAttribName ) {

    m_translationTable.replace( meshAttribName, shaderAttribName );
}

////////////////  IndexedGeometry  ///////////////////////////////

template <typename T>
IndexedGeometry<T>::IndexedGeometry( const std::string& name,
                                     typename base::CoreGeometry&& geom,
                                     typename base::MeshRenderMode renderMode ) :
    base( name, renderMode ) {
    loadGeometry( std::move( geom ) );
}

template <typename T>
void IndexedGeometry<T>::loadGeometry( T&& mesh ) {
    setIndicesDirty();
    base::loadGeometry_common( std::move( mesh ) );

    // indices
    base::m_mesh.attach( IndicesObserver( this ) );
}

template <typename T>
void IndexedGeometry<T>::updateGL_specific_impl() {
    if ( !m_indices ) {
        m_indices      = globjects::Buffer::create();
        m_indicesDirty = true;
    }
    if ( m_indicesDirty ) {
        /// this one do not work since m_indices is not a std::vector
        // m_indices->setData( m_mesh.m_indices, GL_DYNAMIC_DRAW );
        m_numElements =
            base::m_mesh.getIndices().size() * base::CoreGeometry::IndexType::RowsAtCompileTime;

        m_indices->setData(
            static_cast<gl::GLsizeiptr>( base::m_mesh.getIndices().size() *
                                         sizeof( typename base::CoreGeometry::IndexType ) ),
            base::m_mesh.getIndices().data(),
            GL_STATIC_DRAW );
        m_indicesDirty = false;
    }
    if ( !base::m_vao ) { base::m_vao = globjects::VertexArray::create(); }
    base::m_vao->bind();
    base::m_vao->bindElementBuffer( m_indices.get() );
    base::m_vao->unbind();
}

template <typename T>
void IndexedGeometry<T>::render( const ShaderProgram* prog ) {
    if ( base::m_vao ) {
        GL_CHECK_ERROR;
        base::m_vao->bind();
        base::autoVertexAttribPointer( prog );
        GL_CHECK_ERROR;
        base::m_vao->drawElements( static_cast<GLenum>( base::m_renderMode ),
                                   GLsizei( m_numElements ),
                                   GL_UNSIGNED_INT,
                                   nullptr );
        GL_CHECK_ERROR;
        base::m_vao->unbind();
        GL_CHECK_ERROR;
    }
}

///////// PointCloud //////////

PointCloud::PointCloud( const std::string& name,
                        typename base::CoreGeometry&& geom,
                        typename base::MeshRenderMode renderMode ) :
    base( name, renderMode ) {
    loadGeometry( std::move( geom ) );
}

PointCloud::PointCloud( const std::string& name, typename base::MeshRenderMode renderMode ) :
    base( name, renderMode ) {}

/////////  LineMesh ///////////

LineMesh::LineMesh( const std::string& name,
                    typename base::CoreGeometry&& geom,
                    typename base::MeshRenderMode renderMode ) :
    base( name, std::move( geom ), renderMode ) {}

LineMesh::LineMesh( const std::string& name, typename base::MeshRenderMode renderMode ) :
    base( name, renderMode ) {}

/////////  PolyMesh ///////////

template <typename T>
size_t GeneralMesh<T>::getNumFaces() const {
    return this->getCoreGeometry().getIndices().size();
}

template <typename T>
void GeneralMesh<T>::updateGL_specific_impl() {
    if ( !this->m_indices ) {
        this->m_indices      = globjects::Buffer::create();
        this->m_indicesDirty = true;
    }
    if ( this->m_indicesDirty ) {
        m_triangleIndices = Core::Geometry::triangulate( this->m_mesh.getIndices() );

        /// this one do not work since m_indices is not a std::vector
        // m_indices->setData( m_mesh.m_indices, GL_DYNAMIC_DRAW );
        this->m_numElements = m_triangleIndices.size() * GeneralMesh::IndexType::RowsAtCompileTime;

        this->m_indices->setData( static_cast<gl::GLsizeiptr>( m_triangleIndices.size() *
                                                               sizeof( GeneralMesh::IndexType ) ),
                                  m_triangleIndices.data(),
                                  GL_STATIC_DRAW );
        this->m_indicesDirty = false;
    }
    if ( !base::m_vao ) { base::m_vao = globjects::VertexArray::create(); }
    base::m_vao->bind();
    base::m_vao->bindElementBuffer( this->m_indices.get() );
    base::m_vao->unbind();
}

} // namespace Data
} // namespace Engine
} // namespace Ra
