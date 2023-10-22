#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <QWidget>

#include <nodes/FlowScene>
#include <nodes/FlowView>

namespace Ra {
namespace Dataflow {
namespace Core {
class DataflowGraph;
class NodeFactorySet;
} // namespace Core

/**
 *
 */
namespace QtGui {
namespace GraphEditor {

using namespace Ra::Dataflow::Core;

class RA_DATAFLOW_API GraphEditorView : public QWidget
{
    Q_OBJECT
  public:
    explicit GraphEditorView( QWidget* parent );
    ~GraphEditorView();
    void disconnectAll();
    void connectAll();

    /// Fill the editor with the existing nodes in the graph
    void editGraph( std::shared_ptr<DataflowGraph> g );

    DataflowGraph* editedGraph();

  Q_SIGNALS:
    void needUpdate();

  protected:
    void resizeEvent( QResizeEvent* event ) override;

  private:
    std::vector<QMetaObject::Connection> connections;

    QtNodes::FlowScene* scene { nullptr };
    QtNodes::FlowView* view { nullptr };

    /// Build the registries from the graph's NodeFactory
    void buildAdapterRegistry( const NodeFactorySet& factory );

    std::shared_ptr<DataflowGraph> m_dataflowGraph { nullptr };

    /// Node creator registry to be used when creating a node in the editor
    std::shared_ptr<QtNodes::DataModelRegistry> m_editorRegistry;
};

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
