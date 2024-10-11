#include <Headless/CLIViewer.hpp>

#ifdef USE_EGL_CONTEXT
#    include <Headless/OpenGLContext/EglOpenGLContext.hpp>
#else
#    include <Headless/OpenGLContext/GlfwOpenGLContext.hpp>
#endif

#include <iostream>

#include <Engine/RadiumEngine.hpp>
#include <Engine/Rendering/ForwardRenderer.hpp>

#include <Core/Resources/Resources.hpp>

#ifdef IO_HAS_ASSIMP
#    include <IO/AssimpLoader/AssimpFileLoader.hpp>
#endif

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

using namespace Ra::Headless;
int main( int argc, const char* argv[] ) {
    //! [Creating the viewer with custom parameters]
    bool showWindow { false };
    glbinding::Version glVersion { 4, 4 };
#ifdef USE_EGL_CONTEXT
    CLIViewer viewer { std::make_unique<EglOpenGLContext>( glVersion ) };
#else
    CLIViewer viewer { std::make_unique<GlfwOpenGLContext>( glVersion ) };
#endif
    viewer.addFlag( "-w,--window", showWindow, "Map the viewer window." );
    //! [Creating the viewer with custom parameters]

    //! [Configuring the viewer : initialize OpenGL and the Engine]
    if ( int code = viewer.init( argc, argv ) ) { return code; }
    auto viewerParameters = viewer.getCommandLineParameters();
    //! [Configuring the viewer : initialize OpenGL and the Engine]

    //! [Verifying the OpenGL version available to the engine]
    if ( glVersion != viewer.getEngine()->getOpenGLVersion() ) {
        std::cout << "OpenGL version mismatch : requested " << glVersion.toString()
                  << " -- available " << viewer.getEngine()->getOpenGLVersion().toString()
                  << std::endl;
    }
    //! [Verifying the OpenGL version available to the engine]

    //! [Populating the viewer with needed services]
    viewer.setRenderer( new Ra::Engine::Rendering::ForwardRenderer() );
#ifdef IO_HAS_ASSIMP
    viewer.addDataFileLoader( new Ra::IO::AssimpFileLoader() );
#endif
    //! [Populating the viewer with needed services]

    //! [Bind the OpenGLContext if needed]
    viewer.bindOpenGLContext();
    //! [Bind the OpenGLContext if needed]

    //! [Setup the Scene to render]
    if ( viewer.getDataFileName().empty() ) {
        auto rp              = Ra::Core::Resources::getResourcesPath();
        std::string filename = *rp + "/Examples/Headless/Assets/radium-logo.dae";
        viewer.setDataFileName( filename );
    }
    // load the given scene
    viewer.loadScene();
    // Activate the first camera on the given scene if any. Create a default camera if needed.
    viewer.setCamera();
    // Compile the scene.
    // Must be called after setCamera so that there is a valid camera in the scene.
    viewer.compileScene();
    //! [Setup the Scene to render]

    //! [Running the application]
    auto saveFrame = [&viewer]( const std::string& filename ) {
        size_t w, h;
        std::cout << "Writing image " << filename << std::endl;
        auto writtenPixels = viewer.grabFrame( w, h );
        stbi_write_png( filename.c_str(), w, h, 4, writtenPixels.get(), w * 4 * sizeof( uchar ) );
    };

    auto render = [&viewer]( float time_step ) { viewer.oneFrame( time_step ); };

    //! [Attach a mouse event listener]
    auto mouseListenerId = viewer.getWindow().mouseListener().attach(
        []( int button, int action, int modifiers, int x, int y ) {
            std::cout << "mouseEvent : button " << button << " // action " << action
                      << " // modifiers " << modifiers << " // position " << x << ", " << y
                      << std::endl;
        } );
    //! [Attach a mouse event listener]
    if ( showWindow ) {
        if ( viewerParameters.m_animationEnable ) {
            viewer.showWindow( true, OpenGLContext::EventMode::TIMEOUT );
        }
        else { viewer.showWindow( true, OpenGLContext::EventMode::WAIT ); }
        viewer.renderLoop( render );
        viewer.showWindow( false );
    }
    else {
        if ( viewerParameters.m_animationEnable ) {
            // compute 2s of animation at 30fps
            float duration = 2;
            float fps      = 30;
            for ( int i = 0; i < duration * fps; ++i ) {
                render( 1.f / fps );
                auto filename = viewerParameters.m_imgPrefix + std::to_string( i ) + ".png";
                saveFrame( filename );
            }
        }
        else {
            // compute one picture
            render( 0 );
            auto filename = viewerParameters.m_imgPrefix + ".png";
            saveFrame( filename );
        }
    }
    //! [Running the application]

    //! [Detach mouse event listener]
    viewer.getWindow().mouseListener().detach( mouseListenerId );
    //! [Detach mouse event listener]

    //! [Unbind the OpenGLContext when no more needed]
    viewer.bindOpenGLContext( false );
    //! [Unbind the OpenGLContext when no more needed]
    return 0;
}
