#pragma once

#include <vector>

#include <Core/RaCore.hpp>
#include <Eigen/Core>

namespace Ra {
namespace Core {
/// This class stores a D-dimensional grid of elements of arbitrary type.
/// in a contiguous memory block. Elements are stored in column-major order.
/// e.g. for a 3x3x3 array the element vector looks like
///  [A000, A100, A200, A010,... A222].
/// Elements are accessible with a D-dimensional Vector, or linearly with
/// iterators, thanks to the std-like interface provided.
template <typename T, uint D>
class Grid
{

  public:
    // public types and constants.
    static const uint Dimension = D; /// Dimension of our grid
    using IdxVector =
        Eigen::Matrix<uint, D, 1>; /// A vector of the size of the grid along each dimension.
    using OffsetVector = Eigen::Matrix<int, D, 1>; /// A vector of signed offsets.

    /// This class implements an iterator though elements of the grid that
    /// can be referenced with a linear index or a D-dimensional uint vector.
    class Iterator
    {
      public:
        //
        // Construction
        //

        /// Constructor from size and linear index.
        explicit Iterator( const IdxVector& size, uint startIdx = 0 );

        /// Constructor from size and vector index.
        Iterator( const IdxVector& size, const IdxVector& startIdx );

        /// Constructor from grid and linear index.
        explicit Iterator( const Grid<T, D>& grid, uint startIdx = 0 );

        /// Constructor from grid and vector index.
        Iterator( const Grid<T, D>& grid, const IdxVector& startIdx );

        /// Default copy constructor and assignment operator.
        Iterator( const Iterator& other )            = default;
        Iterator& operator=( const Iterator& other ) = default;

        /// Cast to the an iterator in a different type grid.
        template <typename T2>
        typename Grid<T2, D>::Iterator cast() const;

        //
        // Basic getters and setters
        //

        /// Set an existing iterator to a linear index value.
        inline void setFromLinear( uint idx );

        /// Set an existing iterator to a vector index value.
        inline void setFromVector( const IdxVector& idx );

        /// Get the current linear index.
        inline uint getLinear() const;

        /// Get the current vector index.
        inline IdxVector getVector() const;

        /// Get the associated grid size.
        inline const IdxVector& getGridSize() const;

        /// Returns true if the current index is out of the grid.
        inline bool isOut() const;

        /// Returns true if the current index is inside the grid.
        inline bool isIn() const;

        //
        // Increment and decrement operators
        //

        /// Advance the iterator to the next-element
        inline Iterator& operator++();
        inline Iterator operator++( int );

        /// Move the iterator back to the previous element
        inline Iterator& operator--();
        inline Iterator operator--( int );

        //
        // Add and substract operators with linear offset
        //

        /// Advance the iterator with an offset of i elements
        inline Iterator& operator+=( uint i );

        /// Rewind the iterator with an offset of i elements
        inline Iterator& operator-=( uint i );

        //
        // Add and substract operators with vector offset
        //

        // Note : these convenience functions do not do any kind
        // of check. If you get out of bounds on one dimension, the iterator
        // might still be valid but not pointing to the element you think
        // it should.

        inline Iterator& operator+=( const IdxVector& idx );
        inline Iterator& operator-=( const IdxVector& idx );
        inline Iterator& operator+=( const OffsetVector& idx );

        /// Returns true if adding offset idx with current iterator will act
        /// as expected and false if it will put it out of bounds.
        bool isValidOffset( const OffsetVector& idx );

        //
        // Comparison operators
        //

        // Note : comparing iterators of different grid sizes will assert.

        inline bool operator==( const Iterator& other ) const;
        inline bool operator<( const Iterator& other ) const;

        //
        // Extra operators
        //

        // Declare binary operators directly (see C++ faq 35.16)
        friend Iterator operator+( const Iterator& it, uint i ) {
            Iterator copy( it );
            return copy += i;
        }
        friend Iterator operator-( const Iterator& it, uint i ) {
            Iterator copy( it );
            return copy -= i;
        }
        friend Iterator operator+( const Iterator& it, const IdxVector& idx ) {
            Iterator copy( it );
            return copy += idx;
        }
        friend Iterator operator-( const Iterator& it, const IdxVector& idx ) {
            Iterator copy( it );
            return copy -= idx;
        }
        friend Iterator operator+( const Iterator& it, const OffsetVector& idx ) {
            Iterator copy( it );
            return copy += idx;
        }

