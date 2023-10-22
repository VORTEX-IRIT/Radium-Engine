#pragma once
#include <Dataflow/Core/Node.hpp>

#include <functional>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace Functionals {

/** \brief Filter on iterable collection.
 * \tparam coll_t the collection to filter. Must respect the SequenceContainer requirements
 * \tparam v_t (optional), type of the element in the collection. Default to coll_t::value_type
 * \see https://en.cppreference.com/w/cpp/named_req/SequenceContainer
 *
 * This node apply an operator f on its input such that to keep only elements validated by a
 * predicate :
 *
 * This node has two inputs :
 *   - in : port accepting the input data of type coll_t. Must be linked.
 *   - f : port accepting an operator with profile std::function<bool( const v_t& )>.
 *   Link to this port is not mandatory, the operator might be set once for the node.
 *   If this port is linked, the operator will be taken from the port.
 *
 * This node has one output :
 *   - out : port giving a coll_t such that out = std::copy_if(a, f)
 */
template <typename coll_t, typename v_t = typename coll_t::value_type>
class FilterNode : public Node
{
  public:
    /**
     * unaryPredicate Type
     */
    using UnaryPredicate = std::function<bool( const v_t& )>;

    /**
     * \brief Construct a filter accepting all its input ( true() lambda )
     * \param instanceName
     */
    explicit FilterNode( const std::string& instanceName );

    /**
     * \brief Construct a filter with the given predicate
     * \param instanceName
     * \param predicate
     */
    FilterNode( const std::string& instanceName, UnaryPredicate predicate );

    void init() override;
    bool execute() override;

    /// Sets the filtering predicate on the node
    void setFilterFunction( UnaryPredicate predicate );
    Node::PortInPtr<coll_t> getInPort() { return m_portIn; }
    Node::PortInPtr<UnaryPredicate> getPredicatePort() { return m_portPredicate; }
    Node::PortOutPtr<coll_t> getOutPort() { return m_portOut; }

  protected:
    FilterNode( const std::string& instanceName,
                const std::string& typeName,
                UnaryPredicate predicate );

    void toJsonInternal( nlohmann::json& data ) const override { Node::toJsonInternal( data ); }
    bool fromJsonInternal( const nlohmann::json& data ) override {
        return Node::fromJsonInternal( data );
    }

  private:
    UnaryPredicate m_predicate;
    coll_t m_elements;

    /// @{
    /// \brief Alias for the ports (allow simpler access)
    Node::PortInPtr<coll_t> m_portIn;
    Node::PortInPtr<UnaryPredicate> m_portPredicate;
    Node::PortOutPtr<coll_t> m_portOut;
    /// @}
  public:
    static const std::string& getTypename();
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

template <typename coll_t, typename v_t>
FilterNode<coll_t, v_t>::FilterNode( const std::string& instanceName ) :
    FilterNode( instanceName, getTypename(), []( v_t ) { return true; } ) {}

template <typename coll_t, typename v_t>
FilterNode<coll_t, v_t>::FilterNode( const std::string& instanceName, UnaryPredicate predicate ) :
    FilterNode( instanceName, getTypename(), predicate ) {}

template <typename coll_t, typename v_t>
void FilterNode<coll_t, v_t>::setFilterFunction( UnaryPredicate predicate ) {
    m_predicate = predicate;
}

template <typename coll_t, typename v_t>
void FilterNode<coll_t, v_t>::init() {
    Node::init();
    m_elements.clear();
}

template <typename coll_t, typename v_t>
bool FilterNode<coll_t, v_t>::execute() {
    auto f = m_portPredicate->isLinked() ? m_portPredicate->getData() : m_predicate;
    // The following test will always be true if the node was integrated in a compiled graph
    if ( m_portIn->isLinked() ) {
        const auto& inData = m_portIn->getData();
        m_elements.clear();
        // m_elements.reserve( inData.size() ); // --> this is not a requirement of
        // SequenceContainer
        std::copy_if( inData.begin(), inData.end(), std::back_inserter( m_elements ), f );
    }
    return true;
}

template <typename coll_t, typename v_t>
const std::string& FilterNode<coll_t, v_t>::getTypename() {
    static std::string demangledName =
        std::string { "Filter<" } + Ra::Dataflow::Core::simplifiedDemangledType<coll_t>() + ">";
    return demangledName;
}

template <typename coll_t, typename v_t>
FilterNode<coll_t, v_t>::FilterNode( const std::string& instanceName,
                                     const std::string& typeName,
                                     UnaryPredicate filterFunction ) :
    Node( instanceName, typeName ),
    m_predicate( filterFunction ),
    m_portIn { addInputPort<coll_t>( "in" ) },
    m_portPredicate { addInputPort<UnaryPredicate>( "f" ) },
    m_portOut { addOutputPort<coll_t>( &m_elements, "out" ) } {
    m_portIn->mustBeLinked();
}

} // namespace Functionals
} // namespace Core
} // namespace Dataflow
} // namespace Ra
