#pragma once

#include <memory>
#include <vector>

#include <QObject>
#include <QtPlugin>

#include <PluginBase/PluginContext.hpp> // do not use forward declaration to ease usage in Plugin

class QWidget;
class QMenu;
class QAction;
class QOpenGLContext;
class QString;

namespace Ra {

namespace Engine {
namespace Rendering {
class Renderer;
}
} // namespace Engine

namespace Core {
namespace Asset {
class FileLoaderInterface;
}
} // namespace Core

namespace Plugins {
/**
 * \brief Interface class for Radiums plugins
 *
 * To write your own plugin, you need to write a class as follow:
 * \code
    class MyPlugin : public QObject, Ra::Plugins::RadiumPluginInterface
    {
        Q_OBJECT
        Q_RADIUM_PLUGIN_METADATA
        Q_INTERFACES( Ra::Plugins::RadiumPluginInterface )

    public:
        MyPlugin();
        ~MyPlugin() override;

        virtual void registerPlugin( const Ra::Plugins::Context& context ) override;

        virtual bool doAddWidget( QString& name ) override;
        virtual QWidget* getWidget() override;

        virtual bool doAddMenu() override;
        virtual QMenu* getMenu() override;

        virtual bool doAddAction( int &nb ) override;
        virtual QAction* getAction( int id ) override;
    };
    \endcode

    \note `Q_RADIUM_PLUGIN_METADATA`: we use this macro to load a default metadata files,
    which sets the basic metadata required by Radium. The macro is defined in `RaPluginBase.hpp`.
    If you need to use your own file, you have to extend the filespluginMetaDataXXXX.json, packaged
    with Radium in the following folder
      - source tree: src/PluginBase/
      - install tree: lib/cmake/Radium/
    Then, replace `Q_RADIUM_PLUGIN_METADATA` by
    `Q_PLUGIN_METADATA( IID "RadiumEngine.PluginInterface"  FILE "yourPluginMetaDataRelease.json" )`
    or
    `Q_PLUGIN_METADATA( IID "RadiumEngine.PluginInterface"  FILE "yourPluginMetaDataDebug.json" )`
    depending on the debug mode
 */
class RadiumPluginInterface
{

  public:
    virtual ~RadiumPluginInterface() {}

    /**
     * \brief Pass arguments for plugin initialization.
     * This method must create and register all the services that the plugin wants to offer :
     * system, renderers, materials, Ui ...
     *
     * \param context : plugin context containing the engine and UI interfaces.
     */
    virtual void registerPlugin( const Context& context ) = 0;

    /**
     * \brief Tells wether the plugin wants to add a widget
     * (inside the UI tab widget) or not. If it does, it must provide
     * a name for it, and getWidget() will be called.
     * \param name Name of the tab to be added, if needed.
     * \return True if the plugin wants to add a widget, false otherwise
     */
    virtual bool doAddWidget( QString& name ) = 0;

    /**
     * \brief Creates the widget to be added to the ui and then returns it.
     * If connections are needed (between plugin ui and plugin internals)
     * they have to be created here.
     * \return The created and configured widget
     * \todo Find a better name for this ?
     */
    virtual QWidget* getWidget() = 0;

    /**
     * \brief Tells wether the plugin wants to add a menu
     * or not. If it does, getMenu() will be called.
     * \return True if the plugin wants to add a menu, false otherwise
     */
    virtual bool doAddMenu() = 0;

    /**
     * \brief Creates to menu to be added to the ui and then returns it.
     * \return The created menu
     * \todo Find a better name ?
     */
    virtual QMenu* getMenu() = 0;

    /**
     * \brief Tells wether the plugin wants to add actions
     * or not. If it does, getAction() will be called for each one of them.
     * \param nb the number of action the plugin wants to add.
     * \return True if the plugin wants to add a menu, false otherwise
     */
    virtual bool doAddAction( int& nb ) = 0;

    /**
     * \brief Returns the action to be added to the ui and then returns it.
     * \return The action to add.
     * \warning This mechanism implies QAction copies and does not allow to control the appearance
     * of multiple actions with interlocked behaviors, e.g. play/pause and stop buttons.
     */
    virtual QAction* getAction( int id ) = 0;

    /**
     * Tells if the plugin will add a renderer
     * \return true if the plugin offer rendering service
     */
    virtual bool doAddRenderer() { return false; }

    /**
     * \brief addRenderers
     *
     * \param rds the renderers that the plugin wants to expose to the application.
     * \warning Allocated renderers are given to the application that takes ownership. They
     * MUST not be destroyed by the plugin
     */
    virtual void addRenderers(
        [[maybe_unused]] std::vector<std::shared_ptr<Ra::Engine::Rendering::Renderer>>* rds ) {}

    /**
     * Tells if the system will add a file loader
     * \return true if a file loader service is offered
     */
    virtual bool doAddFileLoader() { return false; }

    /**
     * Add the file loader services offered by the plugin
     * \param fl The set of file loader to add
     */
    virtual void addFileLoaders(
        [[maybe_unused]] std::vector<std::shared_ptr<Core::Asset::FileLoaderInterface>>* fl ) {}

    /**
     * \brief openGlInitialize
     *
     * \param context (const PluginContext& context) the Plugin context
     * \see https://herbsutter.com/2013/06/05/gotw-91-solution-smart-pointer-parameters/ for the
     * reason a const QOpenGLContext * is given \warning Allocated renderers are given to the
     * application and MUST not be destroyed by the plugin
     * \note Plugins that want to manage OpenGL resources MUST ENSURE at the time they create/access
     * these resources that the current openGLContext is the one pass when initializing OpenGL here
     * and stored in the PluginContext ...
     * \note When this function is called from the application, The active OpenGL Context is the one
     * that will be used for rendering.
     *
     * Developers of plugins that need to manage OpenGL resources in a direct way (recommended for
     * plugins that manage OpenGL resources for rendering or interaction with rendering) must
     * implement this method and memorize the viewer of the PluginContext. Each time they need to
     * create or destroy an OpenGL resource, they must activate this context (viewer->makeCurrent())
     * and realease it after usage (viewer->doneCurrent())
     * \see Ra::Gui::Viewer
     */
    virtual void openGlInitialize( [[maybe_unused]] const Context& context ) {}

    /**
     * Tells if the plugin offer OpenGL based services that need to be initialized after the OpenGL
     * context is created.
     * \return true if plugin offers OppenGL services
     */
    virtual bool doAddROpenGLInitializer() { return false; }
};
} // namespace Plugins
} // namespace Ra

#define RadiumPluginInterface_IID "RadiumEngine.PluginInterface"
Q_DECLARE_INTERFACE( Ra::Plugins::RadiumPluginInterface, RadiumPluginInterface_IID )
