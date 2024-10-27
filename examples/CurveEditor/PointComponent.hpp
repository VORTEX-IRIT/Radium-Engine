#pragma once

#include <Engine/RadiumEngine.hpp>
#include <Engine/Scene/Entity.hpp>
#include <Engine/Scene/System.hpp>
#include <Engine/Scene/SystemDisplay.hpp>

class PointComponent : public Ra::Engine::Scene::Component
{

  public:
    PointComponent( Ra::Engine::Scene::Entity* entity,
                    Ra::Core::Vector3 point,
                    const std::vector<unsigned int>& curveId,
                    const std::string& name,
                    Ra::Core::Utils::Color color );

    /// This function is called when the component is properly
    /// setup, i.e. it has an entity.
    void initialize() override;

    enum State { DEFAULT = 0, SMOOTH, SYMETRIC };

    State m_state { DEFAULT };
    Ra::Core::Vector3 m_point;
    Ra::Core::Vector3 m_defaultPoint;
    Ra::Core::Utils::Color m_color;
    std::vector<unsigned int> m_curveId;
};
