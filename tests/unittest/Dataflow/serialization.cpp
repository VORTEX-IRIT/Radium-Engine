#include <catch2/catch.hpp>

#include <string>
#include <vector>

#include <iostream>

#include <Core/Utils/StdFilesystem.hpp>

#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Nodes/Functionals/CoreDataFunctionals.hpp>
#include <Dataflow/Core/Nodes/Sinks/CoreDataSinks.hpp>
#include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>

TEST_CASE( "Dataflow/Core/DataflowGraph", "[Dataflow][Core][DataflowGraph]" ) {
    SECTION( "Execution and modification of a graph" ) {
        using namespace Ra::Dataflow::Core;
        using DataType = Scalar;
        DataflowGraph g { "original graph" };
        g.addJsonMetaData(
            { { "extra", { { "info", "missing operators on functional node" } } } } );
        auto source_a = std::make_shared<Sources::SingleDataSourceNode<DataType>>( "a" );
        g.addNode( source_a );
        auto a        = g.getDataSetter( "a_to" );
        auto source_b = std::make_shared<Sources::SingleDataSourceNode<DataType>>( "b" );
        g.addNode( source_b );
        auto b    = g.getDataSetter( "b_to" );
        auto sink = std::make_shared<Sinks::SinkNode<DataType>>( "r" );
        g.addNode( sink );
        auto r                       = g.getDataGetter( "r_from" );
        using TestNode               = Functionals::BinaryOpNode<DataType, DataType, DataType>;
        TestNode::BinaryOperator add = []( TestNode::Arg1_type a,
                                           TestNode::Arg2_type b ) -> TestNode::Res_type {
            return a + b;
        };
        auto op_unique = std::make_shared<TestNode>( "addition" );
        op_unique->setOperator( add );
        auto added = g.addNode( op_unique );
        REQUIRE( added );
        g.addLink( source_a, "to", op_unique, "a" );
        g.addLink( op_unique, "r", sink, "from" );
        g.addLink( source_b, "to", op_unique, "b" );

        // execution of the original graph
        DataType x { 1_ra };
        a->setData( &x );
        DataType y { 2_ra };
        b->setData( &y );
        // Execute initial graph";
        g.execute();
        auto z = r->getData<DataType>();
        REQUIRE( z == x + y );

        // Save the graph
        std::string tmpdir { "tmpDir4Tests" };
        std::filesystem::create_directories( tmpdir );
        g.saveToJson( tmpdir + "/GraphSerializationTest.json" );
        g.destroy();
        // this does nothing as g was destroyed
        g.execute();

        // Create a new graph and load from the saved graph
        DataflowGraph g1 { "loaded graph" };
        g1.loadFromJson( tmpdir + "/GraphSerializationTest.json" );

        // Setting the unserializable data on nodes (functions)
        auto addition = g1.getNode( "addition" );
        REQUIRE( addition != nullptr );
        REQUIRE( addition->getTypeName() == Functionals::BinaryOpScalar::getTypename() );
        auto typedAddition = std::dynamic_pointer_cast<Functionals::BinaryOpScalar>( addition );
        REQUIRE( typedAddition != nullptr );
        if ( typedAddition != nullptr ) { typedAddition->setOperator( add ); }

        // Execute loaded graph
        // Data delivered by the source nodes are the one saved by the original graph
        g1.execute();
        auto r_loaded  = g1.getDataGetter( "r_from" );
        auto& z_loaded = r_loaded->getData<DataType>();
        REQUIRE( z_loaded == z );

        auto a_loaded = g1.getDataSetter( "a_to" );
        auto b_loaded = g1.getDataSetter( "b_to" );
        DataType xp { 2_ra };
        a_loaded->setData( &xp );
        DataType yp { 3_ra };
        b_loaded->setData( &yp );
        g1.execute();
        REQUIRE( z_loaded == 5 );

        // Reset sources to use the loaded data loaded
        g1.releaseDataSetter( "a_to" );
        g1.releaseDataSetter( "b_to" );
        g1.execute();
        REQUIRE( z_loaded == z );

        // reactivate the dataSetter and change the data delivered by a (copy data into a)
        g1.activateDataSetter( "b_to" );
        auto loadedSource_a =
            std::dynamic_pointer_cast<Sources::SingleDataSourceNode<DataType>>( g1.getNode( "a" ) );
        Scalar newX = 3_ra;
        loadedSource_a->setData( newX );
        g1.execute();
        REQUIRE( z_loaded == 6 );
        std::filesystem::remove_all( tmpdir );
    }
}
