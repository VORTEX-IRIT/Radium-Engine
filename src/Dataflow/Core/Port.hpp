#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/TypeDemangler.hpp>

#include <string>
#include <typeinfo>

#include <Core/Utils/Observable.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

class Node;

/**
 * \brief Base class for nodes' ports
 * A port is a strongly typed extremity of connections between nodes.
 * \warning when comparing and using typed port, beware of the const qualifier
 * that is not always exposed by the C++ type system. There are some undefined behavior concerning
 * const_casts and const qualifier in the C++ documentation
 * (https://en.cppreference.com/w/cpp/language/const_cast).
 *
 */
class RA_DATAFLOW_API PortBase
{
  private:
    /// The name of the port.
    std::string m_name { "" };
    /// The port's data's type's index.
    std::type_index m_type;
    /// A pointer to the node this port belongs to.
    Node* m_node { nullptr }; /// \todo switch to shared_ptr ?

  protected:
    /// Flag that tells if the port is linked.
    bool m_isLinked { false };
    /// Flag that tells if the port must have a connection
    bool m_isLinkMandatory { false };

  public:
    /// \name Constructors
    /// @{
    /// \brief delete default constructors.
    /// \see https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual
    /// Constructors.
    PortBase()                             = delete;
    PortBase( const PortBase& )            = delete;
    PortBase& operator=( const PortBase& ) = delete;

    /// @param name The name of the port.
    /// @param type The data's type's hash.
    /// @param node The pointer to the node associated with the port.
    PortBase( const std::string& name, std::type_index type, Node* node );
    /// @}

    /// \brief make PortBase a base abstract class
    virtual ~PortBase() = default;

    /// Gets the port's name.
    const std::string& getName() const;
    /// Gets the type of the data (efficient for comparisons).
    std::type_index getType() const;
    /// Gets a pointer to the node this port belongs to.
    Node* getNode() const;
    virtual bool hasData();
    // TODO : getData() to avoid dynamic_cast to get the data of the PortOut.
    /// Returns true if the port is linked
    bool isLinked() const;
    /// Returns true if the port is flagged as being mandatory linked
    bool isLinkMandatory() const;
    /// Flags the port as being mandatory linked
    void mustBeLinked();
    virtual PortBase* getLink() = 0;
    virtual bool accept( PortBase* other );
    virtual bool connect( PortBase* other ) = 0;
    virtual bool disconnect()               = 0;
    /// Returns a reflected (In <-> Out) port of the same type
    virtual PortBase* reflect( Node* node, std::string name ) = 0;
    /// Returns true if the port is an input port
    virtual bool is_input() { return false; }

    /// Allows to get data stored at this port if it is an output port.
    /// This method copy the data onto the given object
    /// @params t The reference to store the data of this port
    template <typename T>
    void getData( T& t );

    /// Allows to get data stored at this port if it is an output port.
    /// This method do not copy the data but gives a reference to the transmitted object.
    /// TODO Verify the robustness of this
    /// @params t The reference to store the data of this port
    template <typename T>
    T& getData();

    /// Check if this port is an output port, then takes a pointer to the data this port will point
    /// to.
    /// @param data The pointer to the data.
    template <typename T>
    void setData( T* data );

    std::string getTypeName() const;
};

/**
 * \brief Output port delivering data of Type T.
 * Output port stores a non-owning pointer to the data that will be made available on a connection.
 * \tparam T The type of the delivered data.
 */
template <typename T>
class PortOut : public PortBase
{
  private:
    /// The data the port points to.

    /// Use raw ptr, data belongs to node and can be plain stack variable
    T* m_data { nullptr };

  public:
    using DataType = T;
    /// \name Constructors
    /// @{
    /// \brief delete default constructors.
    PortOut()                            = delete;
    PortOut( const PortOut& )            = delete;
    PortOut& operator=( const PortOut& ) = delete;
    /// Constructor.
    /// @param name The name of the port.
    /// @param node The pointer to the node associated with the port.
    /// \todo remove this one
    PortOut( const std::string& name, Node* node );
    PortOut( Node* node, const std::string& name );
    PortOut( Node* node, T* data, const std::string& name );
    /// @}

