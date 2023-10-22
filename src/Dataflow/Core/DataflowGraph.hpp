#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/Enumerator.hpp>
#include <Dataflow/Core/Node.hpp>
#include <Dataflow/Core/NodeFactory.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

/**
 *  \todo : clarify what is a source and how to use it
 *      if sources must be used through interface port only, delete the set_data on all sources
 *  \todo Are node aliases a should-have (to make editing more user friendly)???
 */

/**
 * \brief Represent a set of connected nodes that define a Direct Acyclic Computational Graph
 * Ownership of nodes is given to the graph at construction time.
 * \todo make a "graph embedding node" that allow to seemlesly integrate a graph as a node in
 * another graph
 *      --> Edition of a graph will allow loading and saving to a file directly
 *      --> Edition of an embeded graph will defer loading and saving to the parent graph
 *          --> for this, need to decide if a subgraph is stored in the json of its parent or in a
 * separate file
 */
class RA_DATAFLOW_API DataflowGraph : public Node
{
  public:
    /// Constructor.
    /// The nodes pointing to external data are created here.
    /// \param name The name of the render graph.
    explicit DataflowGraph( const std::string& name );
    virtual ~DataflowGraph() = default;

    void init() override;
    bool execute() override;
    void destroy() override;

    /// \brief Set the factories to use when loading a graph.
    /// \param factories new factory set that will replace the existing factory set if any.
    void setNodeFactories( std::shared_ptr<NodeFactorySet> factories ) { m_factories = factories; }

    /// \brief Get the node factories associated with the graph.
    /// \return returns nullptr if no factory set is associated with the graph.
    std::shared_ptr<NodeFactorySet> getNodeFactories() const { return m_factories; }

    /// \brief Add a factory to the factory set of the graph.
    /// Creates the factory set if it does not exists
    /// \param f  a shared pointer to the factory to be added. The name of this factory
    void addFactory( std::shared_ptr<NodeFactory> f );

    /// \brief Remove a factory from the factory set of the graph.
    /// \param name the name of the factory to remove
    /// \return true if the factory was found and removed
    bool removeFactory( const std::string& name ) { return m_factories->removeFactory( name ); }

    /// \brief Loads nodes and links from a JSON file.
    /// \param jsonFilePath The path to the JSON file.
    /// \return true if the file was loaded, false if an error occurs.
    bool loadFromJson( const std::string& jsonFilePath );

    /// \brief Saves nodes and links to a JSON file.
    /// \param jsonFilePath The path to the JSON file.
    void saveToJson( const std::string& jsonFilePath );

    /// \brief Adds a node to the render graph.
    /// Adds interface ports to the node newNode and the corresponding input and output ports to
    /// the graph.
    /// \param newNode The node to add to the graph.
    /// \return a pair with a bool and a raw pointer to the Node. If the bool is true, the raw
    /// pointer is owned by the graph. If the bool is false, the raw pointer ownership is left to
    /// the caller.
    virtual bool addNode( std::shared_ptr<Node> newNode );
    template <typename T, typename... U>
    std::shared_ptr<T> addNode( U&&... u );

    /// \brief Removes a node from the render graph.
    /// Removes input and output ports, corresponding to interface ports of the node, from the
    /// graph. \param node The node to remove from the graph. \return true if the node was removed
    /// and the given pointer is set to nullptr, false else
    virtual bool removeNode( std::shared_ptr<Node> node );

    /// Connects two nodes of the render graph.
    /// The two nodes must already be in the render graph (with the addNode(Node* newNode)
    /// function), the first node's in port must be free and the connected in port and out port must
    /// have the same type of data.
    /// \param nodeFrom The node that contains the out port.
    /// \param nodeFromOutputName The name of the out port in nodeFrom.
    /// \param nodeTo The node that contains the in port.
    /// \param nodeToInputName The name of the in port in nodeTo.
    bool addLink( const std::shared_ptr<Node>& nodeFrom,
                  const std::string& nodeFromOutputName,
                  const std::shared_ptr<Node>& nodeTo,
                  const std::string& nodeToInputName );

    bool addLink( const std::shared_ptr<Node>& nodeFrom,
                  Node::PortIndex portOutIdx,
                  const std::shared_ptr<Node>& nodeTo,
                  Node::PortIndex portInIdx );

    bool addLink( Node::PortBaseRawPtr outputPort, Node::PortBaseRawPtr inputPort );

    template <typename T, typename U>
    bool addLink( const std::shared_ptr<PortOut<T>>& outputPort,
                  const std::shared_ptr<PortIn<U>>& inputPort );

    ///
    /// \brief Removes the link connected to a node's input port
    /// \param node the node to unlink
    /// \param nodeInputName the name of the port to unlink
    /// \return true if link is removed, false if not.
    bool removeLink( std::shared_ptr<Node>, const std::string& nodeInputName );

    /// \brief Get the vector of all the nodes on the graph
    /// \return
    const std::vector<std::shared_ptr<Node>>& getNodes() const { return m_nodes; }

