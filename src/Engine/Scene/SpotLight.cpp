#include <Engine/Data/RenderParameters.hpp>
#include <Engine/Scene/SpotLight.hpp>

namespace Ra {
namespace Engine {
namespace Scene {
SpotLight::SpotLight( Entity* entity, const std::string& name ) :
    Light( entity, Light::SPOT, name ) {
    getRenderParameters().setVariable( "light.spot.position", m_position );
    getRenderParameters().setVariable( "light.spot.direction", m_direction );
    getRenderParameters().setVariable( "light.spot.innerAngle", m_innerAngle );
    getRenderParameters().setVariable( "light.spot.outerAngle", m_outerAngle );
    getRenderParameters().setVariable( "light.spot.attenuation.constant", m_attenuation.constant );
    getRenderParameters().setVariable( "light.spot.attenuation.linear", m_attenuation.linear );
    getRenderParameters().setVariable( "light.spot.attenuation.quadratic",
                                       m_attenuation.quadratic );
}

std::string SpotLight::getShaderInclude() const {
    return "Spot";
}

} // namespace Scene
} // namespace Engine
} // namespace Ra
