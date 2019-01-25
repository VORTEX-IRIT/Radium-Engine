#include <glbinding/Binding.h>
#include <glbinding/ContextInfo.h>
#include <glbinding/Version.h>
// Do not import namespace to prevent glbinding/QTOpenGL collision
#include <glbinding/gl/gl.h>

#include <globjects/globjects.h>

#include <Engine/RadiumEngine.hpp>

#include <Core/Asset/FileData.hpp>
#include <Core/Utils/StringUtils.hpp>
#include <GuiBase/Viewer/Viewer.hpp>

#include <iostream>

#include <QOpenGLContext>

#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <Core/Containers/MakeShared.hpp>
#include <Core/Math/Math.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/Log.hpp>
#include <Core/Utils/StringUtils.hpp>

#include <Engine/Component/Component.hpp>
#include <Engine/Renderer/Camera/Camera.hpp>
#include <Engine/Renderer/Light/DirLight.hpp>
#include <Engine/Renderer/Renderer.hpp>

#include <Engine/Managers/EntityManager/EntityManager.hpp>
#include <Engine/Managers/SystemDisplay/SystemDisplay.hpp>
#include <Engine/Renderer/RenderTechnique/ShaderProgramManager.hpp>
#include <Engine/Renderer/Renderers/ForwardRenderer.hpp>

#include <GuiBase/Utils/KeyMappingManager.hpp>
#include <GuiBase/Utils/Keyboard.hpp>
#include <GuiBase/Utils/PickingManager.hpp>

#include <GuiBase/Viewer/Gizmo/GizmoManager.hpp>
#include <GuiBase/Viewer/TrackballCamera.hpp>

