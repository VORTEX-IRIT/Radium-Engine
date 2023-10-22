#include <Dataflow/Core/Nodes/Functionals/CoreDataFunctionals.hpp>
#include <Dataflow/Core/Nodes/Private/FunctionalsNodeFactory.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

namespace NodeFactoriesManager {

namespace Private {

/** TODO : replace this by factory autoregistration at compile time */
#define ADD_FUNCTIONALS_TO_FACTORY( FACTORY, NAMESPACE, SUFFIX )                \
    FACTORY->registerNodeCreator<NAMESPACE::ArrayFilter##SUFFIX>(               \
        NAMESPACE::ArrayFilter##SUFFIX::getTypename() + "_", #NAMESPACE );      \
    FACTORY->registerNodeCreator<NAMESPACE::ArrayTransformer##SUFFIX>(          \
        NAMESPACE::ArrayTransformer##SUFFIX::getTypename() + "_", #NAMESPACE ); \
    FACTORY->registerNodeCreator<NAMESPACE::ArrayReducer##SUFFIX>(              \
        NAMESPACE::ArrayReducer##SUFFIX::getTypename() + "_", #NAMESPACE );     \
    FACTORY->registerNodeCreator<NAMESPACE::BinaryOp##SUFFIX>(                  \
        NAMESPACE::BinaryOp##SUFFIX::getTypename() + "_", #NAMESPACE );         \
    FACTORY->registerNodeCreator<NAMESPACE::BinaryOp##SUFFIX##Array>(           \
        NAMESPACE::BinaryOp##SUFFIX##Array::getTypename() + "_", #NAMESPACE );  \
    FACTORY->registerNodeCreator<NAMESPACE::BinaryPredicate##SUFFIX>(           \
        NAMESPACE::BinaryPredicate##SUFFIX::getTypename() + "_", #NAMESPACE )

/*
 * not yet supported
    FACTORY->registerNodeCreator<NAMESPACE::BinaryPredicate##SUFFIX##Array>(    \
        NAMESPACE::BinaryPredicate##SUFFIX##Array::getTypename() + "_", #NAMESPACE )
*/

void registerFunctionalsFactories( NodeFactorySet::mapped_type factory ) {
    /* --- Functionals */
#ifdef CORE_USE_DOUBLE
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Float );
#else
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Double );
#endif
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Scalar );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Int );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, UInt );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Color );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector2f );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector2d );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector3f );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector3d );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector4f );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector4d );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector2i );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector2ui );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector3i );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector3ui );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector4i );
    ADD_FUNCTIONALS_TO_FACTORY( factory, Functionals, Vector4ui );
}
} // namespace Private
} // namespace NodeFactoriesManager
} // namespace Core
} // namespace Dataflow
} // namespace Ra
