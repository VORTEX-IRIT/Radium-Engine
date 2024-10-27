#include "MainWindow.hpp"
#include "BezierUtils/CubicBezierApproximation.hpp"
#include "PointFactory.hpp"
#include <Core/Geometry/Curve2D.hpp>
#include <Core/Geometry/RayCast.hpp>
#include <Core/Math/Interpolation.hpp>
#include <Core/Utils/Log.hpp>
#include <CurveFactory.hpp>
#include <Engine/Data/Material.hpp>
#include <Engine/Data/Mesh.hpp>
#include <Engine/Data/ViewingParameters.hpp>
#include <Engine/Rendering/ForwardRenderer.hpp>
#include <Engine/Rendering/RenderObject.hpp>
#include <Engine/Rendering/RenderObjectManager.hpp>
#include <Gui/SelectionManager/SelectionManager.hpp>
#include <Gui/TreeModel/EntityTreeModel.hpp>
#include <Gui/Utils/PickingManager.hpp>
#include <Gui/Viewer/CameraManipulator.hpp>
#include <Gui/Viewer/Viewer.hpp>
#include <QVBoxLayout>

using namespace Ra::Gui;
using namespace Ra::Engine;
using namespace Ra::Engine::Rendering;
using namespace Ra::Core::Utils;

MainWindow::MainWindow( QWidget* parent ) : MainWindowInterface( parent ) {
    if ( objectName().isEmpty() ) setObjectName( QString::fromUtf8( "RadiumSimpleWindow" ) );

    // Initialize the minimum tools for a Radium-Guibased Application
    m_viewer = new MyViewer();
    m_viewer->setObjectName( QStringLiteral( "m_viewer" ) );
    m_viewer->setBackgroundColor( Color::White() );
    m_engine = Ra::Engine::RadiumEngine::getInstance();

    // Initialize the scene interactive representation
    m_sceneModel       = new Ra::Gui::ItemModel( Ra::Engine::RadiumEngine::getInstance(), this );
    m_selectionManager = new Ra::Gui::SelectionManager( m_sceneModel, this );

    // initialize Gui for the application
    auto viewerWidget = QWidget::createWindowContainer( m_viewer );
    viewerWidget->setAutoFillBackground( false );
    setCentralWidget( viewerWidget );
    setWindowTitle( QString( "Radium player" ) );
    setMinimumSize( 800, 600 );

    // Dock widget
    QDockWidget* dockWidget = new QDockWidget( "Dock", this );
    QWidget* myWidget       = new QWidget();
    QVBoxLayout* layout     = new QVBoxLayout();
    m_editCurveButton       = new QPushButton( "Edit polyline" );
    m_button                = new QPushButton( "smooth" );
    m_button->setCheckable( true );
    m_hidePolylineButton = new QPushButton( "Hide initial polyline" );
    m_hidePolylineButton->setCheckable( true );
    m_symetryButton = new QPushButton( "Symetry" );
    m_symetryButton->setCheckable( true );
    m_symetryButton->setEnabled( false );
    m_button->setEnabled( false );
    layout->addWidget( m_hidePolylineButton );
    layout->addWidget( m_editCurveButton );
    layout->addWidget( m_button );
    layout->addWidget( m_symetryButton );
    myWidget->setLayout( layout );
    dockWidget->setWidget( myWidget );
    dockWidget->setAllowedAreas( Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea );
    addDockWidget( Qt::LeftDockWidgetArea, dockWidget );
    m_dockWidget = dockWidget;
    createConnections();
}

MainWindow::~MainWindow() = default;

MyViewer* MainWindow::getViewer() {
    return m_viewer;
}

Ra::Gui::SelectionManager* MainWindow::getSelectionManager() {
    return m_selectionManager;
}

Ra::Gui::Timeline* MainWindow::getTimeline() {
    return nullptr;
}

void MainWindow::updateUi( Ra::Plugins::RadiumPluginInterface* plugin ) {
    QString name;
    if ( plugin->doAddWidget( name ) ) { m_dockWidget->setWidget( plugin->getWidget() ); }
}

void MainWindow::onFrameComplete() {}

void MainWindow::addRenderer( const std::string&,
                              std::shared_ptr<Ra::Engine::Rendering::Renderer> e ) {
    e->enableDebugDraw( false );
    m_viewer->addRenderer( e );
}
void MainWindow::prepareDisplay() {
    m_selectionManager->clear();
    if ( m_viewer->prepareDisplay() ) { emit frameUpdate(); }
    m_viewer->getRenderer()->toggleDrawDebug();
}

void MainWindow::cleanup() {
    m_viewer = nullptr;
}

void MainWindow::createConnections() {
    connect( m_viewer, &MyViewer::toggleBrushPicking, this, &MainWindow::toggleCirclePicking );
    connect( m_viewer, &MyViewer::onMouseMove, this, &MainWindow::handleMouseMoveEvent );
    connect( m_viewer, &MyViewer::onMouseRelease, this, &MainWindow::handleMouseReleaseEvent );
    connect( m_viewer, &MyViewer::onMousePress, this, &MainWindow::handleMousePressEvent );
    connect(
        m_editCurveButton, &QPushButton::pressed, this, &MainWindow::onEditPolylineButtonPressed );
    connect( m_hidePolylineButton,
             &QPushButton::clicked,
             this,
             &MainWindow::onHidePolylineButtonClicked );
    connect( m_button, &QPushButton::clicked, this, &MainWindow::onSmoothButtonClicked );
    connect( m_symetryButton, &QPushButton::clicked, this, &MainWindow::onSymetryButtonClicked );
    connect(
        m_viewer, &MyViewer::onMouseDoubleClick, this, &MainWindow::handleMouseDoubleClickEvent );
}

