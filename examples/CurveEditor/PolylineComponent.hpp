#pragma once

#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/Entity.hpp>
#include <Engine/Scene/System.hpp>
#include <Engine/Scene/SystemDisplay.hpp>

class PolylineComponent : public Ra::Engine::Scene::Component
{

  public:
    PolylineComponent( Ra::Engine::Scene::Entity* entity, Ra::Core::Vector3Array polylinePoints );

    /// This function is called when the component is properly
    /// setup, i.e. it has an entity.
    void initialize() override;

    Ra::Core::Vector3Array m_polylinePts;
};
