// Include Radium base application and its simple Gui
#include <Gui/BaseApplication.hpp>
#include <Gui/Viewer/Viewer.hpp>

// include the Engine/entity/component interface
#include <Core/Geometry/MeshPrimitives.hpp>
#include <Engine/Scene/EntityManager.hpp>
#include <Engine/Scene/GeometryComponent.hpp>
#include <Engine/Scene/GeometrySystem.hpp>

// To access render objects
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/RenderObjectManager.hpp>

// To access picking results
#include <Engine/Rendering/ForwardRenderer.hpp>
#include <Gui/RadiumWindow/SimpleWindowFactory.hpp>

// Shader
#include <Engine/Data/ShaderConfigFactory.hpp>

// Gui tools
#include <Gui/ParameterSetEditor/MaterialParameterEditor.hpp>

// radium file loading
#include <Core/Asset/FileLoaderInterface.hpp>

#include <memory>

// Qt Widgets
#include <QAction>
#include <QDockWidget>
#include <QFileDialog>
#include <QMenu>
#include <QMenuBar>
#include <QSettings>

#ifdef USE_RADIUMGLTF
#    include <Engine/Scene/SystemDisplay.hpp>
#    include <IO/Gltf/Loader/glTFFileLoader.hpp>
#    include <IO/Gltf/Writer/glTFFileWriter.hpp>
#    include <glTFLibrary.hpp>
#endif

using namespace Ra::Gui::Widgets;

/* ----------------------------------------------------------------------------------- */
/**
 * SimpleWindow for demonstration
 */
class DemoWindow : public Ra::Gui::SimpleWindow
{
  public:
    DemoWindow() : Ra::Gui::SimpleWindow() {

        // Build the demo interface (menus ...)
        auto menuBar        = new QMenuBar( this );
        auto fileMenu       = menuBar->addMenu( "&File" );
        auto fileOpenAction = new QAction( "&Open..." );
        fileOpenAction->setShortcuts( QKeySequence::Open );
        fileOpenAction->setStatusTip( "Open a file." );
        fileMenu->addAction( fileOpenAction );

        // Connect the menu
        auto openFile = [this]() {
            QString filter;
            QString allexts;
            auto engine = Ra::Engine::RadiumEngine::getInstance();
            for ( const auto& loader : engine->getFileLoaders() ) {
                QString exts;
                for ( const auto& e : loader->getFileExtensions() ) {
                    exts.append( QString::fromStdString( e ) + " " );
                }
                allexts.append( exts + " " );
                filter.append( QString::fromStdString( loader->name() ) + " (" + exts + ");;" );
            }
            // add a filter concetenatting all the supported extensions
            filter.prepend( "Supported files (" + allexts + ");;" );

            // remove the last ";;" of the string
            filter.remove( filter.size() - 2, 2 );

            QSettings settings;
            auto path     = settings.value( "files/load", QDir::homePath() ).toString();
            auto pathList = QFileDialog::getOpenFileNames( this, "Open Files", path, filter );

            if ( !pathList.empty() ) {
                Ra::Engine::RadiumEngine::getInstance()->getEntityManager()->deleteEntities();
                settings.setValue( "files/load", pathList.front() );
                engine->loadFile( pathList.front().toStdString() );
                engine->releaseFile();
                this->prepareDisplay();
                emit this->getViewer()->needUpdate();
            }
        };
        connect( fileOpenAction, &QAction::triggered, openFile );

#ifdef USE_RADIUMGLTF
        // register the gltf loader
        std::shared_ptr<Ra::Core::Asset::FileLoaderInterface> loader =
            std::make_shared<GLTF::glTFFileLoader>();
        Ra::Engine::RadiumEngine::getInstance()->registerFileLoader( loader );

        // allow to save in gltf format
        auto fileSaveAction = new QAction( "&Save..." );
        fileSaveAction->setShortcuts( QKeySequence::Save );
        fileSaveAction->setStatusTip( "Save as GLTF." );
        fileMenu->addAction( fileSaveAction );

        auto saveFile = [this]() {
            QString fileName = QFileDialog::getSaveFileName(
                this, tr( "Save as ..." ), "", tr( "GLTF 2.0 (*.gltf)" ) );
            std::vector<Ra::Engine::Scene::Entity*> toExport;
            auto entityManager = Ra::Engine::RadiumEngine::getInstance()->getEntityManager();
            auto entities      = entityManager->getEntities();
            std::copy_if(
                entities.begin(), entities.end(), std::back_inserter( toExport ), []( auto e ) {
                    return e != Ra::Engine::Scene::SystemEntity::getInstance();
                } );
            GLTF::glTFFileWriter writer { fileName.toStdString(), "textures/", false };
            writer.write( toExport );
        };
        connect( fileSaveAction, &QAction::triggered, saveFile );
#endif

        // add the menu bar
        setMenuBar( menuBar );

        // add the dock with a MaterialParameterEditor
        m_dock = new QDockWidget( "Material editor" );
        m_dock->setFeatures( QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable );

        m_matParamsEditor = new Ra::Gui::MaterialParameterEditor;

        // Update the viewer whenever a parameter gets modified
        auto on_materialParametersModified = [this]( const std::string& nm ) {
            std::cout << "Edited parameter : " << nm << "\n";
            if ( m_editedRo ) {
                // manage transparency status changes
                // TODO this should be done automatically by Material/renderTechnique update
                // And this should be simplified by removing duplicates in the storage of material
                // parameters
                // --> store only material data in the RenderParameter ?s
                m_editedRo->getMaterial()->updateFromParameters();
                m_editedRo->setTransparent( m_editedRo->getMaterial()->isTransparent() );
            }

            getViewer()->needUpdate();
        };
        connect( m_matParamsEditor,
                 &Ra::Gui::MaterialParameterEditor::materialParametersModified,
                 on_materialParametersModified );

        m_dock->setWidget( m_matParamsEditor );
        addDockWidget( Qt::LeftDockWidgetArea, m_dock );

        connect(
            getViewer(), &Ra::Gui::Viewer::rightClickPicking, this, &DemoWindow::handlePicking );
    }

