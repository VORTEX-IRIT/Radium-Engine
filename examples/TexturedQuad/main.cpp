// Include Radium base application and its simple Gui
#include <Engine/Data/BlinnPhongMaterial.hpp>
#include <Engine/Data/LambertianMaterial.hpp>
#include <Engine/Data/SimpleMaterial.hpp>
#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>

// include the Engine/entity/component interface
#include <Core/Asset/BlinnPhongMaterialData.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/GeometrySystem.hpp>

int main( int argc, char* argv[] ) {
    Ra::Gui::BaseApplication app( argc, argv );
    app.initialize( Ra::Gui::SimpleWindowFactory {} );

    auto quad = Ra::Core::Geometry::makeZNormalQuad( { 1_ra, 1_ra }, {}, true );

    //! [Creating a texture]
    constexpr int width  = 192;
    constexpr int height = 512;
    constexpr int size   = width * height;
    std::shared_ptr<unsigned char[]> data( new unsigned char[size] );

    // fill with some function
    for ( int i = 0; i < width; ++i ) {
        for ( int j = 0; j < height; j++ ) {
            data[( i * height + j )] =
                (unsigned char)( 255.0 * std::abs( std::sin( j * i * M_PI / 64.0 ) *
                                                   std::cos( j * i * M_PI / 96.0 ) ) );
        }
    }

    // these values will be used when engine initialize texture GL representation.
    Ra::Engine::Data::TextureParameters textureParameters { "myTexture", {}, {} };
    textureParameters.image.format         = gl::GLenum::GL_RED;
    textureParameters.image.internalFormat = gl::GLenum::GL_R8;
    textureParameters.image.width          = width;
    textureParameters.image.height         = height;
    textureParameters.image.texels         = data;

    auto textureHandle = app.m_engine->getTextureManager()->addTexture( textureParameters );
    //! [Creating a texture]

    //! [Create an entity and component to draw or data]
    auto e = app.m_engine->getEntityManager()->createEntity( "Textured quad" );

    auto material = std::make_shared<Ra::Engine::Data::LambertianMaterial>( "myMaterialData" );
    material->addTexture( Ra::Engine::Data::TextureSemantics::LambertianMaterial::TEX_COLOR,
                          textureHandle );

    // the entity get's this new component ownership. A bit wired since hidden in ctor.
    new Ra::Engine::Scene::TriangleMeshComponent( "Quad Mesh", e, std::move( quad ), material );
    //! [Create an entity and component to draw or data]

    app.m_mainWindow->prepareDisplay();

    return app.exec();
}
