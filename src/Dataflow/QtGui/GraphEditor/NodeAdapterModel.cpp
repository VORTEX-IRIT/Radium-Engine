#include <Dataflow/QtGui/GraphEditor/NodeAdapterModel.hpp>

#include <Dataflow/QtGui/GraphEditor/WidgetFactory.hpp>

#include <Gui/Widgets/ControlPanel.hpp>

#include <QColorDialog>
#include <QJsonArray>
#include <QObject>

#include <filesystem>

namespace Ra {
namespace Dataflow {
namespace QtGui {
namespace GraphEditor {

namespace NodeDataModelTools {
/**
 * Construct the widget needed to edit all the EditableParameter of the given node
 * @param node The node to decorate
 * @return the widget (of type RadiumAddons::Gui::Widgets::ControlPanel) exposing the parameters
 */
QWidget* getWidget( Node* node );

/**
 * Update the state of the widget to reflect the ones of the node.
 * @param node
 * @param widget
 */
void updateWidget( Node* node, QWidget* widget );

/**
 * Convert a QJsonObject to a nlohmann::json
 */
void QJsonObjectToNlohmannObject( const QJsonObject& p, nlohmann::json& data );

/**
 * Convert a nlohmann::json to a QJsonObject
 */
void NlohmannObjectToQJsonObject( const nlohmann::json& data, QJsonObject& p );
} // namespace NodeDataModelTools

using namespace Ra::Dataflow::Core;
using namespace Ra::Gui::Widgets;

NodeAdapterModel::NodeAdapterModel( std::shared_ptr<DataflowGraph> graph,
                                    std::shared_ptr<Node> n ) :
    m_node { n }, m_dataflowGraph { graph } {
    m_uuid = QUuid::createUuid();
    m_inputsConnected.resize( m_node->getInputs().size() );
    m_widget = NodeDataModelTools::getWidget( m_node.get() );
    NodeDataModelTools::updateWidget( m_node.get(), m_widget );
    checkConnections();
}

void NodeAdapterModel::checkConnections() const {
    int errors = 0;
    for ( size_t i = 0; i < m_inputsConnected.size(); i++ ) {
        if ( !m_inputsConnected[i] && m_node->getInputs()[i]->isLinkMandatory() ) { errors++; }
    }

    if ( errors == 0 ) {
        m_validationState = QtNodes::NodeValidationState::Valid;
        m_validationError = QString( "" );
    }
    else {
        if ( errors > 1 ) {
            m_validationError = QString(
                std::string( std::to_string( errors ) + " mandatory ports are not linked (*)." )
                    .c_str() );
        }
        else { m_validationError = "1 mandatory port is not linked (*)."; }

        m_validationState = QtNodes::NodeValidationState::Error;
    }
}

unsigned int NodeAdapterModel::nPorts( QtNodes::PortType portType ) const {
    unsigned int result = 1;

    switch ( portType ) {
    case QtNodes::PortType::In: {
        result = m_node->getInputs().size();
        break;
    }

    case QtNodes::PortType::Out: {
        result = m_node->getOutputs().size();
        break;
    }

    default: {
        break;
    }
    }

    return result;
}

QtNodes::NodeDataType NodeAdapterModel::dataType( QtNodes::PortType portType,
                                                  QtNodes::PortIndex portIndex ) const {
    QtNodes::NodeDataType result { "incorrect", "incorrect" };

    switch ( portType ) {
    case QtNodes::PortType::In: {
        std::string mandatory = ( m_node->getInputs()[portIndex]->isLinkMandatory() ) ? "*" : "";
        return IOToDataType( m_node->getInputs()[portIndex]->getTypeName(),
                             m_node->getInputs()[portIndex]->getName() + mandatory );
    }

    case QtNodes::PortType::Out: {
        return IOToDataType( m_node->getOutputs()[portIndex]->getTypeName(),
                             m_node->getOutputs()[portIndex]->getName() );
    }
    default:
        break;
    }

    checkConnections();

    return result;
}

std::shared_ptr<QtNodes::NodeData> NodeAdapterModel::outData( QtNodes::PortIndex port ) {
    return std::make_shared<ConnectionStatusData>( m_node, m_node->getOutputs()[port]->getName() );
}

void NodeAdapterModel::setInData( std::shared_ptr<QtNodes::NodeData> data, int port ) {
    auto connectionData = dynamic_cast<ConnectionStatusData*>( data.get() );
    if ( connectionData ) {
        // Add connection
        m_dataflowGraph->addLink( connectionData->getNode(),
                                  connectionData->getOutputName(),
                                  m_node,
                                  m_node->getInputs()[port]->getName() );
        m_inputsConnected[port] = true;
    }
    else {
        // Remove connection
        m_dataflowGraph->removeLink( m_node, m_node->getInputs()[port]->getName() );
        m_inputsConnected[port] = false;
    }
    checkConnections();
}

QtNodes::NodeDataType NodeAdapterModel::IOToDataType( const std::string& typeName,
                                                      const std::string& ioName ) const {
    return QtNodes::NodeDataType { typeName.c_str(), ioName.c_str() };
}

void NodeAdapterModel::updateState() {
    int i = 0;
    for ( const auto& in : m_node->getInputs() ) {
        if ( in->isLinked() ) { m_inputsConnected[i] = true; }
        i++;
    }
    checkConnections();
}

void NodeAdapterModel::addMetaData( QJsonObject& json ) {
    nlohmann::json data;
    NodeDataModelTools::QJsonObjectToNlohmannObject( json, data );
    m_node->addJsonMetaData( data );
}

NodeAdapterModel::~NodeAdapterModel() {
    m_dataflowGraph->removeNode( m_node );
}

QJsonObject NodeAdapterModel::save() const {
    QJsonObject o; // = QtNodes::NodeDataModel::save();
    nlohmann::json nodeData;
    m_node->toJson( nodeData );
    NodeDataModelTools::NlohmannObjectToQJsonObject( nodeData, o );
    return o;
}

void NodeAdapterModel::restore( QJsonObject const& p ) {
    // QtNodes::NodeDataModel::restore( p );
    //  1 - convert the QJsonObject to nlohmann::json
    nlohmann::json nodeData;
    NodeDataModelTools::QJsonObjectToNlohmannObject( p, nodeData );
    // 2 - call fromjson on the node using this json object
    m_node->fromJson( nodeData );
    // 3 - update the widget according to the editable parameters
    NodeDataModelTools::updateWidget( m_node.get(), m_widget );
}

namespace NodeDataModelTools {

QWidget* getWidget( Node* node ) {
    if ( node->getEditableParameters().size() == 0 ) { return nullptr; }

    auto controlPanel = new ControlPanel( "Editable parameters", false, nullptr );
    controlPanel->beginLayout( QBoxLayout::TopToBottom );
    for ( size_t i = 0; i < node->getEditableParameters().size(); i++ ) {
        auto edtParam      = node->getEditableParameters()[i].get();
        QWidget* newWidget = WidgetFactory::createWidget( edtParam );
        if ( newWidget ) {
            newWidget->setParent( controlPanel );
            newWidget->setObjectName( edtParam->getName().c_str() );
            controlPanel->addLabel( edtParam->getName() );
            controlPanel->addWidget( newWidget );

            if ( i != node->getEditableParameters().size() - 1 ) { controlPanel->addSeparator(); }
        }
    }
    controlPanel->endLayout();
    controlPanel->setVisible( true );

    return controlPanel;
}

void updateWidget( Node* node, QWidget* widget ) {
    if ( node->getEditableParameters().size() == 0 ) { return; }
    for ( size_t i = 0; i < node->getEditableParameters().size(); i++ ) {
        auto edtParam = node->getEditableParameters()[i].get();
        if ( !WidgetFactory::updateWidget( widget, edtParam ) ) {
            LOG( Ra::Core::Utils::logWARNING )
                << "NodeAdapterModel : unable to update parameter " << edtParam->getName()
                << " on node " << node->getInstanceName() << " (" << node->getTypeName() << ")";
        }
    }
}

/** Convert from Qt::Json to nlohmann::json */
void QJsonEntryToNlohmannEntry( const QString& key,
                                const QJsonValue& value,
                                nlohmann::json& data ) {
    switch ( value.type() ) {
    case QJsonValue::Bool:
        data[key.toStdString()] = value.toBool();
        break;
    case QJsonValue::Double:
        // as there is no QJsonValue::Int, manage explicitely keys with int value :(
        // TODO find a better way to do that ...
        // type is a specific entry of envmapdatasource
        if ( key.compare( "type" ) == 0 ) { data[key.toStdString()] = int( value.toDouble() ); }
        else { data[key.toStdString()] = Scalar( value.toDouble() ); }

        break;
    case QJsonValue::String:
        data[key.toStdString()] = value.toString().toStdString();
        break;
    case QJsonValue::Array: {
        auto jsArray = value.toArray();
        switch ( jsArray.first().type() ) {
        case QJsonValue::Double: {
            std::vector<Scalar> array;
            for ( auto v : jsArray ) {
                array.push_back( Scalar( v.toDouble() ) );
            }
            data[key.toStdString()] = array;
            break;
        }
        default:
            LOG( Ra::Core::Utils::logERROR )
                << "Only Scalar arrays are supported for Json conversion.";
        }
    } break;
    default:
        LOG( Ra::Core::Utils::logERROR ) << "QJson to nlohmann::json : QtJson value type "
                                         << value.type() << " is not suported.";
    }
}

void NlohmannEntryToQJsonEntry( const nlohmann::json& data, QJsonValue& value ) {
    switch ( data.type() ) {
    case nlohmann::detail::value_t::boolean: {
        auto v = data.get<bool>();
        value  = v;
    } break;
    case nlohmann::detail::value_t::number_float: {
        auto v = double( data.get<float>() );
        value  = v;
    } break;
    case nlohmann::detail::value_t::number_integer: {
        auto v = data.get<int>();
        value  = v;
    } break;
    case nlohmann::detail::value_t::number_unsigned: {
        auto v = data.get<unsigned int>();
        value  = int( v );
    } break;
    case nlohmann::detail::value_t::string: {
        auto v = data.get<std::string>();
        value  = v.c_str();
    } break;
    case nlohmann::detail::value_t::array: {
        QJsonArray a;
        QJsonValue v;
        for ( auto& x : data.items() ) {
            NlohmannEntryToQJsonEntry( x.value(), v );
            a.append( v );
        }
        value = a;
    } break;
    default:
        LOG( Ra::Core::Utils::logERROR ) << "nlohmann::json to QJson : nlohmann json type "
                                         << int( data.type() ) << " is not supported.";
    }
}

void QJsonObjectToNlohmannObject( const QJsonObject& p, nlohmann::json& data ) {
    for ( const auto& key : p.keys() ) {
        auto value = p.value( key );
        if ( value.isObject() ) {
            nlohmann::json j;
            QJsonObjectToNlohmannObject( value.toObject(), j );
            data[key.toStdString()] = j;
        }
        else { QJsonEntryToNlohmannEntry( key, value, data ); }
    }
}

void NlohmannObjectToQJsonObject( const nlohmann::json& data, QJsonObject& p ) {
    for ( const auto& [key, value] : data.items() ) {
        if ( value.is_object() ) {
            QJsonObject o;
            NlohmannObjectToQJsonObject( data[key], o );
            p.insert( key.c_str(), o );
        }
        else {
            QJsonValue v;
            NlohmannEntryToQJsonEntry( value, v );
            p.insert( key.c_str(), v );
        }
    }
}

} // namespace NodeDataModelTools

} // namespace GraphEditor
} // namespace QtGui
} // namespace Dataflow
} // namespace Ra
