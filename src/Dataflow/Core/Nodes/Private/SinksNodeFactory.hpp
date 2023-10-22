#pragma once
#include <Dataflow/RaDataflow.hpp>

#include <Dataflow/Core/NodeFactory.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace NodeFactoriesManager {
namespace Private {

void registerSinksFactories( NodeFactorySet::mapped_type factory );

} // namespace Private
} // namespace NodeFactoriesManager
} // namespace Core
} // namespace Dataflow
} // namespace Ra