    // Handle the picking signal generated by the viewer
    void handlePicking( const Ra::Engine::Rendering::Renderer::PickingResult& pickingResult ) {
        //! [processing the picking info]
        if ( pickingResult.getRoIdx().isValid() ) {
            auto roManager      = Ra::Engine::RadiumEngine::getInstance()->getRenderObjectManager();
            m_editedRo          = roManager->getRenderObject( pickingResult.getRoIdx() );
            auto pickedMaterial = m_editedRo->getMaterial();
            m_matParamsEditor->setupFromMaterial( pickedMaterial );
        }
    }

  private:
    QDockWidget* m_dock { nullptr };
    Ra::Gui::MaterialParameterEditor* m_matParamsEditor;
    std::shared_ptr<Ra::Engine::Rendering::RenderObject> m_editedRo { nullptr };
};

class DemoWindowFactory : public Ra::Gui::BaseApplication::WindowFactory
{
  public:
    inline Ra::Gui::MainWindowInterface* createMainWindow() const override {
        auto window = new DemoWindow();
        return window;
    }
};
/* ----------------------------------------------------------------------------------- */

/**
 * main function.
 */
int main( int argc, char* argv[] ) {

    //! [Instatiating the application]
    Ra::Gui::BaseApplication app( argc, argv );
    //! [Instatiating the application]

    //! [Initializing the application]
    app.initialize( DemoWindowFactory() );
#ifdef USE_RADIUMGLTF
    app.m_mainWindow->getViewer()->makeCurrent();
    // initialize the use of GLTF library
    GLTF::initializeGltf();
    app.m_mainWindow->getViewer()->doneCurrent();
#endif
    app.setContinuousUpdate( false );
    //! [Initializing the application]

    return app.exec();
}
