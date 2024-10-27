#pragma once

#include "PointComponent.hpp"
#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/Component.hpp>
#include <Engine/Scene/EntityManager.hpp>

class PointFactory
{
  public:
    static PointComponent*
    createPointComponent( Ra::Engine::Scene::Entity* e,
                          Ra::Core::Vector3 point,
                          const std::vector<unsigned int>& curveId,
                          int id,
                          Ra::Core::Utils::Color color = Ra::Core::Utils::Color::Black() ) {
        std::string name = "CtrlPt_" + std::to_string( id );
        auto c           = new PointComponent( e, point, curveId, name, color );
        c->initialize();
        return c;
    }

    static PointComponent*
    createPointComponent( Ra::Engine::Scene::Entity* e,
                          Ra::Core::Vector3 point,
                          const std::vector<unsigned int>& curveId,
                          const std::string& name,
                          Ra::Core::Utils::Color color = Ra::Core::Utils::Color::Black() ) {
        auto c = new PointComponent( e, point, curveId, name, color );
        c->initialize();
        return c;
    }

    static Ra::Engine::Scene::Entity*
    createPointEntity( Ra::Core::Vector3 point,
                       const std::string& name,
                       const std::vector<unsigned int>& curveId,
                       Ra::Core::Utils::Color color = Ra::Core::Utils::Color::Grey() ) {
        auto engine                  = Ra::Engine::RadiumEngine::getInstance();
        Ra::Engine::Scene::Entity* e = engine->getEntityManager()->createEntity( name );
        createPointComponent( e, point, curveId, name, color );
        return e;
    }

  private:
};