    /// Gets a specific node according to its instance name.
    /// \param instanceNameNode The instance name of the node.
    std::shared_ptr<Node> getNode( const std::string& instanceNameNode ) const;

    /// Gets the nodes ordered by level (after compilation)
    const std::vector<std::vector<Node*>>& getNodesByLevel() const { return m_nodesByLevel; }

    /// Compile the render graph to check its validity and simplify it.
    /// The compilation has multiple goals:
    /// - Remove the nodes that have no direct or indirect connections to sink nodes
    /// - Order the nodes by level according to their dependencies
    /// - Check if every mandatory port is linked
    bool compile() override;

    /// Gets the number of nodes
    size_t getNodesCount() const { return m_nodes.size(); }

    /// Deletes all nodes from the render graph.
    virtual void clearNodes();

    /// Test if the graph is compiled
    bool isCompiled() const { return m_ready; }

    /// Mark the graph as needing recompilation (useful to force recompilation and resources
    /// update)
    inline void needsRecompile();

    /// \brief Gets an output port connected to the named input port of the graph.
    /// Return the connected output port if success, sharing the ownership with the caller.
    /// This output port could then be used through setter->setData( ptr ) to set the graph
    /// input from the data pointer owned by the caller. \note As ownership is shared with the
    /// caller, the graph must survive the returned pointer to be able to use the dataSetter..
    /// \params portName The name of the input port of the graph
    std::shared_ptr<PortBase> getDataSetter( const std::string& portName );

    /// \brief disconnect the data setting port from its inputs.
    bool releaseDataSetter( const std::string& portName );
    /// \brief connect the data setting port from its inputs.
    bool activateDataSetter( const std::string& portName );

    /// \brief Returns an alias to the named output port of the graph.
    /// Allows to get the data stored at this port after the execution of the graph.
    /// \note ownership is left to the graph, not shared. The graph must survive the returned
    /// pointer to be able to use the dataGetter.
    /// \params portName the name of the output port
    PortBase* getDataGetter( const std::string& portName );

    /// \brief Data setter descriptor.
    /// A Data setter descriptor is composed of an output port (linked by construction to an
    /// input port of the graph), its name and its type. Use setData on the output port to pass
    /// data to the graph
    using DataSetterDesc = std::tuple<std::shared_ptr<PortBase>, std::string, std::string>;

    /// \brief Data getter descriptor.
    /// A Data getter descriptor is composed of an output port (belonging to any node of the
    /// graph), its name and its type. Use getData on the output port to extract data from the
    /// graph. \note, a dataGetter is valid only after successful compilation of the graph.
    /// \todo find a way to test the validity of the getter (invalid if no path exists from any
    /// source port to the associated sink port)
    using DataGetterDesc = std::tuple<PortBase*, std::string, std::string>;

    /// Creates a vector that stores all the existing DataSetters (\see getDataSetter) of the
    /// graph.
    /// TODO : Verify why, when listing the data setters, they are connected ...
    std::vector<DataSetterDesc> getAllDataSetters() const;

    /// Creates a vector that stores all the existing DataGetters (\see getDataGetter) of the
    /// graph. A tuple is composed of an output port belonging to the graph, its name its type.
    std::vector<DataGetterDesc> getAllDataGetters() const;
    bool findNode2( const Node* node ) const;

    bool shouldBeSaved() { return m_shouldBeSaved; }

    static const std::string& getTypename();

    /**
     * \brief Load a graph from the given file.
     * \param filename
     * Any type of graph that inherits from DataflowGraph can be loaded by this function as soon
     * as the appropriate constructor is registered in the node factory. \return The loaded
     * graph, as a DataFlowGraph pointer to be downcast to the correct type
     */
    static std::shared_ptr<DataflowGraph> loadGraphFromJsonFile( const std::string& filename );

    /**
     * \brief protect nodes and links from deletion.
     * \param on true to protect, false to unprotect.
     */
    void setNodesAndLinksProtection( bool on ) { m_nodesAndLinksProtected = on; }

    /**
     * \brief get the protection status protect nodes and links from deletion
     * \return the protection status
     */
    bool getNodesAndLinksProtection() const { return m_nodesAndLinksProtected; }

    using Node::addInput;
    using Node::addOutput;

  protected:
    /** Allow derived class to construct the graph with their own static type
     */
    DataflowGraph( const std::string& instanceName, const std::string& typeName );

    bool fromJsonInternal( const nlohmann::json& data ) override;
    void toJsonInternal( nlohmann::json& ) const override;

    /// \brief Test if a node can be added to a graph
    /// \param newNode const naked pointer to the candidate node
    /// \return true if ownership could be transferred to the graph.
    virtual bool canAdd( const Node* newNode ) const;

    /// Returns the index of the given node in the graph.
    /// if there is none, returns -1.
    /// \param name The name of the node to find.
    int findNode( const Node* node ) const;

