#pragma once

#include <Core/CoreMacros.hpp>

#include <Core/Utils/BijectiveAssociation.hpp>

#include <initializer_list>
#include <vector>

namespace Ra {
namespace Core {
namespace Utils {

/**
 * \brief This class manage the bijective association between string and integral representation
 * of an enumeration.
 *
 * \tparam EnumBaseType the underlying type (as returned by typename std::underlying_type_t<Enum>)
 * of the enumeration to manage.
 *
 */

template <typename EnumBaseType>
class EnumConverter
{
    /// \todo think of manage both EnumType and EnumUnderlyingType
    /// Eg using EnumUnderlyingType = std::underling_type_t<Enum>;
    /// getEnumeratorUnderlying ->EnumUnderlyingType
    /// getEnumerator -> Enum
  public:
    explicit EnumConverter( std::initializer_list<std::pair<EnumBaseType, std::string>> pairs );

    std::string getEnumerator( EnumBaseType v ) const;
    EnumBaseType getEnumerator( const std::string& v ) const;
    std::vector<std::string> getEnumerators() const;

  private:
    Core::Utils::BijectiveAssociation<EnumBaseType, std::string> m_valueToString;
};

/* -------------------------------------------------------------------------------------------
 * Implementation of template functions
 * -----------------------------------------------------------------------------------------*/

template <typename EnumBaseType>
EnumConverter<EnumBaseType>::EnumConverter(
    std::initializer_list<std::pair<EnumBaseType, std::string>> pairs ) :
    m_valueToString { pairs } {}

template <typename EnumBaseType>
std::string EnumConverter<EnumBaseType>::getEnumerator( EnumBaseType v ) const {
    return m_valueToString( v );
}

template <typename EnumBaseType>
EnumBaseType EnumConverter<EnumBaseType>::getEnumerator( const std::string& v ) const {
    return m_valueToString.key( v );
}

template <typename EnumBaseType>
std::vector<std::string> EnumConverter<EnumBaseType>::getEnumerators() const {
    std::vector<std::string> keys;
    keys.reserve( m_valueToString.size() );
    for ( const auto& p : m_valueToString ) {
        keys.push_back( p.second );
    }
    return keys;
}

} // namespace Utils
} // namespace Core
} // namespace Ra
