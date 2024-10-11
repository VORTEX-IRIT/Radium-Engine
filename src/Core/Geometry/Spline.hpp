#pragma once

#include <Core/Containers/VectorArray.hpp>
#include <Core/Math/Math.hpp>
#include <Core/RaCore.hpp>

#include <algorithm>
#include <vector>

namespace Ra {
namespace Core {
namespace Geometry {
/**
 * \class Spline
 *
 * \brief Handling spline curves of arbitrary dimensions
 * \note This class use the efficient blossom algorithm to compute a position on
 * the curve.
 * \tparam D : dimension of the curve.
 * \tparam K  :order of the curve (min 2)
 */
template <uint D, uint K = 2>
class Spline
{
  public:
    enum Type {
        UNIFORM,
        OPEN_UNIFORM ///< Connected to the first and last control points
    };

    using Vector = typename Eigen::Matrix<Scalar, D, 1>;

  public:
    /// Type of the nodal vector
    /// \param type : nodal vector type (uniform, open_uniform)
    /// This will define the behavior of the spline with its control points
    /// as well as its speed according to its parameter.
    explicit inline Spline( Type type = OPEN_UNIFORM );

    /// Set the position of the spline control points.
    inline void setCtrlPoints( const Core::VectorArray<Vector>& points );

    /// Get the control points of the spline
    inline const Core::VectorArray<Vector>& getCtrlPoints() const;

    /// The the nodal vector type
    inline void setType( Type type );

    /// Evaluate position of the spline
    /// \param u : curve parameter ranging from [0; 1]
    inline Vector f( Scalar u ) const;

    /// Evaluate speed of the spline
    inline Vector df( Scalar u ) const;

  private:
    // -------------------------------------------------------------------------
    /// \name Class tools
    // -------------------------------------------------------------------------

    inline void assertSplines() const;

    /// set value and size of the nodal vector depending on the current number
    /// of control points
    inline void setNodalVector();

    /// Set values of the nodal vector to be uniform
    inline void setNodeToUniform();

    /// Set values of the nodal vector to be open uniform
    inline void setNodeToOpenUniform();

    /// Evaluate the equation of a splines using the blossom algorithm
    /// \param u : the curve parameter which range from the values
    /// [node[k-1]; node[point.size()]]
    /// \param point : the control points which size must be at least equal to
    /// the order of the spline (point.size() >= k)
    /// \param k : the spline order (degree == k-1)
    /// \param node : the nodal vector which defines the speed of the spline
    /// parameter u. The nodal vector size must be equal to (k + point.size())
    /// \param off : offset to apply to the nodal vector 'node' before reading
    /// from it. this is useful to compute derivatives.
    static inline Vector eval( Scalar u,
                               const Core::VectorArray<Vector>& points,
                               const std::vector<Scalar>& node,
                               uint k,
                               int off = 0 );

    static inline Vector evalRec( Scalar u,
                                  const Core::VectorArray<Vector>& points,
                                  const std::vector<Scalar>& node,
                                  uint k );

    // -------------------------------------------------------------------------
    /// \name attributes
    // -------------------------------------------------------------------------

