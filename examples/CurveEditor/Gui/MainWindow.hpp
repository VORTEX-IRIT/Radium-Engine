#pragma once

#include <Core/Geometry/Curve2D.hpp>

#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/Renderer.hpp>

#include "CurveEditor.hpp"
#include "MyViewer.hpp"
#include "PointComponent.hpp"
#include <Gui/MainWindowInterface.hpp>
#include <Gui/RaGui.hpp>
#include <Gui/TreeModel/EntityTreeModel.hpp>
#include <PluginBase/RadiumPluginInterface.hpp>
#include <QDockWidget>
#include <QItemSelection>
#include <QKeyEvent>
#include <QMainWindow>
#include <QPushButton>

/// This class manages most of the GUI of the application :
/// top menu, side toolbar and side dock.
class MainWindow : public Ra::Gui::MainWindowInterface
{
    Q_OBJECT
  public:
    /// Constructor and destructor.
    explicit MainWindow( QWidget* parent = nullptr );
    virtual ~MainWindow() override;
    MyViewer* getViewer() override;
    Ra::Gui::SelectionManager* getSelectionManager() override;
    Ra::Gui::Timeline* getTimeline() override;
    void updateUi( Ra::Plugins::RadiumPluginInterface* plugin ) override;
    void onFrameComplete() override;
    void addRenderer( const std::string& name,
                      std::shared_ptr<Ra::Engine::Rendering::Renderer> e ) override;
    void toggleCirclePicking( bool on );
    Ra::Core::Vector3 getWorldPos( int x, int y );
    void handleHover( int mouseX, int mouseY );
    void handlePicking( int mouseX, int mouseY );
    void setPolyline( Ra::Core::VectorArray<Ra::Core::Geometry::Curve2D::Vector> polyline ) {
        m_polyline = polyline;
    }
    void setInitialPolyline( Ra::Engine::Scene::Entity* e ) { m_initialPolyline = e; }

  public slots:
    void prepareDisplay() override;
    void cleanup() override;
    // Display help dialog about Viewer key-bindings
    void displayHelpDialog() override;
    void handleMouseMoveEvent( QMouseEvent* event );
    void handleMouseReleaseEvent( QMouseEvent* event );
    void handleMousePressEvent( QMouseEvent* event );
    void handleMouseDoubleClickEvent( QMouseEvent* event );
    void onEditPolylineButtonPressed();
    void onHidePolylineButtonClicked();
    void onSmoothButtonClicked();
    void onSymetryButtonClicked();
  signals:
    void frameUpdate();

  private:
    void createConnections();

    void processSavedPoint();

    MyViewer* m_viewer;
    Ra::Gui::SelectionManager* m_selectionManager;
    Ra::Gui::ItemModel* m_sceneModel;
    Ra::Engine::RadiumEngine* m_engine;
    QDockWidget* m_dockWidget;
    QPushButton* m_button;
    QPushButton* m_editCurveButton;
    QPushButton* m_hidePolylineButton;
    QPushButton* m_symetryButton;
    bool m_clicked = false;
    bool m_isTracking { false };
    bool m_hovering { false };
    bool m_edited { false };
    Ra::Core::VectorArray<Ra::Core::Geometry::Curve2D::Vector> m_polyline;
    Ra::Engine::Scene::Entity* m_initialPolyline;

    CurveEditor* m_curveEditor { nullptr };
};
