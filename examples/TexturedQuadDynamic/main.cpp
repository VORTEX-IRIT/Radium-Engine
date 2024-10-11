
// Include Radium base application and its simple Gui
#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>
#include <Gui/Viewer/Viewer.hpp>

// include the Engine/entity/component interface
#include <Core/Asset/BlinnPhongMaterialData.hpp>
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Engine/Data/TextureManager.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/GeometrySystem.hpp>

#include <chrono>
#include <thread>

int main( int argc, char* argv[] ) {
    Ra::Gui::BaseApplication app( argc, argv );
    app.initialize( Ra::Gui::SimpleWindowFactory {} );

    auto quad = Ra::Core::Geometry::makeZNormalQuad( { 1_ra, 1_ra }, {}, true );

    constexpr int width  = 192;
    constexpr int height = 512;
    constexpr int size   = width * height;
    // make shared do not support array before c++ 20,
    auto data = std::shared_ptr<unsigned char[]>( new unsigned char[size]() );
    // fill with some function
    for ( int i = 0; i < width; ++i ) {
        for ( int j = 0; j < height; j++ ) {
            data[( i * height + j )] =
                (unsigned char)( 255.0 * std::abs( std::sin( j * i * M_PI / 64.0 ) *
                                                   std::cos( j * i * M_PI / 96.0 ) ) );
        }
    }

    Ra::Engine::Data::TextureParameters textureParameters { "myTexture", {}, {} };
    textureParameters.image.format         = gl::GLenum::GL_RED;
    textureParameters.image.internalFormat = gl::GLenum::GL_R8;
    textureParameters.image.width          = width;
    textureParameters.image.height         = height;
    textureParameters.image.texels         = data;
    textureParameters.sampler.minFilter    = gl::GLenum::GL_LINEAR_MIPMAP_LINEAR;

    auto e = app.m_engine->getEntityManager()->createEntity( "Textured quad" );

    //! [Add texture to material]
    auto textureManager = app.m_engine->getTextureManager();
    auto textureHandle  = textureManager->addTexture( textureParameters );
    auto texture        = textureManager->getTexture( textureHandle );
    auto material = std::make_shared<Ra::Engine::Data::BlinnPhongMaterial>( "myMaterialData" );
    material->addTexture( Ra::Engine::Data::TextureSemantics::BlinnPhongMaterial::TEX_DIFFUSE,
                          textureHandle );
    //! [Add texture to material]

    // remove glossy highlight
    material->setSpecularColor( Ra::Core::Utils::Color::Black() );

    // the entity get's this new component ownership. a bit wired since hidden in ctor.
    new Ra::Engine::Scene::TriangleMeshComponent( "Quad Mesh", e, std::move( quad ), material );
    //! [Create an entity and component to draw or data]

    //! [Tell the window that something is to be displayed]
    app.m_mainWindow->prepareDisplay();
    //! [Tell the window that something is to be displayed]

    constexpr int nSec = 4;
    // terminate the app after nSec second (approximatively). Camera can be moved using mouse moves.
    auto thread = std::thread( [=, &app, &texture]() {
        const auto& startChrono = std::chrono::high_resolution_clock::now();
        int dec                 = 0;

        for ( int iSec = 0; iSec < nSec; ++iSec ) {
            const auto& endChrono = startChrono + ( iSec + 1 ) * std::chrono::milliseconds( 1000 );

            while ( std::chrono::high_resolution_clock::now() < endChrono ) {
                auto newData = std::shared_ptr<unsigned char[]>( new unsigned char[size]() );

                for ( int i = 0; i < width; ++i ) {
                    for ( int j = 0; j < height; j++ ) {
                        newData[( i * height + j )] =
                            (unsigned char)( 200.0 * std::abs( std::sin( float( dec ) / 40_ra ) ) *
                                             std::abs( std::sin( j * i * M_PI / 64.0 ) *
                                                       std::cos( j * i * M_PI / 96.0 ) ) );
                    }
                }

                texture->updateData( newData ); // update data multiple times per frame to check if
                                                // only one update is done per frame.
                ++dec;
            }
        }
        std::cout << dec / (float)nSec << " texture updates per second." << std::endl;
        app.appNeedsToQuit();
    } );

    std::promise<void> exitSignal;
    auto thread2 = std::thread( [=, &texture, &exitSignal]() {
        auto future = exitSignal.get_future();
        while ( future.wait_for( std::chrono::milliseconds( 1 ) ) == std::future_status::timeout ) {
            auto newData = std::shared_ptr<unsigned char[]>( new unsigned char[size]() );
            for ( int i = 0; i < width; ++i ) {
                for ( int j = 0; j < height; j++ ) {
                    newData[( i * height + j )] = ( i * 255 ) / width;
                }
            }
            texture->updateData( newData );
            std::this_thread::sleep_for( std::chrono::milliseconds( 100 ) );
        }
    } );

    const auto ret = app.exec();

    thread.join();
    exitSignal.set_value();
    thread2.join();

    return ret;
}
