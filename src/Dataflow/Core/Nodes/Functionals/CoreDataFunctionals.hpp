#pragma once
#include <Dataflow/Core/Nodes/Functionals/FilterNode.hpp>
#include <Dataflow/Core/Nodes/Functionals/ReduceNode.hpp>
#include <Dataflow/Core/Nodes/Functionals/TransformNode.hpp>

#include <Dataflow/Core/Nodes/Functionals/BinaryOpNode.hpp>

#include <Core/Containers/VectorArray.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Color.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace Functionals {

using namespace Ra::Core;

// This macro does not end with semicolon. To be added when calling it
#define DECLARE_FUNCTIONALS( SUFFIX, TYPE )                                      \
    using ArrayFilter##SUFFIX      = FilterNode<Ra::Core::VectorArray<TYPE>>;    \
    using ArrayTransformer##SUFFIX = TransformNode<Ra::Core::VectorArray<TYPE>>; \
    using ArrayReducer##SUFFIX     = ReduceNode<Ra::Core::VectorArray<TYPE>>;    \
    using BinaryOp##SUFFIX         = BinaryOpNode<TYPE>;                         \
    using BinaryOp##SUFFIX##Array  = BinaryOpNode<Ra::Core::VectorArray<TYPE>>;  \
    using BinaryPredicate##SUFFIX  = BinaryOpNode<TYPE, TYPE, bool>

/* Not yet supported
    using BinaryPredicate##SUFFIX##Array =  BinaryOpNode<Ra::Core::VectorArray<TYPE>,
   Ra::Core::VectorArray<TYPE>, bool>
*/

DECLARE_FUNCTIONALS( Float, float );
DECLARE_FUNCTIONALS( Double, double );
DECLARE_FUNCTIONALS( Scalar, Scalar );
DECLARE_FUNCTIONALS( Int, int );
DECLARE_FUNCTIONALS( UInt, unsigned int );
DECLARE_FUNCTIONALS( Color, Utils::Color );
DECLARE_FUNCTIONALS( Vector2f, Vector2f );
DECLARE_FUNCTIONALS( Vector2d, Vector2d );
DECLARE_FUNCTIONALS( Vector3f, Vector3f );
DECLARE_FUNCTIONALS( Vector3d, Vector3d );
DECLARE_FUNCTIONALS( Vector4f, Vector4f );
DECLARE_FUNCTIONALS( Vector4d, Vector4d );
DECLARE_FUNCTIONALS( Vector2i, Vector2i );
DECLARE_FUNCTIONALS( Vector3i, Vector3i );
DECLARE_FUNCTIONALS( Vector4i, Vector4i );
DECLARE_FUNCTIONALS( Vector2ui, Vector2ui );
DECLARE_FUNCTIONALS( Vector3ui, Vector3ui );
DECLARE_FUNCTIONALS( Vector4ui, Vector4ui );

// TODO, instanciate nodes for otherypes ?

#undef DECLARE_FUNCTIONALS
} // namespace Functionals
} // namespace Core
} // namespace Dataflow
} // namespace Ra
