#include <catch2/catch.hpp>

#include <memory>
#include <string>
#include <vector>

#include <iostream>

#include <Dataflow/Core/DataflowGraph.hpp>

#include <Dataflow/Core/Nodes/Functionals/CoreDataFunctionals.hpp>
#include <Dataflow/Core/Nodes/Sinks/CoreDataSinks.hpp>
#include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>

//! [Create a source to sink graph for type T]
using namespace Ra::Dataflow::Core;
template <typename DataType_a, typename DataType_b = DataType_a, typename DataType_r = DataType_a>
std::tuple<DataflowGraph*, std::shared_ptr<PortBase>, std::shared_ptr<PortBase>, PortBase*>
createGraph(
    const std::string& name,
    typename Functionals::BinaryOpNode<DataType_a, DataType_b, DataType_r>::BinaryOperator f ) {
    using TestNode = Functionals::BinaryOpNode<DataType_a, DataType_b, DataType_r>;
    auto g         = new DataflowGraph { name };

    auto source_a = std::make_shared<Sources::SingleDataSourceNode<DataType_a>>( "a" );
    g->addNode( source_a );
    auto a = g->getDataSetter( "a_to" );
    REQUIRE( a->getNode() == g );

    auto source_b = std::make_shared<Sources::SingleDataSourceNode<DataType_b>>( "b" );
    g->addNode( source_b );
    auto b = g->getDataSetter( "b_to" );
    REQUIRE( b->getNode() == g );

    auto sink = std::make_shared<Sinks::SinkNode<DataType_r>>( "r" );
    g->addNode( sink );
    auto r = g->getDataGetter( "r_from" );
    REQUIRE( r->getNode() == g );

    auto op = std::make_shared<TestNode>( "operator", f );
    // op->setOperator( f );
    g->addNode( op );

    REQUIRE( g->addLink( source_a, "to", op, "a" ) );
    REQUIRE( g->addLink( op, "r", sink, "from" ) );
    REQUIRE( !g->compile() );
    // this will not execute the graph as it does not compile
    g->execute();
    REQUIRE( !g->isCompiled() );
    // add missing link
    REQUIRE( g->addLink( source_b, "to", op, "b" ) );

    return { g, a, b, r };
}

