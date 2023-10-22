#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/NodeFactory.hpp>
namespace Ra {
namespace Dataflow {
namespace Core {
namespace NodeFactoriesManager {

/**
 * \brief Create the node system default factory.
 *
 * The default factory of the node system contains instantiation of the nodes below for the
 * following type
 *   - Scalar, float, double int, unsigned int
 *   - Color, VectorDf, VectorDd, VectorDi, VectorDi (where D in {2, 3, 4}
 *
 * List of instanced nodes for any TYPE above
 *   - SingleDataSourceNode<TYPE> and SingleDataSourceNode<Ra::Core::VectorArray<TYPE>>
 *   - FunctionSourceNode<TYPE, const TYPE&>, FunctionSourceNode<TYPE, const TYPE&, const TYPE&>
 *   - FunctionSourceNode<bool, const TYPE&>, FunctionSourceNode<bool, const TYPE&, const TYPE&>
 *   - SinkNode<TYPE>, SinkNode<Ra::Core::VectorArray<TYPE>
 *   - FilterNode<Ra::Core::VectorArray<TYPE>>
 *   - TransformNode<Ra::Core::VectorArray<TYPE>>, ReduceNode<Ra::Core::VectorArray<TYPE>>
 *   - BinaryOpNode<TYPE>, BinaryOpNode<Ra::Core::VectorArray<TYPE>>, BinaryOpNode<TYPE, TYPE, bool>
 *
 * All these node might be serialized/unserialized without any additional nor custom factory.
 *
 * If needed, the definition of all these type aliases can be included using one of the headers
 *  - #include <Dataflow/Core/Nodes/Functionals/CoreDataFunctionals.hpp>
 *  - #include <Dataflow/Core/Nodes/Sinks/CoreDataSinks.hpp>
 *  - #include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>
 */
RA_DATAFLOW_API void registerStandardFactories();
} // namespace NodeFactoriesManager
} // namespace Core
} // namespace Dataflow
} // namespace Ra
