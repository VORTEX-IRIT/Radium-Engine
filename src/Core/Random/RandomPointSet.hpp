#pragma once
#include <Core/RaCore.hpp>

#include <Core/Types.hpp>

#include <random>

namespace Ra {
namespace Core {

/** \brief Random point set utilities.
 *
 *  This namespace contains functions, classes and utilities for 1D, 2D and 3D random point sets
 * management.
 *
 */
namespace Random {

/**
 * \brief Defines some helper constexpr functions
 */
namespace internal {
Scalar constexpr sqrtNewtonRaphsonHelper( Scalar x, Scalar curr, Scalar prev ) {
    return curr == prev ? curr : sqrtNewtonRaphsonHelper( x, 0.5_ra * ( curr + x / curr ), curr );
}
Scalar constexpr sqrtConstExpr( Scalar x ) {
    return sqrtNewtonRaphsonHelper( x, x, 0_ra );
}
} // namespace internal

/** \brief Implements the fibonacci sequence
 * i --> i/phi
 * where phi = (1 + sqrt(5)) / 2
 */
class RA_CORE_API FibonacciSequence
{

    static constexpr Scalar phi = ( 1_ra + internal::sqrtConstExpr( 5_ra ) ) / 2_ra;
    size_t n;

  public:
    explicit FibonacciSequence( size_t number );
    // copyable
    FibonacciSequence( const FibonacciSequence& )            = default;
    FibonacciSequence& operator=( const FibonacciSequence& ) = default;
    // movable
    FibonacciSequence( FibonacciSequence&& )            = default;
    FibonacciSequence& operator=( FibonacciSequence&& ) = default;
    virtual ~FibonacciSequence()                        = default;

    size_t range();
    Scalar operator()( size_t i );
};

/** \brief 1D Van der Corput sequence
 * only implemented for 32bits floats (converted out to Scalar)
 */
struct RA_CORE_API VanDerCorputSequence {
    Scalar operator()( unsigned int bits );
};

/** \brief Implements the 2D fibonacci Point set
 * points follow the FibonacciSequence
 * (i, N) => [i / phi, i / N]
 */
class RA_CORE_API FibonacciPointSet
{
    FibonacciSequence seq;

  public:
    explicit FibonacciPointSet( size_t n );
    // copyable
    FibonacciPointSet( const FibonacciPointSet& )            = default;
    FibonacciPointSet& operator=( const FibonacciPointSet& ) = default;
    // movable
    FibonacciPointSet( FibonacciPointSet&& )            = default;
    FibonacciPointSet& operator=( FibonacciPointSet&& ) = default;
    virtual ~FibonacciPointSet()                        = default;

    size_t range();
    Ra::Core::Vector2 operator()( size_t i );
};

/** \brief 2D Hammersley point set
 *
 */
class RA_CORE_API HammersleyPointSet
{
    VanDerCorputSequence seq;
    size_t n;

  public:
    explicit HammersleyPointSet( size_t number );
    // copyable
    HammersleyPointSet( const HammersleyPointSet& )            = default;
    HammersleyPointSet& operator=( const HammersleyPointSet& ) = default;
    // movable
    HammersleyPointSet( HammersleyPointSet&& )            = default;
    HammersleyPointSet& operator=( HammersleyPointSet&& ) = default;
    virtual ~HammersleyPointSet()                         = default;

    size_t range();
    Ra::Core::Vector2 operator()( size_t i );
};

/** \brief 2D Random point set
 */
class RA_CORE_API MersenneTwisterPointSet
{
    std::mt19937 gen;
    std::uniform_real_distribution<Scalar> seq;
    size_t n;

  public:
    explicit MersenneTwisterPointSet( size_t number );
    // copyable
    MersenneTwisterPointSet( const MersenneTwisterPointSet& )            = default;
    MersenneTwisterPointSet& operator=( const MersenneTwisterPointSet& ) = default;
    // movable
    MersenneTwisterPointSet( MersenneTwisterPointSet&& )            = default;
    MersenneTwisterPointSet& operator=( MersenneTwisterPointSet&& ) = default;
    virtual ~MersenneTwisterPointSet()                              = default;

    size_t range();
    Ra::Core::Vector2 operator()( size_t );
};

// https://observablehq.com/@mbostock/spherical-fibonacci-lattice
// http://holger.dammertz.org/stuff/notes_HammersleyOnHemisphere.html
/** \brief Map a [0, 1)^2 point set on the unit sphere
 * \tparam PointSet the random point set type to map on sphere
 */
template <class PointSet>
class SphericalPointSet
{
    PointSet p;
    Ra::Core::Vector3 projectOnSphere( const Ra::Core::Vector2&& pt );

  public:
    explicit SphericalPointSet( size_t n );
    // copyable
    SphericalPointSet( const SphericalPointSet& )            = default;
    SphericalPointSet& operator=( const SphericalPointSet& ) = default;
    // movable
    SphericalPointSet( SphericalPointSet&& )            = default;
    SphericalPointSet& operator=( SphericalPointSet&& ) = default;
    virtual ~SphericalPointSet()                        = default;

    size_t range();
    Ra::Core::Vector3 operator()( size_t i );
};

// ------------------------------------------------------------------------
// ------------------------ inline methods --------------------------------

template <class PointSet>
Ra::Core::Vector3 SphericalPointSet<PointSet>::projectOnSphere( const Ra::Core::Vector2&& pt ) {
    Scalar theta = std::acos( 2 * pt[1] - 1 ); // 0 <= tetha <= pi
    Scalar phi   = 2_ra * Scalar( M_PI ) * pt[0];
    return { std::sin( theta ) * std::cos( phi ),
             std::sin( theta ) * std::sin( phi ),
             std::cos( theta ) };
}

template <class PointSet>
SphericalPointSet<PointSet>::SphericalPointSet( size_t n ) : p( n ) {}

template <class PointSet>
size_t SphericalPointSet<PointSet>::range() {
    return p.range();
}

template <class PointSet>
Ra::Core::Vector3 SphericalPointSet<PointSet>::operator()( size_t i ) {
    return projectOnSphere( p( i ) );
}

} // namespace Random
} // namespace Core
} // namespace Ra
