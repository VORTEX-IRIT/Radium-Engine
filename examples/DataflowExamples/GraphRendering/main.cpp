// Include Radium base application and its simple Gui

#include <Core/Asset/FileLoaderInterface.hpp>
#include <Core/Utils/Log.hpp>

#include <Engine/Rendering/ForwardRenderer.hpp>
#include <Engine/Scene/EntityManager.hpp>

#include <Dataflow/Rendering/Renderer/ControllableRenderer.hpp>

#include <Gui/BaseApplication.hpp>
#include <Gui/RadiumWindow/SimpleWindow.hpp>
#include <Gui/Viewer/Viewer.hpp>

// Qt Widgets
#include <QtWidgets>

/* ----------------------------------------------------------------------------------- */
using namespace Ra::Core::Utils;
using namespace Ra::Engine;
using namespace Ra::Gui;
using namespace Ra::Dataflow::Rendering::Renderer;
using namespace Ra::Dataflow::Rendering;

/**
 * Extending Ra::SimpleWindow with some menus for demonstration purpose
 */
class DemoWindowFactory : public BaseApplication::WindowFactory
{
    std::vector<std::shared_ptr<Rendering::Renderer>> m_renderers;

    static void addFileMenu( MainWindowInterface* window ) {
        // Add a menu to load a scene
        auto fileMenu       = window->menuBar()->addMenu( "&File" );
        auto fileOpenAction = new QAction( "&Open...", window );
        fileOpenAction->setShortcuts( QKeySequence::Open );
        fileOpenAction->setStatusTip( "Open a file." );
        fileMenu->addAction( fileOpenAction );

        // Connect the menu
        auto openFile = [window]() {
            QString filter;
            QString allexts;
            auto engine = RadiumEngine::getInstance();
            for ( const auto& loader : engine->getFileLoaders() ) {
                QString exts;
                for ( const auto& e : loader->getFileExtensions() ) {
                    exts.append( QString::fromStdString( e ) + " " );
                }
                allexts.append( exts + " " );
                filter.append( QString::fromStdString( loader->name() ) + " (" + exts + ");;" );
            }
            // add a filter concatenating all the supported extensions
            filter.prepend( "Supported files (" + allexts + ");;" );

            // remove the last ";;" of the string
            filter.remove( filter.size() - 2, 2 );

            QSettings settings;
            auto path     = settings.value( "files/load", QDir::homePath() ).toString();
            auto pathList = QFileDialog::getOpenFileNames( window, "Open Files", path, filter );

            if ( !pathList.empty() ) {
                engine->getEntityManager()->deleteEntities();
                settings.setValue( "files/load", pathList.front() );
                engine->loadFile( pathList.front().toStdString() );
                engine->releaseFile();
                window->prepareDisplay();
                emit window->getViewer()->needUpdate();
            }
        };
        QAction::connect( fileOpenAction, &QAction::triggered, openFile );

        // Add an exit entry
        auto exitAct = fileMenu->addAction( "E&xit", window, &QWidget::close );
        exitAct->setShortcuts( QKeySequence::Quit );
    }

    static void
    addRendererMenu( MainWindowInterface* window,
                     const std::vector<std::shared_ptr<Rendering::Renderer>>& renderers ) {
        auto renderMenu = window->menuBar()->addMenu( "&Renderer" );
        int renderNum   = 0;

        for ( const auto& rndr : renderers ) {
            window->addRenderer( rndr->getRendererName(), rndr );
            auto rndAct = new QAction( rndr->getRendererName().c_str(), window );
            renderMenu->addAction( rndAct );
            QAction::connect( rndAct, &QAction::triggered, [renderNum, window]() {
                window->getViewer()->changeRenderer( renderNum );
                window->getViewer()->needUpdate();
            } );
            ++renderNum;
        }
    }

  public:
    explicit DemoWindowFactory(
        const std::vector<std::shared_ptr<Rendering::Renderer>>& renderers ) :
        m_renderers( renderers ) {}

    inline Ra::Gui::MainWindowInterface* createMainWindow() const override {
        auto window = new SimpleWindow();
        addFileMenu( window );
        addRendererMenu( window, m_renderers );
        return window;
    }
};
/* ----------------------------------------------------------------------------------- */
// Renderer controller
#include <Dataflow/Rendering/Nodes/RenderNodes/ClearColorNode.hpp>
#include <Dataflow/Rendering/Nodes/Sinks/DisplaySinkNode.hpp>
#include <Dataflow/Rendering/Nodes/Sources/Scene.hpp>
#include <Dataflow/Rendering/Nodes/Sources/TextureSourceNode.hpp>

