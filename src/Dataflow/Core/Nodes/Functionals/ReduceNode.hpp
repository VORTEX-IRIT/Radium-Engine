#pragma once
#include <Dataflow/Core/Node.hpp>

#include <functional>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace Functionals {

/** \brief Reduce an iterable collection using a given operator
 * \tparam coll_t the collection to reduce. Must respect the SequenceContainer requirements
 * \tparam v_t (optional), type of the element in the collection. Default to coll_t::value_type
 * \see https://en.cppreference.com/w/cpp/named_req/SequenceContainer
 *
 * This node has three inputs :
 *   - in : port accepting a coll_t data. Linking to this port is mandatory
 *   - f : port accepting an operator with profile std::function<v_t( const v_t& )>.
 *   Linking to this port is not mandatory, the operator might be set once for the node or is the
 * default value for v_t. If this port is linked, the operator will be taken from the port.
 *   - init : port accepting a v_t to serve as initial value.
 *   Linking to this port is not mandatory.
 *   If this port is linked, the initial value will be taken from the port.
 *
 * This node has one output :
 *   - out : port giving a v_t such that out = std::reduce(in, f, init) (std::accumulate if less
 * than C++17)
 */
// TODO, allow to specify the type of the reduced information. This will allow, e.g, given a
//  collection of X, to compute a tuple containing mean and standard deviation of the collection
//  as reduction (using online Welford algo).
template <typename coll_t, typename v_t = typename coll_t::value_type>
class ReduceNode : public Node
{
  public:
    /**
     * Trasformation operator profile
     */
    using ReduceOperator = std::function<v_t( const v_t&, const v_t& )>;

    /**
     * \brief Construct an identity transformer
     * \param instanceName
     */
    explicit ReduceNode( const std::string& instanceName );

    /**
     * \brief Construct a transformer with the given operator
     * \param instanceName
     * \param op
     * \param initialValue
     */
    ReduceNode( const std::string& instanceName, ReduceOperator op, v_t initialValue = v_t {} );

    void init() override;
    bool execute() override;

    /// Sets the operator on the node
    void setOperator( ReduceOperator op, v_t initialValue = v_t {} );

  protected:
    ReduceNode( const std::string& instanceName,
                const std::string& typeName,
                ReduceOperator op,
                v_t initialValue );

    void toJsonInternal( nlohmann::json& data ) const override { Node::toJsonInternal( data ); }
    bool fromJsonInternal( const nlohmann::json& data ) override {
        return Node::fromJsonInternal( data );
    }

  private:
    ReduceOperator m_operator;
    v_t m_result;
    v_t m_init;

    /// @{
    /// \brief Alias for the ports (allow simpler access)
    Node::PortInPtr<coll_t> m_portIn { new PortIn<coll_t>( "in", this ) };
    Node::PortInPtr<ReduceOperator> m_portF { new PortIn<ReduceOperator>( "f", this ) };
    Node::PortInPtr<v_t> m_portInit { new PortIn<v_t>( "init", this ) };
    Node::PortOutPtr<v_t> m_portOut { new PortOut<v_t>( "out", this ) };
    /// @}

  public:
    static const std::string& getTypename();
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

template <typename coll_t, typename v_t>
ReduceNode<coll_t, v_t>::ReduceNode( const std::string& instanceName ) :
    ReduceNode(
        instanceName,
        getTypename(),
        []( const v_t& a, const v_t& ) -> v_t { return a; },
        v_t {} ) {}

template <typename coll_t, typename v_t>
ReduceNode<coll_t, v_t>::ReduceNode( const std::string& instanceName,
                                     ReduceOperator op,
                                     v_t initialValue ) :
    ReduceNode( instanceName, getTypename(), op, initialValue ) {}

template <typename coll_t, typename v_t>
void ReduceNode<coll_t, v_t>::setOperator( ReduceOperator op, v_t initialValue ) {
    m_operator = op;
    m_init     = initialValue;
}

template <typename coll_t, typename v_t>
void ReduceNode<coll_t, v_t>::init() {
    Node::init();
    m_result = m_init;
}

template <typename coll_t, typename v_t>
bool ReduceNode<coll_t, v_t>::execute() {
    auto f   = m_portF->isLinked() ? m_portF->getData() : m_operator;
    auto iv  = m_portInit->isLinked() ? m_portInit->getData() : m_init;
    m_result = iv;
    // The following test will always be true if the node was integrated in a compiled graph
    if ( m_portIn->isLinked() ) {
        const auto& inData = m_portIn->getData();
        m_result           = std::accumulate( inData.begin(), inData.end(), iv, f );
    }
    return true;
}

template <typename coll_t, typename v_t>
const std::string& ReduceNode<coll_t, v_t>::getTypename() {
    static std::string demangledName =
        std::string { "Reduce<" } + Ra::Dataflow::Core::simplifiedDemangledType<coll_t>() + ">";
    return demangledName;
}

template <typename coll_t, typename v_t>
ReduceNode<coll_t, v_t>::ReduceNode( const std::string& instanceName,
                                     const std::string& typeName,
                                     ReduceOperator op,
                                     v_t initialValue ) :
    Node( instanceName, typeName ),
    m_operator( op ),
    m_init( initialValue ),
    m_portIn { addInputPort<coll_t>( "in" ) },
    m_portF { addInputPort<ReduceOperator>( "f" ) },
    m_portInit { addInputPort<v_t>( "init" ) },
    m_portOut { addOutputPort<v_t>( &m_result, "out" ) }

{
    m_portIn->mustBeLinked();
}

} // namespace Functionals
} // namespace Core
} // namespace Dataflow
} // namespace Ra
