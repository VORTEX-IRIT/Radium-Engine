#pragma once
#include <Dataflow/Core/Nodes/Sources/FunctionSource.hpp>
#include <Dataflow/Core/Nodes/Sources/SingleDataSourceNode.hpp>

#include <Core/Containers/VectorArray.hpp>
#include <Core/Types.hpp>
#include <Core/Utils/Color.hpp>

#include <iostream>
#include <utility>

namespace Ra {
namespace Dataflow {
namespace Core {
namespace Sources {
// TODO unify editable and non editable data sources

// declare synonyms for convenient sources
// This macro does not end with semicolon. To be added when calling it
#define DECLARE_COREDATA_SOURCES( PREFIX, TYPE )                                              \
    using PREFIX##Source                = SingleDataSourceNode<TYPE>;                         \
    using PREFIX##ArraySource           = SingleDataSourceNode<Ra::Core::VectorArray<TYPE>>;  \
    using PREFIX##UnaryFunctionSource   = FunctionSourceNode<TYPE, const TYPE&>;              \
    using PREFIX##BinaryFunctionSource  = FunctionSourceNode<TYPE, const TYPE&, const TYPE&>; \
    using PREFIX##UnaryPredicateSource  = FunctionSourceNode<bool, const TYPE&>;              \
    using PREFIX##BinaryPredicateSource = FunctionSourceNode<bool, const TYPE&, const TYPE&>;

using namespace Ra::Core;

// bool could not be declared as others, because of the specificity of std::vector<bool> that is not
// compatible with Ra::Core::VectorArray implementation see
// https://en.cppreference.com/w/cpp/container/vector_bool Right now, there is no
// Ra::Core::VectorArray of bool
using BooleanSource = SingleDataSourceNode<bool>;
DECLARE_COREDATA_SOURCES( Float, float )
DECLARE_COREDATA_SOURCES( Double, double )
DECLARE_COREDATA_SOURCES( Scalar, Scalar )
DECLARE_COREDATA_SOURCES( Int, int )
DECLARE_COREDATA_SOURCES( UInt, unsigned int )
DECLARE_COREDATA_SOURCES( Color, Utils::Color )
DECLARE_COREDATA_SOURCES( Vector2f, Vector2f )
DECLARE_COREDATA_SOURCES( Vector2d, Vector2d )
DECLARE_COREDATA_SOURCES( Vector3f, Vector3f )
DECLARE_COREDATA_SOURCES( Vector3d, Vector3d )
DECLARE_COREDATA_SOURCES( Vector4f, Vector4f )
DECLARE_COREDATA_SOURCES( Vector4d, Vector4d )
DECLARE_COREDATA_SOURCES( Vector2i, Vector2i )
DECLARE_COREDATA_SOURCES( Vector3i, Vector3i )
DECLARE_COREDATA_SOURCES( Vector4i, Vector4i )
DECLARE_COREDATA_SOURCES( Vector2ui, Vector2ui )
DECLARE_COREDATA_SOURCES( Vector3ui, Vector3ui )
DECLARE_COREDATA_SOURCES( Vector4ui, Vector4ui )

#undef DECLARE_COREDATA_SOURCES

// Partial specialisation for editable data sources
#define SPECIALIZE_EDITABLE_SOURCE( TYPE, NAME )                                             \
    template <>                                                                              \
    inline SingleDataSourceNode<TYPE>::SingleDataSourceNode( const std::string& name ) :     \
        SingleDataSourceNode( name, SingleDataSourceNode<TYPE>::getTypename() ) {            \
        setEditable( #NAME );                                                                \
    }                                                                                        \
                                                                                             \
    template <>                                                                              \
    inline void SingleDataSourceNode<TYPE>::toJsonInternal( nlohmann::json& data ) const {   \
        data[#NAME] = *getData();                                                            \
    }                                                                                        \
                                                                                             \
    template <>                                                                              \
    inline bool SingleDataSourceNode<TYPE>::fromJsonInternal( const nlohmann::json& data ) { \
        if ( data.contains( #NAME ) ) {                                                      \
            TYPE v = data[#NAME];                                                            \
            setData( v );                                                                    \
        }                                                                                    \
        return true;                                                                         \
    }

SPECIALIZE_EDITABLE_SOURCE( bool, boolean )
SPECIALIZE_EDITABLE_SOURCE( float, number )
SPECIALIZE_EDITABLE_SOURCE( double, number )
SPECIALIZE_EDITABLE_SOURCE( int, value )
SPECIALIZE_EDITABLE_SOURCE( unsigned int, value )

#undef SPECIALIZE_EDITABLE_SOURCE

// Color specialization need different implementation (as well as any Ra::Vectorxx)
template <>
inline SingleDataSourceNode<Ra::Core::Utils::Color>::SingleDataSourceNode(
    const std::string& name ) :
    SingleDataSourceNode( name, SingleDataSourceNode<Ra::Core::Utils::Color>::getTypename() ) {
    setEditable( "color" );
}

template <>
inline void
SingleDataSourceNode<Ra::Core::Utils::Color>::toJsonInternal( nlohmann::json& data ) const {
    data["color"] = Ra::Core::Utils::Color::linearRGBTosRGB( *getData() );
}

template <>
inline bool
SingleDataSourceNode<Ra::Core::Utils::Color>::fromJsonInternal( const nlohmann::json& data ) {
    if ( data.contains( "color" ) ) {
        std::array<Scalar, 3> c = data["color"];
        auto v =
            Ra::Core::Utils::Color::sRGBToLinearRGB( Ra::Core::Utils::Color( c[0], c[1], c[2] ) );
        setData( std::move( v ) );
    }
    return true;
}

// Ra::Core::Vector4 specialization
template <>
inline SingleDataSourceNode<Ra::Core::Vector4>::SingleDataSourceNode( const std::string& name ) :
    SingleDataSourceNode( name, SingleDataSourceNode<Ra::Core::Vector4>::getTypename() ) {
    setEditable( "vector" );
}

template <>
inline void SingleDataSourceNode<Ra::Core::Vector4>::toJsonInternal( nlohmann::json& data ) const {
    data["vector"] = *getData();
}

template <>
inline bool
SingleDataSourceNode<Ra::Core::Vector4>::fromJsonInternal( const nlohmann::json& data ) {
    if ( data.contains( "vector" ) ) {
        std::array<Scalar, 4> v = data["vector"];
        auto vec                = Ra::Core::Vector4( v[0], v[1], v[2], v[3] );
        setData( vec );
    }
    return true;
}

// Ra::Core::Vector3 specialization
template <>
inline SingleDataSourceNode<Ra::Core::Vector3>::SingleDataSourceNode( const std::string& name ) :
    SingleDataSourceNode( name, SingleDataSourceNode<Ra::Core::Vector3>::getTypename() ) {
    setEditable( "vector" );
}

template <>
inline void SingleDataSourceNode<Ra::Core::Vector3>::toJsonInternal( nlohmann::json& data ) const {
    data["vector"] = *getData();
}

template <>
inline bool
SingleDataSourceNode<Ra::Core::Vector3>::fromJsonInternal( const nlohmann::json& data ) {
    if ( data.contains( "vector" ) ) {
        std::array<Scalar, 3> v = data["vector"];
        auto vec                = Ra::Core::Vector3( v[0], v[1], v[2] );
        setData( vec );
    }
    return true;
}

// Ra::Core::Vector2 specialization
template <>
inline SingleDataSourceNode<Ra::Core::Vector2>::SingleDataSourceNode( const std::string& name ) :
    SingleDataSourceNode( name, SingleDataSourceNode<Ra::Core::Vector2>::getTypename() ) {
    setEditable( "vector" );
}

template <>
inline void SingleDataSourceNode<Ra::Core::Vector2>::toJsonInternal( nlohmann::json& data ) const {
    data["vector"] = *getData();
}

template <>
inline bool
SingleDataSourceNode<Ra::Core::Vector2>::fromJsonInternal( const nlohmann::json& data ) {
    if ( data.contains( "vector" ) ) {
        std::array<Scalar, 2> v = data["vector"];
        auto vec                = Ra::Core::Vector2( v[0], v[1] );
        setData( vec );
    }
    return true;
}

} // namespace Sources
} // namespace Core
} // namespace Dataflow
} // namespace Ra
