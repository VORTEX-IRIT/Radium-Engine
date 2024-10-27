#include <CurveComponent.hpp>

#include <Core/Asset/FileData.hpp>
#include <Core/Containers/MakeShared.hpp>
#include <Core/Geometry/Curve2D.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Core/Geometry/TopologicalMesh.hpp>
#include <Core/Tasks/Task.hpp>
#include <Core/Tasks/TaskQueue.hpp>
#include <Core/Utils/Timer.hpp>

#include <Engine/Data/Mesh.hpp>
#include <Engine/Data/PlainMaterial.hpp>
#include <Engine/FrameInfo.hpp>
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/RenderObjectManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <utility>

#ifdef IO_USE_ASSIMP
#    include <IO/AssimpLoader/AssimpFileLoader.hpp>
#endif

#include <random>

using namespace Ra;
using namespace Ra::Core;
using namespace Ra::Core::Utils;
using namespace Ra::Core::Geometry;
using namespace Ra::Engine;
using namespace Ra::Engine::Rendering;
using namespace Ra::Engine::Data;
using namespace Ra::Engine::Scene;

/**
 * This file contains a minimal radium/qt application which shows the geometrical primitives
 * supported by Radium
 */

CurveComponent::CurveComponent( Ra::Engine::Scene::Entity* entity,
                                Vector3Array ctrlPts,
                                const std::string& name ) :
    Ra::Engine::Scene::Component( name, entity ), m_ctrlPts( ctrlPts ) {}

/// This function is called when the component is properly
/// setup, i.e. it has an entity.
void CurveComponent::initialize() {
    auto plainMaterial              = make_shared<PlainMaterial>( "Plain Material" );
    plainMaterial->m_perVertexColor = true;

    auto bezier = CubicBezier( Vector2( m_ctrlPts[0].x(), m_ctrlPts[0].z() ),
                               Vector2( m_ctrlPts[1].x(), m_ctrlPts[1].z() ),
                               Vector2( m_ctrlPts[2].x(), m_ctrlPts[2].z() ),
                               Vector2( m_ctrlPts[3].x(), m_ctrlPts[3].z() ) );

    auto bezierVertices = Vector3Array();
    auto bezierColors   = Vector4Array();
    for ( unsigned int i = 0; i <= 100; i++ ) {
        float u = float( i ) / 100.f;
        auto fu = bezier.f( u );
        bezierVertices.push_back( Vector3( fu.x(), 0, fu.y() ) );
        bezierColors.push_back( { 1, 0, 0, 1 } );
    }

    // Render mesh
    auto renderObject1 =
        RenderObject::createRenderObject( "PolyMesh",
                                          this,
                                          RenderObjectType::Geometry,
                                          DrawPrimitives::LineStrip( bezierVertices, bezierColors ),
                                          Ra::Engine::Rendering::RenderTechnique {} );
    renderObject1->setMaterial( plainMaterial );
    addRenderObject( renderObject1 );

    auto renderObject2 = RenderObject::createRenderObject(
        "PolyMesh1",
        this,
        RenderObjectType::Geometry,
        DrawPrimitives::LineStrip( { m_ctrlPts[0], m_ctrlPts[1] }, { { 0, 0, 0, 1 } } ),
        Ra::Engine::Rendering::RenderTechnique {} );
    renderObject2->setMaterial( plainMaterial );
    addRenderObject( renderObject2 );

    auto renderObject3 = RenderObject::createRenderObject(
        "PolyMesh1",
        this,
        RenderObjectType::Geometry,
        DrawPrimitives::LineStrip( { m_ctrlPts[2], m_ctrlPts[3] }, { { 0, 0, 0, 1 } } ),
        Ra::Engine::Rendering::RenderTechnique {} );
    renderObject3->setMaterial( plainMaterial );
    addRenderObject( renderObject3 );
}
