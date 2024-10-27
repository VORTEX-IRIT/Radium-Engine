#pragma once

#include "CurveComponent.hpp"
#include <Engine/Scene/Component.hpp>
#include <Engine/Scene/EntityManager.hpp>

class CurveFactory
{
  public:
    static CurveComponent*
    createCurveComponent( Ra::Engine::Scene::Entity* e, Ra::Core::Vector3Array ctrlPts, int id ) {
        std::string name = "Curve_" + std::to_string( id );
        auto c           = new CurveComponent( e, ctrlPts, name );
        c->initialize();
        return c;
    }
    static CurveComponent* createCurveComponent( Ra::Engine::Scene::Entity* e,
                                                 Ra::Core::Vector3Array ctrlPts,
                                                 const std::string& name ) {
        auto c = new CurveComponent( e, ctrlPts, name );
        c->initialize();
        return c;
    }
    static Ra::Engine::Scene::Entity* createCurveEntity( Ra::Core::Vector3Array ctrlPts,
                                                         const std::string& name ) {
        auto engine                  = Ra::Engine::RadiumEngine::getInstance();
        Ra::Engine::Scene::Entity* e = engine->getEntityManager()->createEntity( name );
        createCurveComponent( e, ctrlPts, name );
        return e;
    }

  private:
};
