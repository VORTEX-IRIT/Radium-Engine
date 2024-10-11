#pragma once
#include <Gui/MainWindowInterface.hpp>
#include <Gui/RaGui.hpp>
#include <Gui/TreeModel/EntityTreeModel.hpp>

namespace Ra {
namespace Gui {

/**
 * Definition of a minimal Radium Window.
 * This minimal window allows to display radium Engine entities.
 */

class RA_GUI_API SimpleWindow : public Ra::Gui::MainWindowInterface
{
    Q_OBJECT

  public:
    /** Constructors and destructor.
     *  https://en.cppreference.com/w/cpp/language/rule_of_three
     */
    /** \{ */
    explicit SimpleWindow( uint w = 800, uint h = 640, QWidget* parent = nullptr );
    SimpleWindow( const SimpleWindow& )            = delete;
    SimpleWindow& operator=( const SimpleWindow& ) = delete;
    SimpleWindow( SimpleWindow&& )                 = delete;
    SimpleWindow& operator=( SimpleWindow&& )      = delete;
    ~SimpleWindow() override;
    /**\}*/

    /// Access the viewer, i.e. the rendering widget.
    Ra::Gui::Viewer* getViewer() override;

    /// Access the selection manager.
    Ra::Gui::SelectionManager* getSelectionManager() override;

    /// Access the timeline.
    Ra::Gui::Timeline* getTimeline() override;

    /// Update the ui from the plugins loaded.
    void updateUi( Ra::Plugins::RadiumPluginInterface* plugin ) override;

    /// Update the UI ( most importantly gizmos ) to the modifications of the
    /// engine/
    void onFrameComplete() override;

    /// Add render in the application: UI, viewer - this method does not associate a control panel.
    void addRenderer( const std::string& name,
                      std::shared_ptr<Ra::Engine::Rendering::Renderer> e ) override;

  public slots:
    /**
     * Called when a scene is ready to display to parameterize the application window and the
     * viewer.
     */
    void prepareDisplay() override;

    /// Cleanup resources.
    void cleanup() override;

    // Display help dialog about Viewer key-bindings
    void displayHelpDialog() override;

  signals:
    /// Emitted when frame must be updated
    void frameUpdate();

  private:
    /// create the UI connections
    void createConnections();

    /// viewer widget
    std::unique_ptr<Ra::Gui::Viewer> m_viewer;

    /// Stores and manages the current selection.
    /// Even if no selection is provided by this application, used plugins require this
    std::unique_ptr<Ra::Gui::SelectionManager> m_selectionManager;

    /// Stores the internal model of engine objects for selection and visibility.
    std::unique_ptr<Ra::Gui::ItemModel> m_sceneModel;
};

} // namespace Gui
} // namespace Ra
