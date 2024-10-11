#pragma once
#include <QApplication>
#include <QTimer>

#include <Core/Tasks/TaskQueue.hpp>
#include <Gui/Viewer/Viewer.hpp>

/* This file contains a minimal radium/qt application which shows the
classic "Spinning Cube" demo. */

/// Our minimal application uses QTimer to be called at a regular frame rate.
class MinimalApp : public QApplication
{
    Q_OBJECT

  public:
    /** IMPORTANT : the argc parameter must be a reference for calling the QApplication constructor
        \see http://doc.qt.io/qt-5/qapplication.html#QApplication
    */
    MinimalApp( int& argc, char** argv );
    ~MinimalApp();

    void initialize();

  public slots:

    /// This function is the basic "game loop" iteration of the engine.
    /// It starts the rendering then advance all systems by one frame.
    void frame();
    void onGLInitialized();
    void onRequestEngineOpenGLInitialization();

  public:
    // pointer to engine singleton
    Ra::Engine::RadiumEngine* m_engine { nullptr };

    // Task queue
    std::unique_ptr<Ra::Core::TaskQueue> m_taskQueue { nullptr };

    // Pointer to Qt/OpenGL Viewer widget.
    std::unique_ptr<Ra::Gui::Viewer> m_viewer { nullptr };

    // Timer to wake us up at every frame start.
    QTimer* m_frameTimer { nullptr };

    // Our framerate
    uint m_targetFps { 60 };

    uint m_manipulatorIndex { 0 };

    void changeCameraManipulator();

}; // end class
