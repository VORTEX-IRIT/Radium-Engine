#pragma once

#include "PolylineComponent.hpp"
#include <Core/Geometry/Curve2D.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/Component.hpp>
#include <Engine/Scene/EntityManager.hpp>

class PolylineFactory
{
  public:
    static PolylineComponent* createPolylineComponent( Ra::Engine::Scene::Entity* e,
                                                       Ra::Core::Vector3Array polylinePts ) {
        auto c = new PolylineComponent( e, polylinePts );
        c->initialize();
        return c;
    }
    static Ra::Engine::Scene::Entity*
    createCurveEntity( Ra::Core::VectorArray<Ra::Core::Geometry::Curve2D::Vector> polyline,
                       const std::string& name ) {
        auto engine                  = Ra::Engine::RadiumEngine::getInstance();
        Ra::Engine::Scene::Entity* e = engine->getEntityManager()->createEntity( name );
        Ra::Core::Vector3Array polylinePts;
        for ( auto& pt : polyline ) {
            polylinePts.push_back( Ra::Core::Vector3( pt.x(), 0, pt.y() ) );
        }
        createPolylineComponent( e, polylinePts );
        return e;
    }

  private:
};
