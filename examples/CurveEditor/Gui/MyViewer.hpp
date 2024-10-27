#pragma once

#include <Gui/Viewer/Viewer.hpp>

class MyViewer : public Ra::Gui::Viewer
{
    Q_OBJECT;

  public:
    MyViewer();
    virtual ~MyViewer();

  signals:
    void onMouseDoubleClick( QMouseEvent* event );

  public slots:
    void mouseDoubleClickEvent( QMouseEvent* event ) override;

  private:
    /* data */
};
