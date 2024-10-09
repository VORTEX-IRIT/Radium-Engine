#pragma once

#include <Core/Asset/GeometryData.hpp>
#include <Core/Asset/VolumeData.hpp>
#include <Core/Containers/MakeShared.hpp>
#include <Core/Geometry/IndexedGeometry.hpp>
#include <Core/Geometry/TriangleMesh.hpp>
#include <Core/Geometry/Volume.hpp>
#include <Engine/Data/BlinnPhongMaterial.hpp>
#include <Engine/Data/Material.hpp>
#include <Engine/Data/MaterialConverters.hpp>
#include <Engine/Data/Mesh.hpp>
#include <Engine/RaEngine.hpp>
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Scene/Component.hpp>
#include <Engine/Scene/ComponentMessenger.hpp>
#include <Engine/Scene/Entity.hpp>

namespace Ra {
namespace Engine {
namespace Data {
class VolumeObject;
} // namespace Data

namespace Scene {

/// Abstract interface of a geometric compoennet in the Engine.
class RA_ENGINE_API GeometryComponent : public Component
{
  public:
    GeometryComponent( const std::string& name, Entity* entity ) :
        Component( name, entity ), m_contentName( name ) {}
    ~GeometryComponent() override {}

    void initialize() override {}

    //  public:
    // Component communication management
    virtual void setupIO( const std::string& id );
    void setContentName( const std::string& name ) { m_contentName = name; }
    //    void setDeformable( bool b );

    /// Returns the index of the associated RO (the display mesh)
    //    Ra::Core::Utils::Index getRenderObjectIndex() const;
  protected:
  private:
    const Ra::Core::Utils::Index* roIndexRead() const;

  protected:
    // the index of the renderObject
    Ra::Core::Utils::Index m_roIndex {};
    std::string m_contentName {};
};

/*!
 * \brief Main class to convert Ra::Core::Asset::GeometryData to Ra::Engine::Mesh
 *
 * Exports access to the mesh geometry:
 *  - SurfaceMeshComponent: get, rw (set vertices, normals and triangles dirty)
 *  - Vertices: rw (if deformable)
 *  - normals: rw (if deformable)
 *  - triangles: rw (if deformable)
 */
template <typename CoreMeshType>
class SurfaceMeshComponent : public GeometryComponent
{
    using base = GeometryComponent;

  public:
    using RenderMeshType = typename Data::RenderMeshType::getType<CoreMeshType>::Type;

    inline SurfaceMeshComponent( const std::string& name,
                                 Entity* entity,
                                 const Ra::Core::Asset::GeometryData* data );
    inline SurfaceMeshComponent( const std::string& name,
                                 Entity* entity,
                                 std::shared_ptr<RenderMeshType> data );

    /*!
     * Constructor from an existing mesh
     * \warning Moves the mesh and takes its ownership
     */
    inline SurfaceMeshComponent( const std::string& name,
                                 Entity* entity,
                                 CoreMeshType&& mesh,
                                 Core::Asset::MaterialData* mat = nullptr );
    inline SurfaceMeshComponent( const std::string& name,
                                 Entity* entity,
                                 CoreMeshType&& mesh,
                                 std::shared_ptr<Data::Material> mat );
    ~SurfaceMeshComponent() override = default;

    /// Returns the current display geometry.
    inline const CoreMeshType& getCoreGeometry() const;
    inline RenderMeshType* getDisplayable();

    // Component communication management
    inline void setupIO( const std::string& id ) override;
    inline void setDeformable( bool b );

  private:
    inline void generateMesh( const Ra::Core::Asset::GeometryData* data );

    inline std::shared_ptr<Data::Material>
    convertMatdataToMaterial( const Core::Asset::MaterialData* data );

    inline void finalizeROFromGeometry( std::shared_ptr<Data::Material> roMaterial,
                                        Core::Transform transform );

    // Give access to the mesh and (if deformable) to update it
    inline const CoreMeshType* getMeshOutput() const;
    inline CoreMeshType* getMeshRw();

  private:
    // directly hold a reference to the displayMesh to simplify accesses in handlers
    std::shared_ptr<RenderMeshType> m_displayMesh { nullptr };
};

using TriangleMeshComponent        = SurfaceMeshComponent<Ra::Core::Geometry::MultiIndexedGeometry>;
using GeometryDisplayableComponent = SurfaceMeshComponent<Ra::Core::Geometry::MultiIndexedGeometry>;
using QuadMeshComponent            = SurfaceMeshComponent<Ra::Core::Geometry::MultiIndexedGeometry>;
using PolyMeshComponent            = SurfaceMeshComponent<Ra::Core::Geometry::MultiIndexedGeometry>;

/// \warning, WIP
/// \todo doc.
class RA_ENGINE_API PointCloudComponent : public GeometryComponent
{
    using base = GeometryComponent;

  public:
    PointCloudComponent( const std::string& name,
                         Entity* entity,
                         const Ra::Core::Asset::GeometryData* data );

