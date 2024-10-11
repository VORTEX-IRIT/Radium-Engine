#include <Engine/Data/VolumetricMaterial.hpp>

#include <Engine/Data/ShaderConfigFactory.hpp>
#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/Data/Texture.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Rendering/RenderTechnique.hpp>

#include <fstream>

namespace Ra {
namespace Engine {
namespace Data {

static const std::string materialName { "Volumetric" };

nlohmann::json VolumetricMaterial::s_parametersMetadata = {};

VolumetricMaterial::VolumetricMaterial( const std::string& name ) :
    Material( name, materialName, Material::MaterialAspect::MAT_DENSITY ) {}

VolumetricMaterial::~VolumetricMaterial() {}

void VolumetricMaterial::updateGL() {
    if ( !isDirty() ) { return; }

    updateRenderingParameters();
    setClean();
}

void VolumetricMaterial::updateFromParameters() {
    auto& renderParameters = getParameters();
    m_sigma_a              = renderParameters.getVariable<Core::Utils::Color>( "material.sigma_a" );
    m_sigma_s              = renderParameters.getVariable<Core::Utils::Color>( "material.sigma_s" );
    m_g                    = renderParameters.getVariable<Scalar>( "material.g" );
    m_scale                = renderParameters.getVariable<Scalar>( "material.scale" );
    m_stepsize             = renderParameters.getVariable<Scalar>( "material.stepsize" );
}

void VolumetricMaterial::updateRenderingParameters() {
    auto& renderParameters = getParameters();
    renderParameters.setVariable( "material.sigma_a", m_sigma_a );
    renderParameters.setVariable( "material.sigma_s", m_sigma_s );
    renderParameters.setVariable( "material.g", m_g );
    renderParameters.setVariable( "material.scale", m_scale );
    if ( m_stepsize < 0 ) {
        auto dim = std::sqrt( Scalar( m_texture->getWidth() * m_texture->getWidth() +
                                      m_texture->getHeight() * m_texture->getHeight() +
                                      m_texture->getDepth() * m_texture->getDepth() ) );
        renderParameters.setVariable( "material.stepsize", 1._ra / dim );
    }
    else { renderParameters.setVariable( "material.stepsize", m_stepsize ); }
    renderParameters.setTexture( "material.density", m_texture );
    renderParameters.setVariable( "material.modelToDensity", m_modelToMedium.matrix() );
}

bool VolumetricMaterial::isTransparent() const {
    return false;
}

void VolumetricMaterial::registerMaterial() {
    // For resources access (glsl files) in a filesystem
    auto resourcesRootDir { RadiumEngine::getInstance()->getResourcesDir() };
    auto shaderProgramManager = RadiumEngine::getInstance()->getShaderProgramManager();

    shaderProgramManager->addShaderProgram(
        { { "ComposeVolume" },
          resourcesRootDir + "Shaders/2DShaders/Basic2D.vert.glsl",
          resourcesRootDir + "Shaders/Materials/Volumetric/ComposeVolumeRender.frag.glsl" } );

    // adding the material glsl implementation file
    shaderProgramManager->addNamedString(
        "/Volumetric.glsl", resourcesRootDir + "Shaders/Materials/Volumetric/Volumetric.glsl" );

    // registering re-usable shaders
    Data::ShaderConfiguration lpconfig(
        "Volumetric",
        resourcesRootDir + "Shaders/Materials/Volumetric/Volumetric.vert.glsl",
        resourcesRootDir + "Shaders/Materials/Volumetric/Volumetric.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( lpconfig );

    // registering re-usable shaders
    Data::ShaderConfiguration trconfig(
        "VolumetricOIT",
        resourcesRootDir + "Shaders/Materials/Volumetric/Volumetric.vert.glsl",
        resourcesRootDir + "Shaders/Materials/Volumetric/VolumetricOIT.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( trconfig );

    //    Ra::Engine::ShaderConfiguration zprepassconfig(
    //        "ZprepassRayMarching", "Shaders/Materials/RayMarching/RayMarching.vert.glsl",
    //        "Shaders/Materials/RayMarching/DepthAmbientRayMarching.frag.glsl" );
    //    Ra::Engine::ShaderConfigurationFactory::addConfiguration( zprepassconfig );

    //    Ra::Engine::ShaderConfiguration transparentpassconfig(
    //        "LitOITRayMarching", "Shaders/Materials/RayMarching/RayMarching.vert.glsl",
    //        "Shaders/Materials/RayMarching/LitOITRayMarching.frag.glsl" );
    //    Ra::Engine::ShaderConfigurationFactory::addConfiguration( transparentpassconfig );

    // Registering technique
    Rendering::EngineRenderTechniques::registerDefaultTechnique(
        "Volumetric",

        []( Rendering::RenderTechnique& rt, bool /*isTransparent*/ ) {
            /*
            auto lightpassconfig =
                Ra::Engine::ShaderConfigurationFactory::getConfiguration( "Volumetric" );
            rt.setConfiguration( *lightpassconfig,
                                 DefaultRenderingPasses::LIGHTING_OPAQUE );
            */
            auto passconfig = Data::ShaderConfigurationFactory::getConfiguration( "Volumetric" );
            rt.setConfiguration( *passconfig,
                                 Rendering::DefaultRenderingPasses::LIGHTING_VOLUMETRIC );
        } );

    // Registering parameters metadata
    ParameterSetEditingInterface::loadMetaData( materialName, s_parametersMetadata );
}

void VolumetricMaterial::unregisterMaterial() {
    Rendering::EngineRenderTechniques::removeDefaultTechnique( "Volumetric" );
}

} // namespace Data
} // namespace Engine
} // namespace Ra