  private:
    // Internal helper functions
    /// Internal compilation function that allows to go back in the render graph while filling
    /// an information map. \param current The current node. \param infoNodes The map that
    /// contains information about nodes.
    void backtrackGraph( Node* current,
                         std::unordered_map<Node*, std::pair<int, std::vector<Node*>>>& infoNodes );
    /// Internal compilation function that allows to go through the render graph, using an
    /// information map.
    /// \param current The current node.
    /// \param infoNodes The map that contains information about nodes.
    int goThroughGraph( Node* current,
                        std::unordered_map<Node*, std::pair<int, std::vector<Node*>>>& infoNodes );

    /// \brief Adds an input port to the graph and associate it with a dataSetter.
    /// This port is aliased as an interface port in a source node of the graph.
    /// This function checks if there is no input port with the same name already
    /// associated with the graph.
    bool addSetter( PortBase* in );

    /// \brief Remove the given setter from the graph
    /// \param setterName
    /// \return true if the setter was removed, false else.
    bool removeSetter( const std::string& setterName );

    /// \brief Adds an out port for a Graph and register it as a dataGetter.
    /// This port is aliased as an interface port in a sink node of the graph.
    /// This function checks if there is no out port with the same name already
    /// associated with the graph.
    /// \param out The port to add.
    bool addGetter( PortBase* out );

    /// \brief Remove the given getter from the graph
    /// \param getterName
    /// \return true if the getter was removed, false else.
    bool removeGetter( const std::string& getterName );

    bool checkNodeValidity( const Node* nodeFrom, const Node* nodeTo );
    static bool checkPortCompatibility( const Node* nodeFrom,
                                        Node::PortIndex portOutIdx,
                                        const PortBase* portOut,
                                        const Node* nodeTo,
                                        Node::PortIndex portInIdx,
                                        const PortBase* portIn );
    class RA_DATAFLOW_API Log
    {
      public:
        static void alreadyLinked( const Node* node, const PortBase* port );
        static void addLinkTypeMismatch( const Node* nodeFrom,
                                         Node::PortIndex portOutIdx,
                                         const PortBase* portOut,
                                         const Node* nodeTo,
                                         Node::PortIndex portInIdx,
                                         const PortBase* portIn );
        static void unableToFind( const std::string& type, const std::string& instanceName );
        static void
        badPortIdx( const std::string& type, const std::string& instanceName, Node::PortIndex idx );
    };

    /// Flag that indicates if the graph should be saved to a file
    /// This flag is useless outside an load/edit/save scenario
    bool m_shouldBeSaved { false };

    /// Flag set after successful compilation indicating graph is ready to be executed
    /// This flag is reset as soon as the graph is modified.
    bool m_ready { false };

    /// The node factory to use for loading
    std::shared_ptr<NodeFactorySet> m_factories;
    /// The unordered list of nodes.
    std::vector<std::shared_ptr<Node>> m_nodes;
    // Internal node levels representation
    /// The list of nodes ordered by levels.
    /// Two nodes at the same level have no dependency between them.
    std::vector<std::vector<Node*>> m_nodesByLevel;

    /// Data setters management : used to pass parameter to the graph when the graph is not
    /// embedded into another graph (inputs are here for this case). A dataSetter is an
    /// outputPort, associated to an input port of the graph. The connection between these ports
    /// can be activated/deactivated using activateDataSetter/releaseDataSetter
    using DataSetter = std::pair<DataSetterDesc, PortBase*>;
    std::map<std::string, DataSetter> m_dataSetters;

    bool m_nodesAndLinksProtected { false };
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

inline void DataflowGraph::addFactory( std::shared_ptr<NodeFactory> f ) {
    if ( !m_factories ) { m_factories.reset( new NodeFactorySet ); }
    m_factories->addFactory( f );
}
template <typename T, typename... U>
std::shared_ptr<T> DataflowGraph::addNode( U&&... u ) {
    auto ret = std::make_shared<T>( std::forward<U>( u )... );
    if ( addNode( ret ) ) return ret;
    return nullptr;
}

template <typename T, typename U>
bool DataflowGraph::addLink( const std::shared_ptr<PortOut<T>>& outputPort,
                             const std::shared_ptr<PortIn<U>>& inputPort ) {
    using namespace Ra::Core::Utils;

    static_assert( std::is_same_v<T, U>, "in and out port type mismatch" );
    auto nodeFrom = outputPort->getNode();
    auto nodeTo   = inputPort->getNode();

    if ( !checkNodeValidity( nodeFrom, nodeTo ) ) { return false; }

    if ( inputPort->isLinked() ) {
        Log::alreadyLinked( nodeTo, inputPort.get() );
        return false;
    }
    inputPort->connect( outputPort.get() );

    // The state of the graph changes, set it to not ready
    needsRecompile();
    return true;
}

inline void DataflowGraph::needsRecompile() {
    m_shouldBeSaved = true;
    m_ready         = false;
}

inline const std::string& DataflowGraph::getTypename() {
    static std::string demangledTypeName { "Core DataflowGraph" };
    return demangledTypeName;
}

} // namespace Core
} // namespace Dataflow
} // namespace Ra
