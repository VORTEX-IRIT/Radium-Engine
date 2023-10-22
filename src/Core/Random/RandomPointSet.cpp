#include <Core/Random/RandomPointSet.hpp>

namespace Ra {
namespace Core {
namespace Random {

FibonacciSequence::FibonacciSequence( size_t number ) : n { std::max( size_t( 5 ), number ) } {}

size_t FibonacciSequence::range() {
    return n;
}
Scalar FibonacciSequence::operator()( size_t i ) {
    return Scalar( i ) / phi;
}

Scalar VanDerCorputSequence::operator()( unsigned int bits ) {
    bits = ( bits << 16u ) | ( bits >> 16u );
    bits = ( ( bits & 0x55555555u ) << 1u ) | ( ( bits & 0xAAAAAAAAu ) >> 1u );
    bits = ( ( bits & 0x33333333u ) << 2u ) | ( ( bits & 0xCCCCCCCCu ) >> 2u );
    bits = ( ( bits & 0x0F0F0F0Fu ) << 4u ) | ( ( bits & 0xF0F0F0F0u ) >> 4u );
    bits = ( ( bits & 0x00FF00FFu ) << 8u ) | ( ( bits & 0xFF00FF00u ) >> 8u );
    return Scalar( float( bits ) * 2.3283064365386963e-10_ra ); // / 0x100000000
}

FibonacciPointSet::FibonacciPointSet( size_t n ) : seq( n ) {}

size_t FibonacciPointSet::range() {
    return seq.range();
}
Ra::Core::Vector2 FibonacciPointSet::operator()( size_t i ) {
    return { seq( i ), Scalar( i ) / Scalar( range() ) };
}

HammersleyPointSet::HammersleyPointSet( size_t number ) : n( number ) {}

size_t HammersleyPointSet::range() {
    return n;
}

Ra::Core::Vector2 HammersleyPointSet::operator()( size_t i ) {
    return { Scalar( i ) / Scalar( range() ), seq( i ) };
}

MersenneTwisterPointSet::MersenneTwisterPointSet( size_t number ) :
    gen( 0 ), seq( 0._ra, 1._ra ), n( number ) {}

size_t MersenneTwisterPointSet::range() {
    return n;
}
Ra::Core::Vector2 MersenneTwisterPointSet::operator()( size_t ) {
    return { seq( gen ), seq( gen ) };
}

} // namespace Random
} // namespace Core
} // namespace Ra
