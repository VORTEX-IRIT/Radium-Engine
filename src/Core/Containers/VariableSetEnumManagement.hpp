#pragma once

#include <Core/Containers/VariableSet.hpp>
#include <Core/Utils/EnumConverter.hpp>

#include <string>

/// hepler function to manage enum as underlying types in VariableSet
namespace Ra {
namespace Core {
namespace VariableSetEnumManagement {

/**
 * \brief Associate a converter for enumerated type to the given variable name
 * \tparam EnumBaseType The enum base type to manage (\see Ra::Core::Utils::EnumConverter)
 * \param name
 * \param converter
 */
template <typename EnumBaseType>
void addEnumConverter( VariableSet& vs,
                       const std::string& name,
                       std::shared_ptr<Core::Utils::EnumConverter<EnumBaseType>> converter );

/**
 * \brief Search for a converter associated with an enumeration variable
 * \tparam EnumBaseType The enum base type to manage (\see Ra::Core::Utils::EnumConverter)
 * \param name the name of the parameter
 * \return an optional containing the converter or false if no converter is found.
 */
template <typename EnumBaseType>
Core::Utils::optional<std::shared_ptr<Core::Utils::EnumConverter<EnumBaseType>>>
getEnumConverter( const VariableSet& vs, const std::string& name );

/**
 * \brief Return the string associated to the actual value of an enumeration variable
 * \tparam Enum The enum type (\see Ra::Core::Utils::EnumConverter)
 * \param name The name of the enum variable
 * \param value The value to convert
 * \return
 */
template <typename Enum, typename std::enable_if<std::is_enum<Enum> {}, bool>::type = true>
std::string getEnumString( const VariableSet& vs, const std::string& name, Enum value );

/**
 * \brief (overload) Return the string associated to the actual value of a n enumeration
 * variable, from a value with underlying_type<Enum>. \tparam EnumBaseType The underlying enum
 * type (\see Ra::Core::Utils::EnumConverter) \param name The name of the enum variable \param
 * value The value to convert \return
 */
template <typename EnumBaseType>
std::string
getEnumString( const VariableSet& vs,
               const std::string& name,
               EnumBaseType value,
               typename std::enable_if<!std::is_enum<EnumBaseType> {}, bool>::type = true );

/**
 * \brief set the value of the given enumeration variable, according to a string representation
 * of an enum. \note If there is no EnumConverter associated with the variable name, the string
 * is registered in the RenderParameter set. \param name Name of the variable \param value value
 * of the variable
 */
void RA_CORE_API setEnumVariable( VariableSet& vs,
                                  const std::string& name,
                                  const std::string& value );

void RA_CORE_API setEnumVariable( VariableSet& vs, const std::string& name, const char* value );

template <typename T>
void setEnumVariable( VariableSet& vs, const std::string& name, T value ) {
    auto v = static_cast<typename std::underlying_type<T>::type>( value );
    vs.setVariable( name, v );
}

template <typename T>
auto getEnumVariable( const VariableSet& vs, const std::string& name ) -> const T& {
    static_assert( std::is_enum<T>::value );
    return reinterpret_cast<const T&>(
        vs.getVariable<typename std::underlying_type<T>::type>( name ) );
}

template <typename T>
auto getEnumVariable( VariableSet& vs, const std::string& name ) -> T& {
    return const_cast<T&>( getEnumVariable<T>( const_cast<const VariableSet&>( vs ), name ) );
}

template <typename EnumBaseType>
void addEnumConverter( VariableSet& vs,
                       const std::string& name,
                       std::shared_ptr<Core::Utils::EnumConverter<EnumBaseType>> converter ) {

    // typed converter
    auto converterHandle = vs.setVariable( name, converter );

    // string string converter/setter for setEnumVariable
    std::function<void( Core::VariableSet&, const std::string&, const std::string& )>
        convertingFunction = [converter_ = converterHandle.first]( Core::VariableSet& vs_,
                                                                   const std::string& name_,
                                                                   const std::string& value_ ) {
            vs_.setVariable( name_, converter_->second->getEnumerator( value_ ) );
        };
    vs.setVariable( name, convertingFunction );
}

template <typename EnumBaseType>
Core::Utils::optional<std::shared_ptr<Core::Utils::EnumConverter<EnumBaseType>>>
getEnumConverter( const VariableSet& vs, const std::string& name ) {
    auto storedConverter =
        vs.existsVariable<std::shared_ptr<Core::Utils::EnumConverter<EnumBaseType>>>( name );
    if ( storedConverter ) { return ( *storedConverter )->second; }
    return {};
}

template <typename EnumBaseType>
std::string getEnumString( const VariableSet& vs,
                           const std::string& name,
                           EnumBaseType value,
                           typename std::enable_if<!std::is_enum<EnumBaseType> {}, bool>::type ) {
    auto storedConverter =
        vs.existsVariable<std::shared_ptr<Core::Utils::EnumConverter<EnumBaseType>>>( name );
    if ( storedConverter ) { return ( *storedConverter )->second->getEnumerator( value ); }
    LOG( Ra::Core::Utils::logWARNING ) << name + " is not a registered Enum with underlying type " +
                                              Ra::Core::Utils::demangleType<EnumBaseType>() + ".";
    return "";
}

template <typename Enum, typename std::enable_if<std::is_enum<Enum> {}, bool>::type>
std::string getEnumString( const VariableSet& vs, const std::string& name, Enum value ) {
    using EnumBaseType = typename std::underlying_type_t<Enum>;
    return getEnumString( vs, name, EnumBaseType( value ) );
}

} // namespace VariableSetEnumManagement
} // namespace Core
} // namespace Ra