    /*!
     * Constructor from an existing mesh
     * \warning Moves the mesh and takes its ownership
     */
    PointCloudComponent( const std::string& name,
                         Entity* entity,
                         Core::Geometry::PointCloud&& mesh,
                         Core::Asset::MaterialData* mat = nullptr );

    ~PointCloudComponent() override;

    void initialize() override;

    /// Returns the current display geometry.
    const Ra::Core::Geometry::PointCloud& getCoreGeometry() const;
    Data::PointCloud* getGeometry();

    /// set the splat size for rendering
    inline void setSplatSize( float s ) { m_splatSize = s; }

    /// get the splat size for rendering
    inline float getSplatSize() const { return m_splatSize; }

  public:
    // Component communication management
    void setupIO( const std::string& id ) override;
    void setDeformable( bool b );

  private:
    void generatePointCloud( const Ra::Core::Asset::GeometryData* data );

    void finalizeROFromGeometry( const Core::Asset::MaterialData* data, Core::Transform transform );

    // Give access to the mesh and (if deformable) to update it
    const Ra::Core::Geometry::PointCloud* getMeshOutput() const;
    Ra::Core::Geometry::PointCloud* getPointCloudRw();

  private:
    // directly hold a reference to the displayMesh to simplify accesses in handlers
    std::shared_ptr<Data::PointCloud> m_displayMesh { nullptr };
    // The diameter of the splat when rendered
    float m_splatSize { 0.0025f };
};

/*-----------------------------------------------------------------------------------------------*/

/*!
 * \brief Main class to convert Ra::Core::Asset::VolumeData to Ra::Engine::VolumeObject
 *
 * Exports access to the volume:
 *  - VolumeObject: get, rw
 */
class RA_ENGINE_API VolumeComponent : public Component
{
  public:
    VolumeComponent( const std::string& name,
                     Entity* entity,
                     const Ra::Core::Asset::VolumeData* data );
    ~VolumeComponent() override;

    void initialize() override;

  public:
    // Component communication management
    void setupIO( const std::string& id );
    void setContentName( const std::string& name );

    /// Returns the index of the associated RO (the display volume)
    Ra::Core::Utils::Index getRenderObjectIndex() const;
    Data::VolumeObject* getDisplayVolume();

  private:
    void generateVolumeRender( const Ra::Core::Asset::VolumeData* data );

    const Ra::Core::Geometry::AbstractVolume* getVolumeOutput() const;
    Ra::Core::Geometry::AbstractVolume* getVolumeRw();

    const Ra::Core::Utils::Index* roIndexRead() const;

