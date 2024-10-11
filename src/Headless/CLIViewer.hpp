#pragma once

#include <Core/Tasks/TaskQueue.hpp>
#include <Headless/CLIBaseApplication.hpp>
#include <Headless/OpenGLContext/OpenGLContext.hpp>

#include <Core/Utils/Index.hpp>

#include <functional>

#include <glbinding/Version.h>

namespace Ra {
namespace Core {
namespace Asset {
class FileLoaderInterface;
class Camera;
} // namespace Asset
} // namespace Core

namespace Engine {
class RadiumEngine;
namespace Rendering {
class Renderer;
}
} // namespace Engine

namespace Headless {

/**
 * Base class for radium based cmdline application.
 *
 * This is the Headless pendant of the Ra::Gui::Viewer class, with only a very little
 * interaction capabilities.
 *
 */
class HEADLESS_API CLIRadiumEngineApp : public CLIBaseApplication
{
  public:
    /**
     * \brief Construct the viewer using an OpenGL context of the given version
     * \param context the opengl context, e.g. EglOpenGLContext or GlfwOpenGLContext.
     */
    explicit CLIRadiumEngineApp( std::unique_ptr<OpenGLContext> context );
    /// Base destructor
    virtual ~CLIRadiumEngineApp();

    /// To have the same API to access Engine than in Qt based application.
    const Engine::RadiumEngine* getEngine() const { return m_engine; }

    /**
     * \brief Application initialization method.
     *
     * This method is called by the main function to initialize the app giving its parameters.
     * \param argc number of parameter
     * \param argv array of string representing the parameters
     * \return 0 if the application is correctly initialized or an application dependant error code
     * if something went wrong.
     *
     * Supported command line parameters are --help and --file \<filename\> from
     * CLIRadiumApplication and the following parameters :
     *   - --size \<width x height\> : the size of the rendered picture
     *   - --animation : load the Radium animation system
     *   - --env \<env_map\> : load and use the given environment map.
     */
    int init( int argc, const char* argv[] ) override;

    /** Register OpenGL addons
     *  The given functor will be called with the OpenGL context bound
     */
    void openGlAddOns( std::function<void()> f );

    /** Activate/deactivate the OpenGL context */
    void bindOpenGLContext( bool on = true );

    //  private:
  protected: /// \todo make privaet and check access
    /// Headless OpenGLContext
    std::unique_ptr<OpenGLContext> m_glContext;

    /// is the engine initialized ?
    bool m_engineInitialized { false };

    /// Instance of the radium engine.
    Ra::Engine::RadiumEngine* m_engine;
};

class HEADLESS_API CLIViewer : public CLIRadiumEngineApp
{
  public:
    struct ViewerParameters {
        /// Load animation system at startup
        bool m_animationEnable { false };
        /// Size of the image
        std::array<int, 2> m_size { { 512, 512 } };
        /// image name prefix
        std::string m_imgPrefix { "frame" };
        /// The data file to manage
        std::string m_dataFile = { "" };
    };

  public:
    /**
     * \brief Construct the viewer using an OpenGL context of the given version
     * \param context the opengl context, e.g. EglOpenGLContext or GlfwOpenGLContext.
     */
    explicit CLIViewer( std::unique_ptr<OpenGLContext> context );
    /// Base destructor
    virtual ~CLIViewer();

    /**
     * \brief Application initialization method.
     *
     * This method is called by the main function to initialize the app giving its parameters.
     * \param argc number of parameter
     * \param argv array of string representing the parameters
     * \return 0 if the application is correctly initialized or an application dependant error code
     * if something went wrong.
     *
     * Supported command line parameters are --help and --file \<filename\> from
     * CLIRadiumApplication and the following parameters :
     *   - --size \<width x height\> : the size of the rendered picture
     *   - --animation : load the Radium animation system
     *   - --env \<env_map\> : load and use the given environment map.
     */
    int init( int argc, const char* argv[] ) override;

    /**
     * Render one frame of the scene attached to the engine for the given time
     * stamp.
     *
     * \return 0 if the image was correctly computed or an application dependant error code if
     * something went wrong.
     */
    int oneFrame( float timeStep = 1.f / 60.f );

    /**
     * Set the renderer to use to compute an image
     *  The app takes ownership of the give pointer.
     */
    void setRenderer( Ra::Engine::Rendering::Renderer* renderer );

    /**
     * Add a data loader to the engine
     */
    void addDataFileLoader( Ra::Core::Asset::FileLoaderInterface* loader );

    /**
     * Loads the scene from the datafile given at the command line or set using setDataFileName
     * using the available loader (if any).
     */
    void loadScene();

    /** Instantiate the scene and prepare the rendering.
     * This method prepare all render techniques for the active renderer.
     */
    void compileScene();

    /** \brief Define the camera to be used for rendering.
     * If the index is invalid according to the engine's camera manager, set the camera from index 0
     * after, if needed, creating and adding to the manager a default camera.
     * \param camIdx the requested camera from the engine's camera manager
     * \return the used index from the manager (will differ if the requested camera does not exist)
     */
    Ra::Core::Utils::Index setCamera( Ra::Core::Utils::Index camIdx = 0 );

    /** Set the image prefix ("frame" by default) */
    void setImageNamePrefix( std::string s );

    /** Get the current viewer command line parameters parameters */
    const ViewerParameters& getCommandLineParameters() const;

    /** grab the last rendered frame */
    std::unique_ptr<unsigned char[]> grabFrame( size_t& w, size_t& h ) const;

    /** show/hide the opengl window */
    void showWindow( bool on                       = true,
                     OpenGLContext::EventMode mode = OpenGLContext::EventMode::POLL,
                     float delay                   = 1.f / 60.f );

    /** \brief Gives access to the window to add event processing callbacks
     *
     * example :
     * \code{.cpp}
         idListener = getWindow().mouseListener.attach( mouseEventProcessingFunction );
       \endcode
      */
    inline OpenGLContext& getWindow();

    /** If a window is shown, launch the interactive render loop */
    void renderLoop( std::function<void( float )> render );

    /**
     * Get the filename given using the option --file \<filename\> or -f \<filename\>
     */
    inline std::string getDataFileName() const;

    /**
     * Set the filename given using the option --file \<filename\> or -f \<filename\>
     */
    inline void setDataFileName( std::string filename );

  protected:
    /// Observer of the resize event on the OpenGLContext
    void resize( int width, int height );

  private:
    /// Shared instance of the renderer
    std::shared_ptr<Ra::Engine::Rendering::Renderer> m_renderer;

    /// The camera for rendering
    Ra::Core::Asset::Camera* m_camera { nullptr };

    /// The application parameters
    ViewerParameters m_parameters;

    /// is the window shown ?
    bool m_exposedWindow { false };
};

inline std::string CLIViewer::getDataFileName() const {
    return m_parameters.m_dataFile;
}

inline void CLIViewer::setDataFileName( std::string filename ) {
    m_parameters.m_dataFile = std::move( filename );
}

inline OpenGLContext& CLIViewer::getWindow() {
    return *m_glContext;
}

} // namespace Headless
} // namespace Ra
