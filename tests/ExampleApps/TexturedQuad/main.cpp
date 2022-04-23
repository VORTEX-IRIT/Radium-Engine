// Include Radium base application and its simple Gui
#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>

// include the Engine/entity/component interface
#include <Core/Asset/BlinnPhongMaterialData.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/GeometrySystem.hpp>

#include <QTimer>

int main( int argc, char* argv[] ) {
    //! [Creating the application]
    Ra::Gui::BaseApplication app( argc, argv );
    app.initialize( Ra::Gui::SimpleWindowFactory {} );
    //! [Creating the application]

    //! [Creating a quad geometry with texture coordinates]
    auto quad = Ra::Core::Geometry::makeZNormalQuad( { 1_ra, 1_ra }, {}, true );
    //! [Creating a quad geometry with texture coordinates]

    //! [Creating a texture]
    unsigned char data[192 * 512];
    // fill with some function
    for ( int i = 0; i < 192; ++i ) {
        for ( int j = 0; j < 512; j++ ) {
            data[( i * 512 + j )] =
                (unsigned char)( 255.0 * std::abs( std::sin( j * i * M_PI / 64.0 ) *
                                                   std::cos( j * i * M_PI / 96.0 ) ) );
        }
    }
    auto& myTexture = app.m_engine->getTextureManager()->addTexture( "myTexture", 192, 512, data );
    // these values will be used when engine initialize texture GL representation.
    myTexture.format         = gl::GLenum::GL_RED;
    myTexture.internalFormat = gl::GLenum::GL_R8;
    //! [Creating a texture]

    //! [Create an entity and component to draw or data]
    auto e = app.m_engine->getEntityManager()->createEntity( "Textured quad" );

    Ra::Core::Asset::BlinnPhongMaterialData matData( "myMaterialData" );
    matData.m_hasTexDiffuse = true;
    // this name has to be the same as texManager added texture name
    matData.m_texDiffuse = "myTexture";

    // the entity get's this new component ownership. a bit wired since hidden in ctor.
    new Ra::Engine::Scene::TriangleMeshComponent( "Quad Mesh", e, std::move( quad ), &matData );

    //! [Tell the window that something is to be displayed]
    app.m_mainWindow->prepareDisplay();
    //! [Tell the window that something is to be displayed]

    return app.exec();
}
