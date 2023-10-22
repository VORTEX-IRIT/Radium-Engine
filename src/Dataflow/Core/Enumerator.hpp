#pragma once

#include <initializer_list>
#include <vector>

#include <Core/Utils/Observable.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

/**
 * \brief This class might be removed in a future and replaced by an instantiation of
 * Ra::Core::Utils::BijectiveAssociation.
 *
 * This class allows to associate Values of type T to an int.
 * Used right now to build the Node edition UI.
 * \tparam T
 */
template <typename T>
class Enumerator : public Ra::Core::Utils::Observable<const Enumerator<T>&>
{
    std::vector<T> m_values;
    size_t m_currentIndex { 0 };
    T* m_currentValue;

  public:
    explicit Enumerator( std::initializer_list<T> values );
    const T& get() const;
    size_t size() const;
    bool set( size_t p );
    bool set( const T& v );
    typename std::vector<T>::const_iterator begin() const;
    typename std::vector<T>::const_iterator end() const;
    const T& operator[]( size_t p ) const;
};

// -----------------------------------------------------------------
// ---------------------- inline methods ---------------------------

template <typename T>
Enumerator<T>::Enumerator( std::initializer_list<T> values ) :
    m_values { values }, m_currentValue { m_values.data() } {}

template <typename T>
const T& Enumerator<T>::get() const {
    return *m_currentValue;
}

template <typename T>
size_t Enumerator<T>::size() const {
    return m_values.size();
}

template <typename T>
bool Enumerator<T>::set( size_t p ) {
    if ( p < m_values.size() ) {
        m_currentValue = m_values.data() + p;
        m_currentIndex = p;
        this->notify( *this );
        return true;
    }
    else { return false; }
}

template <typename T>
bool Enumerator<T>::set( const T& v ) {
    size_t p = 0;
    for ( const auto& e : m_values ) {
        if ( e == v ) { return set( p ); }
        p++;
    }
    return false;
}

template <typename T>
typename std::vector<T>::const_iterator Enumerator<T>::begin() const {
    return m_values.cbegin();
}

template <typename T>
typename std::vector<T>::const_iterator Enumerator<T>::end() const {
    return m_values.cend();
}

template <typename T>
const T& Enumerator<T>::operator[]( size_t p ) const {
    return m_values.at( p );
}

} // namespace Core
} // namespace Dataflow
} // namespace Ra
