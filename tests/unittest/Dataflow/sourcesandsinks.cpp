#include <catch2/catch.hpp>

#include <string>
#include <vector>

#include <iostream>

#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Nodes/Sinks/CoreDataSinks.hpp>
#include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>

using namespace Ra::Dataflow::Core;

//! [Create a source to sink graph for type T]
template <typename T>
void testGraph( const std::string& name, T in, T& out ) {
    auto g      = std::make_unique<DataflowGraph>( name );
    auto source = std::make_shared<Sources::SingleDataSourceNode<T>>( "in" );
    auto sink   = std::make_shared<Sinks::SinkNode<T>>( "out" );
    g->addNode( source );
    g->addNode( sink );
    auto linked = g->addLink( source, "to", sink, "from" );
    if ( !linked ) { std::cerr << "Error linking source and sink nodes.\n"; }
    REQUIRE( linked );

    auto input = g->getDataSetter( "in_to" );
    REQUIRE( input != nullptr );
    auto output = g->getDataGetter( "out_from" );
    REQUIRE( output != nullptr );

    auto compiled = g->compile();
    if ( !compiled ) { std::cerr << "Error compiling graph.\n"; }
    REQUIRE( compiled );

    std::cout << "Setting " << simplifiedDemangledType<T>() << " data on interface port ... ";
    input->setData( &in );

    g->execute();

    T r = output->getData<T>();
    std::cout << "Getting a " << simplifiedDemangledType( r ) << " from interface port ... ";
    out = r;

    g->releaseDataSetter( "in_to" );
    source->setData( in );

    nlohmann::json graphData;
    g->toJson( graphData );
    g->destroy();

    g = std::make_unique<DataflowGraph>( name );
    g->fromJson( graphData );
    auto ok = g->execute();
    REQUIRE( ok );
}
//! [Create a source to sink graph for type T]
TEST_CASE( "Dataflow/Core/Sources and Sinks", "[Dataflow][Core][Sources and Sinks]" ) {
    SECTION( "Operations on base type : Scalar" ) {
        using DataType = Scalar;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";

        DataType x { 3.141592_ra };
        DataType y { 0_ra };
        testGraph<DataType>( "Test on Scalar", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : float" ) {
        using DataType = float;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { 3.141592 };
        DataType y { 0 };
        testGraph<DataType>( "Test on float", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : double" ) {
        using DataType = double;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { 3.141592 };
        DataType y { 0 };
        testGraph<DataType>( "Test on float", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : int" ) {
        using DataType = int;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { -3 };
        DataType y { 0 };
        testGraph<DataType>( "Test on int", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : unsigned int" ) {
        using DataType = unsigned int;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { 3 };
        DataType y { 0 };
        testGraph<DataType>( "Test on unsigned int", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : bool" ) {
        using DataType = bool;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { true };
        DataType y { false };
        testGraph<DataType>( "Test on bool", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : Vector2" ) {
        using DataType = Ra::Core::Vector2;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { 1_ra, 2_ra };
        DataType y;
        testGraph<DataType>( "Test on Vector2", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : Vector3" ) {
        using DataType = Ra::Core::Vector3;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { 1_ra, 2_ra, 3_ra };
        DataType y;
        testGraph<DataType>( "Test on Vector3", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : Vector4" ) {
        using DataType = Ra::Core::Vector4;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x { 1_ra, 2_ra, 3_ra, 4_ra };
        DataType y;
        testGraph<DataType>( "Test on Vector4", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
    SECTION( "Operations on base type : Color" ) {
        using DataType = Ra::Core::Utils::Color;
        std::cout << "Test on " << simplifiedDemangledType<DataType>() << " ... ";
        DataType x = Ra::Core::Utils::Color::Skin();
        DataType y;
        testGraph<DataType>( "Test on Color", x, y );

        REQUIRE( x == y );
        std::cout << " ... DONE!\n";
    }
}