namespace Ra {

using namespace Core::Utils; // log

Gui::Viewer::Viewer( QScreen* screen ) :
    QWindow( screen ),
    m_context( nullptr ),
    m_currentRenderer( nullptr ),
    m_pickingManager( nullptr ),
    m_isBrushPickingEnabled( false ),
    m_brushRadius( 10 ),
    m_camera( nullptr ),
    m_gizmoManager( nullptr ),
#ifdef RADIUM_MULTITHREAD_RENDERING
    m_renderThread( nullptr ),
#endif
    m_glInitStatus( false ) {
    setMinimumSize( QSize( 800, 600 ) );

    setSurfaceType( OpenGLSurface );
    m_pickingManager = new PickingManager();
}

Gui::Viewer::~Viewer() {
    if ( m_glInitStatus.load() )
    {
        m_context->makeCurrent( this );
        m_renderers.clear();

        if ( m_gizmoManager != nullptr )
        {
            delete m_gizmoManager;
        }
        m_context->doneCurrent();
    }
}

void Gui::Viewer::createGizmoManager() {
    if ( m_gizmoManager == nullptr )
    {
        m_gizmoManager = new GizmoManager( this );
    }
}

int Gui::Viewer::addRenderer( std::shared_ptr<Engine::Renderer> e ) {
    // initial state and lighting (deferred if GL is not ready yet)
    if ( m_glInitStatus.load() )
    {
        m_context->makeCurrent( this );
        intializeRenderer( e.get() );
        m_context->doneCurrent();
    } else
    {
        LOG( logINFO ) << "[Viewer] New Renderer (" << e->getRendererName()
                       << ") added before GL being Ready: deferring initialization...";
    }

    m_renderers.push_back( e );

    return m_renderers.size() - 1;
}

void Gui::Viewer::setBackgroundColor( const Core::Utils::Color& background ) {
    m_backgroundColor = background;
    for ( auto renderer : m_renderers )
        renderer->setBackgroundColor( background );
}

void Gui::Viewer::enableDebug() {
    glbinding::setCallbackMask( glbinding::CallbackMask::After |
                                glbinding::CallbackMask::ParametersAndReturnValue );
    glbinding::setAfterCallback( []( const glbinding::FunctionCall& call ) {
        std::cerr << call.function->name() << "(";
        for ( unsigned i = 0; i < call.parameters.size(); ++i )
        {
            std::cerr << call.parameters[i]->asString();
            if ( i < call.parameters.size() - 1 )
            {
                std::cerr << ", ";
            }
        }
        std::cerr << ")";
        if ( call.returnValue )
        {
            std::cerr << " -> " << call.returnValue->asString();
        }
        std::cerr << std::endl;
    } );
}

void Gui::Viewer::makeCurrent() {
    CORE_ASSERT( m_glInitStatus, "[Viewer::makeCurrent] OpenGLContext not created!" );
    m_context->makeCurrent( this );
}

void Gui::Viewer::doneCurrent() {
    CORE_ASSERT( m_glInitStatus, "[Viewer::makeCurrent] OpenGLContext not created!" );
    m_context->doneCurrent();
}

void Gui::Viewer::initializeGL() {
    m_glInitStatus = true;
    m_context->makeCurrent( this );

    LOG( logINFO ) << "*** Radium Engine Viewer ***";
    Engine::ShaderProgramManager::createInstance( "Shaders/Default.vert.glsl",
                                                  "Shaders/Default.frag.glsl" );

    // initialize renderers added before GL was ready
    if ( !m_renderers.empty() )
    {
        for ( auto& rptr : m_renderers )
        {
            intializeRenderer( rptr.get() );
            LOG( logINFO ) << "[Viewer] Deferred initialization of " << rptr->getRendererName();
        }
    }

    emit glInitialized();
    m_context->doneCurrent();

    // this code is usefull only if glInitialized() connected slot does not add a renderer
    // On Windows, actually, the signal seems to be not fired (DLL_IMPORT/EXPORT problem ?
    if ( m_renderers.empty() )
    {
        LOG( logINFO )
            << "Renderers fallback: no renderer added, enabling default (Forward Renderer)";

        m_context->makeCurrent( this );
        std::shared_ptr<Ra::Engine::Renderer> e( new Ra::Engine::ForwardRenderer() );
        m_context->doneCurrent();

        addRenderer( e );
    }

    if ( m_currentRenderer == nullptr )
    {
        changeRenderer( 0 );
    }
}

Gui::CameraInterface* Gui::Viewer::getCameraInterface() {
    return m_camera.get();
}

Gui::GizmoManager* Gui::Viewer::getGizmoManager() {
    return m_gizmoManager;
}

const Engine::Renderer* Gui::Viewer::getRenderer() const {
    return m_currentRenderer;
}

Engine::Renderer* Gui::Viewer::getRenderer() {
    return m_currentRenderer;
}

Gui::PickingManager* Gui::Viewer::getPickingManager() {
    return m_pickingManager;
}

void Gui::Viewer::onAboutToCompose() {
    // This slot function is called from the main thread as part of the event loop
    // when the GUI is about to update. We have to wait for the rendering to finish.
    m_currentRenderer->lockRendering();
}

void Gui::Viewer::onFrameSwapped() {
    // This slot is called from the main thread as part of the event loop when the
    // GUI has finished displaying the rendered image, so we unlock the renderer.
    m_currentRenderer->unlockRendering();
}

void Gui::Viewer::onAboutToResize() {
    // Like swap buffers, resizing is a blocking operation and we have to wait for the rendering
    // to finish before resizing.
    m_currentRenderer->lockRendering();
}

void Gui::Viewer::onResized() {
    m_currentRenderer->unlockRendering();
}

void Gui::Viewer::intializeRenderer( Engine::Renderer* renderer ) {
    // see issue #261 Qt Event order and default viewport management (Viewer.cpp)
    // https://github.com/STORM-IRIT/Radium-Engine/issues/261
#ifndef OS_MACOS
    gl::glViewport( 0, 0, width(), height() );
#endif
    renderer->initialize( width(), height() );
    renderer->setBackgroundColor( m_backgroundColor );
    // resize camera viewport since it might be 0x0
    m_camera->resizeViewport( width(), height() );
    // do this only when the renderer has something to render and that there is no lights
    /*
    if ( m_camera->hasLightAttached() )
    {
        renderer->addLight( m_camera->getLight() );
    }
    */
    renderer->lockRendering();
}

void Gui::Viewer::resizeGL( int width_, int height_ ) {
    if ( isExposed() )
    {
        // Renderer should have been locked by previous events.
        m_context->makeCurrent( this );

        // see issue #261 Qt Event order and default viewport management (Viewer.cpp)
        // https://github.com/STORM-IRIT/Radium-Engine/issues/261
#ifndef OS_MACOS
        gl::glViewport( 0, 0, width(), height() );
#endif
        m_camera->resizeViewport( width_, height_ );
        m_currentRenderer->resize( width_, height_ );
        m_context->doneCurrent();
    }
}

Engine::Renderer::PickingMode Gui::Viewer::getPickingMode() const {
    auto keyMap = Gui::KeyMappingManager::getInstance();
    if ( Gui::isKeyPressed(
             keyMap->getKeyFromAction( Gui::KeyMappingManager::FEATUREPICKING_VERTEX ) ) )
    {
        return m_isBrushPickingEnabled ? Engine::Renderer::C_VERTEX : Engine::Renderer::VERTEX;
    }
    if ( Gui::isKeyPressed(
             keyMap->getKeyFromAction( Gui::KeyMappingManager::FEATUREPICKING_EDGE ) ) )
    {
        return m_isBrushPickingEnabled ? Engine::Renderer::C_EDGE : Engine::Renderer::EDGE;
    }
    if ( Gui::isKeyPressed(
             keyMap->getKeyFromAction( Gui::KeyMappingManager::FEATUREPICKING_TRIANGLE ) ) )
    {
        return m_isBrushPickingEnabled ? Engine::Renderer::C_TRIANGLE : Engine::Renderer::TRIANGLE;
    }
    return Engine::Renderer::RO;
}

void Gui::Viewer::mousePressEvent( QMouseEvent* event ) {
    using Core::Utils::Color;

    if ( !m_glInitStatus.load() )
    {
        event->ignore();
        return;
    }

    auto keyMap = Gui::KeyMappingManager::getInstance();
    if ( keyMap->actionTriggered( event, Gui::KeyMappingManager::VIEWER_BUTTON_CAST_RAY_QUERY ) &&
         isKeyPressed( keyMap->getKeyFromAction( Gui::KeyMappingManager::VIEWER_RAYCAST_QUERY ) ) )
    {
        LOG( logINFO ) << "Raycast query are disabled";
        auto r = m_camera->getCamera()->getRayFromScreen( Core::Vector2( event->x(), event->y() ) );
        RA_DISPLAY_POINT( r.origin(), Color::Cyan(), 0.1f );
        RA_DISPLAY_RAY( r, Color::Yellow() );
    } else if ( keyMap->getKeyFromAction( Gui::KeyMappingManager::TRACKBALLCAMERA_MANIPULATION ) ==
                event->button() )
    {
        m_camera->handleMousePressEvent( event );
    } else if ( keyMap->actionTriggered( event,
                                         Gui::KeyMappingManager::GIZMOMANAGER_MANIPULATION ) )
    {
        m_currentRenderer->addPickingRequest( {Core::Vector2( event->x(), height() - event->y() ),
                                               GuiBase::MouseButton::RA_MOUSE_LEFT_BUTTON,
                                               Engine::Renderer::RO} );
        if ( m_gizmoManager != nullptr )
        {
            m_gizmoManager->handleMousePressEvent( event );
        }
    } else if ( keyMap->actionTriggered( event,
                                         Gui::KeyMappingManager::VIEWER_BUTTON_PICKING_QUERY ) )
    {
        // Check picking
        Engine::Renderer::PickingQuery query = {Core::Vector2( event->x(), height() - event->y() ),
                                                GuiBase::MouseButton::RA_MOUSE_RIGHT_BUTTON,
                                                getPickingMode()};
        m_currentRenderer->addPickingRequest( query );
    }
}

void Gui::Viewer::mouseReleaseEvent( QMouseEvent* event ) {
    m_camera->handleMouseReleaseEvent( event );
    if ( m_gizmoManager != nullptr )
    {
        m_gizmoManager->handleMouseReleaseEvent( event );
    }
}

void Gui::Viewer::mouseMoveEvent( QMouseEvent* event ) {
    if ( m_glInitStatus.load() )
    {
        m_camera->handleMouseMoveEvent( event );
        if ( m_gizmoManager != nullptr )
        {
            m_gizmoManager->handleMouseMoveEvent( event );
        }
        m_currentRenderer->setMousePosition( Ra::Core::Vector2( event->x(), event->y() ) );
        if ( Gui::KeyMappingManager::getInstance()->actionTriggered(
                 event, Gui::KeyMappingManager::VIEWER_BUTTON_PICKING_QUERY ) )
        {
            // Check picking
            Engine::Renderer::PickingQuery query = {
                Core::Vector2( event->x(), ( height() - event->y() ) ),
                GuiBase::MouseButton::RA_MOUSE_RIGHT_BUTTON, getPickingMode()};
            m_currentRenderer->addPickingRequest( query );
        }
    } else
        event->ignore();
}

void Gui::Viewer::wheelEvent( QWheelEvent* event ) {
    if ( m_glInitStatus.load() )
    {
        if ( m_isBrushPickingEnabled && isKeyPressed( Qt::Key_Shift ) )
        {
            m_brushRadius +=
                ( event->angleDelta().y() * 0.01 + event->angleDelta().x() * 0.01 ) > 0 ? 5 : -5;
            m_brushRadius = std::max( m_brushRadius, Scalar( 5 ) );
            m_currentRenderer->setBrushRadius( m_brushRadius );
        } else
        { m_camera->handleWheelEvent( event ); }
    } else
    { event->ignore(); }
}

void Gui::Viewer::keyPressEvent( QKeyEvent* event ) {
    if ( m_glInitStatus.load() )
    {
        keyPressed( event->key() );
        m_camera->handleKeyPressEvent( event );
    } else
    { event->ignore(); }

    // Do we need this ?
    // QWindow::keyPressEvent(event);
}

void Gui::Viewer::keyReleaseEvent( QKeyEvent* event ) {
    keyReleased( event->key() );
    m_camera->handleKeyReleaseEvent( event );

    auto keyMap = Gui::KeyMappingManager::getInstance();
    if ( keyMap->actionTriggered( event, Gui::KeyMappingManager::VIEWER_TOGGLE_WIREFRAME ) &&
         !event->isAutoRepeat() )
    {
        m_currentRenderer->toggleWireframe();
    }
    if ( keyMap->actionTriggered( event, Gui::KeyMappingManager::FEATUREPICKING_MULTI_CIRCLE ) &&
         event->modifiers() == Qt::NoModifier && !event->isAutoRepeat() )
    {
        m_isBrushPickingEnabled = !m_isBrushPickingEnabled;
        m_currentRenderer->setBrushRadius( m_isBrushPickingEnabled ? m_brushRadius : 0 );
        emit toggleBrushPicking( m_isBrushPickingEnabled );
    }

    // Do we need this ?
    // QWindow::keyReleaseEvent(event);
}

void Gui::Viewer::resizeEvent( QResizeEvent* event ) {
    //       LOG( logDEBUG ) << "Gui::Viewer --> Got resize event : "  << width() << 'x' <<
    //       height();

    if ( !m_glInitStatus.load() )
    {
        initializeGL();
    }

    if ( !m_currentRenderer || !m_camera )
        return;

    resizeGL( event->size().width(), event->size().height() );
}

void Gui::Viewer::showEvent( QShowEvent* /*ev*/ ) {
    //       LOG( logDEBUG ) << "Gui::Viewer --> Got show event : " << width() << 'x' << height();
    if ( !m_context )
    {
        m_context.reset( new QOpenGLContext() );
        m_context->create();
        m_context->makeCurrent( this );
        // no need to initalize glbinding. globjects (magically) do this internally.
        globjects::init( globjects::Shader::IncludeImplementation::Fallback );

        LOG( logINFO ) << "*** Radium Engine OpenGL context ***";
        LOG( logINFO ) << "Renderer (glbinding) : " << glbinding::ContextInfo::renderer();
        LOG( logINFO ) << "Vendor   (glbinding) : " << glbinding::ContextInfo::vendor();
        LOG( logINFO ) << "OpenGL   (glbinding) : " << glbinding::ContextInfo::version().toString();
        LOG( logINFO ) << "GLSL                 : "
                       << gl::glGetString( gl::GLenum( GL_SHADING_LANGUAGE_VERSION ) );

        m_context->doneCurrent();
    }

    if ( !m_camera )
    {
        // quick fix meanwhile camera management refactoring
        // see issue #339 Crash when using -f option
        // https://github.com/STORM-IRIT/Radium-Engine/issues/339
        // here width and height might be  0 ! need to resize camera afterward
        m_camera.reset( new Gui::TrackballCamera( width(), height() ) );

        // Lights are components. So they must be attached to an entity. Attache headlight to system
        // Entity
        auto light =
            new Engine::DirectionalLight( Ra::Engine::SystemEntity::getInstance(), "headlight" );
        light->setColor( Ra::Core::Utils::Color::Grey( Scalar( 1.0 ) ) );
        m_camera->attachLight( light );
    }
}

void Gui::Viewer::exposeEvent( QExposeEvent* /*ev*/ ) {
    //       LOG( logDEBUG ) << "Gui::Viewer --> Got exposed event : " << width() << 'x' <<
    //       height();
}

void Gui::Viewer::reloadShaders() {
    CORE_ASSERT( m_glInitStatus.load(), "OpenGL needs to be initialized reload shaders." );

    // FIXME : check thread-saefty of this.
    m_currentRenderer->lockRendering();

    m_context->makeCurrent( this );
    m_currentRenderer->reloadShaders();
    m_context->doneCurrent();

    m_currentRenderer->unlockRendering();
}

void Gui::Viewer::displayTexture( const QString& tex ) {
    CORE_ASSERT( m_glInitStatus.load(), "OpenGL needs to be initialized to display textures." );
    m_context->makeCurrent( this );
    m_currentRenderer->lockRendering();
    m_currentRenderer->displayTexture( tex.toStdString() );
    m_currentRenderer->unlockRendering();
    m_context->doneCurrent();
}

bool Gui::Viewer::changeRenderer( int index ) {
    if ( m_glInitStatus.load() && m_renderers[index] )
    {
        m_context->makeCurrent( this );

        if ( m_currentRenderer != nullptr )
        {
            m_currentRenderer->lockRendering();
        }

        m_currentRenderer = m_renderers[index].get();
        // renderers in m_renderers are supposed to be locked
        m_currentRenderer->resize( width(), height() );
        m_currentRenderer->unlockRendering();

        LOG( logINFO ) << "[Viewer] Set active renderer: " << m_currentRenderer->getRendererName();

        // resize camera viewport since the one in show event might have 0x0
        m_camera->resizeViewport( width(), height() );

        m_context->doneCurrent();
        emit rendererReady();

        return true;
    }
    return false;
}

// Asynchronous rendering implementation

void Gui::Viewer::startRendering( const Scalar dt ) {
    CORE_ASSERT( m_glInitStatus.load(), "OpenGL needs to be initialized before rendering." );

    CORE_ASSERT( m_currentRenderer != nullptr, "No renderer found." );

    m_pickingManager->clear();
    m_context->makeCurrent( this );

    Engine::ViewingParameters data{m_camera->getViewMatrix(), m_camera->getProjMatrix(), dt};

    // FIXME : move this outside of the rendering loop. must be done once per renderer ...
    // if there is no light on the renderer, add the head light attached to the camera ...
    if ( !m_currentRenderer->hasLight() )
    {
        if ( m_camera->hasLightAttached() )
            m_currentRenderer->addLight( m_camera->getLight() );
        else
            LOG( logDEBUG ) << "Unable to attach the head light!";
    }
    m_currentRenderer->render( data );
}

void Gui::Viewer::waitForRendering() {
    if ( isExposed() )
    {
        m_context->swapBuffers( this );
    }

    m_context->doneCurrent();
}

void Gui::Viewer::processPicking() {
    CORE_ASSERT( m_glInitStatus.load(), "OpenGL needs to be initialized before rendering." );

    CORE_ASSERT( m_currentRenderer != nullptr, "No renderer found." );

    CORE_ASSERT( m_currentRenderer->getPickingQueries().size() ==
                     m_currentRenderer->getPickingResults().size(),
                 "There should be one result per query." );

    for ( uint i = 0; i < m_currentRenderer->getPickingQueries().size(); ++i )
    {
        const Engine::Renderer::PickingQuery& query = m_currentRenderer->getPickingQueries()[i];
        if ( query.m_button == GuiBase::MouseButton::RA_MOUSE_LEFT_BUTTON )
        {
            emit leftClickPicking( m_currentRenderer->getPickingResults()[i].m_roIdx );
        } else if ( query.m_button == GuiBase::MouseButton::RA_MOUSE_RIGHT_BUTTON )
        {
            const auto& result = m_currentRenderer->getPickingResults()[i];
            m_pickingManager->setCurrent( result );
            emit rightClickPicking( result );
        }
    }
}

void Gui::Viewer::fitCameraToScene( const Core::Aabb& aabb ) {
    if ( !aabb.isEmpty() )
    {
        CORE_ASSERT( m_camera != nullptr, "No camera found." );
        m_camera->fitScene( aabb );
    } else
    { LOG( logINFO ) << "Unable to fit the camera to the scene : empty Bbox."; }
}

std::vector<std::string> Gui::Viewer::getRenderersName() const {
    std::vector<std::string> ret;

    for ( const auto& renderer : m_renderers )
    {
        if ( renderer )
        {
            ret.push_back( renderer->getRendererName() );
        }
    }

    return ret;
}

void Gui::Viewer::grabFrame( const std::string& filename ) {
    m_context->makeCurrent( this );

    size_t w, h;
    auto writtenPixels = m_currentRenderer->grabFrame( w, h );

    std::string ext = Core::Utils::getFileExt( filename );

    if ( ext == "bmp" )
    {
        stbi_write_bmp( filename.c_str(), w, h, 4, writtenPixels.get() );
    } else if ( ext == "png" )
    {
        stbi_write_png( filename.c_str(), w, h, 4, writtenPixels.get(), w * 4 * sizeof( uchar ) );
    } else
    { LOG( logWARNING ) << "Cannot write frame to " << filename << " : unsupported extension"; }

    m_context->doneCurrent();
}

void Gui::Viewer::enablePostProcess( int enabled ) {
    m_currentRenderer->enablePostProcess( enabled );
}

void Gui::Viewer::enableDebugDraw( int enabled ) {
    m_currentRenderer->enableDebugDraw( enabled );
}

} // namespace Ra
