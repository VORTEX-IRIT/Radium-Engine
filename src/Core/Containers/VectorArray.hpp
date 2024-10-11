#pragma once

#include <Core/Containers/AlignedStdVector.hpp>
#include <Core/RaCore.hpp>
#include <Core/Utils/ContainerIntrospectionInterface.hpp>

#include <type_traits> // std::is_integral

namespace Ra {
namespace Core {

/** Internal class used to compute types and values used by VectorArray, to handle both
 * eigen fixed sized matrix and scalar type as mappable to eigen matrices.
 */
template <typename V, bool isArithmetic, bool isEigen>
struct VectorArrayTypeHelperInternal {};

/**
 * Arithmetic and fixed sized eigen types are mappable to Eigen matrices.
 * other types are storable in a VectorArray, but could not get map (e.g. NumberOfComponents <= 0).
 */
template <typename V>
struct VectorArrayTypeHelper : public VectorArrayTypeHelperInternal<
                                   V,
                                   std::is_arithmetic<V>::value || std::is_enum<V>::value,
                                   std::is_base_of<typename Eigen::MatrixBase<V>, V>::value> {};

/**
 * \brief This class implements ContainerIntrospectionInterface for AlignedStdVector.
 *
 * It provides Eigen::Map functionality if the underlying component allows it (i.e. fixed size).
 */
template <typename V>
class VectorArray : public AlignedStdVector<V>, public Utils::ContainerIntrospectionInterface
{
  private:
    using TypeHelper = VectorArrayTypeHelper<V>;

  public:
    static constexpr int NumberOfComponents = TypeHelper::NumberOfComponents;
    /**
     * component_type is different from value_type defined in Base class, e.g. for
     * VectorArray<Vector3> value_type is Vector3 while component_type is Scalar.
     * but for VectorArray<Scalar> value_type = component_type = Scalar.
     */
    using component_type = typename TypeHelper::component_type;
    using Matrix    = Eigen::Matrix<component_type, TypeHelper::NumberOfComponents, Eigen::Dynamic>;
    using MatrixMap = Eigen::Map<Matrix>;
    using ConstMatrixMap = Eigen::Map<const Matrix>;

    /** Inheriting constructors from std::vector */
    using AlignedStdVector<V>::AlignedStdVector;

    /** \name Container Introsection implementation */
    /// \{
    size_t getSize() const override { return this->size(); }
    size_t getNumberOfComponents() const override { return std::max( 0, NumberOfComponents ); }
    size_t getBufferSize() const override { return getSize() * sizeof( V ); }
    int getStride() const override { return sizeof( V ); }
    const void* dataPtr() const override { return this->data(); }
    /// \}

    /** \name Eigen::Map getter
     * Map data to an Eigen::Matrix, only defined when NumberOfComponents > 0 (e.g. for arithmetic
     * types and fixed size eigen vectors).
     */
    /// \{
    /** Returns the array as an Eigen Matrix Map. */
    template <int N = NumberOfComponents>
    std::enable_if_t<( N > 0 ), MatrixMap> getMap() {
        CORE_ASSERT( !this->empty(), "Cannot map an empty vector " );
        return MatrixMap( TypeHelper::getData( this ),
                          TypeHelper::NumberOfComponents,
                          Eigen::Index( this->size() ) );
    }

    /** Returns the array as an Eigen Matrix Map (const version). */
    template <int N = NumberOfComponents>
    std::enable_if_t<( N > 0 ), ConstMatrixMap> getMap() const {
        CORE_ASSERT( !this->empty(), "Cannot map an empty vector " );
        return ConstMatrixMap( TypeHelper::getConstData( this ),
                               TypeHelper::NumberOfComponents,
                               Eigen::Index( this->size() ) );
    }
    /// \}
};

template <typename V>
struct VectorArrayTypeHelperInternal<V, true, false> {
    using component_type                    = V; // arithmetic types are component types
    static constexpr int NumberOfComponents = 1;
    static inline component_type* getData( VectorArray<V>* v ) { return v->data(); }
    static inline const component_type* getConstData( const VectorArray<V>* v ) {
        return v->data();
    }
};

template <typename V>
struct VectorArrayTypeHelperInternal<V, false, true> {
    using component_type                    = typename V::Scalar;   // use eigen scalar as component
    static constexpr int NumberOfComponents = V::RowsAtCompileTime; // i.e. -1 for dynamic size
    static inline component_type* getData( VectorArray<V>* v ) { return v->data()->data(); }
    static inline const component_type* getConstData( const VectorArray<V>* v ) {
        return v->data()->data();
    }
};

template <typename V>
struct VectorArrayTypeHelperInternal<V, false, false> {
    using component_type                    = V;
    static constexpr int NumberOfComponents = 0; // no component for other types, i.e. not mappable
    static inline component_type* getData( VectorArray<V>* v ) { return v->data(); }
    static inline const component_type* getConstData( const VectorArray<V>* v ) {
        return v->data();
    }
};

// Convenience aliases
using Vector1Array = VectorArray<Scalar>;
#define DEFINE_CONVENIENCE_MATRIX_ALIAS( NAME, TYPE, DIM ) \
    using NAME = VectorArray<Eigen::Matrix<TYPE, DIM, 1>>;
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector2Array, Scalar, 2 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector3Array, Scalar, 3 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector4Array, Scalar, 4 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( VectorNArray, Scalar, Eigen::Dynamic )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector1iArray, int, 1 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector2iArray, int, 2 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector3iArray, int, 3 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector4iArray, int, 4 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( VectorNiArray, int, Eigen::Dynamic )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector1uArray, uint, 1 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector2uArray, uint, 2 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector3uArray, uint, 3 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( Vector4uArray, uint, 4 )
DEFINE_CONVENIENCE_MATRIX_ALIAS( VectorNuArray, uint, Eigen::Dynamic )
#undef DEFINE_CONVENIENCE_MATRIX_ALIAS

// Notes :
// Using a map for eigen integration was recommended by [1].
// According to this document [2], it is necessary to use a special allocator to enforce
// alignment...
// But actually it is not necessary to do so in C++11 [3] (it is actually harmful
// as it removes some vector features such as initializer lists).
// Finally we use our own aligned allocator.

// [1] https://forum.kde.org/viewtopic.php?f=74&t=126959
// [2] http://eigen.tuxfamily.org/dox-devel/group__TopicStlContainers.html
// [3] http://eigen.tuxfamily.org/bz/show_bug.cgi?id=829

} // namespace Core
} // namespace Ra
