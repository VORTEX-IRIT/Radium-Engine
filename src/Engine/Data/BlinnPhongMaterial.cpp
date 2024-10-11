#include <Core/Asset/BlinnPhongMaterialData.hpp>
#include <Engine/Data/BlinnPhongMaterial.hpp>
#include <Engine/Data/MaterialConverters.hpp>
#include <Engine/Data/ShaderConfigFactory.hpp>
#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/RadiumEngine.hpp>
#include <Engine/Rendering/RenderTechnique.hpp>

#include <fstream>

namespace Ra {
namespace Engine {
namespace Data {
static const std::string materialName { "BlinnPhong" };

nlohmann::json BlinnPhongMaterial::s_parametersMetadata = {};

BlinnPhongMaterial::BlinnPhongMaterial( const std::string& instanceName ) :
    Material( instanceName, materialName, Material::MaterialAspect::MAT_OPAQUE ) {}

void BlinnPhongMaterial::updateRenderingParameters() {
    // update the rendering parameters
    auto& renderParameters = getParameters();
    renderParameters.setVariable( "material.kd", m_kd );
    renderParameters.setVariable( "material.hasPerVertexKd", m_perVertexColor );
    renderParameters.setVariable( "material.renderAsSplat", m_renderAsSplat );
    renderParameters.setVariable( "material.ks", m_ks );
    renderParameters.setVariable( "material.ns", m_ns );
    renderParameters.setVariable( "material.alpha", std::min( m_alpha, m_kd[3] ) );
    Texture* tex = getTexture( BlinnPhongMaterial::TextureSemantic::TEX_DIFFUSE );
    if ( tex != nullptr ) { renderParameters.setTexture( "material.tex.kd", tex ); }
    renderParameters.setVariable( "material.tex.hasKd", tex != nullptr );
    tex = getTexture( BlinnPhongMaterial::TextureSemantic::TEX_SPECULAR );
    if ( tex != nullptr ) { renderParameters.setTexture( "material.tex.ks", tex ); }
    renderParameters.setVariable( "material.tex.hasKs", tex != nullptr );
    tex = getTexture( BlinnPhongMaterial::TextureSemantic::TEX_NORMAL );
    if ( tex != nullptr ) { renderParameters.setTexture( "material.tex.normal", tex ); }
    renderParameters.setVariable( "material.tex.hasNormal", tex != nullptr );
    tex = getTexture( BlinnPhongMaterial::TextureSemantic::TEX_SHININESS );
    if ( tex != nullptr ) { renderParameters.setTexture( "material.tex.ns", tex ); }
    renderParameters.setVariable( "material.tex.hasNs", tex != nullptr );
    tex = getTexture( BlinnPhongMaterial::TextureSemantic::TEX_ALPHA );
    if ( tex != nullptr ) { renderParameters.setTexture( "material.tex.alpha", tex ); }
    renderParameters.setVariable( "material.tex.hasAlpha", tex != nullptr );
}

void BlinnPhongMaterial::updateGL() {
    if ( !isDirty() ) { return; }

    updateRenderingParameters();
    setClean();
}

void BlinnPhongMaterial::updateFromParameters() {
    auto& renderParameters = getParameters();
    m_kd                   = renderParameters.getVariable<Core::Utils::Color>( "material.kd" );
    m_perVertexColor       = renderParameters.getVariable<bool>( "material.hasPerVertexKd" );
    m_renderAsSplat        = renderParameters.getVariable<bool>( "material.renderAsSplat" );
    m_ks                   = renderParameters.getVariable<Core::Utils::Color>( "material.ks" );
    m_ns                   = renderParameters.getVariable<Scalar>( "material.ns" );
    m_alpha                = renderParameters.getVariable<Scalar>( "material.alpha" );
}

bool BlinnPhongMaterial::isTransparent() const {
    return ( m_alpha < 1_ra ) || ( m_kd[3] < 1_ra ) || Material::isTransparent();
}

void BlinnPhongMaterial::registerMaterial() {
    // For resources access (glsl files) in a filesystem
    auto resourcesRootDir { RadiumEngine::getInstance()->getResourcesDir() };
    auto shaderProgramManager = RadiumEngine::getInstance()->getShaderProgramManager();

    // Defining the material converter
    EngineMaterialConverters::registerMaterialConverter( materialName,
                                                         BlinnPhongMaterialConverter() );

    // adding the material glsl implementation file
    shaderProgramManager->addNamedString(
        "/BlinnPhong.glsl", resourcesRootDir + "Shaders/Materials/BlinnPhong/BlinnPhong.glsl" );
    // registering re-usable shaders
    Data::ShaderConfiguration lpconfig(
        "BlinnPhong",
        resourcesRootDir + "Shaders/Materials/BlinnPhong/BlinnPhong.vert.glsl",
        resourcesRootDir + "Shaders/Materials/BlinnPhong/BlinnPhong.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( lpconfig );

    Data::ShaderConfiguration zprepassconfig(
        "ZprepassBlinnPhong",
        resourcesRootDir + "Shaders/Materials/BlinnPhong/BlinnPhong.vert.glsl",
        resourcesRootDir + "Shaders/Materials/BlinnPhong/BlinnPhongZPrepass.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( zprepassconfig );

    Data::ShaderConfiguration transparentpassconfig(
        "LitOITBlinnPhong",
        resourcesRootDir + "Shaders/Materials/BlinnPhong/BlinnPhong.vert.glsl",
        resourcesRootDir + "Shaders/Materials/BlinnPhong/LitOITBlinnPhong.frag.glsl" );
    Data::ShaderConfigurationFactory::addConfiguration( transparentpassconfig );

    // Registering technique
    Rendering::EngineRenderTechniques::registerDefaultTechnique(
        materialName,

        []( Rendering::RenderTechnique& rt, bool isTransparent ) {
            // Configure the technique to render this object using forward Renderer or any
            // compatible one Main pass (Mandatory) : BlinnPhong
            auto lightpass = Data::ShaderConfigurationFactory::getConfiguration( "BlinnPhong" );
            rt.setConfiguration( *lightpass, Rendering::DefaultRenderingPasses::LIGHTING_OPAQUE );

            // Z prepass (Recommended) : DepthAmbiantPass
            auto zprepass =
                Data::ShaderConfigurationFactory::getConfiguration( "ZprepassBlinnPhong" );
            rt.setConfiguration( *zprepass, Rendering::DefaultRenderingPasses::Z_PREPASS );
            // Transparent pass (0ptional) : If Transparent ... add LitOIT
            if ( isTransparent ) {
                auto transparentpass =
                    Data::ShaderConfigurationFactory::getConfiguration( "LitOITBlinnPhong" );
                rt.setConfiguration( *transparentpass,
                                     Rendering::DefaultRenderingPasses::LIGHTING_TRANSPARENT );
            }
        } );

    // Registering parameters metadata
    ParameterSetEditingInterface::loadMetaData( materialName, s_parametersMetadata );
}

void BlinnPhongMaterial::unregisterMaterial() {
    EngineMaterialConverters::removeMaterialConverter( materialName );
    Rendering::EngineRenderTechniques::removeDefaultTechnique( materialName );
}

Material*
BlinnPhongMaterialConverter::operator()( const Ra::Core::Asset::MaterialData* toconvert ) {
    auto result = new BlinnPhongMaterial( toconvert->getName() );
    // we are sure here that the concrete type of "toconvert" is BlinnPhongMaterialData
    // static cst is safe here
    auto source = static_cast<const Ra::Core::Asset::BlinnPhongMaterialData*>( toconvert );

    if ( source->hasDiffuse() ) result->m_kd = source->m_diffuse;
    if ( source->hasSpecular() ) result->m_ks = source->m_specular;
    if ( source->hasShininess() ) result->m_ns = source->m_shininess;
    if ( source->hasOpacity() ) result->m_alpha = source->m_opacity;
    TextureParameters data;
    data.sampler.wrapS     = GL_REPEAT;
    data.sampler.wrapT     = GL_REPEAT;
    data.sampler.minFilter = GL_LINEAR_MIPMAP_LINEAR;
    auto texManager        = RadiumEngine::getInstance()->getTextureManager();
    if ( source->hasDiffuseTexture() ) {
        data.name  = "diffuse";
        data.image = texManager->loadTextureImage( source->m_texDiffuse, true );
        result->addTexture( BlinnPhongMaterial::TextureSemantic::TEX_DIFFUSE, data );
    }
    if ( source->hasSpecularTexture() ) {
        data.name  = "specular";
        data.image = texManager->loadTextureImage( source->m_texSpecular, true );
        result->addTexture( BlinnPhongMaterial::TextureSemantic::TEX_SPECULAR, data );
    }
    if ( source->hasShininessTexture() ) {
        data.name  = "shininess";
        data.image = texManager->loadTextureImage( source->m_texShininess, false );
        result->addTexture( BlinnPhongMaterial::TextureSemantic::TEX_SHININESS, data );
    }

    if ( source->hasOpacityTexture() ) {
        data.name  = "opacity";
        data.image = texManager->loadTextureImage( source->m_texOpacity, false );
        result->addTexture( BlinnPhongMaterial::TextureSemantic::TEX_ALPHA, data );
    }
    if ( source->hasNormalTexture() ) {
        data.name  = "normal";
        data.image = texManager->loadTextureImage( source->m_texNormal, false );
        result->addTexture( BlinnPhongMaterial::TextureSemantic::TEX_NORMAL, data );
    }
    return result;
}
} // namespace Data
} // namespace Engine
} // namespace Ra
