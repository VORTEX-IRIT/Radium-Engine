#pragma once

/// \file Types.hpp
/// This file contains definitions of aliases for basic vector classes.

#include <Core/RaCore.hpp>

#include <Eigen/Core>
#include <Eigen/Geometry>
#include <Eigen/Sparse>

/// Radium Namespaces prefix
namespace Ra {

/// This namespace contains everything "low level", related to data, datastuctures, and computation.
namespace Core {

/// \name
/// Common vector types
/// \{
using VectorN  = Eigen::Matrix<Scalar, Eigen::Dynamic, 1>;
using VectorNf = Eigen::VectorXf;
using VectorNd = Eigen::VectorXd;

using Vector4  = Eigen::Matrix<Scalar, 4, 1>;
using Vector4f = Eigen::Vector4f;
using Vector4d = Eigen::Vector4d;

using Vector3  = Eigen::Matrix<Scalar, 3, 1>;
using Vector3f = Eigen::Vector3f;
using Vector3d = Eigen::Vector3d;

using Vector2  = Eigen::Matrix<Scalar, 2, 1>;
using Vector2f = Eigen::Vector2f;
using Vector2d = Eigen::Vector2d;

using VectorNi = Eigen::VectorXi;
using Vector2i = Eigen::Vector2i;
using Vector3i = Eigen::Vector3i;
using Vector4i = Eigen::Vector4i;

using VectorNui = Eigen::Matrix<uint, Eigen::Dynamic, 1>;
using Vector1ui = Eigen::Matrix<uint, 1, 1>;
using Vector2ui = Eigen::Matrix<uint, 2, 1>;
using Vector3ui = Eigen::Matrix<uint, 3, 1>;
using Vector4ui = Eigen::Matrix<uint, 4, 1>;
/// \}

/// \name
/// Ray types
/// \{
using Ray  = Eigen::ParametrizedLine<Scalar, 3>;
using Rayf = Eigen::ParametrizedLine<float, 3>;
using Rayd = Eigen::ParametrizedLine<double, 3>;
/// \}

/// \name
/// Common matrix types
/// \{
using MatrixN = Eigen::Matrix<Scalar, Eigen::Dynamic, Eigen::Dynamic>;
using Matrix4 = Eigen::Matrix<Scalar, 4, 4>;
using Matrix3 = Eigen::Matrix<Scalar, 3, 3>;
using Matrix2 = Eigen::Matrix<Scalar, 2, 2>;

using MatrixNf = Eigen::MatrixXf;
using Matrix4f = Eigen::Matrix4f;
using Matrix3f = Eigen::Matrix3f;
using Matrix2f = Eigen::Matrix2f;

using MatrixNd = Eigen::MatrixXd;
using Matrix4d = Eigen::Matrix4d;
using Matrix3d = Eigen::Matrix3d;
using Matrix2d = Eigen::Matrix2d;

using MatrixNui = Eigen::Matrix<uint, Eigen::Dynamic, Eigen::Dynamic>;

// using Diagonal = Eigen::DiagonalMatrix< Scalar, Eigen::Dynamic >;
using Diagonal =
    Eigen::SparseMatrix<Scalar>; // Not optimized for Diagonal matrices, but the operations between
                                 // Sparse and Diagonal are not defined
using Sparse = Eigen::SparseMatrix<Scalar>;
/// \}

/// \name
/// Transforms and rotations
/// \{
using Quaternion  = Eigen::Quaternion<Scalar>;
using Quaternionf = Eigen::Quaternionf;
using Quaterniond = Eigen::Quaterniond;

using Transform  = Eigen::Transform<Scalar, 3, Eigen::Affine>;
using Transformf = Eigen::Affine3f;
using Transformd = Eigen::Affine3d;

using Aabb  = Eigen::AlignedBox<Scalar, 3>;
using Aabbf = Eigen::AlignedBox3f;
using Aabbd = Eigen::AlignedBox3d;

using AngleAxis  = Eigen::AngleAxis<Scalar>;
using AngleAxisf = Eigen::AngleAxisf;
using AngleAxisd = Eigen::AngleAxisd;

using Translation  = Eigen::Translation<Scalar, 3>;
using Translationf = Eigen::Translation3f;
using Translationd = Eigen::Translation3d;
/// \}

} // namespace Core
} // namespace Ra