class MyRendererController : public RenderGraphController
{
  private:
    static void inspectGraph( DataflowGraph& g ) {
        // Factories used by the graph
        auto factories = g.getNodeFactories();
        std::cout << "Used factories by the graph \"" << g.getInstanceName() << "\" with type \""
                  << g.getTypeName() << "\" :\n";
        for ( const auto& f : *( factories.get() ) ) {
            std::cout << "\t" << f.first << "\n";
        }

        // Nodes of the graph
        auto nodes = g.getNodes();
        std::cout << "Nodes of the graph " << g.getInstanceName() << " (" << nodes->size()
                  << ") :\n";
        for ( const auto& n : *( nodes ) ) {
            std::cout << "\t\"" << n->getInstanceName() << "\" of type \"" << n->getTypeName()
                      << "\"\n";
            // Inspect input, output and interfaces of the node
            std::cout << "\t\tInput ports :\n";
            for ( const auto& p : n->getInputs() ) {
                std::cout << "\t\t\t\"" << p->getName() << "\" with type " << p->getTypeName()
                          << "\n";
            }
            std::cout << "\t\tOutput ports :\n";
            for ( const auto& p : n->getOutputs() ) {
                std::cout << "\t\t\t\"" << p->getName() << "\" with type " << p->getTypeName()
                          << "\n";
            }
            std::cout << "\t\tInterface ports :\n";
            for ( const auto& p : n->getInterfaces() ) {
                std::cout << "\t\t\t\"" << p->getName() << "\" with type " << p->getTypeName()
                          << "\n";
            }
        }

        // Nodes by level after the compilation
        auto c  = g.compile();
        auto cn = g.getNodesByLevel();
        std::cout << "Nodes of the graph, sorted by level when compiling the graph :\n";
        for ( size_t i = 0; i < cn->size(); ++i ) {
            std::cout << "\tLevel " << i << " :\n";
            for ( const auto n : ( *cn )[i] ) {
                std::cout << "\t\t\"" << n->getInstanceName() << "\"\n";
            }
        }

        // describe the graph interface : inputs and outputs port of the whole graph (not of the
        // nodes)
        std::cout << "Inputs and output nodes of the graph " << g.getInstanceName() << " :\n";
        auto inputs = g.getAllDataSetters();
        std::cout << "\tInput ports (" << inputs.size() << ") are :\n";
        for ( auto& [ptrPort, portName, portType] : inputs ) {
            std::cout << "\t\t\"" << portName << "\" accepting type \"" << portType << "\"\n";
        }
        auto outputs = g.getAllDataGetters();
        std::cout << "\tOutput ports (" << outputs.size() << ") are :\n";
        for ( auto& [ptrPort, portName, portType] : outputs ) {
            std::cout << "\t\t\"" << portName << "\" generating type \"" << portType << "\"\n";
        }
    }

  public:
    using RenderGraphController::RenderGraphController;

    /// Configuration function.
    /// Called once at the configuration of the renderer
    /// If a graph should be loaded at configure time, it was set on the controller using
    /// deferredLoadGraph(...) before configuring the renderer.
    void configure( ControllableRenderer* renderer, int w, int h ) override {
        LOG( logINFO ) << "MyRendererController::configure";
        RenderGraphController::configure( renderer, w, h );
        if ( m_renderGraph == nullptr ) {
            // a graph was not given on the command line, build a simple one
            m_renderGraph = std::make_unique<RenderingGraph>( "Demonstration graph" );
            m_renderGraph->setShaderProgramManager( m_shaderMngr );
            auto sceneNode = new SceneNode( "Scene" );
            m_renderGraph->addNode( sceneNode );
            auto resultNode = new DisplaySinkNode( "Images" );
            m_renderGraph->addNode( resultNode );
            auto textureSource = new ColorTextureNode( "Beauty" );
            m_renderGraph->addNode( textureSource );
            auto clearNode = new ClearColorNode( " Clear" );
            m_renderGraph->addNode( clearNode );

            bool linksOK = true;
            linksOK      = m_renderGraph->addLink(
                textureSource, "texture", clearNode, "colorTextureToClear" );
            linksOK = linksOK && m_renderGraph->addLink( clearNode, "image", resultNode, "Beauty" );

            inspectGraph( *m_renderGraph );

            // force recompilation and introspection of the graph by the renderer
            m_renderGraph->m_ready = false;
            notify();
        }
    };

#if 0
    /// Resize function
    /// Called each time the renderer is resized
    void resize( int w, int h ) override {
        LOG( logINFO ) << "MyRendererController::resize";
        RenderGraphController::resize( w, h );
    };

    /// Update function
    /// Called once before each frame to update the internal state of the renderer
    void update( const Ra::Engine::Data::ViewingParameters& renderData ) override {
        LOG( logINFO ) << "MyRendererController::update";
        RenderGraphController::update( renderData );
    };
#endif
    [[nodiscard]] std::string getRendererName() const override { return "Custom Node Renderer"; }
};

/* ----------------------------------------------------------------------------------- */

/**
 * main function.
 */
int main( int argc, char* argv[] ) {

    //! [Instatiating the application]
    BaseApplication app( argc, argv );
    //! [Instatiating the application]

    //! getting graph argument on the command line
    std::optional<std::string> graphOption { std::nullopt };
    QCommandLineParser parser;
    QCommandLineOption graphOpt(
        { "g", "graph", "nodes" }, "Open a node <graph> at startup.", "graph", "" );
    parser.addOptions( { graphOpt } );
    if ( !parser.parse( app.arguments() ) ) {
        LOG( Ra::Core::Utils::logWARNING )
            << "GraphDemo : Command line parsing failed due to unsupported or "
               "missing options : \n\t"
            << parser.errorText().toStdString();
    }
    if ( parser.isSet( graphOpt ) ) {
        graphOption = parser.value( graphOpt ).toStdString();
        std::cout << "Got a graph option : " << *graphOption << std::endl;
    }
    else {
        std::cout << "No graph option" << std::endl;
    }
    //! getting graph argument on the command line

    //! [Initializing the application]
    std::vector<std::shared_ptr<Rendering::Renderer>> renderers;
    renderers.emplace_back( new Rendering::ForwardRenderer );

    MyRendererController graphController;
    if ( graphOption ) { graphController.deferredLoadGraph( *graphOption ); }
    renderers.emplace_back( new ControllableRenderer( graphController ) );

    app.initialize( DemoWindowFactory( renderers ) );
    app.setContinuousUpdate( false );
    app.addRadiumMenu();
    //! [Initializing the application]

    return app.exec();
}
