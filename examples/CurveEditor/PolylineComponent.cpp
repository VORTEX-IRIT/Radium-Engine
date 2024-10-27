#include <PolylineComponent.hpp>

#include <Core/Asset/FileData.hpp>
#include <Core/Containers/MakeShared.hpp>
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

PolylineComponent::PolylineComponent( Ra::Engine::Scene::Entity* entity,
                                      Vector3Array polylinePoints ) :
    Ra::Engine::Scene::Component( "Polyline Component", entity ), m_polylinePts( polylinePoints ) {}

/// This function is called when the component is properly
/// setup, i.e. it has an entity.
void PolylineComponent::initialize() {
    auto plainMaterial              = make_shared<PlainMaterial>( "Plain Material" );
    plainMaterial->m_perVertexColor = true;

    // Render mesh
    auto renderObject1 = RenderObject::createRenderObject(
        "PolyMesh",
        this,
        RenderObjectType::Geometry,
        DrawPrimitives::LineStrip( m_polylinePts,
                                   Vector4Array { m_polylinePts.size(), { 0, 0, 0.7, 1 } } ),
        Ra::Engine::Rendering::RenderTechnique {} );
    renderObject1->setMaterial( plainMaterial );
    addRenderObject( renderObject1 );
}
