#pragma once

#include <Core/RaCore.hpp>
#include <limits>

namespace Ra {
namespace Core {
namespace Utils {

class RA_CORE_API Index
{
  public:
    using IntegerType = int;

    /// CONSTRUCTOR
    /// Default constructor that allow implicit conversion from integer to Index
    constexpr Index( IntegerType i = s_invalid );
    constexpr Index( const Index& i );

    /// Templated constructor to convert any interger type to index.
    /// \tparam Integer template parameter has to be std::is_integral (static_assert'ed)
    template <typename Integer>
    explicit constexpr Index( Integer i );

    /// DESTRUCTOR: Must not be defined, we need it trivial to be
    /// constexpr
    // ~Index() { }

    /// VALID
    constexpr bool isValid() const;

    /// INVALID
    constexpr bool isInvalid() const;
    constexpr void setInvalid();

    /// \brief return an invalid Index
    static constexpr Index Invalid();
    static constexpr Index Max();

    /// INDEX
    constexpr IntegerType getValue() const;
    constexpr void setValue( const IntegerType i );

    /// CAST
    constexpr operator IntegerType() const; // required for `array[idx]`

    /// OPERATOR
    constexpr Index& operator=( const Index& id );
    constexpr Index& operator++();
    constexpr Index& operator--();

    constexpr Index operator+( const Index& id );
    template <typename Integer>
    constexpr Index operator+( const Integer& i );

    constexpr Index operator-( const Index& id );
    template <typename Integer>
    constexpr Index operator-( const Integer& i );

    constexpr bool operator==( const Index& id );
    template <typename Integer>
    constexpr bool operator==( const Integer& i );

    constexpr bool operator!=( const Index& id );
    template <typename Integer>
    constexpr bool operator!=( const Integer& i );

    constexpr bool operator<( const Index& id1 );
    template <typename Integer>
    constexpr bool operator<( const Integer& i );

    constexpr bool operator<=( const Index& id1 );
    template <typename Integer>
    constexpr bool operator<=( const Integer& i );

    constexpr bool operator>( const Index& id1 );
    template <typename Integer>
    constexpr bool operator>( const Integer& i );

    constexpr bool operator>=( const Index& id1 );
    template <typename Integer>
    constexpr bool operator>=( const Integer& i );

  protected:
    /// VARIABLE
    IntegerType m_idx { s_invalid };

  private:
    /// CONSTANT
    static const IntegerType s_invalid { -1 };
    static const IntegerType s_maxIdx { std::numeric_limits<IntegerType>::max() };
};

/// CONSTRUCTOR
constexpr Index::Index( IntegerType i ) : m_idx( ( i < 0 ) ? s_invalid : i ) {}

template <typename Integer>
constexpr Index::Index( Integer i ) : m_idx( ( ( i < 0 ) || ( i > s_maxIdx ) ) ? s_invalid : i ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
}

constexpr Index::Index( const Index& i ) : m_idx( i.m_idx ) {}

/// VALID
constexpr bool Index::isValid() const {
    return ( m_idx != s_invalid );
}

/// INVALID
constexpr bool Index::isInvalid() const {
    return ( m_idx < 0 );
}
constexpr void Index::setInvalid() {
    m_idx = s_invalid;
}
constexpr Index Index::Invalid() {
    return Index( s_invalid );
}
constexpr Index Index::Max() {
    return Index( s_maxIdx );
}

/// INDEX
constexpr Index::IntegerType Index::getValue() const {
    return m_idx;
}
constexpr void Index::setValue( const IntegerType i ) {
    m_idx = ( i < 0 ) ? s_invalid : i;
}

/// CAST
/// \todo manage other types ? like unsigned with an assert if invalid ?
constexpr Index::operator IntegerType() const {
    return m_idx;
}

/// OPERATOR
constexpr Index& Index::operator=( const Index& id ) {
    m_idx = id.m_idx;
    return *this;
}
constexpr Index& Index::operator++() {
    m_idx++;
    if ( m_idx < 0 ) { m_idx = s_invalid; }
    return *this;
}
constexpr Index& Index::operator--() {
    if ( m_idx != s_invalid ) { m_idx--; }
    return *this;
}

constexpr Index Index::operator+( const Index& id ) {
    if ( isInvalid() || id.isInvalid() ) { return Index::Invalid(); }
    return Index( m_idx + id.m_idx );
}
template <typename Integer>
constexpr Index Index::operator+( const Integer& id ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( *this ) + Index( id );
}

constexpr Index Index::operator-( const Index& id ) {
    if ( isInvalid() || id.isInvalid() ) { return Index::Invalid(); }
    return Index( m_idx - id.m_idx );
}
template <typename Integer>
constexpr Index Index::operator-( const Integer& id ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( *this ) - Index( id );
}

constexpr bool Index::operator==( const Index& id ) {
    return ( m_idx == id.m_idx );
}
template <typename Integer>
constexpr bool Index::operator==( const Integer& i ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( *this == Index( IntegerType( i ) ) );
}

constexpr bool Index::operator!=( const Index& id ) {
    return ( !( *this == id ) );
}
template <typename Integer>
constexpr bool Index::operator!=( const Integer& i ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( !( *this == Index( IntegerType( i ) ) ) );
}

constexpr bool Index::operator<( const Index& id ) {
    if ( isInvalid() || id.isInvalid() ) { return false; }
    return ( m_idx < id.m_idx );
}
template <typename Integer>
constexpr bool Index::operator<( const Integer& i ) {
    return ( *this < Index( IntegerType( i ) ) );
}

constexpr bool Index::operator<=( const Index& id ) {
    if ( isInvalid() || id.isInvalid() ) { return false; }
    return ( m_idx <= id.m_idx );
}
template <typename Integer>
constexpr bool Index::operator<=( const Integer& i ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( *this <= Index( IntegerType( i ) ) );
}

constexpr bool Index::operator>( const Index& id ) {
    if ( isInvalid() || id.isInvalid() ) { return false; }
    return ( m_idx > id.m_idx );
}
template <typename Integer>
constexpr bool Index::operator>( const Integer& i ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( *this > Index( IntegerType( i ) ) );
}

constexpr bool Index::operator>=( const Index& id ) {
    if ( isInvalid() || id.isInvalid() ) { return false; }
    return ( m_idx >= id.m_idx );
}
template <typename Integer>
constexpr bool Index::operator>=( const Integer& i ) {
    static_assert( std::is_integral<Integer>::value, "Integral required." );
    return ( *this >= Index( IntegerType( i ) ) );
}

} // namespace Utils
} // namespace Core
} // namespace Ra
