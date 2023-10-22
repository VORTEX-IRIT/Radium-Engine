#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/Node.hpp>

#include <Core/Utils/StdOptional.hpp>

#include <functional>
#include <iostream>
#include <unordered_map>

namespace Ra {
namespace Dataflow {
namespace Core {

class DataflowGraph;

/**
 * NodeFactory store a set of functions allowing to dynamically create dataflow nodes.
 * A NodeFactory is used when loading a node graph from a json representation of the graph to
 * instantiate all the loaded nodes.
 * Each DataflowGraph must have a reference to the nodeFactory to be used to create all the node he
 * has.
 *
 */
class RA_DATAFLOW_API NodeFactory
{
  public:
    /** Creates an empty factory with the given name */
    explicit NodeFactory( std::string name );

    [[nodiscard]] auto getName() const -> std::string;

    /** Function that creates and initialize a node.
     *  Typical implementation of such a function should do the following :
     *   {
     *     auto node = new ConcreteNodeType( unique_instance_name );
     *     node->fromJson( data);
     *     return node;
     *   }
     */
    using NodeCreatorFunctor = std::function<std::shared_ptr<Node>( const nlohmann::json& data )>;

    /**
     * Associate, for a given concrete node type, a custom NodeCreatorFunctor
     * @tparam T Concrete node type identifier
     * \param nodeCreator Functor to create an node of the corresponding concrete node type.
     * \param nodeCategory Category of the node.
     * \return true if the node creator is successfully added, false if not (e.g. due to a name
     * collision).
     */
    template <typename T>
    auto registerNodeCreator( NodeCreatorFunctor nodeCreator,
                              const std::string& nodeCategory = "RadiumNodes" ) -> bool;

    /**
     * Associate, for a given concrete node type, a generic NodeCreatorFunctor
     * @tparam T Concrete node type identifier
     * \param instanceNamePrefix prefix of the node instance name (will be called "prefix_i" with i
     * a unique number.
     * \param nodeCategory Category of the node.
     * \return true if the node creator is successfully added, false if not (e.g. due to a name
     * collision).
     */
    template <typename T>
    auto registerNodeCreator( const std::string& instanceNamePrefix,
                              const std::string& nodeCategory = "RadiumNodes" ) -> bool;
    /**
     * Associate, for a given concrete node type name, a NodeCreatorFunctor
     * \param nodeType the name of the concrete type
     * (the same as what is obtained by T::getTypename() on a node of type T)
     * \param nodeCreator Functor to create an node of the corresponding concrete node type.
     * \return true if the node creator is successfully added, false if not (e.g. due to a name
     * collision).
     */
    auto registerNodeCreator( const std::string& nodeType,
                              NodeCreatorFunctor nodeCreator,
                              const std::string& nodeCategory = "RadiumNodes" ) -> bool;

    /**
     * Get an unique, increasing node id.
     * \return
     */
    auto nextNodeId() -> size_t;

    /** Create a node of the requested type.
     * The node is filled with the given json content.
     * If owningGraph is non null, the node is added to this graph
     * \param nodeType Type name of the node to be created.
     * \param data json representation of the node data (might be empty)
     * \param owningGraph if non null, the node is added to ths graph
     * \return the new node. Ownership of the returned pointer is left to the caller.
     */
    [[nodiscard]] auto createNode( const std::string& nodeType,
                                   const nlohmann::json& data,
                                   DataflowGraph* owningGraph = nullptr ) -> std::shared_ptr<Node>;

    /**
     * The type of the associative container used to store the factory
     * this container associate a concrete node type name to a pair
     * <concrete instantiation functor, name of the node category>
     * The name of the node category is helpful for graphical NodeGraph editor.
     * By default this is set to be "RadiumNodes"
     */
    using ContainerType =
        std::unordered_map<std::string, std::pair<NodeCreatorFunctor, std::string>>;
    /**
     * Get a const reference on the associative map
     * \return
     */
    [[nodiscard]] auto getFactoryMap() const -> const ContainerType&;

