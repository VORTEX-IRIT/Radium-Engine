#include <catch2/catch.hpp>

#include <Engine/Data/BlinnPhongMaterial.hpp>
#include <Engine/Data/LambertianMaterial.hpp>
#include <Engine/Data/PlainMaterial.hpp>
#include <Engine/Data/VolumetricMaterial.hpp>
#include <Engine/RadiumEngine.hpp>

#include <Headless/CLIViewer.hpp>
#ifdef HEADLESS_HAS_EGL
#    include <Headless/OpenGLContext/EglOpenGLContext.hpp>
#else
#    include <Headless/OpenGLContext/GlfwOpenGLContext.hpp>
#endif
using namespace Ra::Headless;
using namespace Ra::Engine::Data;

struct PrintThemAll {
    using types = RenderParameters::BindableTypes;
    template <typename T>
    void operator()( const std::string& name, const T& ) {
        std::cout << name << " with type " << typeid( T ).name() << "\n";
    }
};
TEST_CASE( "Engine/Data/Materials", "[Engine][Engine/Data][Materials]" ) {

    // Get the Engine and materials initialized
    glbinding::Version glVersion { 4, 4 };
#ifdef HEADLESS_HAS_EGL
    CLIViewer viewer { std::make_unique<EglOpenGLContext>( glVersion ) };
#else
    CLIViewer viewer { std::make_unique<GlfwOpenGLContext>( glVersion ) };
#endif
    const char* testName = "Materials testing";
    auto code            = viewer.init( 1, &testName );

    SECTION( "Blinn-Phong material" ) {
        LOG( Ra::Core::Utils::logINFO ) << "Testing Blinn-Phong material";
        REQUIRE( code == 0 );
        BlinnPhongMaterial bp( "testBlinnPhong" );

        /* Testing default values */
        REQUIRE( bp.getAlpha() == 1.0 );
        REQUIRE( !bp.isColoredByVertexAttrib() );

        bp.setColoredByVertexAttrib( true );
        REQUIRE( bp.isColoredByVertexAttrib() == true );
        /* Setting GL Parameters */
        bp.updateGL();
        auto& bpParameters = bp.getParameters();

        bpParameters.visit( PrintThemAll {} );

        REQUIRE( bpParameters.existsVariable<bool>( "material.hasPerVertexKd" ) );
        REQUIRE( bpParameters.existsVariable<Scalar>( "material.alpha" ) );

        auto& pvc = bpParameters.getVariable<bool>( "material.hasPerVertexKd" );
        REQUIRE( pvc == bp.isColoredByVertexAttrib() );

        auto& alp = bpParameters.getVariable<Scalar>( "material.alpha" );
        REQUIRE( alp == bp.getAlpha() );

        /* changing parameter values */
        bpParameters.setVariable( "material.hasPerVertexKd", !pvc );
        bpParameters.setVariable( "material.alpha", 0.5_ra );
        REQUIRE( pvc != bp.isColoredByVertexAttrib() );
        REQUIRE( alp != bp.getAlpha() );

        /* Updating material parameters from GL parameters */
        bp.updateFromParameters();
        REQUIRE( pvc == bp.isColoredByVertexAttrib() );
        REQUIRE( alp == bp.getAlpha() );
        LOG( Ra::Core::Utils::logINFO ) << "Blinn-Phong material tested.\n";
    }

    SECTION( "Lambertian material" ) {
        LOG( Ra::Core::Utils::logINFO ) << "Testing Lambertian material";
        REQUIRE( code == 0 );
        LambertianMaterial mat( "test LambertianMaterial" );

        REQUIRE( !mat.isColoredByVertexAttrib() );

        mat.setColoredByVertexAttrib( true );
        REQUIRE( mat.isColoredByVertexAttrib() );

        mat.updateGL();
        auto& matParameters = mat.getParameters();

        REQUIRE( matParameters.existsVariable<bool>( "material.perVertexColor" ) );
        auto& pvc = matParameters.getVariable<bool>( "material.perVertexColor" );
        REQUIRE( pvc == mat.isColoredByVertexAttrib() );

        /* changing parameter values */
        matParameters.setVariable( "material.perVertexColor", !pvc );
        REQUIRE( pvc != mat.isColoredByVertexAttrib() );

        /* Updating material parameters from GL parameters */
        mat.updateFromParameters();
        REQUIRE( pvc == mat.isColoredByVertexAttrib() );
        LOG( Ra::Core::Utils::logINFO ) << "Lambertian material tested.\n";
    }

    SECTION( "Plain material" ) {
        LOG( Ra::Core::Utils::logINFO ) << "Testing Plain material";
        REQUIRE( code == 0 );
        PlainMaterial mat( "test PlainMaterial" );

        REQUIRE( !mat.isColoredByVertexAttrib() );

        mat.setColoredByVertexAttrib( true );
        REQUIRE( mat.isColoredByVertexAttrib() );

        mat.updateGL();
        auto& matParameters = mat.getParameters();

        REQUIRE( matParameters.existsVariable<bool>( "material.perVertexColor" ) );
        auto& pvc = matParameters.getVariable<bool>( "material.perVertexColor" );
        REQUIRE( pvc == mat.isColoredByVertexAttrib() );

        /* changing parameter values */
        matParameters.setVariable( "material.perVertexColor", !pvc );
        REQUIRE( pvc != mat.isColoredByVertexAttrib() );

        /* Updating material parameters from GL parameters */
        mat.updateFromParameters();
        REQUIRE( pvc == mat.isColoredByVertexAttrib() );
        LOG( Ra::Core::Utils::logINFO ) << "Plain material tested.\n";
    }

    SECTION( "Volumetric material" ) {
        LOG( Ra::Core::Utils::logINFO ) << "Testing Volumetric material";
        REQUIRE( code == 0 );
        VolumetricMaterial mat( "test VolumetricMaterial" );
        auto d = std::shared_ptr<float[]>( new float[1] );
        d[0]   = 1.f;
        Texture density {
            { "simpleDensity",
              { gl::GL_CLAMP_TO_EDGE,
                gl::GL_CLAMP_TO_EDGE,
                gl::GL_CLAMP_TO_EDGE,
                gl::GL_NEAREST,
                gl::GL_NEAREST },
              { gl::GL_TEXTURE_3D, 1, 1, 1, gl::GL_RED, gl::GL_RED, gl::GL_FLOAT, false, d } } };
        density.initializeNow();
        mat.setTexture( &density );

        REQUIRE( mat.m_g == 0_ra );

        mat.updateGL();
        auto& matParameters = mat.getParameters();
        REQUIRE( matParameters.existsVariable<Scalar>( "material.g" ) );

        auto& g = matParameters.getVariable<Scalar>( "material.g" );
        REQUIRE( g == 0_ra );

        /* changing parameter values */
        matParameters.setVariable( "material.g", 0.5_ra );
        REQUIRE( g != mat.m_g );

        /* Updating material parameters from GL parameters */
        mat.updateFromParameters();
        REQUIRE( g == mat.m_g );
        LOG( Ra::Core::Utils::logINFO ) << "Volumetric material tested.\n";
    }

    SECTION( "Metadata verification" ) {
        BlinnPhongMaterial bp( "testBlinnPhong" );
        auto bpMetadata = bp.getParametersMetadata();
        REQUIRE( bpMetadata.contains( "material.tex.ns" ) );
        REQUIRE( bpMetadata["material.tex.ns"].contains( "type" ) );
        REQUIRE( bpMetadata["material.tex.ns"]["type"] == "texture" );

        LambertianMaterial lm( "test LambertianMaterial" );
        auto lmMetadata = lm.getParametersMetadata();
        REQUIRE( lmMetadata.contains( "material.tex.color" ) );
        REQUIRE( lmMetadata["material.tex.color"].contains( "type" ) );
        REQUIRE( lmMetadata["material.tex.color"]["type"] == "texture" );

        PlainMaterial pm( "test PlainMaterial" );
        auto pmMetadata = pm.getParametersMetadata();
        REQUIRE( pmMetadata.contains( "material.tex.color" ) );
        REQUIRE( pmMetadata["material.tex.color"].contains( "type" ) );
        REQUIRE( pmMetadata["material.tex.color"]["type"] == "texture" );

        VolumetricMaterial vm( "test VolumetricMaterial" );
        auto vmMetadata = vm.getParametersMetadata();
        REQUIRE( vmMetadata.contains( "material.sigma_a" ) );
        REQUIRE( vmMetadata["material.sigma_a"].contains( "type" ) );
        REQUIRE( vmMetadata["material.sigma_a"]["type"] == "array" );

        nlohmann::json destination;
        ParameterSetEditingInterface::loadMetaData( "UnknownMetadata", destination );
        REQUIRE( destination.empty() );
    }
}
