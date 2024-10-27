#include <Engine/RaEngine.hpp>

#include <QApplication>
#include <QString>

#include <QOpenGLContext>

#include <Core/Geometry/Curve2D.hpp>

#include <Engine/Scene/EntityManager.hpp>
#include <Gui/BaseApplication.hpp>
#include <Gui/Viewer/RotateAroundCameraManipulator.hpp>
#include <Gui/Viewer/Viewer.hpp>

#include <Core/Resources/Resources.hpp>

#include "Gui/MainWindow.hpp"
#include "PolylineFactory.hpp"

class MainWindowFactory : public Ra::Gui::BaseApplication::WindowFactory
{
  public:
    using Ra::Gui::BaseApplication::WindowFactory::WindowFactory;
    Ra::Gui::MainWindowInterface* createMainWindow() const { return new MainWindow(); }
};

int main( int argc, char* argv[] ) {

    // Create app and show viewer window
    Ra::Gui::BaseApplication app( argc, argv );
    app.initialize( MainWindowFactory() );
    app.setContinuousUpdate( false );

    // Create polyline from polyline points
    Ra::Core::VectorArray<Ra::Core::Geometry::Curve2D::Vector> polyline = {
        { -7.07583, -7.77924 }, { -6.4575, -7.69091 },   { -5.35333, -7.33757 },
        { -3.27749, -6.54257 }, { -1.68749, -5.39423 },  { -0.671654, -4.20173 },
        { 0.123348, -3.18589 }, { 0.697517, -1.94923 },  { 1.44835, -0.270889 },
        { 1.84585, 1.14245 },   { 1.93419, 3.04162 },    { 1.84585, 4.36662 },
        { 1.36002, 5.82412 },   { 0.874184, 7.10496 },   { 0.167517, 8.2533 },
        { -0.759985, 9.3133 },  { -1.37832, 9.93163 },   { -1.68749, 10.0641 },
        { -2.21749, 10.1083 },  { -2.70333, 9.75497 },   { -3.23333, 9.00413 },
        { -3.49832, 8.16496 },  { -3.71916, 6.70746 },   { -3.71916, 5.38246 },
        { -3.54249, 3.96912 },  { -3.32166, 2.64412 },   { -2.74749, 0.612446 },
        { -1.73166, -1.81673 }, { -0.804153, -3.93673 }, { 0.0350161, -5.7034 },
        { 0.830018, -6.94007 }, { 1.36002, -7.47007 },   { 2.02252, -7.86757 },
        { 3.03835, -8.26508 },  { 3.70086, -8.39758 },   { 4.14253, -8.30924 },
        { 4.80503, -8.00007 } };
    auto window = static_cast<MainWindow*>( app.m_mainWindow.get() );
    window->setPolyline( polyline );

    auto initialPolyline = PolylineFactory::createCurveEntity( polyline, "Initial polyline" );
    window->setInitialPolyline( initialPolyline );

    app.m_mainWindow->prepareDisplay();

    app.m_mainWindow->getViewer()->setCameraManipulator( new Ra::Gui::RotateAroundCameraManipulator(
        *( app.m_mainWindow->getViewer()->getCameraManipulator() ),
        app.m_mainWindow->getViewer() ) );

    return app.exec();
}