  private:
    ContainerType m_nodesCreators;
    size_t m_nodesCreated { 0 };
    std::string m_name;
};

/**
 * NodeFactorySet store a set of NodeFactory
 */
class RA_DATAFLOW_API NodeFactorySet
{
  public:
    using container_type = std::map<std::string, std::shared_ptr<NodeFactory>>;

    using key_type    = container_type::key_type;
    using mapped_type = container_type::mapped_type;
    using value_type  = container_type::value_type;

    using const_iterator = container_type::const_iterator;
    using iterator       = container_type::iterator;

    /**
     * Add a factory to the set of factories available
     * \param factory the factory
     * \return true if the factory was inserted, false if the insertion was prevented by an
     * already existing factory with the same name.
     */
    auto addFactory( mapped_type factory ) -> bool;

    /**
     * \brief Test if a factory exists in the set with the given name
     * \param name The name of the factory to search for
     * \return an optional that is empty (evaluates to false) if no factory exists with the given
     * name or that contains the existing factory.
     */
    auto hasFactory( const key_type& name ) -> Ra::Core::Utils::optional<mapped_type>;

    /**
     * \brief Remove the identified factory from the set
     * \param name the name of the factory to remove
     * \return true if the factory was removed, false if the factory does not exist in the set.
     */
    auto removeFactory( const key_type& name ) -> bool;

    /**
     *
     * \return the created node, nullptr if there is no construction functor registered for the
     * type.
     */
    /**
     * \brief Create a node using one of the functor (if it exists) registered in one factory for
     * the given type name. \param nodeType name of the node type (as simplified by Radium
     * demangler) to create \param data json data to fill the created node \param owningGraph Graph
     * in which the node should be added, if not nullptr. \return The created node, nullptr in case
     * of failure
     */
    [[nodiscard]] auto createNode( const std::string& nodeType,
                                   const nlohmann::json& data,
                                   DataflowGraph* owningGraph = nullptr ) -> std::shared_ptr<Node>;

    /* Wrappers to the interface of the underlying container
     * see https://en.cppreference.com/w/cpp/container/map
     */
    auto begin() const -> const_iterator;
    auto end() const -> const_iterator;
    auto cbegin() const -> const_iterator;
    auto cend() const -> const_iterator;
    auto find( const key_type& key ) const -> const_iterator;
    auto insert( value_type value ) -> std::pair<iterator, bool>;
    auto erase( const key_type& key ) -> size_t;

