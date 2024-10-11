#pragma once

#include <Engine/RaEngine.hpp>
#include <Engine/Scene/Light.hpp>

namespace Ra {
namespace Engine {

namespace Scene {
class Entity;
/** Point light for rendering
 *
 */
class RA_ENGINE_API PointLight final : public Ra::Engine::Scene::Light
{
  public:
    explicit PointLight( Entity* entity, const std::string& name = "pointlight" );
    ~PointLight() override = default;

    void setPosition( const Eigen::Matrix<Scalar, 3, 1>& pos ) override;
    inline const Eigen::Matrix<Scalar, 3, 1>& getPosition() const;

    inline void setAttenuation( const Attenuation& attenuation );
    inline void setAttenuation( Scalar constant, Scalar linear, Scalar quadratic );
    inline const Attenuation& getAttenuation() const;

    std::string getShaderInclude() const override;

  private:
    Eigen::Matrix<Scalar, 3, 1> m_position { 0, 0, 0 };

    Attenuation m_attenuation { 1, 0, 0 };
};

// ---------------------------------------------------------------------------------------------
// ---- inline methods implementation

inline void PointLight::setPosition( const Eigen::Matrix<Scalar, 3, 1>& pos ) {
    m_position = pos;
    getRenderParameters().setVariable( "light.point.position", m_position );
}

inline const Eigen::Matrix<Scalar, 3, 1>& PointLight::getPosition() const {
    return m_position;
}

inline void PointLight::setAttenuation( const PointLight::Attenuation& attenuation ) {
    m_attenuation = attenuation;
    getRenderParameters().setVariable( "light.point.attenuation.constant", m_attenuation.constant );
    getRenderParameters().setVariable( "light.point.attenuation.linear", m_attenuation.linear );
    getRenderParameters().setVariable( "light.point.attenuation.quadratic",
                                       m_attenuation.quadratic );
}

inline void PointLight::setAttenuation( Scalar constant, Scalar linear, Scalar quadratic ) {
    setAttenuation( { constant, linear, quadratic } );
}

inline const PointLight::Attenuation& PointLight::getAttenuation() const {
    return m_attenuation;
}
} // namespace Scene
} // namespace Engine
} // namespace Ra
