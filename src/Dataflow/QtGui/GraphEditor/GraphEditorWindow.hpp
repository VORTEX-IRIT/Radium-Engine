#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <QMainWindow>
#include <QtWidgets>

#include <Dataflow/QtGui/GraphEditor/GraphEditorView.hpp>

#include <Dataflow/Core/DataflowGraph.hpp>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {

using namespace Ra::Dataflow::Core;

/**
 * \brief Window widget to edit a Node Graph.
 * This class just wrap a Ra::Dataflow::QtGui::GraphEditor::GraphEditorView in a main window with
 * several services :
 *   - The window can be used as a standalone editor if no parameter is given to the constructor.
 *   - The window can be used to edit an existing graph by giving the graph to the constructor.
 *
 *   connect the client to the needUpdate() signal to be notified of a change in the edited graph.
 */
class RA_DATAFLOW_API GraphEditorWindow : public QMainWindow
{
    Q_OBJECT
  public:
    explicit GraphEditorWindow( std::shared_ptr<DataflowGraph> graph = nullptr );
    ~GraphEditorWindow();

    void loadFile( const QString& fileName );
#if 0
    // not sure to neeed this
    void resetGraph( std::shared_ptr<DataflowGraph> graph = nullptr );
#endif

  signals:
    void needUpdate();

  protected:
    void closeEvent( QCloseEvent* event ) override;

  private slots:
    void newFile();
    void open();
    bool save();
    bool saveAs();
    void about();
    void documentWasModified();

  private:
    void createActions();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    bool maybeSave();
    bool saveFile( const QString& fileName );
    void setCurrentFile( const QString& fileName );
    QString strippedName( const QString& fullFileName );

    GraphEditorView* m_graphEdit { nullptr };
    QString m_curFile;

    std::shared_ptr<DataflowGraph> m_graph { nullptr };
};

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
