#include <Engine/Scene/PointLight.hpp>

#include <Engine/Data/RenderParameters.hpp>

namespace Ra {
namespace Engine {
namespace Scene {
PointLight::PointLight( Entity* entity, const std::string& name ) :
    Light( entity, Light::POINT, name ) {
    getRenderParameters().setVariable( "light.point.position", m_position );
    getRenderParameters().setVariable( "light.point.attenuation.constant", m_attenuation.constant );
    getRenderParameters().setVariable( "light.point.attenuation.linear", m_attenuation.linear );
    getRenderParameters().setVariable( "light.point.attenuation.quadratic",
                                       m_attenuation.quadratic );
}

std::string PointLight::getShaderInclude() const {
    return "Point";
}

} // namespace Scene
} // namespace Engine
} // namespace Ra
