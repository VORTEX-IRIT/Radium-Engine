#include <glbinding-aux/ContextInfo.h>
#include <glbinding-aux/debug.h>
#include <glbinding-aux/types_to_string.h>
#include <glbinding/Binding.h>
#include <glbinding/glbinding.h>

#include <glbinding/Version.h>
// Do not import namespace to prevent glbinding/QTOpenGL collision
#include <glbinding/gl/gl.h>

#include <globjects/globjects.h>

// include radium engine here to prevent glbinding incompatibility with gl.h
#include <Gui/Viewer/Viewer.hpp>

#include <iostream>

#include <QOpenGLContext>

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb/stb_image_write.h>

#include <Core/Asset/Camera.hpp>
#include <Core/Asset/FileData.hpp>
#include <Core/Containers/MakeShared.hpp>
#include <Core/Math/Math.hpp>
#include <Core/Utils/Color.hpp>
#include <Core/Utils/Log.hpp>
#include <Core/Utils/StringUtils.hpp>
#include <Engine/Data/ShaderProgramManager.hpp>
#include <Engine/Data/ViewingParameters.hpp>
#include <Engine/Rendering/ForwardRenderer.hpp>
#include <Engine/Rendering/Renderer.hpp>
#include <Engine/Scene/CameraComponent.hpp>
#include <Engine/Scene/CameraManager.hpp>
#include <Engine/Scene/Component.hpp>
#include <Engine/Scene/DirLight.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/SystemDisplay.hpp>
#include <Gui/AboutDialog/RadiumHelpDialog.hpp>
#include <Gui/Utils/KeyMappingManager.hpp>
#include <Gui/Utils/Keyboard.hpp>
#include <Gui/Utils/PickingManager.hpp>
#include <Gui/Viewer/FlightCameraManipulator.hpp>
#include <Gui/Viewer/Gizmo/GizmoManager.hpp>
#include <Gui/Viewer/RotateAroundCameraManipulator.hpp>
#include <Gui/Viewer/TrackballCameraManipulator.hpp>
#include <Gui/Viewer/Viewer.hpp>

#include <QApplication>
#include <QMouseEvent>
#include <QPainter>
#include <QTimer>