    Core::VectorArray<Vector> m_points; ///< Control points
    Core::VectorArray<Vector> m_vecs;   ///< Control points differences
    std::vector<Scalar> m_node;         ///< Nodal vector
    Type m_type;                        ///< Nodal vector type
};

template <uint D, uint K>
inline Spline<D, K>::Spline( typename Spline<D, K>::Type type ) : m_type( type ) {
    static_assert( K >= 2, "Order must be at least two" );
    m_points.resize( K, Vector::Zero() );
    m_vecs.resize( K - 1, Vector::Zero() );
    m_node.resize( K + K, 0.f );
    assertSplines();
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline void
Spline<D, K>::setCtrlPoints( const Core::VectorArray<typename Spline<D, K>::Vector>& points ) {
    m_points = points;
    m_vecs.resize( points.size() - 1, Vector::Zero() );
    for ( uint i = 0; i < m_vecs.size(); ++i ) {
        m_vecs[i] = m_points[i + 1] - m_points[i];
    }
    setNodalVector();
    assertSplines();

    for ( uint i = 0; i < m_vecs.size(); ++i ) {
        m_vecs[i] /= m_node[K + i] - m_node[i + 1];
    }
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline const Core::VectorArray<typename Spline<D, K>::Vector>& Spline<D, K>::getCtrlPoints() const {
    return m_points;
}

// -----------------------------------------------------------------------------

/// The the nodal vector type
template <uint D, uint K>
inline void Spline<D, K>::setType( Type type ) {
    m_type = type;
    setNodalVector();
    assertSplines();
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline typename Spline<D, K>::Vector Spline<D, K>::f( Scalar u ) const {
    u = std::clamp( u, Scalar( 0 ), Scalar( 1 ) );
    return eval( u, m_points, m_node, K );
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline typename Spline<D, K>::Vector Spline<D, K>::df( Scalar u ) const {
    u = std::clamp( u, Scalar( 0 ), Scalar( 1 ) );
    return eval( u, m_vecs, m_node, K - 1, 1 ) * Scalar( K - 1 );
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline void Spline<D, K>::assertSplines() const {
    CORE_ASSERT( m_points.size() >= K, "Not enough points" );
    CORE_ASSERT( m_node.size() == ( K + m_points.size() ), "Wrong nodal vector size" );
    CORE_ASSERT( m_points.size() == ( m_vecs.size() + 1 ), "Wrong point / diffs size" );
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline void Spline<D, K>::setNodalVector() {
    switch ( m_type ) {
    case OPEN_UNIFORM:
        setNodeToOpenUniform();
        break;
    case UNIFORM:
        setNodeToUniform();
        break;
    }
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline void Spline<D, K>::setNodeToUniform() {
    const uint n = m_points.size() - 1;
    m_node.resize( K + n + 1 );

    Scalar step = 1.f / Scalar( n - K + 2 );
    for ( uint i = 0; i < m_node.size(); ++i ) {
        m_node[i] = Scalar( i ) * step - step * (Scalar)( K - 1 );
    }
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline void Spline<D, K>::setNodeToOpenUniform() {
    m_node.resize( K + m_points.size() );

    uint acc = 1;
    for ( uint i = 0; i < m_node.size(); ++i ) {
        if ( i < K ) { m_node[i] = 0.f; }
        else if ( i >= ( m_points.size() + 1 ) ) { m_node[i] = 1.f; }
        else {
            m_node[i] = Scalar( acc ) / Scalar( m_points.size() + 1 - K );
            acc++;
        }
    }
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline typename Spline<D, K>::Vector Spline<D, K>::eval( Scalar u,
                                                         const Core::VectorArray<Vector>& points,
                                                         const std::vector<Scalar>& node,
                                                         uint k,
                                                         int off ) {
    CORE_ASSERT( k >= 2, "K must be at least 2" );
    CORE_ASSERT( points.size() >= k, "Not enough points" );
    uint dec = 0;
    // TODO: better search with dichotomy ?
    // TODO: check for overflow
    while ( u > node[dec + k + off] ) {
        dec++;
    }

    // TODO: use buffers in attributes for better performances ?
    Core::VectorArray<Vector> pOut( k, Vector::Zero() );
    for ( uint i = dec, j = 0; i < ( dec + k ); ++i, ++j ) {
        pOut[j] = points[i];
    }

    std::vector<Scalar> nodeOut( k + k - 2, Scalar( 0 ) );
    for ( uint i = ( dec + 1 ), j = 0; i < ( dec + k + k - 1 ); ++i, ++j ) {
        nodeOut[j] = node[i + off];
    }
    return evalRec( u, pOut, nodeOut, k );
}

// -----------------------------------------------------------------------------

template <uint D, uint K>
inline typename Spline<D, K>::Vector Spline<D, K>::evalRec( Scalar u,
                                                            const Core::VectorArray<Vector>& points,
                                                            const std::vector<Scalar>& node,
                                                            uint k ) {
    if ( points.size() == 1 ) { return points[0]; }

    // TODO: use buffers in attributes for better performances ?
    Core::VectorArray<Vector> pOut( k - 1, Vector::Zero() );

    for ( uint i = 0; i < ( k - 1 ); ++i ) {
        const Scalar n0 = node[i + k - 1];
        const Scalar n1 = node[i];
        const Scalar f0 = ( n0 - u ) / ( n0 - n1 );
        const Scalar f1 = ( u - n1 ) / ( n0 - n1 );

        pOut[i] = points[i] * f0 + points[i + 1] * f1;
    }

    std::vector<Scalar> nodeOut( node.size() - 2 );

    for ( uint i = 1; i < node.size() - 1; ++i ) {
        nodeOut[i - 1] = node[i];
    }
    return evalRec( u, pOut, nodeOut, k - 1 );
}
} // namespace Geometry
} // namespace Core
} // namespace Ra