void MainWindow::onSmoothButtonClicked() {
    m_curveEditor->setSmooth( m_button->isChecked() );
    m_symetryButton->setEnabled( true );
}

void MainWindow::onSymetryButtonClicked() {
    m_curveEditor->setSymetry( m_symetryButton->isChecked() );
}

void MainWindow::onHidePolylineButtonClicked() {
    auto roMgr = Ra::Engine::RadiumEngine::getInstance()->getRenderObjectManager();
    auto ro =
        roMgr->getRenderObject( m_initialPolyline->getComponents()[0]->getRenderObjects()[0] );
    if ( m_hidePolylineButton->isChecked() ) { ro->setVisible( false ); }
    else
        ro->setVisible( true );
    m_viewer->needUpdate();
}

void MainWindow::onEditPolylineButtonPressed() {
    if ( m_polyline.empty() || m_edited ) return;
    m_edited      = true;
    m_curveEditor = new CurveEditor( m_polyline, m_viewer );
}

void MainWindow::processSavedPoint() {
    auto savedPoint = m_curveEditor->getSavedPoint();
    if ( savedPoint ) {
        if ( savedPoint->m_state == PointComponent::DEFAULT ) {
            m_button->setChecked( false );
            m_button->setEnabled( true );
            m_symetryButton->setEnabled( false );
            m_symetryButton->setChecked( false );
        }
        else if ( savedPoint->m_state == PointComponent::SMOOTH ) {
            m_button->setEnabled( true );
            m_button->setChecked( true );
            m_symetryButton->setEnabled( true );
            m_symetryButton->setChecked( false );
        }
        else {
            m_button->setEnabled( true );
            m_button->setChecked( true );
            m_symetryButton->setEnabled( true );
            m_symetryButton->setChecked( true );
        }
    }
    else {
        m_button->setEnabled( false );
        m_button->setChecked( false );
        m_symetryButton->setEnabled( false );
        m_symetryButton->setChecked( false );
    }
}

void MainWindow::handleMousePressEvent( QMouseEvent* event ) {
    if ( m_edited ) {
        if ( event->button() == Qt::RightButton ) m_clicked = true;
        processSavedPoint();
    }
}

void MainWindow::displayHelpDialog() {
    m_viewer->displayHelpDialog();
}

void MainWindow::toggleCirclePicking( bool on ) {
    m_isTracking = on;
    centralWidget()->setMouseTracking( on );
}

Ra::Core::Vector3 MainWindow::getWorldPos( int x, int y ) {
    const auto r = m_viewer->getCameraManipulator()->getCamera()->getRayFromScreen( { x, y } );
    std::vector<Scalar> t;
    // {0,0,0} a point and {0,1,0} the normal
    if ( Ra::Core::Geometry::RayCastPlane( r, { 0, 0, 0 }, { 0, 1, 0 }, t ) ) {
        // vsPlane return at most one intersection t
        return r.pointAt( t[0] );
    }
    return Ra::Core::Vector3();
}

void MainWindow::handleMouseReleaseEvent( QMouseEvent* event ) {
    m_clicked = false;
    if ( m_curveEditor == nullptr ) return;
    if ( m_curveEditor->getSelected() && event->button() == Qt::RightButton ) {
        m_curveEditor->updateCurves( true );
        m_curveEditor->resetSelected();
        m_hovering = false;
    }
}

void MainWindow::handleMouseDoubleClickEvent( QMouseEvent* event ) {
    if ( m_curveEditor == nullptr ) return;
    auto worldPos = getWorldPos( event->x(), event->y() );

    auto modifiers = event->modifiers();

    m_viewer->makeCurrent();
    if ( modifiers == Qt::ShiftModifier ) {
        m_curveEditor->addPointAtEnd( worldPos );
        m_viewer->doneCurrent();
        m_viewer->getRenderer()->buildAllRenderTechniques();
        m_viewer->needUpdate();
        return;
    }

    m_curveEditor->addPointInCurve( worldPos, event->pos().x(), event->pos().y() );

    m_viewer->doneCurrent();
    m_viewer->getRenderer()->buildAllRenderTechniques();
    m_viewer->needUpdate();
}

void MainWindow::handleMouseMoveEvent( QMouseEvent* event ) {
    if ( m_curveEditor == nullptr ) return;
    int mouseX = event->pos().x();
    int mouseY = event->pos().y();
    handleHover( mouseX, mouseY );
    handlePicking( mouseX, mouseY );
}

void MainWindow::handleHover( int mouseX, int mouseY ) {
    m_viewer->makeCurrent();
    auto camera = m_viewer->getCameraManipulator()->getCamera();
    Ra::Engine::Rendering::Renderer::PickingQuery query {
        Ra::Core::Vector2( mouseX, height() - mouseY ),
        Ra::Engine::Rendering::Renderer::MANIPULATION,
        Ra::Engine::Rendering::Renderer::RO };
    Ra::Engine::Data::ViewingParameters renderData {
        camera->getViewMatrix(), camera->getProjMatrix(), 0 };

    auto pres = m_viewer->getRenderer()->doPickingNow( query, renderData );
    m_viewer->doneCurrent();

    if ( pres.getRoIdx() >= 0 && ( !m_hovering ) ) {
        auto ro    = m_engine->getRenderObjectManager()->getRenderObject( pres.getRoIdx() );
        m_hovering = m_curveEditor->processHover( ro );
    }
    else if ( m_hovering && !( m_clicked ) ) {
        m_curveEditor->processUnhovering();
        m_hovering = false;
    }
}

void MainWindow::handlePicking( int mouseX, int mouseY ) {
    if ( m_curveEditor->getSelected() != nullptr && m_clicked ) {
        auto worldPos = getWorldPos( mouseX, mouseY );
        m_curveEditor->processPicking( worldPos );
    }
}