  private:
    Ra::Core::Utils::Index m_volumeIndex {};
    std::string m_contentName {};
    std::shared_ptr<Data::VolumeObject> m_displayVolume { nullptr };
};

template <typename CoreMeshType>
SurfaceMeshComponent<CoreMeshType>::SurfaceMeshComponent(
    const std::string& name,
    Entity* entity,
    const Ra::Core::Asset::GeometryData* data ) :
    GeometryComponent( name, entity ), m_displayMesh( nullptr ) {
    generateMesh( data );
}

template <>
RA_ENGINE_API SurfaceMeshComponent<Ra::Core::Geometry::MultiIndexedGeometry>::SurfaceMeshComponent(
    const std::string& name,
    Entity* entity,
    Ra::Core::Geometry::MultiIndexedGeometry&& mesh,
    Core::Asset::MaterialData* mat );

template <typename CoreMeshType>
SurfaceMeshComponent<CoreMeshType>::SurfaceMeshComponent( const std::string& name,
                                                          Entity* entity,
                                                          std::shared_ptr<RenderMeshType> data ) :
    GeometryComponent( name, entity ), m_displayMesh( data ) {
    finalizeROFromGeometry( convertMatdataToMaterial( nullptr ), Core::Transform::Identity() );
}

template <typename CoreMeshType>
SurfaceMeshComponent<CoreMeshType>::SurfaceMeshComponent( const std::string& name,
                                                          Entity* entity,
                                                          CoreMeshType&& mesh,
                                                          Core::Asset::MaterialData* mat ) :
    GeometryComponent( name, entity ),
    m_displayMesh( new RenderMeshType( name, std::move( mesh ) ) ) {
    setContentName( name );
    finalizeROFromGeometry( convertMatdataToMaterial( mat ), Core::Transform::Identity() );
}

template <typename CoreMeshType>
SurfaceMeshComponent<CoreMeshType>::SurfaceMeshComponent(
    const std::string& name,
    Entity* entity,
    CoreMeshType&& mesh,
    std::shared_ptr<Ra::Engine::Data::Material> mat ) :
    GeometryComponent( name, entity ),
    m_displayMesh( new RenderMeshType( name, std::move( mesh ) ) ) {
    setContentName( name );
    finalizeROFromGeometry( mat, Core::Transform::Identity() );
}
template <typename CoreMeshType>
void SurfaceMeshComponent<CoreMeshType>::generateMesh( const Ra::Core::Asset::GeometryData* data ) {
    m_contentName     = data->getName();
    m_displayMesh     = Ra::Core::make_shared<RenderMeshType>( m_contentName );
    CoreMeshType mesh = Data::createCoreMeshFromGeometryData<CoreMeshType>( data );

    m_displayMesh->loadGeometry( std::move( mesh ) );

    finalizeROFromGeometry(
        convertMatdataToMaterial( data->hasMaterial() ? &( data->getMaterial() ) : nullptr ),
        data->getFrame() );
}

template <>
RA_ENGINE_API void SurfaceMeshComponent<Ra::Core::Geometry::MultiIndexedGeometry>::generateMesh(
    const Ra::Core::Asset::GeometryData* data );

template <typename CoreMeshType>
std::shared_ptr<Data::Material> SurfaceMeshComponent<CoreMeshType>::convertMatdataToMaterial(
    const Core::Asset::MaterialData* data ) {
    // The technique for rendering this component
    std::shared_ptr<Data::Material> roMaterial;
    // First extract the material from asset or create a default one
    if ( data != nullptr ) {
        auto converter = Data::EngineMaterialConverters::getMaterialConverter( data->getType() );
        auto mat       = converter.second( data );
        roMaterial.reset( mat );
    }
    else {
        auto mat = new Data::BlinnPhongMaterial( m_contentName + "_DefaultBPMaterial" );
        mat->setRenderAsSplat( m_displayMesh->getNumFaces() == 0 );
        mat->setColoredByVertexAttrib( m_displayMesh->getCoreGeometry().hasAttrib(
            Ra::Core::Geometry::getAttribName( Ra::Core::Geometry::VERTEX_COLOR ) ) );
        roMaterial.reset( mat );
    }
    return roMaterial;
}

template <typename CoreMeshType>
void SurfaceMeshComponent<CoreMeshType>::finalizeROFromGeometry(
    std::shared_ptr<Data::Material> roMaterial,
    Core::Transform transform ) {

    // initialize with a default rendertechique that draws nothing
    std::string roName( m_name + "_" + m_contentName + "_RO" );
    auto ro = Rendering::RenderObject::createRenderObject( roName,
                                                           this,
                                                           Rendering::RenderObjectType::Geometry,
                                                           m_displayMesh,
                                                           Rendering::RenderTechnique {} );
    ro->setTransparent( roMaterial->isTransparent() );
    ro->setMaterial( roMaterial );
    setupIO( m_contentName );
    ro->setLocalTransform( transform );
    m_roIndex = addRenderObject( ro );
}

#ifndef CHECK_MESH_NOT_NULL
#    define CHECK_MESH_NOT_NULL                \
                                               \
        CORE_ASSERT( m_displayMesh != nullptr, \
                     "DisplayMesh should exist while component is alive" );
#    define CHECK_MESH_NOT_NULL_UNDEF
#endif

template <typename CoreMeshType>
const CoreMeshType& SurfaceMeshComponent<CoreMeshType>::getCoreGeometry() const {
    CHECK_MESH_NOT_NULL;
    return m_displayMesh->getCoreGeometry();
}

template <typename CoreMeshType>
typename SurfaceMeshComponent<CoreMeshType>::RenderMeshType*
SurfaceMeshComponent<CoreMeshType>::getDisplayable() {
    CHECK_MESH_NOT_NULL;
    return m_displayMesh.get();
}

template <typename CoreMeshType>
void SurfaceMeshComponent<CoreMeshType>::setupIO( const std::string& id ) {
    CHECK_MESH_NOT_NULL;

    const auto& cm = ComponentMessenger::getInstance();
    auto cbOut     = std::bind( &SurfaceMeshComponent<CoreMeshType>::getMeshOutput, this );
    auto cbRw      = std::bind( &SurfaceMeshComponent<CoreMeshType>::getMeshRw, this );

    cm->registerOutput<CoreMeshType>( getEntity(), this, id, cbOut );
    cm->registerReadWrite<CoreMeshType>( getEntity(), this, id, cbRw );
    if ( std::is_convertible<CoreMeshType*, Core::Geometry::AttribArrayGeometry*>() &&
         !std::is_same<CoreMeshType, Core::Geometry::AttribArrayGeometry>() ) {

        cm->registerOutput<Core::Geometry::AttribArrayGeometry>( getEntity(), this, id, cbOut );
        cm->registerReadWrite<Core::Geometry::AttribArrayGeometry>( getEntity(), this, id, cbRw );
    }

    base::setupIO( id );
}

template <typename CoreMeshType>
const CoreMeshType* SurfaceMeshComponent<CoreMeshType>::getMeshOutput() const {
    CHECK_MESH_NOT_NULL;
    return &m_displayMesh->getCoreGeometry();
}

template <typename CoreMeshType>
CoreMeshType* SurfaceMeshComponent<CoreMeshType>::getMeshRw() {
    CHECK_MESH_NOT_NULL;
    return &( m_displayMesh->getCoreGeometry() );
}

#ifdef CHECK_MESH_NOT_NULL_UNDEF
#    undef CHECK_MESH_NOT_NULL
#endif

} // namespace Scene
} // namespace Engine
} // namespace Ra
