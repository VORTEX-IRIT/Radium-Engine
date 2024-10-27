#include "PointComponent.hpp"

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

PointComponent::PointComponent( Ra::Engine::Scene::Entity* entity,
                                Ra::Core::Vector3 point,
                                const std::vector<unsigned int>& curveId,
                                const std::string& name,
                                Color color ) :
    Ra::Engine::Scene::Component( name, entity ),
    m_point( point ),
    m_defaultPoint( point ),
    m_curveId( curveId ),
    m_color( color ) {}

/// This function is called when the component is properly
/// setup, i.e. it has an entity.
void PointComponent::initialize() {
    auto plainMaterial              = make_shared<PlainMaterial>( "Plain Material" );
    plainMaterial->m_color          = m_color;
    plainMaterial->m_perVertexColor = false;

    auto circle = RenderObject::createRenderObject(
        "contourPt_circle",
        this,
        RenderObjectType::Geometry,
        DrawPrimitives::Disk( m_point, { 0_ra, 1_ra, 0_ra }, 0.1, 64, Color::White() ),
        {} );
    circle->setMaterial( plainMaterial );
    addRenderObject( circle );
}
