#include <Dataflow/Core/NodeFactory.hpp>

#include <Dataflow/Core/DataflowGraph.hpp>
#include <memory>

namespace Ra {
namespace Dataflow {
namespace Core {

namespace NodeFactoriesManager {
const std::string dataFlowBuiltInsFactoryName { "DataFlowBuiltIns" };
}

NodeFactory::NodeFactory( std::string name ) : m_name( std::move( name ) ) {}

auto NodeFactory::getName() const -> std::string {
    return m_name;
}

auto NodeFactory::createNode( const std::string& nodeType,
                              const nlohmann::json& data,
                              DataflowGraph* owningGraph ) -> std::shared_ptr<Node> {
    if ( auto itr = m_nodesCreators.find( nodeType ); itr != m_nodesCreators.end() ) {
        auto node = std::shared_ptr<Node> { itr->second.first( data ) };
        if ( owningGraph != nullptr ) { owningGraph->addNode( node ); }
        return node;
    }
    return nullptr;
}

auto NodeFactory::registerNodeCreator( const std::string& nodeType,
                                       NodeCreatorFunctor nodeCreator,
                                       const std::string& nodeCategory ) -> bool {

    if ( auto itr = m_nodesCreators.find( nodeType ); itr == m_nodesCreators.end() ) {
        m_nodesCreators[nodeType] = { std::move( nodeCreator ), nodeCategory };
        return true;
    }
    LOG( Ra::Core::Utils::logWARNING )
        << "NodeFactory (" << getName()
        << ") : trying to add an already existing node creator for type " << nodeType << ".";
    return false;
}

auto NodeFactory::nextNodeId() -> size_t {
    return ++m_nodesCreated;
}

auto NodeFactorySet::createNode( const std::string& nodeType,
                                 const nlohmann::json& data,
                                 DataflowGraph* owningGraph ) -> std::shared_ptr<Node> {
    for ( const auto& itr : m_factories ) {
        if ( auto node = itr.second->createNode( nodeType, data, owningGraph ); node ) {
            return node;
        }
    }
    LOG( Ra::Core::Utils::logERROR ) << "NodeFactorySet: unable to find constructor for "
                                     << nodeType << " in any managed factory.";
    return nullptr;
}

namespace NodeFactoriesManager {

/**
 * \brief Allow static initialization without init order problems
 * \return The manager singleton
 */
auto getFactoryManager() -> NodeFactorySet& {
    static NodeFactorySet s_factoryManager {};
    return s_factoryManager;
}

auto registerFactory( NodeFactorySet::mapped_type factory ) -> bool {
    return getFactoryManager().addFactory( std::move( factory ) );
}

auto createFactory( const NodeFactorySet::key_type& name ) -> NodeFactorySet::mapped_type {
    auto factory = getFactory( name );
    if ( factory == nullptr ) {
        factory = std::make_shared<NodeFactory>( name );
        registerFactory( factory );
    }
    return factory;
}

auto getFactory( const NodeFactorySet::key_type& name ) -> NodeFactorySet::mapped_type {
    auto& fctMgr = getFactoryManager();
    if ( auto factory = fctMgr.find( name ); factory != fctMgr.end() ) { return factory->second; }
    return nullptr;
}

auto unregisterFactory( const NodeFactorySet::key_type& name ) -> bool {
    return getFactoryManager().removeFactory( name );
}

auto getDataFlowBuiltInsFactory() -> NodeFactorySet::mapped_type {
    return getFactory( NodeFactoriesManager::dataFlowBuiltInsFactoryName );
}

} // namespace NodeFactoriesManager

} // namespace Core
} // namespace Dataflow
} // namespace Ra
