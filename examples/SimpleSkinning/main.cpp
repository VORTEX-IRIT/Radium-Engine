#include <Core/Animation/Skeleton.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Core/Tasks/Task.hpp>
#include <Core/Types.hpp>
#include <Engine/FrameInfo.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/CameraManager.hpp>
#include <Engine/Scene/DefaultLightManager.hpp>
#include <Engine/Scene/DirLight.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/SkeletonBasedAnimationSystem.hpp>
#include <Engine/Scene/SkeletonComponent.hpp>
#include <Engine/Scene/SkinningComponent.hpp>
#include <Engine/Scene/System.hpp>
#include <Engine/Scene/SystemDisplay.hpp>
#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>
#include <Gui/Viewer/Viewer.hpp>

using namespace Ra;
using namespace Ra::Core;
using namespace Ra::Core::Animation;
using namespace Ra::Core::Math;
using namespace Ra::Engine;
using namespace Ra::Engine::Scene;

class SkinningSystem : public Scene::System
{
  public:
    void setSkeletonAndSkinning( SkeletonComponent* skel, SkinningComponent* skin ) {
        m_skel = skel;
        m_skin = skin;
    }

    void generateTasks( TaskQueue* q, const FrameInfo& info ) override {

        if ( !m_skel || !m_skin ) return;

        m_t = info.m_animationTime;

        auto animTaskId = q->registerTask( std::make_unique<Ra::Core::FunctionTask>(
            [this]() {
                Transform transform;
                transform =
                    Transform( AngleAxis( cos( m_t + Math::Pi ), Vector3( 1_ra, 0_ra, 0_ra ) ) *
                               Translation( Vector3( 0_ra, 0_ra, 5_ra ) ) );

                m_skel->setBoneTransform( 2, transform, Skeleton::SpaceType::LOCAL );
                transform = Transform( AngleAxis( cos( m_t ), Vector3( 1_ra, 0_ra, 0_ra ) ) *
                                       Translation( Vector3( 0_ra, 0_ra, 5_ra ) ) );

                m_skel->setBoneTransform( 3, transform, Skeleton::SpaceType::LOCAL );
            },
            "AnimTask_" + m_skin->getMeshName() ) );

        auto skinFunc = std::bind( &SkinningComponent::skin, m_skin );
        auto skinTask = std::make_unique<Core::FunctionTask>(
            skinFunc, "SkinnerTask_" + m_skin->getMeshName() );
        auto endFunc = std::bind( &SkinningComponent::endSkinning, m_skin );
        auto endTask = std::make_unique<Core::FunctionTask>(
            endFunc, "SkinnerEndTask_" + m_skin->getMeshName() );

        auto skinTaskId = q->registerTask( std::move( skinTask ) );
        auto endTaskId  = q->registerTask( std::move( endTask ) );
        q->addDependency( animTaskId, skinTaskId );
        q->addDependency( skinTaskId, endTaskId );
    }

  private:
    Scalar m_t { 0_ra };
    SkeletonComponent* m_skel { nullptr };
    SkinningComponent* m_skin { nullptr };
};

void setupScene( Ra::Engine::RadiumEngine* engine ) {

    DefaultLightManager* lightManager =
        static_cast<DefaultLightManager*>( engine->getSystem( "DefaultLightManager" ) );
    auto light = new Engine::Scene::DirectionalLight(
        Ra::Engine::Scene::SystemEntity::getInstance(), "light" );
    lightManager->addLight( light );

    auto animationSystem = new SkinningSystem;
    engine->registerSystem( "Simple animation system", animationSystem );

    auto entity = engine->getEntityManager()->createEntity( "Cylinder" );

    auto cylinder =
        Geometry::makeCylinder( { 0_ra, 0_ra, 0_ra }, { 0_ra, 0_ra, 10_ra }, 2_ra, 16, 32 );

    // component ownership is transfered to entity in component ctor
    auto meshComponent =
        new TriangleMeshComponent( "Cylinder", entity, std::move( cylinder ), nullptr );

    // create a squeleton with three bones.
    std::map<std::string, Core::Transform> boneMatrices;
    std::map<std::string, std::vector<std::pair<uint, Scalar>>> boneWeights;

    auto skel      = Ra::Core::Animation::Skeleton();
    auto transform = Transform::Identity();
    auto rootId    = skel.addRoot( transform, "root" );
    // boneMatrice transform a vertex from object space to rest pose bone space.
    boneMatrices["root"] = skel.getTransform( rootId, Skeleton::SpaceType::MODEL ).inverse();

    // root to first bone is not drawn, so add the first real bone here
    transform         = Transform( Translation( Vector3( 0_ra, 0_ra, 0_ra ) ) );
    auto id1          = skel.addBone( rootId, transform, Skeleton::SpaceType::MODEL, "1" );
    boneMatrices["1"] = skel.getTransform( id1, Skeleton::SpaceType::MODEL ).inverse();

    transform         = Transform( Translation( Vector3( 0_ra, 0_ra, 5_ra ) ) );
    auto id2          = skel.addBone( id1, transform, Skeleton::SpaceType::MODEL, "2" );
    boneMatrices["2"] = skel.getTransform( id2, Skeleton::SpaceType::MODEL ).inverse();

    // weight for bone 2 along z axis (since the cylinder is z-aligned)
    std::vector<std::pair<uint, Scalar>> w;
    auto& vertices = meshComponent->getCoreGeometry().vertices();
    for ( int i = 0; i < vertices.size(); ++i ) {
        w.emplace_back( i, 1_ra - smoothstep( 0.3_ra, 0.7_ra, vertices[i].z() / 10_ra ) );
    }
    boneWeights["2"] = w;

    transform         = Transform( Translation( Vector3( 0_ra, 0_ra, 10_ra ) ) );
    auto id3          = skel.addBone( id2, transform, Skeleton::SpaceType::MODEL, "3" );
    boneMatrices["3"] = skel.getTransform( id3, Skeleton::SpaceType::MODEL ).inverse();

    // second bone weight, to sum to one according to bone 2 weight.
    for ( auto& p : w ) {
        p.second = 1_ra - p.second;
    }
    boneWeights["3"] = w;

    // add skeleton component, to store (and draw) the skinning skeleton
    // this work here since skeleton has an empty name
    /// \todo make skeleton and skinning component communication work better, with name setter on
    /// skeleton, or simply set skeleton in skinning component
    auto skeletonComponent = new SkeletonComponent( "AC_Cylinder", entity );
    skeletonComponent->setSkeleton( skel );
    skeletonComponent->initialize();
    skeletonComponent->setXray( true );
    skeletonComponent->toggleSkeleton( true );

    // skinning component to perform skinning
    auto skinningComponent =
        new SkinningComponent( "SK_Cylinder", SkinningComponent::SkinningType::LBS, entity );
    skinningComponent->setMeshName( meshComponent->getName() );
    skinningComponent->setPerBoneMatrix( std::move( boneMatrices ) );
    skinningComponent->setPerBoneWeight( std::move( boneWeights ) );
    skinningComponent->initialize();

    // add these two to our animationSystem to animate bone and update skinning each frame
    animationSystem->setSkeletonAndSkinning( skeletonComponent, skinningComponent );
}

int main( int argc, char* argv[] ) {
    Gui::BaseApplication app( argc, argv );
    app.initialize( Ra::Gui::SimpleWindowFactory {} );

    app.m_engine->setEndTime( std::numeric_limits<Scalar>::max() );
    app.m_engine->setRealTime( true );
    app.m_engine->play( true );

    setupScene( app.m_engine );

    app.m_mainWindow->prepareDisplay();

    return app.exec();
}
