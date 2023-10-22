#include <Dataflow/QtGui/GraphEditor/GraphEditorView.hpp>

#include <QVBoxLayout>

#include <nodes/ConnectionStyle>
#include <nodes/Node>
#include <nodes/NodeData>

#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/QtGui/GraphEditor/NodeAdapterModel.hpp>
#include <Dataflow/QtGui/GraphEditor/WidgetFactory.hpp>
#include <utility>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {

GraphEditorView::GraphEditorView( QWidget* parent ) : QWidget( parent, Qt::Window ) {
    QtNodes::ConnectionStyle::setConnectionStyle(
        R"(
                {
                    "ConnectionStyle": {
                        "UseDataDefinedColors": true
                    }
                }
                )" );

    QVBoxLayout* l = new QVBoxLayout( this );

    scene = new QtNodes::FlowScene( l );
    view  = new QtNodes::FlowView( scene );

    l->addWidget( view );
    l->setContentsMargins( 0, 0, 0, 0 );
    l->setSpacing( 0 );

    // Create widgets
    WidgetFactory::initializeWidgetFactory();

    connectAll();
}

GraphEditorView::~GraphEditorView() {
    disconnectAll();
}

void GraphEditorView::disconnectAll() {
    for ( size_t i = 0; i < connections.size(); i++ ) {
        QObject::disconnect( connections[i] );
    }
    connections.clear();
}

void GraphEditorView::connectAll() {

    // This one is only here to allow modifying node parameters through the  embedded widget
    // TODO, find another way to do this.
    connections.push_back( QObject::connect(
        scene, &QtNodes::FlowScene::nodeHoverLeft, [this]() { emit needUpdate(); } ) );

    connections.push_back( QObject::connect(
        scene, &QtNodes::FlowScene::nodePlaced, [this]() { emit needUpdate(); } ) );
    connections.push_back( QObject::connect(
        scene, &QtNodes::FlowScene::nodeDeleted, [this]() { emit needUpdate(); } ) );
    connections.push_back( QObject::connect(
        scene, &QtNodes::FlowScene::connectionCreated, [this]() { emit needUpdate(); } ) );
    connections.push_back( QObject::connect(
        scene, &QtNodes::FlowScene::connectionDeleted, [this]() { emit needUpdate(); } ) );
    connections.push_back( QObject::connect(
        scene, &QtNodes::FlowScene::nodeMoved, []( QtNodes::Node& n, const QPointF& ) {
            QJsonObject obj;
            obj["x"] = n.nodeGraphicsObject().pos().x();
            obj["y"] = n.nodeGraphicsObject().pos().y();
            QJsonObject nodeJson;
            nodeJson["position"] = obj;
            n.nodeDataModel()->addMetaData( nodeJson );
        } ) );
}

void GraphEditorView::buildAdapterRegistry( const NodeFactorySet& factories ) {
    m_editorRegistry.reset( new QtNodes::DataModelRegistry );
    for ( const auto& [factoryName, factory] : factories ) {
        for ( const auto& [typeName, creator] : factory->getFactoryMap() ) {
            auto f              = creator.first;
            auto creatorFactory = factory;
            m_editorRegistry->registerModel(
                typeName.c_str(),
                [f, this, creatorFactory]() -> std::unique_ptr<QtNodes::NodeDataModel> {
                    nlohmann::json jsondata;
                    auto node = f( jsondata );
                    this->m_dataflowGraph->addNode( node );
                    this->m_dataflowGraph->addFactory( creatorFactory );
                    return std::make_unique<NodeAdapterModel>( this->m_dataflowGraph, node );
                },
                factoryName.c_str(),
                creator.second.c_str() );
        }
    }
    scene->setRegistry( m_editorRegistry );
}

DataflowGraph* GraphEditorView::editedGraph() {
    return m_dataflowGraph.get();
}

void GraphEditorView::editGraph( std::shared_ptr<DataflowGraph> g ) {
    // Disconnect all event and clear the previous graph
    disconnectAll();
    scene->clearScene();

    // Setup the graph to edit
    m_dataflowGraph = g;
    if ( m_dataflowGraph ) {
        buildAdapterRegistry( NodeFactoriesManager::getFactoryManager() );
        const auto& nodes = m_dataflowGraph->getNodes();
        // NodeToUuid mapping
        std::map<Node*, QString> nodeToUuid;
        // inserting nodes
        for ( const auto& n : nodes ) {
            auto nodeAdapter = std::make_unique<NodeAdapterModel>( m_dataflowGraph, n );
            nodeToUuid.insert( { n.get(), nodeAdapter->uuid() } );
            scene->importNode( std::move( nodeAdapter ) );
        }
        // inserting connections
        for ( const auto& n : nodes ) {
            int numPort = 0;
            for ( const auto& input : n->getInputs() ) {
                if ( input->isLinked() ) {
                    auto portOut     = input->getLink();
                    auto nodeOut     = portOut->getNode();
                    int outPortIndex = 0;
                    for ( const auto& p : nodeOut->getOutputs() ) {
                        if ( p.get() == portOut ) { break; }
                        outPortIndex++;
                    }
                    scene->importConnection(
                        nodeToUuid[nodeOut], outPortIndex, nodeToUuid[n.get()], numPort );
                }
                numPort++;
            }
        }
        scene->setSceneName( m_dataflowGraph->getInstanceName().c_str() );
    }
    else { scene->setSceneName( "untitled" ); }
    scene->iterateOverNodes( []( QtNodes::Node* n ) { n->onNodeSizeUpdated(); } );

    // view->fitInView( view->sceneRect(), Qt::KeepAspectRatio);
    // view->resetTransform();
    view->ensureVisible( view->sceneRect() );
    view->centerOn( view->sceneRect().center() );
    // Re-connect events
    connectAll();
}

// Find the way to see all the scene in the editor (or, at leas 75% of the scene)
void GraphEditorView::resizeEvent( QResizeEvent* ) {
    // view->resetTransform();
    view->ensureVisible( view->sceneRect() );
    view->centerOn( view->sceneRect().center() );
}

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
