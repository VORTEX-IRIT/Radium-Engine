#pragma once
#include <Core/Utils/Observable.hpp>
#include <Headless/RaHeadless.hpp>

#include <array>
#include <functional>
#include <string>

#include <glbinding-aux/ContextInfo.h>
#include <glbinding/Version.h>
#include <glbinding/gl/gl.h>

namespace Ra {
namespace Headless {

/**
 * This class defines the interface for any off-screen OpenGL Context.
 * There are two use cases of such a context :
 *   - headless rendering application.
 *   - simple window with limited interaction for context providers supporting windows
 */
class HEADLESS_API OpenGLContext
{
  public:
    /** \name OpenGL context management
     *  These methods allow to create and manipulate an openGLContext.
     *  Using this function, the openGL context created is an offscreen context with no exposed
     *  window.
     */
    /**\{ */
    /**
     * Create an offscreen openGl context.
     * The created context has the following properties
     *  - OpenGL version requested : (default >= 4.1)
     *  - OpenGL context profile : Core Profile
     *  The created context is associated with a hidden window that can be shown later.
     *  \param glVersion (optional, default is 4.1) indicates the OpenGL version the context MUST
     *  be compatible with. If there is no compatible context, the application will stop
     * \param size
     */
    explicit OpenGLContext( const glbinding::Version& glVersion = { 4, 1 },
                            const std::array<int, 2>& size      = { { 1, 1 } } ) {
        CORE_UNUSED( glVersion );
        CORE_UNUSED( size );
    };
    /// destructor
    virtual ~OpenGLContext() = default;
    /// make the context active
    virtual void makeCurrent() const = 0;
    /// make the context inactive
    virtual void doneCurrent() const = 0;
    /// Check for validity of the context
    virtual bool isValid() const = 0;

    /// Check if the context is associated to a window
    virtual bool isWindow() const { return false; }

    /// Return a string identifying the openGL Context and its supported versions
    [[nodiscard]] virtual std::string getInfo() const;
    /** \} */

    /** \name Window management from an openGL context
     *  These methods allow to display and interact with a simple window associated with the created
     *  OpenGL Context, if any.
     */
    /** \{ */
    /// Identify the event processing method when the window is exposed.
    enum class EventMode : int { POLL = 0, WAIT, TIMEOUT, NUM_MODES };
    /// Show the window
    virtual void show( EventMode /*mode*/, float /*delay*/ ) {};
    /// Hide the window
    virtual void hide() {};
    /// Resize the window
    virtual void resize( const std::array<int, 2>& /*size*/ ) {};
    /// loop on events and execute the functor render after each event
    virtual void renderLoop( std::function<void( float )> /*render*/ ) {};

    /// Give access to the resize event observable so that client can add Observer to this
    /// event.
    ///
    /// The parameters sent to the resize listeners are in pixels and correspond to the OpenGL
    /// Framebuffer size (i.e. the size given to glViewport function)
    /// \see https://www.glfw.org/docs/latest/window_guide.html#window_fbsize
    Ra::Core::Utils::Observable<int, int>& resizeListener() { return m_resizers; }

    // TODO : give access to the DPI ratio
    // https://www.glfw.org/docs/latest/window_guide.html#window_scale

    /// Give access to the keyboard event observable so that client can add Observer to this
    /// event.
    ///
    /// The parameters sent to the keyboard listeners are
    /// the keyboard key, platform-specific scancode, key action and modifier bits.
    /// \see https://www.glfw.org/docs/latest/input_guide.html#input_keyboard
    Ra::Core::Utils::Observable<int, int, int, int>& keyboardListener() {
        return m_keyboardObservers;
    }

    /// Give access to the mouse event observable so that client can add Observer to this
    /// event.
    ///
    /// The parameters sent to the mouse listeners are
    /// the mouse button, button action and modifier bits as well as the mouse position
    /// in pixel unit and in the FrameBuffer space. The origin is at the top left of the
    /// framebuffer.
    /// \see https://www.glfw.org/docs/latest/input_guide.html#input_mouse
    Ra::Core::Utils::Observable<int, int, int, int, int>& mouseListener() {
        return m_mouseObservers;
    }

