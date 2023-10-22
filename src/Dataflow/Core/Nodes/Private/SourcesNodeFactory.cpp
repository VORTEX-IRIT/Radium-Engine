#include <Dataflow/Core/Nodes/Private/SourcesNodeFactory.hpp>
#include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>

namespace Ra {
namespace Dataflow {
namespace Core {

namespace NodeFactoriesManager {

namespace Private {

/** TODO : replace this by factory autoregistration at compile time */
#define ADD_SOURCES_TO_FACTORY( FACTORY, NAMESPACE, PREFIX )                        \
    FACTORY->registerNodeCreator<NAMESPACE::PREFIX##Source>(                        \
        NAMESPACE::PREFIX##Source::getTypename() + "_", #NAMESPACE );               \
    FACTORY->registerNodeCreator<NAMESPACE::PREFIX##ArraySource>(                   \
        NAMESPACE::PREFIX##ArraySource::getTypename() + "_", #NAMESPACE );          \
    FACTORY->registerNodeCreator<NAMESPACE::PREFIX##UnaryFunctionSource>(           \
        NAMESPACE::PREFIX##UnaryFunctionSource::getTypename() + "_", #NAMESPACE );  \
    FACTORY->registerNodeCreator<NAMESPACE::PREFIX##BinaryFunctionSource>(          \
        NAMESPACE::PREFIX##BinaryFunctionSource::getTypename() + "_", #NAMESPACE ); \
    FACTORY->registerNodeCreator<NAMESPACE::PREFIX##UnaryPredicateSource>(          \
        NAMESPACE::PREFIX##UnaryPredicateSource::getTypename() + "_", #NAMESPACE ); \
    FACTORY->registerNodeCreator<NAMESPACE::PREFIX##BinaryPredicateSource>(         \
        NAMESPACE::PREFIX##BinaryPredicateSource::getTypename() + "_", #NAMESPACE )

void registerSourcesFactories( NodeFactorySet::mapped_type factory ) {
    /* --- Sources --- */
    // bool could not be declared as others, because of the specificity of std::vector<bool> that is
    // not compatible with Ra::Core::VectorArray implementation see
    // https://en.cppreference.com/w/cpp/container/vector_bool Right now, there is no
    // Ra::Core::VectorArray of bool
    factory->registerNodeCreator<Sources::BooleanSource>(
        Sources::BooleanSource::getTypename() + "_", "Sources" );
    // prevent Scalar type collision with float or double in factory
#ifdef CORE_USE_DOUBLE
    ADD_SOURCES_TO_FACTORY( factory, Sources, Float );
#else
    ADD_SOURCES_TO_FACTORY( factory, Sources, Double );
#endif
    ADD_SOURCES_TO_FACTORY( factory, Sources, Scalar );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Int );
    ADD_SOURCES_TO_FACTORY( factory, Sources, UInt );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Color );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector2f );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector2d );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector3f );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector3d );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector4f );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector4d );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector2i );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector2ui );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector3i );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector3ui );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector4i );
    ADD_SOURCES_TO_FACTORY( factory, Sources, Vector4ui );
}
} // namespace Private
} // namespace NodeFactoriesManager
} // namespace Core
} // namespace Dataflow
} // namespace Ra
