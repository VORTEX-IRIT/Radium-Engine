#include <Engine/Data/LambertianMaterial.hpp>

#include <Engine/Data/ShaderConfigFactory.hpp>
#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Rendering/RenderTechnique.hpp>

#include <fstream>

namespace Ra {
namespace Engine {
namespace Data {
static const std::string materialName { "Lambertian" };

nlohmann::json LambertianMaterial::s_parametersMetadata = {};

LambertianMaterial::LambertianMaterial( const std::string& instanceName ) :
    SimpleMaterial( instanceName, materialName, Material::MaterialAspect::MAT_OPAQUE ) {}

void LambertianMaterial::registerMaterial() {
    // Get the Radium Resource location on the filesystem
    auto resourcesRootDir { RadiumEngine::getInstance()->getResourcesDir() };
    auto shaderProgramManager = RadiumEngine::getInstance()->getShaderProgramManager();

    shaderProgramManager->addNamedString(
        "/Lambertian.glsl", resourcesRootDir + "Shaders/Materials/Lambertian/Lambertian.glsl" );
    // registering re-usable shaders
    Data::ShaderConfiguration lpconfig(
        "Lambertian",
        resourcesRootDir + "Shaders/Materials/Lambertian/Lambertian.vert.glsl",
        resourcesRootDir + "Shaders/Materials/Lambertian/Lambertian.frag.glsl" );

    Data::ShaderConfigurationFactory::addConfiguration( lpconfig );

    Data::ShaderConfiguration zprepassconfig(
        "ZprepassLambertian",
        resourcesRootDir + "Shaders/Materials/Lambertian/Lambertian.vert.glsl",
        resourcesRootDir + "Shaders/Materials/Lambertian/LambertianZPrepass.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( zprepassconfig );

    // Registering technique
    Rendering::EngineRenderTechniques::registerDefaultTechnique(
        materialName, []( Rendering::RenderTechnique& rt, bool ) {
            // Lighting pass
            auto lightpass = Data::ShaderConfigurationFactory::getConfiguration( "Lambertian" );
            rt.setConfiguration( *lightpass, Rendering::DefaultRenderingPasses::LIGHTING_OPAQUE );
            // Z prepass
            auto zprepass =
                Data::ShaderConfigurationFactory::getConfiguration( "ZprepassLambertian" );
            rt.setConfiguration( *zprepass, Rendering::DefaultRenderingPasses::Z_PREPASS );
        } );

    // Registering parameters metadata
    SimpleMaterial::loadMetaData( s_parametersMetadata );
}

void LambertianMaterial::unregisterMaterial() {
    Rendering::EngineRenderTechniques::removeDefaultTechnique( materialName );
}

void LambertianMaterial::updateFromParameters() {
    auto& renderParameters = getParameters();
    setColor( renderParameters.getVariable<Core::Utils::Color>( "material.color" ) );
    setColoredByVertexAttrib( renderParameters.getVariable<bool>( "material.perVertexColor" ) );
}

nlohmann::json LambertianMaterial::getParametersMetadata() const {
    return s_parametersMetadata;
}

} // namespace Data
} // namespace Engine
} // namespace Ra
