#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Core/Utils/TypesUtils.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

/// \brief Return the human readable version of the type name T with simplified radium type names
template <typename T>
auto simplifiedDemangledType() noexcept -> std::string;

/// \brief Return the human readable version of the type name T with simplified radium type names
template <typename T>
auto simplifiedDemangledType( const T& ) noexcept -> std::string;

/// \brief Return the human readable version of the type name whose index is known, with simplified
/// radium type names
/// \param typeName The typeIndex whose simplified named is requested
/// \return The Radium-simplified type name
RA_DATAFLOW_API auto simplifiedDemangledType( const std::type_index& typeName ) noexcept
    -> std::string;

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

namespace TypeInternal {
RA_DATAFLOW_API auto makeTypeReadable( const std::string& ) -> std::string;
}

template <typename T>
auto simplifiedDemangledType() noexcept -> std::string {
    static auto demangled_name = []() {
        std::string demangledType =
            TypeInternal::makeTypeReadable( Ra::Core::Utils::demangleType<T>() );
        return demangledType;
    }();
    return demangled_name;
}

template <typename T>
auto simplifiedDemangledType( const T& ) noexcept -> std::string {
    return simplifiedDemangledType<T>();
}

inline auto simplifiedDemangledType( const std::type_index& typeName ) noexcept -> std::string {
    return TypeInternal::makeTypeReadable( Ra::Core::Utils::demangleType( typeName ) );
}

} // namespace Core
} // namespace Dataflow
} // namespace Ra
