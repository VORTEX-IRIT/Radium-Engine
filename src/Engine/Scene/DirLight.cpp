#include <Engine/Scene/DirLight.hpp>

#include <Engine/Data/RenderParameters.hpp>

namespace Ra {
namespace Engine {
namespace Scene {
DirectionalLight::DirectionalLight( Entity* entity, const std::string& name ) :
    Light( entity, Light::DIRECTIONAL, name ) {
    getRenderParameters().setVariable( "light.directional.direction", m_direction );
}

std::string DirectionalLight::getShaderInclude() const {
    return "Directional";
}

} // namespace Scene
} // namespace Engine
} // namespace Ra
