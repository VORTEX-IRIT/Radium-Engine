#pragma once

#include <Core/Math/Math.hpp>
#include <Engine/RaEngine.hpp>
#include <Engine/Scene/Light.hpp>

namespace Ra {
namespace Engine {

namespace Scene {
class Entity;
/** Spot light for rendering.
 *
 */
class RA_ENGINE_API SpotLight final : public Ra::Engine::Scene::Light
{

  public:
    explicit SpotLight( Entity* entity, const std::string& name = "spotlight" );
    ~SpotLight() override = default;

    void setPosition( const Eigen::Matrix<Scalar, 3, 1>& position ) override;
    inline const Eigen::Matrix<Scalar, 3, 1>& getPosition() const;

    void setDirection( const Eigen::Matrix<Scalar, 3, 1>& direction ) override;
    inline const Eigen::Matrix<Scalar, 3, 1>& getDirection() const;

    inline void setInnerAngleInRadians( Scalar angle );
    inline void setOuterAngleInRadians( Scalar angle );
    inline void setInnerAngleInDegrees( Scalar angle );
    inline void setOuterAngleInDegrees( Scalar angle );

    inline Scalar getInnerAngle() const;
    inline Scalar getOuterAngle() const;

    inline void setAttenuation( const Attenuation& attenuation );
    inline void setAttenuation( Scalar constant, Scalar linear, Scalar quadratic );
    inline const Attenuation& getAttenuation() const;

    std::string getShaderInclude() const override;

  private:
    Eigen::Matrix<Scalar, 3, 1> m_position { 0, 0, 0 };
    Eigen::Matrix<Scalar, 3, 1> m_direction { 0, -1, 0 };

    Scalar m_innerAngle { Core::Math::PiDiv4 };
    Scalar m_outerAngle { Core::Math::PiDiv2 };

    Attenuation m_attenuation { 1, 0, 0 };
};

// ---------------------------------------------------------------------------------------------
// ---- inline methods implementation

inline void SpotLight::setPosition( const Eigen::Matrix<Scalar, 3, 1>& position ) {
    m_position = position;
    getRenderParameters().setVariable( "light.spot.position", m_position );
}

inline const Eigen::Matrix<Scalar, 3, 1>& Scene::SpotLight::getPosition() const {
    return m_position;
}

inline void SpotLight::setDirection( const Eigen::Matrix<Scalar, 3, 1>& direction ) {
    m_direction = direction.normalized();
    getRenderParameters().setVariable( "light.spot.direction", m_direction );
}

inline const Eigen::Matrix<Scalar, 3, 1>& SpotLight::getDirection() const {
    return m_direction;
}

inline void SpotLight::setInnerAngleInRadians( Scalar angle ) {
    m_innerAngle = angle;
    getRenderParameters().setVariable( "light.spot.innerAngle", m_innerAngle );
}

inline void SpotLight::setOuterAngleInRadians( Scalar angle ) {
    m_outerAngle = angle;
    getRenderParameters().setVariable( "light.spot.outerAngle", m_outerAngle );
}

inline void SpotLight::setInnerAngleInDegrees( Scalar angle ) {
    m_innerAngle = angle * Core::Math::toRad;
}

inline void SpotLight::setOuterAngleInDegrees( Scalar angle ) {
    m_outerAngle = Core::Math::toRad * angle;
}

inline Scalar SpotLight::getInnerAngle() const {
    return m_innerAngle;
}

inline Scalar SpotLight::getOuterAngle() const {
    return m_outerAngle;
}

inline void SpotLight::setAttenuation( const Attenuation& attenuation ) {
    m_attenuation = attenuation;
    getRenderParameters().setVariable( "light.spot.attenuation.constant", m_attenuation.constant );
    getRenderParameters().setVariable( "light.spot.attenuation.linear", m_attenuation.linear );
    getRenderParameters().setVariable( "light.spot.attenuation.quadratic",
                                       m_attenuation.quadratic );
}

inline void SpotLight::setAttenuation( Scalar constant, Scalar linear, Scalar quadratic ) {
    setAttenuation( { constant, linear, quadratic } );
}

inline const SpotLight::Attenuation& SpotLight::getAttenuation() const {
    return m_attenuation;
}

} // namespace Scene
} // namespace Engine
} // namespace Ra