TEST_CASE( "Dataflow/Core/Nodes", "[Dataflow][Core][Nodes]" ) {
    SECTION( "Operations on Scalar" ) {
        using DataType = Scalar;
        using TestNode = Functionals::BinaryOpNode<DataType, DataType, DataType>;
        typename TestNode::BinaryOperator add = []( typename TestNode::Arg1_type a,
                                                    typename TestNode::Arg2_type b ) ->
            typename TestNode::Res_type { return a + b; };

        auto [g, a, b, r] = createGraph<DataType>( "test scalar binary op", add );

        DataType x { 1_ra };
        a->setData( &x );
        REQUIRE( a->getData<DataType>() == x );

        DataType y { 2_ra };
        b->setData( &y );
        REQUIRE( b->getData<DataType>() == y );

        // As graph was modified since last compilation, this will recompile the graph
        g->execute();

        auto& z = r->getData<DataType>();
        REQUIRE( z == x + y );

        std::cout << x << " + " << y << " == " << z << "\n";

        g->destroy();
        delete g;
    }

    SECTION( "Operations on Vectors" ) {
        using DataType = Ra::Core::Vector3;
        using TestNode = Functionals::BinaryOpNode<DataType, DataType, DataType>;
        typename TestNode::BinaryOperator add = []( typename TestNode::Arg1_type a,
                                                    typename TestNode::Arg2_type b ) ->
            typename TestNode::Res_type { return a + b; };

        auto [g, a, b, r] = createGraph<DataType>( "test Vector3 binary op", add );

        DataType x { 1_ra, 2_ra, 3_ra };
        a->setData( &x );
        REQUIRE( a->getData<DataType>() == x );

        DataType y { 3_ra, 2_ra, 1_ra };
        b->setData( &y );
        REQUIRE( b->getData<DataType>() == y );

        g->execute();

        auto& z = r->getData<DataType>();
        REQUIRE( z == x + y );

        std::cout << "[" << x.transpose() << "] + [" << y.transpose() << "] == [" << z.transpose()
                  << "]\n";

        g->destroy();
        delete g;
    }

    SECTION( "Operations on VectorArrays" ) {
        using DataType = Ra::Core::VectorArray<Ra::Core::Vector2>;
        using TestNode = Functionals::BinaryOpNode<DataType, DataType, DataType>;
        typename TestNode::BinaryOperator add = []( typename TestNode::Arg1_type a,
                                                    typename TestNode::Arg2_type b ) ->
            typename TestNode::Res_type { return a + b; };

        auto [g, a, b, r] = createGraph<DataType>( "test Vector3 binary op", add );

        DataType x { { 1_ra, 2_ra }, { 3_ra, 4_ra } };
        a->setData( &x );
        REQUIRE( a->getData<DataType>() == x );

        DataType y { { 5_ra, 6_ra }, { 7_ra, 8_ra } };
        b->setData( &y );
        REQUIRE( b->getData<DataType>() == y );

        g->execute();

        auto& z = r->getData<DataType>();
        for ( size_t i = 0; i < z.size(); i++ ) {
            REQUIRE( z[i] == x[i] + y[i] );
        }

        std::cout << "{ ";
        for ( const auto& t : x ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "} + { ";
        for ( const auto& t : y ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "} = { ";
        for ( const auto& t : z ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "}\n";

        g->destroy();
        delete g;
    }

    SECTION( "Operations between VectorArray and Scalar" ) {
        using DataType_a = Ra::Core::VectorArray<Ra::Core::Vector2>;
        using DataType_b = Scalar;
        // How to do this ? Eigen generates an error due to align allocation
        // using DataType_r = Ra::Core::VectorArray< decltype(  std::declval<Ra::Core::Vector2>() *
        // std::declval<Scalar>() ) >;
        using DataType_r = Ra::Core::VectorArray<Ra::Core::Vector2>;
        using TestNode   = Functionals::BinaryOpNode<DataType_a, DataType_b, DataType_r>;
        typename TestNode::BinaryOperator op = []( typename TestNode::Arg1_type a,
                                                   typename TestNode::Arg2_type b ) ->
            typename TestNode::Res_type { return a * b; };
        auto [g, a, b, r] = createGraph<DataType_a, DataType_b, DataType_r>(
            "test Vector2 x Scalar binary op", op );

        DataType_a x { { 1_ra, 2_ra }, { 3_ra, 4_ra } };
        a->setData( &x );
        REQUIRE( a->getData<DataType_a>() == x );

        DataType_b y { 5_ra };
        b->setData( &y );
        REQUIRE( b->getData<DataType_b>() == y );

        g->execute();

        auto& z = r->getData<DataType_r>();
        for ( size_t i = 0; i < z.size(); i++ ) {
            REQUIRE( z[i] == x[i] * y );
        }

        std::cout << "{ ";
        for ( const auto& t : x ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "} * " << y << " = { ";
        for ( const auto& t : z ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "}\n";

        // change operator
        auto opNode = std::dynamic_pointer_cast<TestNode>( g->getNode( "operator" ) );
        REQUIRE( opNode != nullptr );
        if ( opNode ) {
            typename TestNode::BinaryOperator f = []( typename TestNode::Arg1_type arg1,
                                                      typename TestNode::Arg2_type arg2 ) ->
                typename TestNode::Res_type { return arg1 / arg2; };
            opNode->setOperator( f );
        }
        g->execute();

        for ( size_t i = 0; i < z.size(); i++ ) {
            REQUIRE( z[i] == x[i] / y );
        }

        std::cout << "{ ";
        for ( const auto& t : x ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "} / " << y << " = { ";
        for ( const auto& t : z ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "}\n";
        g->destroy();
        delete g;
    }

    SECTION( "Operations between Scalar and VectorArray" ) {
        using namespace Ra::Dataflow::Core;
        using DataType_a = Scalar;
        using DataType_b = Ra::Core::VectorArray<Ra::Core::Vector2>;
        using DataType_r = Ra::Core::VectorArray<Ra::Core::Vector2>;
        using TestNode   = Functionals::BinaryOpNode<DataType_a, DataType_b, DataType_r>;
        typename TestNode::BinaryOperator op = []( typename TestNode::Arg1_type a,
                                                   typename TestNode::Arg2_type b ) ->
            typename TestNode::Res_type { return a * b; };
        auto [g, a, b, r] = createGraph<DataType_a, DataType_b, DataType_r>(
            "test Vector2 x Scalar binary op", op );

        DataType_a x { 4_ra };
        a->setData( &x );
        REQUIRE( a->getData<DataType_a>() == x );

        DataType_b y { { 1_ra, 2_ra }, { 3_ra, 4_ra } };
        b->setData( &y );
        REQUIRE( b->getData<DataType_b>() == y );

        g->execute();

        auto& z = r->getData<DataType_r>();
        for ( size_t i = 0; i < z.size(); i++ ) {
            REQUIRE( z[i] == x * y[i] );
        }

        std::cout << x << " * { ";
        for ( const auto& t : y ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "} = { ";
        for ( const auto& t : z ) {
            std::cout << "[" << t.transpose() << "] ";
        }
        std::cout << "}\n";
        g->destroy();
        delete g;
    }

    SECTION( "Transform/reduce/filter/test" ) {
        //! [Create a complex transform/reduce graph]
        auto g           = new DataflowGraph( "Complex graph" );
        using VectorType = Ra::Core::VectorArray<Scalar>;

        // Source of a vector of Scalar : random vector
        auto nodeS = std::make_shared<Sources::ScalarArraySource>( "s" );

        // Source of an operator on scalars : f(x) = 2*x
        using DoubleFunction    = Sources::FunctionSourceNode<Scalar, const Scalar&>::function_type;
        DoubleFunction doubleMe = []( const Scalar& x ) -> Scalar { return 2_ra * x; };
        auto nodeD = std::make_shared<Sources::FunctionSourceNode<Scalar, const Scalar&>>( "d" );
        nodeD->setData( &doubleMe );

        // Source of a Scalar : mean neutral element 0_ra
        auto nodeN = std::make_shared<Sources::ScalarSource>( "n" );
        nodeN->setData( 0_ra );

        // Source of a reduction operator : compute the mean using Welford online algo
        using ReduceOperator = Sources::FunctionSourceNode<Scalar, const Scalar&, const Scalar&>;
        struct MeanOperator {
            size_t n { 0 };
            Scalar operator()( const Scalar& m, const Scalar& x ) {
                return m + ( ( x - m ) / ( ++n ) );
            }
        };
        auto nodeM                      = std::make_shared<ReduceOperator>( "m" );
        ReduceOperator::function_type m = MeanOperator();

        // Reduce node : will compute the mean
        using MeanCalculator = Functionals::ReduceNode<VectorType>;
        auto meanCalculator  = std::make_shared<MeanCalculator>( "mean" );

        // Sink for the mean
        auto nodeR = std::make_shared<Sinks::ScalarSink>( "r" );

        // Transform operator, will double the vectors' values
        auto nodeT = std::make_shared<Functionals::ArrayTransformerScalar>( "twice" );

        // Will compute the mean on the doubled vector
        auto doubleMeanCalculator = std::make_shared<MeanCalculator>( "double mean" );

        // Sink for the double mean
        auto nodeRD = std::make_shared<Sinks::ScalarSink>( "rd" );

        // Source for a comparison functor , eg f(x, y) -> 2*x == y
        auto nodePred = std::make_shared<Sources::ScalarBinaryPredicateSource>( "predicate" );
        Sources::ScalarBinaryPredicateSource::function_type predicate =
            []( const Scalar& a, const Scalar& b ) -> bool { return 2_ra * a == b; };
        nodePred->setData( &predicate );

        // Boolean sink for the validation result
        auto sinkB = std::make_shared<Sinks::BooleanSink>( "test" );

        // Node for coparing the results of the computation graph
        auto validator =
            std::make_shared<Functionals::BinaryOpNode<Scalar, Scalar, bool>>( "validator" );

        g->addNode( nodeS );
        g->addNode( nodeD );
        g->addNode( nodeN );
        g->addNode( nodeM );
        g->addNode( nodeR );
        g->addNode( meanCalculator );
        g->addNode( doubleMeanCalculator );
        g->addNode( nodeT );
        g->addNode( nodeRD );

        bool linkAdded;
        linkAdded = g->addLink( nodeS, "to", meanCalculator, "in" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodeM, "f", meanCalculator, "f" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodeN, "to", meanCalculator, "init" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( meanCalculator, "out", nodeR, "from" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodeS, "to", nodeT, "in" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodeD, "f", nodeT, "f" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodeT, "out", doubleMeanCalculator, "in" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( doubleMeanCalculator, "out", nodeRD, "from" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodeM, "f", doubleMeanCalculator, "f" );
        REQUIRE( linkAdded == true );

        g->addNode( nodePred );
        g->addNode( sinkB );
        g->addNode( validator );
        linkAdded = g->addLink( meanCalculator, "out", validator, "a" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( doubleMeanCalculator, "out", validator, "b" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( nodePred, "f", validator, "f" );
        REQUIRE( linkAdded == true );
        linkAdded = g->addLink( validator, "r", sinkB, "from" );
        REQUIRE( linkAdded == true );

        auto input   = g->getDataSetter( "s_to" );
        auto output  = g->getDataGetter( "r_from" );
        auto outputD = g->getDataGetter( "rd_from" );
        auto outputB = g->getDataGetter( "test_from" );
        auto inputR  = g->getDataSetter( "m_f" );
        if ( inputR == nullptr ) { std::cout << "Failed to get the graph function input !!\n"; }

        // Inspect the graph interface : inputs and outputs port
        auto inputs = g->getAllDataSetters();
        std::cout << "Input ports (" << inputs.size() << ") are :\n";
        for ( auto& [ptrPort, portName, portType] : inputs ) {
            std::cout << "\t\"" << portName << "\" accepting type " << portType << "\n";
        }
        auto outputs = g->getAllDataGetters();
        std::cout << "Output ports (" << outputs.size() << ") are :\n";
        for ( auto& [ptrPort, portName, portType] : outputs ) {
            std::cout << "\t\"" << portName << "\" generating type " << portType << "\n";
        }

        if ( !g->compile() ) { std::cout << "Compilation error !!"; }

        // Set input/ouput data
        VectorType test;
        input->setData( &test );

        test.reserve( 10 );
        std::mt19937 gen( 0 );
        std::uniform_real_distribution<> dis( 0.0, 1.0 );
        // Fill the vector with random numbers between 0 and 1
        for ( size_t n = 0; n < test.capacity(); ++n ) {
            test.push_back( dis( gen ) );
        }

        // No need to do this as mean operator source has a copy of a functor
        ReduceOperator::function_type m1 = MeanOperator();
        inputR->setData( &m1 );

        g->execute();

        auto& result  = output->getData<Scalar>();
        auto& resultD = outputD->getData<Scalar>();
        auto& resultB = outputB->getData<bool>();

        std::cout << "Computed mean ( ref ): " << result << "\n";
        std::cout << "Computed mean ( tra ): " << resultD << "\n";
        std::cout << std::boolalpha;
        std::cout << "Ratio  ( expected 2 ): " << resultD / result << " -- validator --> "
                  << resultB << "\n";

        std::cout << '\n';

        REQUIRE( resultD / result == 2_ra );
        REQUIRE( resultB );
        // uncomment this if you want to edit the generated graph with GraphEditor
        //        g->saveToJson( "Transform-reduce.json" );
        g->destroy();
        delete g;
        //! [Create a complex transform/reduce graph]
    }
}
