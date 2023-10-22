#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <string>
#include <typeindex>
#include <vector>

#include <nlohmann/json.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

/// \brief Basic introspection for Node internal data edition
/// This base class gives key information to associate editing capabilities (and gui) to
/// an node internal data.
/// \note This class seems to be very similar in its aim than the Ra::Engine::RenderParameter and
/// their editing capabilities through Ra::Gui::ParameterSetEditor. But, in order to be more
/// general, this class does not depend on Engine.
/// \todo Unify with Ra::Engine::RenderParameter (using Core only parameters set)
///
struct RA_DATAFLOW_API EditableParameterBase {
    /// \name Constructors
    /// @{
    /// \brief delete default constructors.
    EditableParameterBase()                                          = delete;
    EditableParameterBase( const EditableParameterBase& )            = delete;
    EditableParameterBase& operator=( const EditableParameterBase& ) = delete;

    /// Construct an base editable parameter from its name and type hash
    EditableParameterBase( const std::string& name, std::type_index typeIdx );
    ///@}

    virtual ~EditableParameterBase() = default;
    std::string getName() const;
    std::type_index getType() const;
    /// Constraints on the edited data : json object describing the constraints
    /// Add constraints or associated data to the editable.
    void setConstraints( const nlohmann::json& constraints );
    /// get the constraints or the associated data from the editable.
    const nlohmann::json& getConstraints() const;

  private:
    std::string m_name { "" };
    std::type_index m_typeIdx;

    /// Constraints on the edited data
    nlohmann::json m_constraints;
};

template <typename T>
struct EditableParameter : public EditableParameterBase {
    /// \name Constructors
    /// @{
    /// \brief delete default constructors.
    EditableParameter()                                      = delete;
    EditableParameter( const EditableParameter& )            = delete;
    EditableParameter& operator=( const EditableParameter& ) = delete;

    /// Construct an editable parameter from its name and type hash
    EditableParameter( const std::string& name, T& data );
    ///@}

    /// The data to edit.
    /// This is a reference to any data stored in a node and that the user could change
    T& m_data;
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

inline EditableParameterBase::EditableParameterBase( const std::string& name,
                                                     std::type_index typeIdx ) :
    m_name( name ), m_typeIdx( typeIdx ) {}

template <typename T>
EditableParameter<T>::EditableParameter( const std::string& name, T& data ) :
    EditableParameterBase( name, typeid( T ) ), m_data( data ) {}

inline std::string EditableParameterBase::getName() const {
    return m_name;
}

inline std::type_index EditableParameterBase::getType() const {
    return m_typeIdx;
}

inline void EditableParameterBase::setConstraints( const nlohmann::json& constraints ) {
    m_constraints = constraints;
}

inline const nlohmann::json& EditableParameterBase::getConstraints() const {
    return m_constraints;
}
} // namespace Core
} // namespace Dataflow
} // namespace Ra
