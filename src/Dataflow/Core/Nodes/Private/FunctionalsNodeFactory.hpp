#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/NodeFactory.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace NodeFactoriesManager {
namespace Private {

void registerFunctionalsFactories( NodeFactorySet::mapped_type factory );

} // namespace Private
} // namespace NodeFactoriesManager
} // namespace Core
} // namespace Dataflow
} // namespace Ra
