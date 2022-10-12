#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/EditableParameter.hpp>
#include <Dataflow/Core/Port.hpp>

#include <nlohmann/json.hpp>

#include <uuid.h>

#include <cstdint>
#include <iostream>
#include <memory>
#include <string>
#include <typeinfo>
#include <vector>

namespace Ra {
namespace Dataflow {
namespace Core {

/** \brief Base abstract class for all the nodes added and used by the node system.
 * A node represent a function acting on some input data and generating some outputs.
 * To build a computation graph, nodes should be added to the graph, which is itself a node
 * (\see Ra::Dataflow::Core::DataflowGraph) and linked together through their input/output port.
 *
 * Nodes computes their function using the input data collecting from the input ports,
 * in an evaluation context (possibly empty) defined byt their internal data to generate results
 * sent to their output ports.
 *
 */
class RA_DATAFLOW_API Node
{
  public:
    /// \name Constructors
    /// @{
    /// \brief delete default constructors.
    /// \see https://isocpp.github.io/CppCoreGuidelines/CppCoreGuidelines#Rc-copy-virtual
    Node()              = delete;
    Node( const Node& ) = delete;
    Node& operator=( const Node& ) = delete;
    /// @}

    /// \brief make Node a base abstract class
    virtual ~Node() = default;

    /// \brief Two nodes are considered equal if there type and instance names are the same.
    bool operator==( const Node& o_node );

    /// \name Function execution control
    /// @{
    /// \brief Initializes the node content
    /// The init() function should be called once at the beginning of the lifetime of the node by
    /// the owner of the node (the graph which contains the node).
    /// Its goal is to initialize the node's internal data if any.
    /// The base version do nothing.
    virtual void init();

    /// \brief Compile the node to check its validity
    /// Only nodes defining a full computation graph will need to override this method.
    /// The base version do nothing.
    /// \return the compilation status
    virtual bool compile();

    /// \brief Executes the node.
    /// Execute the node function on the input ports (to be fetched) and write the results to the
    /// output ports.
    virtual void execute() = 0;

    /// \brief delete the node content
    /// The destroy() function is called once at the end of the lifetime of the node.
    /// Its goal is to free the internal data that have been allocated.
    virtual void destroy();
    /// @}

    /// \name Control the interfaces of the nodes (inputs, outputs, internal data, ...)
    /// @{
    /// \brief Gets the in ports of the node.
    const std::vector<std::unique_ptr<PortBase>>& getInputs();

    /// \brief Gets the out ports of the node.
    const std::vector<std::unique_ptr<PortBase>>& getOutputs();

    /// \brief Build the interface ports of the node
    const std::vector<PortBase*>& buildInterfaces( Node* parent );

    /// \brief Get the interface ports of the node
    const std::vector<PortBase*>& getInterfaces();

    /// \brief Gets the editable parameters of the node.
    /// used only by the node editor gui to build the editon widget
    const std::vector<std::unique_ptr<EditableParameterBase>>& getEditableParameters();
    /// @}

    /// \name Identification methods
    /// @{
    /// \brief Gets the type name of the node.
    const std::string& getTypeName() const;

    /// \brief Gets the instance name of the node.
    const std::string& getInstanceName() const;

    /// \brief Sets the instance name (rename) the node
    void setInstanceName( const std::string& newName );

    /// \brief Gets the UUID of the node as a string
    std::string getUuid() const;

    /// \brief Sets the UUID of the node from a valid uuid string
    /// \return true if the uuid is set, false if the node already have a valid uid or the string is
    /// invalid
    bool setUuid( const std::string& uid );
    /// @}

    /// \name Serialization of a node
    /// @{
    /// TODO : specify the json format for nodes and what is expected from the following ethods

    /// \brief serialize the content of the node.
    /// Fill the given json object with the json representation of the concrete node.
    void toJson( nlohmann::json& data ) const;

    /// \brief unserialized the content of the node.
    /// Fill the node from its json representation
    void fromJson( const nlohmann::json& data );

    /// \brief Add a metadata to the node to store application specific information.
    /// used, e.g. by the node editor gui to save node position in the graphical canvas.
    void addJsonMetaData( const nlohmann::json& data );

    /// \brief Give access to extra json data stored on the node.
    const nlohmann::json& getJsonMetaData();
    /// @}

    /// \brief Flag that checks if the node is already initialized
    bool m_initialized { false };

    /// \brief Sets the filesystem (real or virtual) location for the pass resources
    inline void setResourcesDir( std::string resourcesRootDir );

  protected:
    /// Construct the base node given its name and type
    /// \param instanceName The name of the node
    /// \param typeName The type name of the node
    Node( const std::string& instanceName, const std::string& typeName );

    /// internal json representation of the Node.
    /// Must be implemented by inheriting classes.
    /// Be careful with template specialization and function member overriding when implementing
    /// this method.
    virtual void fromJsonInternal( const nlohmann::json& ) = 0;

    /// internal json representation of the Node.
    /// Must be implemented by inheriting classes.
    /// Be careful with template specialization and function member overriding when implementing
    /// this method.
    virtual void toJsonInternal( nlohmann::json& ) const = 0;

    /// Adds an in port to the node.
    /// This function checks if the port is an input port, then if there is no in port with the same
    /// name already associated with this node.
    /// \param in The in port to add.
    bool addInput( PortBase* in );

    /// Adds an out port to the node and the data associated with it.
    /// This function checks if there is no out port with the same name already associated with this
    /// node.
    /// \param out The in port to add.
    /// \param data The data associated with the port.
    template <typename T>
    void addOutput( PortOut<T>* out, T* data );

    /// \brief Adds an editable parameter to the node if it does not already exist.
    /// \note the node will take ownership of the editable object.
    /// \param editableParameter The editable parameter to add.
    bool addEditableParameter( EditableParameterBase* editableParameter );

    /// Remove an editable parameter to the node if it does exist.
    /// \param name The name of the editable parameter to remove.
    /// \return true if the editable parameter is found and removed.
    bool removeEditableParameter( const std::string& name );

    /// \brief get a typed reference to the editable parameter.
    /// \tparam E The type of the expected editable parameter.
    /// \param name The name of the editable parameter to get.
    /// \return the pointer to the editable parameter if any, nullptr if not.
    template <typename E>
    EditableParameter<E>* getEditableParameter( const std::string& name );

    /// The deletable status of the node
    bool m_isDeletable { true };

    /// The type name of the node. Initialized once at construction
    std::string m_typeName;
    /// The instance name of the node
    std::string m_instanceName;
    /// The in ports of the node
    std::vector<std::unique_ptr<PortBase>> m_inputs;
    /// The out ports of the node
    std::vector<std::unique_ptr<PortBase>> m_outputs;
    /// The reflected ports of the node if it is only a source or sink node
    std::vector<PortBase*> m_interface;
    /// The editable parameters of the node
    std::vector<std::unique_ptr<EditableParameterBase>> m_editableParameters;

    /// Additional data on the node, added by application or gui or ...
    nlohmann::json m_extraJsonData;

    /// \brief Generates the uuid of the node
    void generateUuid();
    /// The uuid of the node
    uuids::uuid m_uuid;

    /// generator for uuid
    static bool s_uuidGeneratorInitialized;
    static uuids::uuid_random_generator* s_uidGenerator;
    static void createUuidGenerator();

  public:
    /// \brief Returns the demangled type name of the node or any human readable representation of
    /// the type name.
    /// This is a public static member each node must define to be serializable
    static const std::string& getTypename();
};

} // namespace Core
} // namespace Dataflow
} // namespace Ra

#include <Dataflow/Core/Node.inl>