        // Extraneous comparison operators default implementation.
        inline bool operator!=( const Iterator& rhs ) const { return !( *this == rhs ); }
        inline bool operator>( const Iterator& rhs ) const { return rhs < *this; }
        inline bool operator<=( const Iterator& rhs ) const { return !( *this > rhs ); }
        inline bool operator>=( const Iterator& rhs ) const { return !( *this < rhs ); }

      private:
        const IdxVector& m_sizes; /// Size of the underlying grid
        uint m_index;             /// Current linear index of the iterator
    };

  public:
    //
    // Constructors
    //

    /// Construct a grid of a given size and fill it with the given value.
    Grid( const IdxVector& size = IdxVector::Zero(), const T& val = T() ) :
        m_size( size ), m_data( size.prod(), val ) {}

    /// Construct a grid of a given size with values in ()-major format
    Grid( const IdxVector& size, const T* values ) :
        m_size( size ), m_data( size.prod(), values ) {}

    /// Copy constructor and assignment operator.
    Grid( const Grid<T, D>& other )            = default;
    Grid& operator=( const Grid<T, D>& other ) = default;

    //
    // Basic getters
    //

    /// Returns the number of elements stored.
    inline uint size() const;
    /// Returns the size vector (a D-dimensional vector with the size along each dimension).
    inline const IdxVector& sizeVector() const;
    /// Returns true if the grid is empty (i.e. if size() ==0).
    inline bool empty() const;
    /// Erases all data and makes the grid empty.
    inline void clear();

    //
    // Element access
    //

    /// Access an element with a D-dimensional index.
    // Note that since initializer lists are implicitly convertible to
    // Eigen vectors, you can call grid.at({x,y,z})
    inline const T& at( const IdxVector& idx ) const;
    inline T& at( const IdxVector& idx );

    /// Access an element with a linear index.
    inline const T& at( uint idx ) const;
    inline T& at( uint idx );

    /// Access an element with an iterator.
    inline const T& at( const Iterator& it ) const;
    inline T& at( const Iterator& it );

    /// Read only access to the underlying data.
    inline const T* data() const;

    /// Read-write access to the underlying data.
    inline T* data();

    //
    // std::iterators-like interface
    //

    /// Get an iterator on this grid at the first element.
    inline Iterator begin();
    inline Iterator begin() const;

    /// Get an iterator on this grid past the last element.
    inline Iterator end();
    inline Iterator end() const;