  private:
    container_type m_factories;
};

/**
 * Implement a NodeFactoryManager that stores a set of factories available to the  system.
 * Such a manager will be populated with Core::Dataflow node factories (Specialized sources,
 * specialized sink, ...) and will allow users to register its own factories.
 *
 * When creating a graph, the set of needed factories should be given as a constructor parameter
 * or built by adding factories identifier to the graph.
 *
 * When a graph is saved, the name of the factories he needs will be exported as string array
 * in the json.
 *
 * When a graph is loaded, the set of factories is built using the factories array in the json.
 *
 * @note: the factory name "DataFlowBuiltIns" is reserved and correspond to the base nodes available
 * for each dataflow graph (Specialized sources, specialized sink, ...). This factory will be
 * automatically added to all created factory set.
 */
namespace NodeFactoriesManager {
/** Names of the system Builtins factories (automatically added to each graph) */
extern const std::string dataFlowBuiltInsFactoryName;

RA_DATAFLOW_API auto getFactoryManager() -> NodeFactorySet&;

/** Register a factory into the manager.
 * The key will be fetched from the factory (its name)
 */
/**
 * \brief Register a factory into the manager.
 * The key will be fetched from the factory (its name)
 * \param factory
 * \return true if the factory was registered, false if not (e.g. due to name collision).
 */
RA_DATAFLOW_API auto registerFactory( NodeFactorySet::mapped_type factory ) -> bool;

/**
 * \brief Create and register a factory to the manager.
 * \param name The name of the factory to create
 * \return a configurable factory.
 */
RA_DATAFLOW_API auto createFactory( const NodeFactorySet::key_type& name )
    -> NodeFactorySet::mapped_type;

/**
 * \brief Gets the given factory from the manager
 * \param name The name of the factory to get
 * \return  a shared_ptr to the requested factory, nullptr if the factory does not exist.
 */
RA_DATAFLOW_API auto getFactory( const NodeFactorySet::key_type& name )
    -> NodeFactorySet::mapped_type;

/**
 * \brief Unregister the factory from the manager
 * \param name The name of the factory to unregister
 * \return true if the factory was unregistered, false if not (e.g. for names not being managed).
 */
RA_DATAFLOW_API auto unregisterFactory( const NodeFactorySet::key_type& name ) -> bool;

/**
 * \brief Gets the factory for nodes exported by the Core dataflow library.
 * \return
 */
RA_DATAFLOW_API auto getDataFlowBuiltInsFactory() -> NodeFactorySet::mapped_type;
} // namespace NodeFactoriesManager

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

template <typename T>
auto NodeFactory::registerNodeCreator( NodeCreatorFunctor nodeCreator,
                                       const std::string& nodeCategory ) -> bool {
    return registerNodeCreator( T::getTypename(), std::move( nodeCreator ), nodeCategory );
}

template <typename T>
auto NodeFactory::registerNodeCreator( const std::string& instanceNamePrefix,
                                       const std::string& nodeCategory ) -> bool {
    return registerNodeCreator(
        T::getTypename(),
        [this, instanceNamePrefix]( const nlohmann::json& data ) {
            std::string instanceName;
            if ( data.contains( "instance" ) ) { instanceName = data["instance"]; }
            else { instanceName = instanceNamePrefix + std::to_string( this->nextNodeId() ); }
            auto node = std::make_shared<T>( instanceName );
            node->fromJson( data );
            return node;
        },
        nodeCategory );
}

inline auto NodeFactory::getFactoryMap() const -> const NodeFactory::ContainerType& {
    return m_nodesCreators;
}

inline auto NodeFactorySet::addFactory( NodeFactorySet::mapped_type factory ) -> bool {
    const auto [loc, inserted] = insert( { factory->getName(), std::move( factory ) } );
    return inserted;
}

inline auto NodeFactorySet::hasFactory( const NodeFactorySet::key_type& name )
    -> Ra::Core::Utils::optional<NodeFactorySet::mapped_type> {
    if ( auto fct = m_factories.find( name ); fct != m_factories.end() ) { return fct->second; }
    return {};
}

inline auto NodeFactorySet::removeFactory( const NodeFactorySet::key_type& name ) -> bool {
    return erase( name );
}
inline auto NodeFactorySet::begin() const -> NodeFactorySet::const_iterator {
    return m_factories.begin();
}
inline auto NodeFactorySet::end() const -> NodeFactorySet::const_iterator {
    return m_factories.end();
}
inline auto NodeFactorySet::cbegin() const -> NodeFactorySet::const_iterator {
    return m_factories.cbegin();
}
inline auto NodeFactorySet::cend() const -> NodeFactorySet::const_iterator {
    return m_factories.cend();
}
inline auto NodeFactorySet::find( const NodeFactorySet::key_type& key ) const
    -> NodeFactorySet::const_iterator {
    return m_factories.find( key );
}
inline auto NodeFactorySet::insert( NodeFactorySet::value_type value )
    -> std::pair<NodeFactorySet::iterator, bool> {
    return m_factories.insert( std::move( value ) );
}
inline auto NodeFactorySet::erase( const NodeFactorySet::key_type& key ) -> size_t {
    return m_factories.erase( key );
}

} // namespace Core
} // namespace Dataflow
} // namespace Ra