namespace Ra {
namespace Gui {

using namespace Core::Utils; // log
using namespace glbinding;

using ViewerMapping = KeyMappingManageable<Viewer>;

#define KMA_VALUE( x ) KeyMappingManager::KeyMappingAction Viewer::x;
KeyMappingViewer
#undef KMA_VALUE

// Register all keymapings related to the viewer and its managed functionalities (Trackball
// camera, Gizmo, ..)
void Viewer::setupKeyMappingCallbacks() {
    auto keyMappingManager = KeyMappingManager::getInstance();

    // Add default manipulator listener
    keyMappingManager->addListener( TrackballCameraManipulator::configureKeyMapping );
    keyMappingManager->addListener( FlightCameraManipulator::configureKeyMapping );
    keyMappingManager->addListener(
        RotateAroundCameraManipulator::KeyMapping::configureKeyMapping );
    // add viewer related listener
    keyMappingManager->addListener( GizmoManager::configureKeyMapping );
    keyMappingManager->addListener( configureKeyMapping );

    m_keyMappingCallbackManager = KeyMappingCallbackManager { ViewerMapping::getContext() };
    m_keyMappingCallbackManager.addEventCallback( VIEWER_TOGGLE_WIREFRAME, [this]( QEvent* event ) {
        if ( event->type() == QEvent::KeyPress ) m_currentRenderer->toggleWireframe();
    } );
    m_keyMappingCallbackManager.addEventCallback( VIEWER_RELOAD_SHADERS, [this]( QEvent* event ) {
        if ( event->type() == QEvent::KeyPress ) reloadShaders();
    } );
    m_keyMappingCallbackManager.addEventCallback(
        VIEWER_PICKING_MULTI_CIRCLE, [this]( QEvent* event ) {
            if ( event->type() == QEvent::KeyPress ) {
                m_isBrushPickingEnabled = !m_isBrushPickingEnabled;
                m_currentRenderer->setBrushRadius( m_isBrushPickingEnabled ? m_brushRadius : 0 );
                emit toggleBrushPicking( m_isBrushPickingEnabled );
            }
        } );
    m_keyMappingCallbackManager.addEventCallback( VIEWER_SWITCH_CAMERA, []( QEvent* event ) {
        if ( event->type() == QEvent::KeyPress ) {
            auto cameraManager = static_cast<Ra::Engine::Scene::CameraManager*>(
                Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );
            static int idx = 0;
            if ( cameraManager->count() > 0 ) {
                idx %= cameraManager->count();
                cameraManager->activate( idx );
            }
            idx++;
        }
    } );

    m_keyMappingCallbackManager.addEventCallback( VIEWER_CAMERA_FIT_SCENE, [this]( QEvent* event ) {
        if ( event->type() == QEvent::KeyPress ) fitCamera();
    } );
    m_keyMappingCallbackManager.addEventCallback( VIEWER_HELP, [this]( QEvent* event ) {
        if ( event->type() == QEvent::KeyPress ) {
            displayHelpDialog();
            requestActivate();
        }
    } );
}

void Viewer::configureKeyMapping_impl() {
    auto keyMappingManager = KeyMappingManager::getInstance();
    ViewerMapping::setContext( keyMappingManager->getContext( "ViewerContext" ) );
    if ( ViewerMapping::getContext().isInvalid() ) {
        LOG( logINFO )
            << "ViewerContext not defined (maybe the configuration file do not contains it)";
        LOG( Ra::Core::Utils::logERROR ) << "ViewerContext all keymapping invalide !";
        return;
    }

#define KMA_VALUE( XX ) XX = keyMappingManager->getAction( ViewerMapping::getContext(), #XX );
    KeyMappingViewer
#undef KMA_VALUE
}

Viewer::Viewer( QScreen* screen ) :
    WindowQt( screen ),
    m_currentRenderer( nullptr ),
    m_pickingManager( new PickingManager() ),
    m_isBrushPickingEnabled( false ),
    m_brushRadius( 10 ),
    m_camera( nullptr ),
    m_gizmoManager( nullptr ),
    m_keyMappingCallbackManager { ViewerMapping::getContext() } {}

Viewer::~Viewer() {
    if ( m_glInitialized.load() ) {
        makeCurrent();
        m_renderers.clear();

        delete m_gizmoManager;
        doneCurrent();
    }
    delete m_pickingManager;
}

void Viewer::setCameraManipulator( CameraManipulator* ci ) {
    m_camera.reset( ci );
}

CameraManipulator* Viewer::getCameraManipulator() {
    return m_camera.get();
}

void Viewer::resetToDefaultCamera() {

    auto cameraManager = static_cast<Ra::Engine::Scene::CameraManager*>(
        Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );
    cameraManager->resetActiveCamera();
}

GizmoManager* Viewer::getGizmoManager() {
    return m_gizmoManager;
}

const Engine::Rendering::Renderer* Viewer::getRenderer() const {
    return m_currentRenderer;
}

Engine::Rendering::Renderer* Viewer::getRenderer() {
    return m_currentRenderer;
}

int Viewer::addRenderer( const std::shared_ptr<Engine::Rendering::Renderer>& e ) {
    m_renderers.push_back( e );

    // initialize the renderer (deferred if GL is not ready yet)
    if ( m_glInitialized.load() ) {
        makeCurrent();
        initializeRenderer( e.get() );
        LOG( logINFO ) << "[Viewer] New Renderer (" << e->getRendererName() << ") added.";
        doneCurrent();
    }
    else {
        LOG( logINFO ) << "[Viewer] New Renderer (" << e->getRendererName()
                       << ") added with deferred init.";
        m_pendingRenderers.push_back( e );
    }

    return m_renderers.size() - 1;
}

PickingManager* Viewer::getPickingManager() {
    return m_pickingManager;
}

void Viewer::update( const Scalar dt ) {
    CORE_UNUSED( dt );
    if ( m_gizmoManager != nullptr ) { m_gizmoManager->updateValues(); }
}

// Asynchronous rendering implementation

void Viewer::startRendering( const Scalar dt ) {

    CORE_ASSERT( m_glInitialized.load(), "OpenGL needs to be initialized before rendering." );

    CORE_ASSERT( m_currentRenderer != nullptr, "No renderer found." );

    m_pickingManager->clear();
    makeCurrent();

    // update znear/zfar to fit the scene ...
    auto entityManager = Engine::RadiumEngine::getInstance()->getEntityManager();
    if ( entityManager ) {

        // to fit scene only
        // auto aabb = Ra::Engine::RadiumEngine::getInstance()->computeSceneAabb();
        // to fit also debug and system entity aabb
        Core::Aabb aabb {};
        for ( const auto& entity : entityManager->getEntities() ) {
            // entity aabb is in world space
            aabb.extend( entity->computeAabb() );
        }

        if ( !aabb.isEmpty() ) { m_camera->getCamera()->fitZRange( aabb ); }
        else {
            auto cameraManager = static_cast<Ra::Engine::Scene::CameraManager*>(
                Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );

            // scene is empty, reset to defaults bounds
            m_camera->setCameraZNear( cameraManager->defaultCamera.getZNear() );
            m_camera->setCameraZFar( cameraManager->defaultCamera.getZFar() );
        }
    }

    /// \todo FIXME : move this outside of the rendering loop. must be done once per renderer ...
    /// if there is no light on the renderer, add the head light attached to the camera ...
    if ( !m_currentRenderer->hasLight() ) {
        if ( m_camera->hasLightAttached() )
            m_currentRenderer->addLight( m_camera->getLight() );
        else
            LOG( logDEBUG ) << "Unable to attach the head light!";
    }
    Engine::Data::ViewingParameters data {
        m_camera->getCamera()->getViewMatrix(), m_camera->getCamera()->getProjMatrix(), dt };
    m_currentRenderer->render( data );
}

void Viewer::swapBuffers() {
    if ( isExposed() ) { m_context->swapBuffers( this ); }
    doneCurrent();
}

void Viewer::processPicking() {
    CORE_ASSERT( m_glInitialized.load(), "OpenGL needs to be initialized before rendering." );

    CORE_ASSERT( m_currentRenderer != nullptr, "No renderer found." );

    CORE_ASSERT( m_currentRenderer->getPickingQueries().size() ==
                     m_currentRenderer->getPickingResults().size(),
                 "There should be one result per query." );

    for ( uint i = 0; i < m_currentRenderer->getPickingQueries().size(); ++i ) {
        const Engine::Rendering::Renderer::PickingQuery& query =
            m_currentRenderer->getPickingQueries()[i];

        if ( query.m_purpose == Engine::Rendering::Renderer::PickingPurpose::MANIPULATION ) {
            const auto& result = m_currentRenderer->getPickingResults()[i];
            m_pickingManager->setCurrent( result );
            emit rightClickPicking( result );
        }
    }
}

void Viewer::fitCameraToScene( const Core::Aabb& aabb ) {
    if ( !aabb.isEmpty() ) {
        CORE_ASSERT( m_camera != nullptr, "No camera found." );
        m_camera->fitScene( aabb );

        // uncomment to display scene aabb
        // RA_DISPLAY_AABB( aabb, Color::Blue() );
        emit needUpdate();
    }
    else { LOG( logINFO ) << "Unable to fit the camera to the scene : empty Bbox."; }
}

void Viewer::fitCamera() {
    auto aabb = Ra::Engine::RadiumEngine::getInstance()->computeSceneAabb();
    if ( aabb.isEmpty() ) { getCameraManipulator()->resetCamera(); }
    else { fitCameraToScene( aabb ); }
}

std::vector<std::string> Viewer::getRenderersName() const {
    std::vector<std::string> ret;

    for ( const auto& renderer : m_renderers ) {
        if ( renderer ) { ret.push_back( renderer->getRendererName() ); }
    }

    return ret;
}

void Viewer::grabFrame( const std::string& filename ) {
    makeCurrent();

    size_t w, h;
    auto writtenPixels = m_currentRenderer->grabFrame( w, h );

    std::string ext = Core::Utils::getFileExt( filename );

    if ( ext == "bmp" ) { stbi_write_bmp( filename.c_str(), w, h, 4, writtenPixels.get() ); }
    else if ( ext == "png" ) {
        stbi_write_png( filename.c_str(), w, h, 4, writtenPixels.get(), w * 4 * sizeof( uchar ) );
    }
    else {
        LOG( logWARNING ) << "Cannot write frame to " << filename << " : unsupported extension";
    }

    doneCurrent();
}

void Viewer::enableDebug() {
    glbinding::setCallbackMask( glbinding::CallbackMask::After |
                                glbinding::CallbackMask::ParametersAndReturnValue );
    glbinding::setAfterCallback( []( const glbinding::FunctionCall& call ) {
        std::cerr << call.function->name() << "(";
        for ( unsigned i = 0; i < call.parameters.size(); ++i ) {
            std::cerr << call.parameters[i].get();
            if ( i < call.parameters.size() - 1 ) { std::cerr << ", "; }
        }
        std::cerr << ")";
        if ( call.returnValue ) { std::cerr << " -> " << call.returnValue.get(); }
        std::cerr << std::endl;
    } );
}

void Viewer::reloadShaders() {
    CORE_ASSERT( m_glInitialized.load(), "OpenGL needs to be initialized reload shaders." );

    makeCurrent();
    // FIXME : check thread-saefty of this.
    m_currentRenderer->lockRendering();
    m_currentRenderer->reloadShaders();
    m_currentRenderer->unlockRendering();
    doneCurrent();

    emit needUpdate();
}

void Viewer::displayTexture( const QString& tex ) {
    CORE_ASSERT( m_glInitialized.load(), "OpenGL needs to be initialized to display textures." );

    makeCurrent();
    m_currentRenderer->lockRendering();
    m_currentRenderer->displayTexture( tex.toStdString() );
    m_currentRenderer->unlockRendering();
    doneCurrent();

    emit needUpdate();
}

bool Viewer::changeRenderer( int index ) {
    if ( m_glInitialized.load() && m_renderers[index] ) {
        makeCurrent();

        if ( m_currentRenderer != nullptr ) { m_currentRenderer->lockRendering(); }

        m_currentRenderer = m_renderers[index].get();
        // renderers in m_renderers are supposed to be locked
        auto deviceSize = toDevice( { width(), height() } );
        m_currentRenderer->resize( deviceSize.x(), deviceSize.y() );
        // Configure the renderObjects for this renderer
        m_currentRenderer->buildAllRenderTechniques();
        m_currentRenderer->unlockRendering();

        LOG( logINFO ) << "[Viewer] Set active renderer: " << m_currentRenderer->getRendererName();

        // resize camera viewport since the one in show event might have 0x0
        if ( m_camera ) { m_camera->getCamera()->setViewport( deviceSize.x(), deviceSize.y() ); }

        doneCurrent();
        emit rendererReady();

        emit needUpdate();
        return true;
    }
    return false;
}

void Viewer::enablePostProcess( int enabled ) {
    m_currentRenderer->enablePostProcess( enabled );
}

void Viewer::enableDebugDraw( int enabled ) {
    m_currentRenderer->enableDebugDraw( enabled );
}

void Viewer::setBackgroundColor( const Core::Utils::Color& background ) {
    m_backgroundColor = background;
    for ( const auto& renderer : m_renderers )
        renderer->setBackgroundColor( m_backgroundColor );

    emit needUpdate();
}

void Viewer::onAboutToCompose() {
    // This slot function is called from the main thread as part of the event loop
    // when the GUI is about to update. We have to wait for the rendering to finish.
    m_currentRenderer->lockRendering();
}

void Viewer::onAboutToResize() {
    // Like swap buffers, resizing is a blocking operation and we have to wait for the rendering
    // to finish before resizing.
    m_currentRenderer->lockRendering();
}

void Viewer::onResized() {
    m_currentRenderer->unlockRendering();
    emit needUpdate();
}

void Viewer::onFrameSwapped() {
    // This slot is called from the main thread as part of the event loop when the
    // GUI has finished displaying the rendered image, so we unlock the renderer.
    m_currentRenderer->unlockRendering();
}

void Viewer::createGizmoManager() {
    if ( m_gizmoManager == nullptr ) { m_gizmoManager = new GizmoManager( this ); }
}

void Viewer::initializeRenderer( Engine::Rendering::Renderer* renderer ) {
    auto deviceSize = toDevice( { width(), height() } );

    // see issue #261 Qt Event order and default viewport management (Viewer.cpp)
    // https://github.com/STORM-IRIT/Radium-Engine/issues/261
    gl::glViewport( 0, 0, deviceSize.x(), deviceSize.y() );

    renderer->initialize( deviceSize.x(), deviceSize.y() );
    renderer->setBackgroundColor( m_backgroundColor );
    renderer->lockRendering();
}

bool Viewer::initializeGL() {
    globjects::init( getProcAddress );
    // mark openGL as initialized
    m_glInitialized = true;

    LOG( logINFO ) << "*** Radium Engine OpenGL context ***";
    LOG( logINFO ) << "Renderer (glbinding) : " << glbinding::aux::ContextInfo::renderer();
    LOG( logINFO ) << "Vendor   (glbinding) : " << glbinding::aux::ContextInfo::vendor();
    LOG( logINFO ) << "OpenGL   (glbinding) : "
                   << glbinding::aux::ContextInfo::version().toString();
    LOG( logINFO ) << "GLSL                 : "
                   << gl::glGetString( gl::GLenum( GL_SHADING_LANGUAGE_VERSION ) );

    LOG( logINFO ) << "*** Radium Engine Viewer ***";

    // emit the signal so that the client will initialize the OpenGL part of the Engine
    // and custom OpenGL properties
    emit requestEngineOpenGLInitialization();

    // Configure the viewer services
    auto deviceSize = toDevice( { width(), height() } );
    // create default camera interface : trackball
    m_camera = std::make_unique<TrackballCameraManipulator>();
    m_camera->getCamera()->setViewport( deviceSize.x(), deviceSize.y() );

    /// \todo who deletes this light ?
    auto headlight = new Engine::Scene::DirectionalLight(
        Ra::Engine::Scene::SystemEntity::getInstance(), "headlight" );
    headlight->setColor( Ra::Core::Utils::Color::Grey( 1.0_ra ) );

    m_camera->attachLight( headlight );

    // Register to the camera manager active camera changes
    auto cameraManager = static_cast<Ra::Engine::Scene::CameraManager*>(
        Engine::RadiumEngine::getInstance()->getSystem( "DefaultCameraManager" ) );
    cameraManager->activeCameraObservers().attach(
        [this, size = deviceSize]( Core::Utils::Index /*idx*/ ) {
            m_camera->updateCamera();
            m_camera->getCamera()->setViewport( size.x(), size.y() );
        } );

    // Initialize renderers added to the viewer before initializeGL
    for ( auto& rptr : m_pendingRenderers ) {
        initializeRenderer( rptr.get() );
    }
    m_pendingRenderers.clear();

    // create the gizmo manager (Ui)
    createGizmoManager();

    // Signal that OpenGL is initialized
    emit glInitialized();

    // If no renderer was added before that (either by slots on requestEngineOpenGLInitialization
    // or on glInitialized), add default forward renderer
    if ( m_renderers.empty() ) {
        LOG( logINFO ) << "[Viewer] No renderer added, adding default (Forward Renderer)";
        std::shared_ptr<Ra::Engine::Rendering::Renderer> e(
            new Ra::Engine::Rendering::ForwardRenderer() );
        addRenderer( e );
    }

    if ( m_currentRenderer == nullptr ) { changeRenderer( 0 ); }

    return m_glInitialized;
}

void Viewer::resizeGL( QResizeEvent* event ) {
    auto context = activateScopedContext();

    auto deviceSize = toDevice( { event->size().width(), event->size().height() } );
    gl::glViewport( 0, 0, deviceSize.x(), deviceSize.y() );
    m_camera->getCamera()->setViewport( deviceSize.x(), deviceSize.y() );
    m_currentRenderer->resize( deviceSize.x(), deviceSize.y() );

    emit needUpdate();
}

Engine::Rendering::Renderer::PickingMode
Viewer::getPickingMode( const KeyMappingManager::KeyMappingAction& action ) const {
    if ( action == VIEWER_PICKING_VERTEX ) {
        return m_isBrushPickingEnabled ? Engine::Rendering::Renderer::C_VERTEX
                                       : Engine::Rendering::Renderer::VERTEX;
    }
    if ( action == VIEWER_PICKING_EDGE ) {
        return m_isBrushPickingEnabled ? Engine::Rendering::Renderer::C_EDGE
                                       : Engine::Rendering::Renderer::EDGE;
    }
    if ( action == VIEWER_PICKING_TRIANGLE ) {
        return m_isBrushPickingEnabled ? Engine::Rendering::Renderer::C_TRIANGLE
                                       : Engine::Rendering::Renderer::TRIANGLE;
    }
    if ( action == VIEWER_PICKING ) { return Engine::Rendering::Renderer::RO; }
    return Engine::Rendering::Renderer::NONE;
}

void Viewer::propagateEventToParent( QEvent* event ) {
    event->ignore();
    if ( !isTopLevel() ) { QApplication::sendEvent( parent(), event ); }
}

void Viewer::keyPressEvent( QKeyEvent* event ) {
    keyPressed( event->key() );
    if ( !m_glInitialized.load() || event->isAutoRepeat() || !handleKeyPressEvent( event ) )
        propagateEventToParent( event );
    else
        emit needUpdate();
}

void Viewer::keyReleaseEvent( QKeyEvent* event ) {
    if ( !m_glInitialized.load() || !handleKeyReleaseEvent( event ) )
        propagateEventToParent( event );
    else
        emit needUpdate();
    keyReleased( event->key() );
}

void Viewer::mousePressEvent( QMouseEvent* event ) {
    if ( !m_glInitialized.load() ) {
        propagateEventToParent( event );
        return;
    }

    m_currentRenderer->setMousePosition( toDevice( { event->pos().x(), event->pos().y() } ) );

    // get what's under the mouse
    auto result = pickAtPosition( toDevice( { event->pos().x(), height() - event->pos().y() } ) );
    m_depthUnderMouse = result.getDepth();

    handleMousePressEvent( event, result );
    emit onMousePress( event );
    emit needUpdate();
}

void Viewer::mouseReleaseEvent( QMouseEvent* event ) {
    handleMouseReleaseEvent( event );
    emit onMouseRelease( event );
    emit needUpdate();
}

void Viewer::mouseMoveEvent( QMouseEvent* event ) {
    if ( !m_glInitialized.load() ) {
        event->ignore();
        return;
    }

    m_currentRenderer->setMousePosition( toDevice( { event->pos().x(), event->pos().y() } ) );

    auto result = pickAtPosition( toDevice( { event->pos().x(), height() - event->pos().y() } ) );
    m_depthUnderMouse = result.getDepth();

    handleMouseMoveEvent( event, result );
    emit onMouseMove( event );
    emit needUpdate();
}

void Viewer::wheelEvent( QWheelEvent* event ) {

    if ( !m_glInitialized.load() ) {
        event->ignore();
        return;
    }

    handleWheelEvent( event );

    emit needUpdate();
}

void Viewer::showEvent( QShowEvent* ev ) {
    WindowQt::showEvent( ev );
    auto deviceSize = toDevice( { width(), height() } );
    m_camera->getCamera()->setViewport( deviceSize.x(), deviceSize.y() );

    emit needUpdate();
}

void Viewer::focusOutEvent( QFocusEvent* ) {
    releaseAllKeys();
}

std::tuple<KeyMappingManager::KeyMappingAction,
           KeyMappingManager::KeyMappingAction,
           KeyMappingManager::KeyMappingAction>
Viewer::getComponentActions( const Qt::MouseButtons& buttons,
                             const Qt::KeyboardModifiers& modifiers,
                             int key,
                             bool wheel ) {
    auto keyMap = KeyMappingManager::getInstance();

    auto actionCamera =
        keyMap->getAction( keyMap->getContext( "CameraContext" ), buttons, modifiers, key, wheel );
    auto actionGizmo =
        keyMap->getAction( keyMap->getContext( "GizmoContext" ), buttons, modifiers, key, wheel );
    auto actionViewer =
        keyMap->getAction( keyMap->getContext( "ViewerContext" ), buttons, modifiers, key, wheel );
    return { actionCamera, actionGizmo, actionViewer };
}

bool Viewer::handleKeyPressEvent( QKeyEvent* event ) {
    bool eventCatched = false;

    auto [actionCamera, actionGizmo, actionViewer] =
        getComponentActions( Qt::NoButton, event->modifiers(), activeKey(), false );

    // Is keymapping something of the viewer only ?
    // or should be dispatched to all receivers ?

    if ( actionCamera.isValid() ) {
        eventCatched = m_camera->handleKeyPressEvent( event, actionCamera );
    }
    else if ( actionGizmo.isValid() ) {
        // \todo add gizmo manager handleKeyPressEvent
        // m_gizmoManager->handleKeyPressEvent( event, action );
        // eventCatched = true;
    }
    else if ( actionViewer.isValid() ) {
        eventCatched = m_keyMappingCallbackManager.triggerEventCallback( actionViewer, event );
    }
    return eventCatched;
}

bool Viewer::handleKeyReleaseEvent( QKeyEvent* event ) {
    bool eventCatched = false;

    // Is keymapping something of the viewer only ?
    // or should be dispatched to all receivers ?
    auto [actionCamera, actionGizmo, actionViewer] =
        getComponentActions( Qt::NoButton, event->modifiers(), activeKey(), false );

    if ( actionCamera.isValid() ) {
        eventCatched = m_camera->handleKeyReleaseEvent( event, actionCamera );
    }
    else if ( actionViewer.isValid() ) {
        eventCatched = m_keyMappingCallbackManager.triggerEventCallback( actionViewer, event );
    }
    return eventCatched;
}

void Viewer::handleMousePressEvent( QMouseEvent* event,
                                    Ra::Engine::Rendering::Renderer::PickingResult& result ) {

    ///\todo something like explained here
    /// if under mouse objects grabs the action, just send it to the object
    /// so we need to have something like
    /// grabber  = renderobject(ro id)->getMouseManipulator
    /// if(grabber && grabber->handleEvent(event, buttons, modifiers, key){
    /// context = grabber->getContext
    /// currentGrabber = grabber (we need to store it for mouse move)

    // for now just handle one active context
    m_activeContext.setInvalid();

    auto keyMap = KeyMappingManager::getInstance();
    //! [event dispatch]
    auto buttons   = event->buttons();
    auto modifiers = event->modifiers();
    auto key       = activeKey();

    // nothing under mouse ? juste move the camera ...
    if ( result.getRoIdx().isInvalid() ) {
        if ( m_camera->handleMousePressEvent( event, buttons, modifiers, key ) ) {
            m_activeContext = m_camera->mappingContext();
        }
        else {
            // should not pass here, since viewerContext is only for valid picking ...
            m_activeContext = KeyMappingManageable::getContext();
        }
    }
    //! [event dispatch]
    else {
        // something under the mouse, let's check if it's a gizmo ro
        getGizmoManager()->handlePickingResult( result.getRoIdx() );
        if ( getGizmoManager()->handleMousePressEvent(
                 event, buttons, modifiers, key, *m_camera->getCamera() ) ) {
            m_activeContext = GizmoManager::getContext();
        } // if not, try to do camera stuff
        else if ( m_camera->handleMousePressEvent( event, buttons, modifiers, key ) ) {
            m_activeContext = m_camera->mappingContext();
        }
        else {
            m_activeContext  = KeyMappingManageable::getContext();
            auto action      = keyMap->getAction( m_activeContext, buttons, modifiers, key );
            auto pickingMode = getPickingMode( action );

            if ( pickingMode != Ra::Engine::Rendering::Renderer::NONE ) {
                // Push query, we may also do it here ...
                Engine::Rendering::Renderer::PickingQuery query = {
                    toDevice( { event->x(), height() - event->y() } ),
                    Engine::Rendering::Renderer::PickingPurpose::MANIPULATION,
                    pickingMode };
                m_currentRenderer->addPickingRequest( query );
            }
        }
    }
}

void Viewer::handleMouseReleaseEvent( QMouseEvent* event ) {
    if ( m_activeContext == m_camera->mappingContext() ) {
        m_camera->handleMouseReleaseEvent( event );
    }
    if ( m_activeContext == GizmoManager::getContext() ) {
        m_gizmoManager->handleMouseReleaseEvent( event );
    }
    m_activeContext.setInvalid();
}

void Viewer::handleMouseMoveEvent( QMouseEvent* event,
                                   Ra::Engine::Rendering::Renderer::PickingResult& result ) {

    auto keyMap    = KeyMappingManager::getInstance();
    auto buttons   = event->buttons();
    auto modifiers = event->modifiers();
    auto key       = activeKey();
    // if needed can use
    //    auto action = keyMap->getAction( m_activeContext, buttons, modifiers, key );

    if ( m_activeContext == m_camera->mappingContext() ) {
        m_camera->handleMouseMoveEvent( event, buttons, modifiers, key );
    }
    else if ( m_activeContext == GizmoManager::getContext() ) {
        m_gizmoManager->handleMouseMoveEvent(
            event, buttons, modifiers, key, *m_camera->getCamera() );
    }
    else if ( m_activeContext == KeyMappingManageable::getContext() ) {
        auto action      = keyMap->getAction( m_activeContext, buttons, modifiers, key );
        auto pickingMode = getPickingMode( action );
        if ( pickingMode != Ra::Engine::Rendering::Renderer::NONE ) {
            Engine::Rendering::Renderer::PickingQuery query = {
                toDevice( { event->x(), height() - event->y() } ),
                Engine::Rendering::Renderer::PickingPurpose::MANIPULATION,
                pickingMode };
            m_currentRenderer->addPickingRequest( query );
        }
    }
    else { getGizmoManager()->handlePickingResult( result.getRoIdx() ); }
}

void Viewer::handleWheelEvent( QWheelEvent* event ) {

    auto keyMap    = KeyMappingManager::getInstance();
    auto buttons   = event->buttons();
    auto modifiers = event->modifiers();
    auto key       = activeKey();
    auto action =
        keyMap->getAction( KeyMappingManageable::getContext(), buttons, modifiers, key, true );

    if ( action == VIEWER_SCALE_BRUSH && m_isBrushPickingEnabled ) {
        m_brushRadius +=
            ( event->angleDelta().y() * 0.01 + event->angleDelta().x() * 0.01 ) > 0 ? 5 : -5;
        m_brushRadius = std::max( m_brushRadius, Scalar( 5 ) );
        m_currentRenderer->setBrushRadius( m_brushRadius );
    }
    else { m_camera->handleWheelEvent( event, buttons, modifiers, key ); }
}

Ra::Engine::Rendering::Renderer::PickingResult Viewer::pickAtPosition( Core::Vector2 position ) {
    makeCurrent();
    auto result = m_currentRenderer->doPickingNow(
        { position,
          Engine::Rendering::Renderer::PickingPurpose::SELECTION,
          Engine::Rendering::Renderer::RO },
        { m_camera->getCamera()->getViewMatrix(), m_camera->getCamera()->getProjMatrix(), 0. } );

    doneCurrent();
    return result;
}

bool Viewer::prepareDisplay() {
    auto renderer = getRenderer();
    if ( renderer ) {
        makeCurrent();
        getRenderer()->buildAllRenderTechniques();
        auto aabb = Ra::Engine::RadiumEngine::getInstance()->computeSceneAabb();
        if ( aabb.isEmpty() ) { getCameraManipulator()->resetCamera(); }
        else { fitCameraToScene( aabb ); }
        doneCurrent();
        return true;
    }
    return false;
}

void Viewer::displayHelpDialog() {
    if ( !m_helpDialog ) { m_helpDialog.reset( new RadiumHelpDialog() ); }
    m_helpDialog->show();
    m_helpDialog->raise();
    m_helpDialog->activateWindow();
}

KeyMappingManager::KeyMappingAction
Viewer::addCustomAction( const std::string& actionName,
                         const KeyMappingManager::EventBinding& binding,
                         std::function<void( QEvent* )> callback ) {

    return m_keyMappingCallbackManager.addActionAndCallback( actionName, binding, callback );
}

} // namespace Gui
} // namespace Ra