  protected:
    /// Indicate the extends of the grid along each dimension.
    IdxVector m_size;
    /// Storage for the grid data.
    std::vector<T> m_data;
};

// Anonymous helper functions to convert to/from multi-dimensional indices
// If we want to  make the grid row-major we just need to overload these functions
// by reversing the loops.
namespace {

template <typename T, uint D>
inline typename Grid<T, D>::IdxVector
linearToIdxVector( uint linIdx, const typename Grid<T, D>::IdxVector& size ) {
    typename Grid<T, D>::IdxVector result = Grid<T, D>::IdxVector::Zero();

    for ( uint i = 0; i < D; ++i ) {
        result[i] = linIdx % size[i];
        linIdx    = linIdx / size[i];
    }
    return result;
}

template <typename T, uint D>
inline uint idxVectorToLinear( const typename Grid<T, D>::IdxVector& vecIdx,
                               const typename Grid<T, D>::IdxVector& size ) {
    uint result  = 0;
    uint dimProd = 1;
    for ( uint i = 0; i < D; ++i ) {
        result += vecIdx[i] * dimProd;
        dimProd *= size[i];
    }
    return result;
}
} // namespace

// Vector size and data management.
//

template <typename T, uint D>
inline uint Grid<T, D>::size() const {
    CORE_ASSERT( m_data.size() == m_size.prod(), "Inconsistent grid size" );
    return m_data.size();
}

template <typename T, uint D>
inline const typename Grid<T, D>::IdxVector& Grid<T, D>::sizeVector() const {
    CORE_ASSERT( m_data.size() == m_size.prod(), "Inconsistent grid size" );
    return m_size;
}

template <typename T, uint D>
inline bool Grid<T, D>::empty() const {
    CORE_ASSERT( m_data.size() == m_size.prod(), "Inconsistent grid size" );
    return m_data.empty();
}

template <typename T, uint D>
inline void Grid<T, D>::clear() {
    m_data.clear();
    m_size = IdxVector::Zero();
    CORE_ASSERT( empty(), "Inconsistent grid" );
}

template <typename T, uint D>
inline const T* Grid<T, D>::data() const {
    return m_data.data();
}

template <typename T, uint D>
inline T* Grid<T, D>::data() {
    return m_data.data();
}

//
// Individual element access.
//

template <typename T, uint D>
inline const T& Grid<T, D>::at( const typename Grid<T, D>::IdxVector& idx ) const {
    const uint linIdx = idxVectorToLinear<T, D>( idx, sizeVector() );
    CORE_ASSERT( linIdx < m_data.size(), "Invalid vector index" );
    return m_data[linIdx];
}

template <typename T, uint D>
inline T& Grid<T, D>::at( const typename Grid<T, D>::IdxVector& idx ) {
    const uint linIdx = idxVectorToLinear<T, D>( idx, sizeVector() );
    CORE_ASSERT( linIdx < m_data.size(), "Invalid vector index" );
    return m_data[linIdx];
}

template <typename T, uint D>
inline const T& Grid<T, D>::at( uint idx ) const {
    CORE_ASSERT( idx < m_data.size(), "Invalid vector index" );
    return m_data[idx];
}

template <typename T, uint D>
inline T& Grid<T, D>::at( uint idx ) {
    CORE_ASSERT( idx < m_data.size(), "Invalid vector index" );
    return m_data[idx];
}

template <typename T, uint D>
const T& Grid<T, D>::at( const typename Grid<T, D>::Iterator& it ) const {
    CORE_ASSERT( it.getGridSize() == m_size, "Incompatible iterator" );
    return at( it.getLinear() );
}

template <typename T, uint D>
T& Grid<T, D>::at( const typename Grid<T, D>::Iterator& it ) {
    CORE_ASSERT( it.getGridSize() == m_size, "Incompatible iterator" );
    return at( it.getLinear() );
}

//
// Iterators begin / end functions.
//

template <typename T, uint D>
inline typename Grid<T, D>::Iterator Grid<T, D>::begin() {
    return Iterator( *this );
}

template <typename T, uint D>
inline typename Grid<T, D>::Iterator Grid<T, D>::begin() const {
    return Iterator( *this );
}

template <typename T, uint D>
inline typename Grid<T, D>::Iterator Grid<T, D>::end() {
    return Iterator( *this, size() );
}

template <typename T, uint D>
inline typename Grid<T, D>::Iterator Grid<T, D>::end() const {
    return Iterator( *this, size() );
}

//
// Iterators construction
//

template <typename T, uint D>
inline Grid<T, D>::Iterator::Iterator( const typename Grid<T, D>::IdxVector& size, uint startIdx ) :
    m_sizes( size ) {
    setFromLinear( startIdx );
}

template <typename T, uint D>
inline Grid<T, D>::Iterator::Iterator( const typename Grid<T, D>::IdxVector& size,
                                       const typename Grid<T, D>::IdxVector& startIdx ) :
    m_sizes( size ) {
    setFromVector( startIdx );
}

template <typename T, uint D>
inline Grid<T, D>::Iterator::Iterator( const Grid<T, D>& grid, uint startIdx ) :
    m_sizes( grid.sizeVector() ) {
    setFromLinear( startIdx );
}

template <typename T, uint D>
inline Grid<T, D>::Iterator::Iterator( const Grid<T, D>& grid,
                                       const typename Grid<T, D>::IdxVector& startIdx ) :
    m_sizes( grid.sizeVector() ) {
    setFromVector( startIdx );
}

//
// Basic Iterator get/set
//

template <typename T, uint D>
inline void Grid<T, D>::Iterator::setFromLinear( uint i ) {
    m_index = i;
}

template <typename T, uint D>
inline void Grid<T, D>::Iterator::setFromVector( const typename Grid<T, D>::IdxVector& idx ) {
    m_index = idxVectorToLinear<T, D>( idx, m_sizes );
}

template <typename T, uint D>
inline uint Grid<T, D>::Iterator::getLinear() const {
    return m_index;
}

template <typename T, uint D>
inline typename Grid<T, D>::IdxVector Grid<T, D>::Iterator::getVector() const {
    return linearToIdxVector<T, D>( m_index, m_sizes );
}

template <typename T, uint D>
inline bool Grid<T, D>::Iterator::isOut() const {
    return !isIn();
}

template <typename T, uint D>
inline bool Grid<T, D>::Iterator::isIn() const {
    return m_index < m_sizes.prod();
}

//
// Iterator increment and decrement
//

template <typename T, uint D>
typename Grid<T, D>::Iterator& Grid<T, D>::Iterator::operator++() {
    m_index++;
    return *this;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator& Grid<T, D>::Iterator::operator--() {
    m_index--;
    return *this;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator Grid<T, D>::Iterator::operator++( int ) {
    Iterator copy( *this );
    ++( *this );
    return copy;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator Grid<T, D>::Iterator::operator--( int ) {
    Iterator copy( *this );
    --( *this );
    return copy;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator& Grid<T, D>::Iterator::operator+=( uint i ) {
    m_index += i;
    return *this;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator& Grid<T, D>::Iterator::operator-=( uint i ) {
    m_index -= i;
    return *this;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator&
Grid<T, D>::Iterator::operator+=( const typename Grid<T, D>::IdxVector& idx ) {
    CORE_ASSERT( isValidOffset( idx.template cast<int>() ), "Invalid offset vector." );
    setFromVector( getVector() + idx );
    return *this;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator&
Grid<T, D>::Iterator::operator-=( const typename Grid<T, D>::IdxVector& idx ) {
    CORE_ASSERT( isValidOffset( -( idx.template cast<int>() ) ), "Invalid offset vector." );
    setFromVector( getVector() - idx );
    return *this;
}

template <typename T, uint D>
typename Grid<T, D>::Iterator&
Grid<T, D>::Iterator::operator+=( const typename Grid<T, D>::OffsetVector& idx ) {
    CORE_ASSERT( isValidOffset( idx ), "Invalid offset vector" );
    setFromVector( ( getVector().template cast<int>() + idx ).template cast<uint>() );
    return *this;
}

template <typename T, uint D>
bool Grid<T, D>::Iterator::operator==( const typename Grid<T, D>::Iterator& other ) const {
    CORE_ASSERT( m_sizes == other.m_sizes, "Comparing unrelated grid iterators" );
    return m_index == other.m_index;
}

template <typename T, uint D>
bool Grid<T, D>::Iterator::operator<( const typename Grid<T, D>::Iterator& other ) const {
    CORE_ASSERT( m_sizes == other.m_sizes, "Comparing unrelated grid iterators" );
    return m_index < other.m_index;
}

template <typename T, uint D>
const typename Grid<T, D>::IdxVector& Grid<T, D>::Iterator::getGridSize() const {
    return m_sizes;
}

template <typename T, uint D>
template <typename T2>
typename Grid<T2, D>::Iterator Grid<T, D>::Iterator::cast() const {
    return typename Grid<T2, D>::Iterator( m_sizes, m_index );
}

template <typename T, uint D>
bool Grid<T, D>::Iterator::isValidOffset( const typename Grid<T, D>::OffsetVector& idx ) {
    OffsetVector pos = getVector().template cast<int>() + idx;
    return !( ( pos.array() < 0 ).any() ||
              ( pos.array() >= m_sizes.template cast<int>().array() ).any() );
}

} // namespace Core
} // namespace Ra
