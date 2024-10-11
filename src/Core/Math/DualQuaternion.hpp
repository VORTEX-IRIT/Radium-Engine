#pragma once

#include <Core/Math/LinearAlgebra.hpp> // Quaternion operators
#include <Core/RaCore.hpp>
#include <Core/Types.hpp>

namespace Ra {
namespace Core {
/// Dual quaternions are based on the dual-numbers algebra, somewhat
/// analogous to complex numbers, but with the imaginary unit `e` defined
/// such as e*e = 0 ; and using quaternions as the non-dual and dual part.
/// Unit dual quaternions can represent any rigid transformation
/// (rotation + translation).

/// A good reference.
/// http://www.euclideanspace.com/maths/algebra/realNormedAlgebra/other/dualQuaternion/index.htm

class DualQuaternion
{

  public:
    /// Construct an uninitialized dual quaternion.
    inline DualQuaternion() {}

    /// Construct a dual-quaternion from two quaternions.
    inline DualQuaternion( const Quaternion& q0, const Quaternion& qe ) : m_q0( q0 ), m_qe( qe ) {}

    /// Construct a dual-quaternion from a rigid transform
    /// Any non-rigid component (e.g. scale and shear) will be ignored.
    explicit inline DualQuaternion( const Core::Transform& tr );

    /// Default copy constructor and assignment operator.
    DualQuaternion( const DualQuaternion& other )      = default;
    DualQuaternion& operator=( const DualQuaternion& ) = default;

    /// Getters and setters

    inline const Quaternion& getQ0() const;
    inline void setQ0( const Quaternion& q0 );
    inline const Quaternion& getQe() const;
    inline void setQe( const Quaternion& qe );

    /// Operators

    inline DualQuaternion operator+( const DualQuaternion& other ) const;
    inline DualQuaternion operator*( Scalar scalar ) const;

    inline DualQuaternion& operator+=( const DualQuaternion& other );
    inline DualQuaternion& operator*=( Scalar scalar );

    /// Other methods

    /// Set the dual-quaternion from a rigid transform.
    /// Any non-rigid component (e.g. scale and shear) will be ignored.
    inline void setFromTransform( const Transform& t );

    /// Return the corresponding rigid transform. Assume a unit dual quaternion.
    inline Transform getTransform() const;

    /// Normalize the quaternion with the dual-number norm (divides q0 and qe
    /// by q0's norm). Will assert if the norm is zero in debug builds.
    inline void normalize();

    /// Apply the transform represented by the dual quaternion to given vector.
    /// equivalent to translate( rotate (p)).
    inline Vector3 transform( const Vector3& p ) const;

    /// Apply only the rotational part of the dual quaternion to the given vector.
    inline Vector3 rotate( const Vector3& p ) const;

    /// Apply only the translational part of the dual quaternion to the given vector.
    inline Vector3 translate( const Vector3& p ) const;

  private:
    /// Non-dual part (representing the rotation)
    Quaternion m_q0;
    /// Dual part (representing the translation)
    Quaternion m_qe;
};

/// Pre-multiplication of dual quaternion.
inline DualQuaternion operator*( Scalar scalar, const DualQuaternion& dq );

inline const Quaternion& DualQuaternion::getQ0() const {
    return m_q0;
}

inline void DualQuaternion::setQ0( const Quaternion& q0 ) {
    m_q0 = q0;
}

inline const Quaternion& DualQuaternion::getQe() const {
    return m_qe;
}

inline void DualQuaternion::setQe( const Quaternion& qe ) {
    m_qe = qe;
}

inline DualQuaternion DualQuaternion::operator+( const DualQuaternion& other ) const {
    return DualQuaternion( m_q0 + other.m_q0, m_qe + other.m_qe );
}

inline DualQuaternion DualQuaternion::operator*( Scalar scalar ) const {
    return DualQuaternion( scalar * m_q0, scalar * m_qe );
}

inline DualQuaternion& DualQuaternion::operator+=( const DualQuaternion& other ) {
    *this = *this + other;
    return *this;
}

inline DualQuaternion& DualQuaternion::operator*=( Scalar scalar ) {
    *this = *this * scalar;
    return *this;
}

inline void DualQuaternion::normalize() {
    const Scalar norm = m_q0.norm();
    CORE_ASSERT( norm != 0, "Normalizing a null quaternion." );
    m_q0 = m_q0 / norm;
    m_qe = m_qe / norm;
}

inline Vector3 DualQuaternion::transform( const Vector3& p ) const {
    CORE_ASSERT( Ra::Core::Math::areApproxEqual( m_q0.norm(), 1_ra ),
                 "Dual quaternion not normalized" );
    return translate( rotate( p ) );
}

inline Vector3 DualQuaternion::rotate( const Vector3& p ) const {
    CORE_ASSERT( Ra::Core::Math::areApproxEqual( m_q0.norm(), 1_ra ),
                 "Dual quaternion not normalized" );
    return m_q0.toRotationMatrix() * p;
}

inline Vector3 DualQuaternion::translate( const Vector3& p ) const {
    Vector3 v0 = m_q0.vec();
    Vector3 ve = m_qe.vec();
    return p + ( ( ve * m_q0.w() - v0 * m_qe.w() + v0.cross( ve ) ) * 2_ra );
}

inline DualQuaternion::DualQuaternion( const Core::Transform& tr ) {
    setFromTransform( tr );
}

inline Transform DualQuaternion::getTransform() const {
    // Assume the dual quat is normalized.
    CORE_ASSERT( Ra::Core::Math::areApproxEqual( m_q0.norm(), 1_ra ),
                 "Dual quaternion not normalized" );

    Transform result;
    result.linear()      = m_q0.toRotationMatrix();
    result.translation() = ( 2_ra * m_q0 * m_qe.conjugate() ).vec();
    return result;
}

inline void DualQuaternion::setFromTransform( const Transform& t ) {
    m_q0                = Quaternion( t.rotation() );
    Core::Vector4 trans = Core::Vector4::Zero();
    trans.head<3>()     = t.translation();
    m_qe                = 0.5f * Quaternion( trans ) * m_q0;
}

inline DualQuaternion operator*( Scalar scalar, const DualQuaternion& dq ) {
    return dq * scalar;
}

} // namespace Core
} // namespace Ra
