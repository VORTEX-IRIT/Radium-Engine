#include "MyViewer.hpp"

MyViewer::MyViewer() : Ra::Gui::Viewer() {}

void MyViewer::mouseDoubleClickEvent( QMouseEvent* event ) {
    emit onMouseDoubleClick( event );
}

MyViewer::~MyViewer() {}
