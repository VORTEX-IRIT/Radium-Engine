#pragma once
#include <Dataflow/Core/Node.hpp>

#include <functional>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace Functionals {

/** \brief Transform an iterable collection
 * \tparam coll_t the collection to transform. Must respect the SequenceContainer requirements
 * \tparam v_t (optional), type of the element in the collection. Default to coll_t::value_type
 * \see https://en.cppreference.com/w/cpp/named_req/SequenceContainer
 *
 * This node has two inputs :
 *   - in : port accepting a coll_t data. Linking to this port is mandatory
 *   - f : port accepting an operator with profile std::function<v_t( const v_t& )>.
 *   Linking to this port is not mandatory, the operator might be set once for the node.
 *   If this port is linked, the operator will be taken from the port.
 *
 * This node has one output :
 *   - out : port giving a coll_t such that out = std::transform(in, f)
 */
template <typename coll_t, typename v_t = typename coll_t::value_type>
class TransformNode : public Node
{
  public:
    /**
     * Trasformation operator profile
     */
    using TransformOperator = std::function<v_t( const v_t& )>;

    /**
     * \brief Construct an identity transformer
     * \param instanceName
     */
    explicit TransformNode( const std::string& instanceName );

    /**
     * \brief Construct a transformer with the given operator
     * \param instanceName
     * \param filterFunction
     */
    TransformNode( const std::string& instanceName, TransformOperator op );

    void init() override;
    bool execute() override;

    /// Sets the operator on the node
    void setOperator( TransformOperator op );

  protected:
    TransformNode( const std::string& instanceName,
                   const std::string& typeName,
                   TransformOperator op );

    void toJsonInternal( nlohmann::json& data ) const override { Node::toJsonInternal( data ); }
    bool fromJsonInternal( const nlohmann::json& data ) override {
        return Node::fromJsonInternal( data );
    }

  private:
    TransformOperator m_operator;
    coll_t m_elements;

    /// @{
    /// \brief Alias for the ports (allow simpler access)
    PortInPtr<coll_t> m_portIn;
    PortInPtr<TransformOperator> m_portOperator;
    PortOutPtr<coll_t> m_portOut;
    /// @}
  public:
    static const std::string& getTypename();
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

template <typename coll_t, typename v_t>
TransformNode<coll_t, v_t>::TransformNode( const std::string& instanceName ) :
    TransformNode( instanceName, getTypename(), []( v_t ) { return v_t {}; } ) {}

template <typename coll_t, typename v_t>
TransformNode<coll_t, v_t>::TransformNode( const std::string& instanceName, TransformOperator op ) :
    TransformNode( instanceName, getTypename(), op ) {}

template <typename coll_t, typename v_t>
void TransformNode<coll_t, v_t>::setOperator( TransformOperator op ) {
    m_operator = op;
}

template <typename coll_t, typename v_t>
void TransformNode<coll_t, v_t>::init() {
    Node::init();
    m_elements.clear();
}

template <typename coll_t, typename v_t>
bool TransformNode<coll_t, v_t>::execute() {
    auto f = m_portOperator->isLinked() ? m_portOperator->getData() : m_operator;
    // The following test will always be true if the node was integrated in a compiled graph
    if ( m_portIn->isLinked() ) {
        const auto& inData = m_portIn->getData();
        m_elements.clear();
        // m_elements.reserve( inData.size() ); // --> this is not a requirement of
        // SequenceContainer
        std::transform( inData.begin(), inData.end(), std::back_inserter( m_elements ), f );
    }
    return true;
}

template <typename coll_t, typename v_t>
const std::string& TransformNode<coll_t, v_t>::getTypename() {
    static std::string demangledName =
        std::string { "Transform<" } + Ra::Dataflow::Core::simplifiedDemangledType<coll_t>() + ">";
    return demangledName;
}

template <typename coll_t, typename v_t>
TransformNode<coll_t, v_t>::TransformNode( const std::string& instanceName,
                                           const std::string& typeName,
                                           TransformOperator op ) :
    Node( instanceName, typeName ),
    m_operator( op ),
    m_portIn { addInputPort<coll_t>( "in" ) },
    m_portOperator { addInputPort<TransformOperator>( "f" ) },
    m_portOut { addOutputPort<coll_t>( &m_elements, "out" ) } {
    m_portIn->mustBeLinked();
}

} // namespace Functionals
} // namespace Core
} // namespace Dataflow
} // namespace Ra