    /// Give access to the mouse move event observable so that client can add Observer to this
    /// event.
    ///
    /// The parameters sent to the mouse move listeners are the mouse position
    /// in pixel unit and in the FrameBuffer space. The origin is at the top left of the
    /// framebuffer.
    /// \see https://www.glfw.org/docs/latest/input_guide.html#cursor_pos
    Ra::Core::Utils::Observable<int, int>& mouseMoveListener() { return m_mouseMoveObservers; }

    /// Give access to the scroll event observable so that client can add Observer to this
    /// event.
    ///
    /// The parameters sent to the scroll listeners are two-dimensional scroll offsets
    /// in pixel unit and in the FrameBuffer space.
    /// \see https://www.glfw.org/docs/latest/input_guide.html#scrolling
    Ra::Core::Utils::Observable<int, int>& scrollListener() { return m_scrollObservers; }

    /** \} */
  protected:
    /// Process the pending events according to the window show mode
    virtual bool processEvents() { return true; };

    /// Resize callback
    void resizeFrameBuffer( int width, int height );
    /// Resize event observable
    Ra::Core::Utils::Observable<int, int> m_resizers;

    /// Keyboard callback
    void keyboardEventCallback( int key, int scancode, int action, int mods );
    /// Keyboard event observable
    Ra::Core::Utils::Observable<int, int, int, int> m_keyboardObservers;

    /// Mouse callback
    void mouseEventCallback( int button, int action, int mods, int x, int y );
    /// Mouse event observable
    Ra::Core::Utils::Observable<int, int, int, int, int> m_mouseObservers;

    /// Mouse move callback
    void mouseMoveEventCallback( int x, int y );
    /// Mouse move event observable
    Ra::Core::Utils::Observable<int, int> m_mouseMoveObservers;

    /// Scroll callback
    void scrollEventCallback( int xoffset, int yoffset );
    /// Scroll event observable
    Ra::Core::Utils::Observable<int, int> m_scrollObservers;

    /// Event processing mode
    EventMode m_mode { EventMode::POLL };
    /// Timeout delay for event processing
    float m_delay { 1.f / 60.f };
};

inline std::string OpenGLContext::getInfo() const {
    std::stringstream infoText;
    using ContextInfo = glbinding::aux::ContextInfo;
    makeCurrent();
    infoText << "*** OffScreen OpenGL context ***" << std::endl;
    infoText << "Renderer (glbinding) : " << ContextInfo::renderer() << "\n";
    infoText << "Vendor   (glbinding) : " << ContextInfo::vendor() << "\n";
    infoText << "OpenGL   (glbinding) : " << ContextInfo::version().toString() << "\n";
    infoText << "GLSL                 : " << gl::glGetString( gl::GL_SHADING_LANGUAGE_VERSION )
             << "\n";
    doneCurrent();

    return infoText.str();
}

inline void OpenGLContext::resizeFrameBuffer( int width, int height ) {
    gl::glViewport( 0, 0, width, height );
    m_resizers.notify( width, height );
}

inline void OpenGLContext::keyboardEventCallback( int key, int scancode, int action, int mods ) {
    m_keyboardObservers.notify( key, scancode, action, mods );
}

inline void OpenGLContext::mouseEventCallback( int button, int action, int mods, int x, int y ) {
    m_mouseObservers.notify( button, action, mods, x, y );
}

inline void OpenGLContext::mouseMoveEventCallback( int x, int y ) {
    m_mouseMoveObservers.notify( x, y );
}

inline void OpenGLContext::scrollEventCallback( int xoffset, int yoffset ) {
    m_scrollObservers.notify( xoffset, yoffset );
}

} // namespace Headless
} // namespace Ra
