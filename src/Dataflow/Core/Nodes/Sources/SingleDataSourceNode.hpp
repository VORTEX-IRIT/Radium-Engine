#pragma once
#include <Dataflow/Core/Node.hpp>

#include <iostream>
#include <nlohmann/json.hpp>
#include <utility>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace Sources {
/**
 * \brief Base class for nodes that will give access to some input data to the graph.
 * This class can be used to feed nodes on a dataflow graph with some data coming
 * from outside the graph or from the source node itself.
 *
 * The data delivered by the node can be explicitly set/get or can be made editable.
 *
 * @tparam T The type of the data to serve.
 */
template <typename T>
class SingleDataSourceNode : public Node
{
  protected:
    SingleDataSourceNode( const std::string& instanceName, const std::string& typeName );

  public:
    // warning, hacky specialization for set editable
    explicit SingleDataSourceNode( const std::string& name ) :
        SingleDataSourceNode( name, SingleDataSourceNode<T>::getTypename() ) {}

    bool execute() override;

    /** \brief Set the data to be delivered by the node.
     * @param data
     */
    void setData( T data );

    /**
     * \brief Get the delivered data
     * @return The non owning pointer (alias) to the delivered data.
     */
    T* getData() const;

    /**
     * \brief Set the delivered data editable using the given name
     * Give access to the internal data storage.
     * If the node interface is connected, the edition will not result on a propagation to the
     * graph as internal data storage will be superseeded by the data from the interface.
     * @param name Name of the data as it will appear on edition gui. If not given, the default
     * name "Data" will be used.
     */
    void setEditable( const std::string& name = "Data" );

    /**
     * \brief Remove the delivered data from being editable
     * @param name Name of the data given when calling setEditable
     */
    void removeEditable( const std::string& name = "Data" );

    std::shared_ptr<PortOut<T>> getOuputPort() { return m_portOut; }

  protected:
    bool fromJsonInternal( const nlohmann::json& data ) override {
        return Node::fromJsonInternal( data );
    }
    void toJsonInternal( nlohmann::json& data ) const override { Node::toJsonInternal( data ); }

  private:
    /// @{
    /// The data provided by the node
    /// Used to deliver (and edit) data when the interface is not connected.
    T m_localData;
    /// Ownership of this pointer is left to the caller
    T* m_data { &m_localData };
    /// @}

    /// Alias to the output port
    Node::PortOutPtr<T> m_portOut;

  public:
    static const std::string& getTypename();
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

template <typename T>
SingleDataSourceNode<T>::SingleDataSourceNode( const std::string& instanceName,
                                               const std::string& typeName ) :
    Node( instanceName, typeName ), m_portOut { addOutputPort<T>( m_data, "to" ) } {}

template <typename T>
bool SingleDataSourceNode<T>::execute() {
    // interfaces ports are at the same index as output ports
    auto interfacePort = static_cast<PortIn<T>*>( m_interface[0] );
    if ( interfacePort->isLinked() ) {
        // use external storage to deliver data
        m_data = &( interfacePort->getData() );
    }
    else {
        // use local storage to deliver data
        m_data = &m_localData;
    }
    m_portOut->setData( m_data );
    return true;
}

template <typename T>
void SingleDataSourceNode<T>::setData( T data ) {
    m_localData = std::move( data );
}

/// \todo weird to have getData do not return what is set...
template <typename T>
T* SingleDataSourceNode<T>::getData() const {
    return m_data;
}

template <typename T>
void SingleDataSourceNode<T>::setEditable( const std::string& name ) {
    Node::addEditableParameter( new EditableParameter( name, m_localData ) );
}

template <typename T>
void SingleDataSourceNode<T>::removeEditable( const std::string& name ) {
    Node::removeEditableParameter( name );
}

template <typename T>
const std::string& SingleDataSourceNode<T>::getTypename() {
    static std::string demangledTypeName =
        std::string { "Source<" } + Ra::Dataflow::Core::simplifiedDemangledType<T>() + ">";
    return demangledTypeName;
}

} // namespace Sources
} // namespace Core
} // namespace Dataflow
} // namespace Ra
