#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Port.hpp>

#include <fstream>
#include <map>

#include <Core/Utils/Log.hpp>
#include <memory>

namespace Ra {
namespace Dataflow {
namespace Core {

using namespace Ra::Core::Utils;

DataflowGraph::DataflowGraph( const std::string& name ) : DataflowGraph( name, getTypename() ) {}

DataflowGraph::DataflowGraph( const std::string& instanceName, const std::string& typeName ) :
    Node( instanceName, typeName ) {
    // This will allow to edit subgraph in an independent editor
    addEditableParameter( new EditableParameter( instanceName, *this ) );
    // A graph always use the builtin nodes factory
    addFactory( NodeFactoriesManager::getDataFlowBuiltInsFactory() );
}

void DataflowGraph::init() {
    if ( m_ready ) {
        Node::init();
        std::for_each( m_nodesByLevel.begin(), m_nodesByLevel.end(), []( const auto& level ) {
            std::for_each( level.begin(), level.end(), []( auto node ) {
                if ( !node->isInitialized() ) { node->init(); }
            } );
        } );
    }
}

bool DataflowGraph::execute() {
    if ( !m_ready ) {
        if ( !compile() ) { return false; }
    }
    bool result = true;
    std::for_each( m_nodesByLevel.begin(), m_nodesByLevel.end(), [&result]( const auto& level ) {
        std::for_each( level.begin(), level.end(), [&result]( auto node ) {
            result = result && node->execute();
        } );
    } );
    return result;
}

void DataflowGraph::destroy() {
    std::for_each(
        m_nodesByLevel.begin(), m_nodesByLevel.end(), []( auto& level ) { level.clear(); } );
    m_nodesByLevel.clear();
    m_nodes.clear();
    m_factories.reset();
    m_dataSetters.erase( m_dataSetters.begin(), m_dataSetters.end() );
    Node::destroy();
    needsRecompile();
}

void DataflowGraph::saveToJson( const std::string& jsonFilePath ) {
    if ( !jsonFilePath.empty() ) {
        nlohmann::json data;
        toJson( data );
        std::ofstream file( jsonFilePath );
        file << std::setw( 4 ) << data << std::endl;
        m_shouldBeSaved = false;
    }
}

void DataflowGraph::toJsonInternal( nlohmann::json& data ) const {
    nlohmann::json factories   = nlohmann::json::array();
    nlohmann::json nodes       = nlohmann::json::array();
    nlohmann::json connections = nlohmann::json::array();
    nlohmann::json model;
    nlohmann::json graph;

    if ( m_factories ) {
        for ( const auto& [name, factory] : *m_factories ) {
            // do not save the standard factory, it will always be there
            if ( name != NodeFactoriesManager::dataFlowBuiltInsFactoryName ) {
                factories.push_back( name );
            }
        }
        graph["factories"] = factories;
    }

    for ( const auto& n : m_nodes ) {
        nlohmann::json nodeData;
        n->toJson( nodeData );
        nodes.push_back( nodeData );
        for ( const auto& input : n->getInputs() ) {
            if ( input->isLinked() ) {
                nlohmann::json link = nlohmann::json::object();
                auto portOut        = input->getLink();
                auto nodeOut        = portOut->getNode();
                link["out_node"]    = nodeOut->getInstanceName();
                link["out_port"]    = portOut->getName();
                link["in_node"]     = n->getInstanceName();
                link["in_port"]     = input->getName();
                connections.push_back( link );
            }
        }
    }

    // write the common content of the Node to the json data
    graph["nodes"]       = nodes;
    graph["connections"] = connections;
    // Fill the specific concrete node information
    data.emplace( "graph", graph );
}

bool DataflowGraph::loadFromJson( const std::string& jsonFilePath ) {

    if ( !nlohmann::json::accept( std::ifstream( jsonFilePath ) ) ) {
        LOG( logERROR ) << jsonFilePath << " is not a valid json file !!";
        return false;
    }

    std::ifstream file( jsonFilePath );
    nlohmann::json j;
    file >> j;
    m_shouldBeSaved = false;
    return fromJson( j );
}

std::pair<std::shared_ptr<Node>, std::string>
getLinkInfo( const std::string& which,
             const nlohmann::json& linkData,
             const std::map<std::string, std::shared_ptr<Node>>& nodeByName ) {
    std::string field = which + "_node";
    std::shared_ptr<Node> node { nullptr };

    auto itNode = nodeByName.find( linkData[field] );
    if ( itNode != nodeByName.end() ) { node = itNode->second; }
    else {
        // Error, could not find the node
        std::string msg =
            std::string { "Node " } + which + " not found in cache " + " : " + linkData.dump();
        return { nullptr, msg };
    }

    std::string port;
    field = which + "_port";
    if ( linkData.contains( field ) ) {
        auto p = node->getPortByName( which, linkData[field] ).second;
        if ( p != nullptr ) { port = p->getName(); }
    }
    else {
        field = which + "_index";
        if ( linkData.contains( field ) ) {
            auto p = node->getPortByIndex( which, Node::PortIndex { int { linkData[field] } } );
            if ( p != nullptr ) { port = p->getName(); }
        }
    }
    if ( port.empty() ) {
        std::string msg = std::string { "Port " } + which + " not found in node " +
                          node->getInstanceName() + " : " + linkData.dump();
        return { nullptr, msg };
    }
    return { node, port };
}

bool DataflowGraph::fromJsonInternal( const nlohmann::json& data ) {
    if ( data.contains( "graph" ) ) {
        // indicate that the graph must be recompiled after loading
        needsRecompile();
        // load the graph
        m_factories.reset( new NodeFactorySet );
        addFactory( NodeFactoriesManager::getDataFlowBuiltInsFactory() );
        if ( data["graph"].contains( "factories" ) ) {
            auto factories = data["graph"]["factories"];
            for ( const auto& factoryName : factories ) {
                // Do not add factories already registered for the graph.
                if ( m_factories->hasFactory( factoryName ) ) { continue; }
                auto factory = NodeFactoriesManager::getFactory( factoryName );
                if ( factory ) { addFactory( factory ); }
                else {
                    LOG( logERROR )
                        << "DataflowGraph::loadFromJson : Unable to find a factory with name "
                        << factoryName;
                    return false;
                }
            }
        }
        std::map<std::string, std::shared_ptr<Node>> nodeByName;
        auto nodes = data["graph"]["nodes"];
        for ( auto& n : nodes ) {
            if ( !n["model"].contains( "name" ) ) {
                LOG( logERROR ) << "Found a node without model description." << n.dump()
                                << "Unable to build an instance.";
                return false;
            }
            std::string nodeTypeName = n["model"]["name"];
            std::string instanceName;

            if ( n.contains( "instance" ) ) { instanceName = n["instance"]; }
            else {
                LOG( logERROR ) << "Found a node of type " << nodeTypeName
                                << " without identification ";
                return false;
            }
            auto newNode = m_factories->createNode( nodeTypeName, n, this );
            if ( newNode ) {
                if ( !instanceName.empty() ) {
                    auto [it, inserted] = nodeByName.insert( { instanceName, newNode } );
                    if ( !inserted ) {
                        LOG( logERROR ) << "DataflowGraph::loadFromJson : duplicated node name "
                                        << nodeTypeName;
                        return false;
                    }
                }
            }
            else {
                LOG( logERROR ) << "Unable to create the node " << nodeTypeName;
                return false;
            }
        }
        auto links = data["graph"]["connections"];
        for ( auto& l : links ) {
            auto [nodeFrom, fromOutput] = getLinkInfo( "out", l, nodeByName );
            if ( nodeFrom == nullptr ) {
                LOG( logERROR ) << "DataflowGraph::loadFromJson: error when parsing JSON."
                                << " Could not find the link source (" << fromOutput
                                << "). Link not added.";
                return false;
            }
            auto [nodeTo, toInput] = getLinkInfo( "in", l, nodeByName );
            if ( nodeTo == nullptr ) {
                LOG( logERROR ) << "DataflowGraph::loadFromJson: error when parsing JSON."
                                << " Could not find the link source (" << toInput
                                << "). Link not added.";
                return false;
            }
            if ( !addLink( nodeFrom, fromOutput, nodeTo, toInput ) ) {
                LOG( logERROR )
                    << "DataflowGraph::loadFromJson: error when parsing JSON"
                    << ": Could not add a link (missing or wrong information, please refer to "
                       "the previous error messages). Link not added.";
                return false;
            }
        }
    }
    return true;
}

bool DataflowGraph::canAdd( const Node* newNode ) const {
    return findNode( newNode ) == -1;
}

bool DataflowGraph::addNode( std::shared_ptr<Node> newNode ) {
    // Check if the new node already exists (= same name and type)
    if ( canAdd( newNode.get() ) ) {
        if ( newNode->getInputs().empty() || newNode->getOutputs().empty() ) {
            bool ( DataflowGraph::*addGraphIOPort )( PortBase* ) = newNode->getInputs().empty()
                                                                       ? &DataflowGraph::addSetter
                                                                       : &DataflowGraph::addGetter;
            auto& interfaces                                     = newNode->buildInterfaces( this );
            for ( auto p : interfaces ) {
                ( this->*addGraphIOPort )( p );
            }
        }
        m_nodes.emplace_back( std::move( newNode ) );
        needsRecompile();
        return true;
    }
    else { return false; }
}

bool DataflowGraph::removeNode( std::shared_ptr<Node> node ) {
    // This is to prevent graph destruction from the graph editor, depending on how it is used
    if ( m_nodesAndLinksProtected ) { return false; }

    // Check if the node is in the list already exists (= same name)
    int index = -1;
    if ( ( index = findNode( node.get() ) ) == -1 ) { return false; }
    else {
        if ( node->getInputs().empty() || node->getOutputs().empty() ) {
            bool ( DataflowGraph::*removeGraphIOPort )( const std::string& ) =
                node->getInputs().empty() ? &DataflowGraph::removeSetter
                                          : &DataflowGraph::removeGetter;
            for ( auto& p : node->getInterfaces() ) {
                ( this->*removeGraphIOPort )( p->getName() );
            }
        }
        m_nodes.erase( m_nodes.begin() + index );
        needsRecompile();
        return true;
    }
}

bool DataflowGraph::checkPortCompatibility( const Node* nodeFrom,
                                            Node::PortIndex portOutIdx,
                                            const PortBase* portOut,
                                            const Node* nodeTo,
                                            Node::PortIndex portInIdx,
                                            const PortBase* portIn ) {
    // Compare types
    if ( !( portIn->getType() == portOut->getType() ) ) {
        Log::addLinkTypeMismatch( nodeFrom, portOutIdx, portOut, nodeTo, portInIdx, portIn );
        return false;
    }

    // Check if input is connected
    if ( portIn->isLinked() ) {
        Log::alreadyLinked( nodeTo, portIn );
        return false;
    }
    return true;
}

void nodeNotFoundMessage( const std::string& type, const std::string& name, const Node* node ) {
    LOG( logERROR ) << "DataflowGraph::addLink Unable to find " << type << "input port " << name
                    << " from destination node " << node->getInstanceName() << " ("
                    << node->getTypeName() << ")";
}

bool DataflowGraph::addLink( const std::shared_ptr<Node>& nodeFrom,
                             const std::string& nodeFromOutputName,
                             const std::shared_ptr<Node>& nodeTo,
                             const std::string& nodeToInputName ) {
    if ( !checkNodeValidity( nodeFrom.get(), nodeTo.get() ) ) { return false; }

    auto [inputIdx, inputPort] = nodeTo->getInputByName( nodeToInputName );
    if ( !inputPort ) {
        nodeNotFoundMessage( "input", nodeToInputName, nodeTo.get() );
        return false;
    }
    auto [outputIdx, outputPort] = nodeFrom->getOutputByName( nodeFromOutputName );
    if ( !outputPort ) {
        nodeNotFoundMessage( "output", nodeFromOutputName, nodeFrom.get() );
        return false;
    }

    // Compare types
    if ( !checkPortCompatibility(
             nodeFrom.get(), outputIdx, outputPort, nodeTo.get(), inputIdx, inputPort ) ) {
        return false;
    }

    // port can be connected
    inputPort->connect( outputPort );
    // The state of the graph changes, set it to not ready
    needsRecompile();
    return true;
}

bool DataflowGraph::addLink( const std::shared_ptr<Node>& nodeFrom,
                             Node::PortIndex portOutIdx,
                             const std::shared_ptr<Node>& nodeTo,
                             Node::PortIndex portInIdx ) {
    if ( !checkNodeValidity( nodeFrom.get(), nodeTo.get() ) ) { return false; }

    auto portOut = nodeFrom->getOutputByIndex( portOutIdx );
    auto portIn  = nodeTo->getInputByIndex( portInIdx );

    if ( !portOut ) {
        Log::badPortIdx( "output", nodeFrom->getTypeName(), portOutIdx );
        return false;
    }
    if ( !portIn ) {
        Log::badPortIdx( "input", nodeTo->getTypeName(), portInIdx );
        return false;
    }

    // Compare types
    if ( !checkPortCompatibility(
             nodeFrom.get(), portOutIdx, portOut, nodeTo.get(), portInIdx, portIn ) )
        return false;

    // port can be connected
    portIn->connect( portOut );
    // The state of the graph changes, set it to not ready
    needsRecompile();
    return true;
}

bool DataflowGraph::addLink( Node::PortBaseRawPtr outputPort, Node::PortBaseRawPtr inputPort ) {
    if ( !inputPort->is_input() || outputPort->is_input() ) { return false; }
    auto nodeFrom = outputPort->getNode();
    auto nodeTo   = inputPort->getNode();
    if ( !checkNodeValidity( nodeFrom, nodeTo ) ) { return false; }
    // Compare types
    if ( !checkPortCompatibility(
             nodeFrom, Node::PortIndex {}, outputPort, nodeTo, Node::PortIndex {}, inputPort ) ) {
        return false;
    }
    // port can be connected
    inputPort->connect( outputPort );
    // The state of the graph changes, set it to not ready
    needsRecompile();
    return true;
}

bool DataflowGraph::removeLink( std::shared_ptr<Node> node, const std::string& nodeInputName ) {
    // This is to prevent graph destruction from the graph editor, depending on how it is used
    if ( m_nodesAndLinksProtected ) { return false; }

    // Check node's existence in the graph
    if ( findNode( node.get() ) == -1 ) { return false; }

    // Check if node's input exists
    int found = -1;
    int index = 0;
    for ( auto& input : node->getInputs() ) {
        if ( input->getName() == nodeInputName ) {
            found = index;
            break;
        }
        index++;
    }
    if ( found == -1 ) { return false; }

    node->getInputs()[found]->disconnect();
    needsRecompile();
    return true;
}

int DataflowGraph::findNode( const Node* node ) const {
    auto foundIt = std::find_if(
        m_nodes.begin(), m_nodes.end(), [node]( const auto& p ) { return *p == *node; } );
    if ( foundIt != m_nodes.end() ) { return std::distance( m_nodes.begin(), foundIt ); }
    else { return -1; }
}

bool DataflowGraph::findNode2( const Node* node ) const {
    if ( !node ) return false;

    for ( const auto& n : m_nodes ) {
        if ( n.get() == node ) return true;
        auto g = dynamic_cast<DataflowGraph*>( n.get() );
        if ( g ) {
            if ( g->findNode2( node ) ) return true;
        }
    }
    return false;
}

bool DataflowGraph::compile() {
    // Find useful nodes (directly or indirectly connected to a Sink)
    std::unordered_map<Node*, std::pair<int, std::vector<Node*>>> infoNodes;
    for ( auto const& n : m_nodes ) {
        // Find all sinks
        if ( n->getOutputs().size() == 0 ) {
            // Add the sink in the useful nodes set if any of his port is linked
            bool activeSink { false };
            for ( const auto& p : n->getInputs() ) {
                activeSink |= p->isLinked();
            }
            if ( activeSink ) {
                infoNodes.emplace( n.get(), std::pair<int, std::vector<Node*>>( 0, {} ) );
                // recursively add the predecessors of the sink
                backtrackGraph( n.get(), infoNodes );
            }
        }
    }
    // Compute the level (rank of execution) of useful nodes
    int maxLevel = 0;
    for ( auto& infNode : infoNodes ) {
        auto n = infNode.first;
        // Compute the nodes' level starting from sources
        if ( n->getInputs().empty() ) {
            // Tag successors
            for ( auto const successor : infNode.second.second ) {
                infoNodes[successor].first =
                    std::max( infoNodes[successor].first, infoNodes[n].first + 1 );
                maxLevel = std::max( maxLevel,
                                     std::max( infoNodes[successor].first,
                                               goThroughGraph( successor, infoNodes ) ) );
            }
        }
    }
    m_nodesByLevel.clear();
    m_nodesByLevel.resize( infoNodes.size() != 0 ? maxLevel + 1 : 0 );
    for ( auto& infNode : infoNodes ) {
        m_nodesByLevel[infNode.second.first].push_back( infNode.first );
    }

    // For each level
    for ( auto& lvl : m_nodesByLevel ) {
        // For each node
        for ( size_t j = 0; j < lvl.size(); j++ ) {
            if ( !lvl[j]->compile() ) { return m_ready = false; }
            // For each input
            for ( size_t k = 0; k < lvl[j]->getInputs().size(); k++ ) {
                if ( lvl[j]->getInputs()[k]->isLinkMandatory() &&
                     !lvl[j]->getInputs()[k]->isLinked() ) {
                    return m_ready = false;
                }
            }
        }
    }
    m_ready = true;
    init();
    return m_ready;
}

void DataflowGraph::clearNodes() {
    for ( size_t i = 0; i < m_nodesByLevel.size(); i++ ) {
        m_nodesByLevel[i].clear();
        m_nodesByLevel[i].shrink_to_fit();
    }
    m_nodesByLevel.clear();
    m_nodesByLevel.shrink_to_fit();
    m_nodes.erase( m_nodes.begin(), m_nodes.end() );
    m_nodes.shrink_to_fit();
    m_inputs.erase( m_inputs.begin(), m_inputs.end() );
    m_inputs.shrink_to_fit();
    m_outputs.erase( m_outputs.begin(), m_outputs.end() );
    m_outputs.shrink_to_fit();
    m_dataSetters.erase( m_dataSetters.begin(), m_dataSetters.end() );
    m_shouldBeSaved = true;
}

void DataflowGraph::backtrackGraph(
    Node* current,
    std::unordered_map<Node*, std::pair<int, std::vector<Node*>>>& infoNodes ) {
    for ( auto& input : current->getInputs() ) {
        if ( input->getLink() ) {
            Node* previous = input->getLink()->getNode();
            if ( previous ) {
                auto previousInInfoNodes = infoNodes.find( previous );
                if ( previousInInfoNodes != infoNodes.end() ) {
                    // If the previous node is already in the map,
                    // find if the current node is already a successor node
                    auto& previousSuccessors = previousInInfoNodes->second.second;
                    bool foundCurrent        = std::any_of( previousSuccessors.begin(),
                                                     previousSuccessors.end(),
                                                     [current]( auto c ) { return c == current; } );
                    if ( !foundCurrent ) {
                        // If the current node is not a successor node, add it to the list
                        previousSuccessors.push_back( current );
                    }
                }
                else {
                    // Add node to info nodes
                    std::vector<Node*> successors;
                    successors.push_back( current );
                    infoNodes.emplace(
                        previous,
                        std::pair<int, std::vector<Node*>>( 0, std::move( successors ) ) );
                    backtrackGraph( previous, infoNodes );
                }
            }
        }
    }
}

int DataflowGraph::goThroughGraph(
    Node* current,
    std::unordered_map<Node*, std::pair<int, std::vector<Node*>>>& infoNodes ) {
    int maxLevel = 0;
    if ( infoNodes.find( current ) != infoNodes.end() ) {
        for ( auto const& successor : infoNodes[current].second ) {
            infoNodes[successor].first =
                std::max( infoNodes[successor].first, infoNodes[current].first + 1 );
            maxLevel =
                std::max( infoNodes[successor].first, goThroughGraph( successor, infoNodes ) );
        }
    }
    return maxLevel;
}

bool DataflowGraph::addSetter( PortBase* in ) {
    bool found = false;
    if ( m_dataSetters.find( in->getName() ) == m_dataSetters.end() ) {
        addInput( in );
        auto portOut = std::shared_ptr<PortBase>( in->reflect( this, in->getName() ) );
        m_dataSetters.emplace( std::make_pair(
            in->getName(),
            DataSetter { DataSetterDesc { portOut, portOut->getName(), portOut->getTypeName() },
                         in } ) );
        found = true;
    }
    return found;
}

bool DataflowGraph::removeSetter( const std::string& setterName ) {
    bool removed = false;
    auto itS     = m_dataSetters.find( setterName );
    if ( itS != m_dataSetters.end() ) {
        auto& [desPort, in] = itS->second;
        removeInput( in );
        m_dataSetters.erase( itS );
        removed = true;
    }
    return removed;
}

bool DataflowGraph::addGetter( PortBase* out ) {
    if ( out->is_input() ) { return false; }
    // This is very similar to addOutput, except the data can't be set.
    // Data pointer must be set by any sink at compile time, in the init function to refer to
    // the data fetched from the associated input link
    bool found = false;
    // TODO check if this verification is needed ?
    for ( auto& output : m_outputs ) {
        if ( output->getName() == out->getName() ) {
            found = true;
            break;
        }
    }
    if ( !found ) { m_outputs.emplace_back( out ); }
    return !found;
}

bool DataflowGraph::removeGetter( const std::string& getterName ) {
    auto getterP = std::find_if( m_outputs.begin(), m_outputs.end(), [getterName]( const auto& p ) {
        return p->getName() == getterName;
    } );
    bool found   = false;
    if ( getterP != m_outputs.end() ) {
        m_outputs.erase( getterP );
        found = true;
    }
    return found;
}

bool DataflowGraph::releaseDataSetter( const std::string& portName ) {
    auto setter = m_dataSetters.find( portName );
    bool found  = false;
    if ( setter != m_dataSetters.end() ) {
        auto [desc, in] = setter->second;
        in->disconnect();
        found = true;
    }
    return found;
}

// Why is this method useful if it is the same than getDataSetter ?
bool DataflowGraph::activateDataSetter( const std::string& portName ) {
    return getDataSetter( portName ) != nullptr;
}

std::shared_ptr<PortBase> DataflowGraph::getDataSetter( const std::string& portName ) {
    auto setter = m_dataSetters.find( portName );
    std::shared_ptr<PortBase> p { nullptr };
    if ( setter != m_dataSetters.end() ) {
        auto [desc, in] = setter->second;
        p               = std::get<0>( desc );
        in->disconnect();
        p->connect( in );
    }
    return p;
}

std::vector<DataflowGraph::DataSetterDesc> DataflowGraph::getAllDataSetters() const {
    std::vector<DataflowGraph::DataSetterDesc> r;
    r.reserve( m_dataSetters.size() );
    for ( auto& s : m_dataSetters ) {
        r.push_back( s.second.first );
    }
    return r;
}

PortBase* DataflowGraph::getDataGetter( const std::string& portName ) {
    auto portIt = std::find_if( m_outputs.begin(), m_outputs.end(), [portName]( const auto& p ) {
        return p->getName() == portName;
    } );
    PortBase* g { nullptr };
    if ( portIt != m_outputs.end() ) { g = portIt->get(); }
    return g;
}

std::vector<DataflowGraph::DataGetterDesc> DataflowGraph::getAllDataGetters() const {
    std::vector<DataflowGraph::DataGetterDesc> r;
    r.reserve( m_outputs.size() );
    for ( auto& portOut : m_outputs ) {
        r.emplace_back( portOut.get(), portOut->getName(), portOut->getTypeName() );
    }
    return r;
}

std::shared_ptr<Node> DataflowGraph::getNode( const std::string& instanceNameNode ) const {
    auto nodeIt =
        std::find_if( m_nodes.begin(), m_nodes.end(), [instanceNameNode]( const auto& n ) {
            return n->getInstanceName() == instanceNameNode;
        } );
    if ( nodeIt != m_nodes.end() ) { return *nodeIt; }
    LOG( logERROR ) << "DataflowGraph::getNode : The node with the instance name \""
                    << instanceNameNode << "\" has not been found";
    return { nullptr };
}

std::shared_ptr<DataflowGraph> DataflowGraph::loadGraphFromJsonFile( const std::string& filename ) {
    if ( !nlohmann::json::accept( std::ifstream( filename ) ) ) {
        LOG( logERROR ) << filename << " is not a valid json file !!";
        return nullptr;
    }

    std::ifstream jsonFile( filename );
    nlohmann::json j;
    jsonFile >> j;

    bool valid = false;
    if ( j.contains( "instance" ) && j.contains( "model" ) ) {
        valid = j["model"].contains( "name" );
    }
    if ( !valid ) {
        LOG( logERROR ) << "loadGraphFromJsonFile :" << filename
                        << " does not contain a valid json NodeGraph\n";
        return nullptr;
    }
    std::string instanceName = j["instance"];
    std::string graphType    = j["model"]["name"];
    LOG( logINFO ) << "Loading the graph " << instanceName << ", with type " << graphType << "\n";

    auto& fctMngr = Ra::Dataflow::Core::NodeFactoriesManager::getFactoryManager();
    auto ndldd    = fctMngr.createNode( graphType, j );
    if ( ndldd == nullptr ) {
        LOG( logERROR ) << "Unable to load a graph with type " << graphType << "\n";
        return nullptr;
    }

    auto graph = std::dynamic_pointer_cast<DataflowGraph>( ndldd );
    if ( graph != nullptr ) {
        graph->m_shouldBeSaved = false;
        return graph;
    }

    LOG( logERROR ) << "Loaded graph not inheriting from DataflowGraph " << graphType << "\n";

    return nullptr;
}

bool DataflowGraph::checkNodeValidity( const Node* nodeFrom, const Node* nodeTo ) {
    using namespace Ra::Core::Utils; // Check node "from" existence in the graph
    if ( !findNode2( nodeFrom ) ) {
        Log::unableToFind( "initial node", nodeFrom->getInstanceName() );
        return false;
    }

    // Check node "to" existence in the graph
    if ( !findNode2( nodeTo ) ) {
        Log::unableToFind( "destination node", nodeTo->getInstanceName() );
        return false;
    }
    return true;
}

void DataflowGraph::Log::alreadyLinked( const Node* node, const PortBase* port ) {
    LOG( logERROR ) << "DataflowGraph::addLink destination port not available (already linked) for "
                    << node->getInstanceName() << " (" << node->getTypeName() << "), port "
                    << port->getName();
}

void DataflowGraph::Log::addLinkTypeMismatch( const Node* nodeFrom,
                                              Node::PortIndex portOutIdx,
                                              const PortBase* portOut,
                                              const Node* nodeTo,
                                              Node::PortIndex portInIdx,
                                              const PortBase* portIn ) {
    LOG( logERROR ) << "DataflowGraph::addLink type mismatch from " << nodeFrom->getInstanceName()
                    << " (" << nodeFrom->getTypeName() << ") / " << portOut->getName() << " ("
                    << portOutIdx << " with type " << portOut->getTypeName() << ")"
                    << " to " << nodeTo->getInstanceName() << " (" << nodeTo->getTypeName()
                    << ") / " << portIn->getName() << " (" << portInIdx << " with type "
                    << portIn->getTypeName() << ") ";
}

void DataflowGraph::Log::unableToFind( const std::string& type, const std::string& instanceName ) {
    LOG( logERROR ) << "DataflowGraph::addLink Unable to find " << type << " " << instanceName;
}

void DataflowGraph::Log::badPortIdx( const std::string& type,
                                     const std::string& instanceName,
                                     Node::PortIndex idx ) {
    LOG( logERROR ) << "DataflowGraph::addLink node " << instanceName << "as no " << type
                    << " port with index " << idx;
}

} // namespace Core
} // namespace Dataflow
} // namespace Ra
