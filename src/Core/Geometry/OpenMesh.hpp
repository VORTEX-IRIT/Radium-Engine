
#pragma once

#include <Core/RaCore.hpp>
#include <Core/Types.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>

// We use Eigen::Matrix as base type to represent matrices in OpenMesh.

// This is not standard, and several namespace functions are not available in OpenMesh
// for Eigen::Matrix.
// We need these functions to be defined before including OpenMesh, otherwise some
// compilers (e.g. MSVC) will not be able to compile the code.
namespace Eigen {

template <typename Derived>
typename Derived::Scalar dot( Eigen::MatrixBase<Derived> const& v1,
                              Eigen::MatrixBase<Derived> const& v2 ) {
    return v1.dot( v2 );
}

template <typename Derived>
typename Derived::Scalar norm( Eigen::MatrixBase<Derived> const& v1 ) {
    return v1.norm();
}

template <typename Derived>
Eigen::MatrixBase<Derived>& normalize( Eigen::MatrixBase<Derived>& v1 ) {
    v1.normalize();
    return v1;
}

template <typename Derived>
Eigen::MatrixBase<Derived>& vectorize( Eigen::MatrixBase<Derived>& v1, Scalar a ) {
    v1.setConstant( a );
    return v1;
}

template <typename Derived>
typename Derived::Scalar sqrnorm( Eigen::MatrixBase<Derived> const& v1 ) {
    return v1.squaredNorm();
}

template <typename Derived>
typename Derived::template cross_product_return_type<Derived>::type
cross( Eigen::MatrixBase<Derived> const& v1, Eigen::MatrixBase<Derived> const& v2 ) {
    return v1.cross( v2 );
}
} // namespace Eigen

namespace OpenMesh {
template <>
struct vector_traits<Ra::Core::Vector3> {
    using vector_type         = Ra::Core::Vector3;
    using value_type          = Scalar;
    static const size_t size_ = 3;
    static size_t size() { return 3; }
};
} // namespace OpenMesh