    /// Gets a reference to the data this ports points to.
    T& getData();
    /// Takes a pointer to the data this port will point to.
    /// @param data The pointer to the data.
    void setData( T* data );
    /// Returns true if the pointer to the data is not null.
    bool hasData() override;
    /// Returns nullptr because this port is an out port.
    PortBase* getLink() override;
    /// Returns false because out ports can not accept connection.
    /// @param o The other port to test the connection.
    bool accept( PortBase* ) override;
    /// Calls the connect(PortBase* o) function of the o node because out ports can not connect.
    /// Also sets m_isLinked.
    /// @param o The other port to connect.
    bool connect( PortBase* o ) override;
    /// Returns false because out ports can not disconnect.
    bool disconnect() override;
    /// Returns a portIn of the same type
    PortBase* reflect( Node* node, std::string name ) override;
};

/**
 * \brief Input port accepting data of type T.
 * An input port does not staore the data but is an accessor to the data stored on the connected
 * output port. An Input port is observable and notify its observers at each  connect/disconnect
 * event. \tparam T The accepted data type
 */
template <typename T>
class PortIn : public PortBase,
               public Ra::Core::Utils::Observable<const std::string&, const PortIn<T>&, bool>
{
  private:
    /// A pointer to the out port this port is connected to.
    PortOut<T>* m_from = nullptr;

  public:
    using DataType = T;
    /// \name Constructors
    /// @{
    /// \brief delete default constructors.
    PortIn()                           = delete;
    PortIn( const PortIn& )            = delete;
    PortIn& operator=( const PortIn& ) = delete;
    /// Constructor.
    /// @param name The name of the port.
    /// @param node The pointer to the node associated with the port.
    ///\todo remove this one
    PortIn( const std::string& name, Node* node );
    PortIn( Node* node, const std::string& name );
    /// @}

    /// Gets the out port this port is connected to.
    PortBase* getLink() override;
    /// Returns true if the port is linked to an output port that has data.
    bool hasData() override;
    /// Gets a reference to the data pointed by the connected out port.
    /// \note no verification is made about the availability of the data.
    T& getData();
    /// Checks if there is not out port already connected and if the data types are the same.
    /// @param o The other port to test the connection
    bool accept( PortBase* other ) override;
    /// Connects this in port and the other out port if there is no out port already connected and
    /// if the data types are the same. Also sets m_isLinked.
    /// @param o The other port to connect.
    bool connect( PortBase* other ) override;
    /// Disconnects this port if it is connected.
    /// Also sets m_isLinked to false.
    bool disconnect() override;
    /// Returns a portOut of the same type
    PortBase* reflect( Node* node, std::string name ) override;
    /// Returns true if the port is an input port
    bool is_input() override;
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

inline PortBase::PortBase( const std::string& name, std::type_index type, Node* node ) :
    m_name( name ), m_type( type ), m_node( node ) {}

inline const std::string& PortBase::getName() const {
    return m_name;
}

inline std::type_index PortBase::getType() const {
    return m_type;
}

inline std::string PortBase::getTypeName() const {
    return simplifiedDemangledType( m_type );
}

inline Node* PortBase::getNode() const {
    return m_node;
}

inline bool PortBase::hasData() {
    return false;
}

inline bool PortBase::isLinked() const {
    return m_isLinked;
}

inline bool PortBase::isLinkMandatory() const {
    return m_isLinkMandatory;
}

inline void PortBase::mustBeLinked() {
    m_isLinkMandatory = true;
}

inline bool PortBase::accept( PortBase* other ) {
    return m_type == other->getType();
}

template <typename T>
PortOut<T>::PortOut( const std::string& name, Node* node ) : PortBase( name, typeid( T ), node ) {}

template <typename T>
PortOut<T>::PortOut( Node* node, const std::string& name ) : PortBase( name, typeid( T ), node ) {}

template <typename T>
PortOut<T>::PortOut( Node* node, T* data, const std::string& name ) :
    PortBase( name, typeid( T ), node ), m_data { data } {}

template <typename T>
T& PortOut<T>::getData() {
    return *m_data;
}

template <typename T>
void PortOut<T>::setData( T* data ) {
    m_data = data;
}

template <typename T>
bool PortOut<T>::hasData() {
    return m_data != nullptr;
}

template <typename T>
PortBase* PortOut<T>::getLink() {
    return nullptr;
}

template <typename T>
bool PortOut<T>::accept( PortBase* ) {
    return false;
}

template <typename T>
bool PortOut<T>::connect( PortBase* o ) {
    m_isLinked = o->connect( this );
    return m_isLinked;
}

template <typename T>
void PortBase::setData( T* data ) {
    if ( !is_input() ) {
        auto thisOut = dynamic_cast<PortOut<T>*>( this );
        if ( thisOut ) {
            thisOut->setData( data );
            return;
        }
        LOG( Ra::Core::Utils::logERROR )
            << "Unable to set data with type " << simplifiedDemangledType( *data ) << " on port "
            << getName() << " which expect " << getTypeName() << ".\n";
        std::abort();
    }
    LOG( Ra::Core::Utils::logERROR )
        << "Could not call  PortBase::setData(T* data) on the input port " << getName() << ".\n";
    std::abort();
}

template <typename T>
void PortBase::getData( T& t ) {
    if ( !is_input() ) {
        auto thisOut = dynamic_cast<PortOut<T>*>( this );
        if ( thisOut ) {
            t = thisOut->getData();
            return;
        }
        LOG( Ra::Core::Utils::logERROR )
            << "Unable to get data with type " << simplifiedDemangledType<T>() << " on port "
            << getName() << " which expect " << getTypeName() << ".\n";
        std::abort();
    }
    LOG( Ra::Core::Utils::logERROR )
        << "Could not call PortBase::getData( T& t ) on the input port " << getName() << ".\n";
    std::abort();
}

template <typename T>
T& PortBase::getData() {
    if ( !is_input() ) {
        auto thisOut = dynamic_cast<PortOut<T>*>( this );
        if ( thisOut ) { return thisOut->getData(); }
        LOG( Ra::Core::Utils::logERROR )
            << "Unable to get data with type " << simplifiedDemangledType<T>() << " on port "
            << getName() << " which expect " << getTypeName() << ".\n";
        std::abort();
    }
    LOG( Ra::Core::Utils::logERROR )
        << "Could not call T& PortBase::getData<T>() on the input port " << getName() << ".\n";
    std::abort();
}

template <typename T>
bool PortOut<T>::disconnect() {
    return false;
}

template <typename T>
PortBase* PortOut<T>::reflect( Node* node, std::string name ) {
    return new PortIn<DataType>( name, node );
}

/**
 * PortIn is an Observable<PortIn<T>&> that notifies its observers at connect/disconnect event
 * @tparam T
 */
template <typename T>
PortIn<T>::PortIn( const std::string& name, Node* node ) : PortBase( name, typeid( T ), node ) {}

template <typename T>
PortIn<T>::PortIn( Node* node, const std::string& name ) : PortBase( name, typeid( T ), node ) {}

template <typename T>
PortBase* PortIn<T>::getLink() {
    return m_from;
}

template <typename T>
T& PortIn<T>::getData() {
    return m_from->getData();
}

template <typename T>
inline bool PortIn<T>::hasData() {
    if ( isLinked() ) { return m_from->hasData(); }
    return false;
}

template <typename T>
bool PortIn<T>::accept( PortBase* other ) {
    if ( !m_from && ( other->getType() == getType() ) ) { return PortBase::accept( other ); }
    return false;
}

template <typename T>
bool PortIn<T>::connect( PortBase* other ) {
    if ( accept( other ) ) {
        m_from     = static_cast<PortOut<T>*>( other );
        m_isLinked = true;
        // notify after connect
        this->notify( getName(), *this, true );
    }
    return m_isLinked;
}

template <typename T>
bool PortIn<T>::disconnect() {
    if ( m_isLinked ) {
        // notify before disconnect
        this->notify( getName(), *this, false );
        m_from     = nullptr;
        m_isLinked = false;
        return true;
    }
    return false;
}
template <typename T>
PortBase* PortIn<T>::reflect( Node* node, std::string name ) {
    return new PortOut<DataType>( name, node );
}

template <typename T>
bool PortIn<T>::is_input() {
    return true;
}

} // namespace Core
} // namespace Dataflow
} // namespace Ra
