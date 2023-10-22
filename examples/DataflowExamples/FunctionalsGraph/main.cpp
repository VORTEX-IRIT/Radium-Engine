#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Nodes/Functionals/ReduceNode.hpp>
#include <Dataflow/Core/Nodes/Functionals/TransformNode.hpp>
#include <Dataflow/Core/Nodes/Sinks/SinkNode.hpp>
#include <Dataflow/Core/Nodes/Sources/FunctionSource.hpp>
#include <Dataflow/Core/Nodes/Sources/SingleDataSourceNode.hpp>

#include <random>

using namespace Ra::Dataflow::Core;

/* ----------------------------------------------------------------------------------- */
/**
 * \brief Demonstrate how to use a graph to filter a collection
 */
int main( int argc, char* argv[] ) {
    //! [Creating an empty graph]
    DataflowGraph g { "helloGraph" };
    //! [Creating an empty graph]

    //! [Creating Nodes]
    using TransformOperatorSource = Sources::FunctionSourceNode<Scalar, const Scalar&>;
    auto sourceNode =
        std::make_shared<Sources::SingleDataSourceNode<std::vector<Scalar>>>( "Source" );
    auto mapSource = std::make_shared<TransformOperatorSource>( "MapOperator" );
    auto transformNode =
        std::make_shared<Functionals::TransformNode<std::vector<Scalar>>>( "Transformer" );
    // can't use auto here, must explicitely define the type
    Functionals::TransformNode<std::vector<Scalar>>::TransformOperator oneMinusMe =
        []( const Scalar& i ) -> Scalar { return 1_ra - i; };
    transformNode->setOperator( oneMinusMe );

    auto reduceNode = std::make_shared<Functionals::ReduceNode<std::vector<Scalar>>>( "Minimum" );
    Functionals::ReduceNode<std::vector<Scalar>>::ReduceOperator getMin =
        []( const Scalar& a, const Scalar& b ) -> Scalar { return std::min( a, b ); };
    reduceNode->setOperator( getMin, std::numeric_limits<Scalar>::max() );

    auto scalarSinkNode = std::make_shared<Sinks::SinkNode<Scalar>>( "ScalarSink" );

    auto sinkNode = std::make_shared<Sinks::SinkNode<std::vector<Scalar>>>( "Sink" );
    //! [Creating Nodes]

    //! [Adding Nodes to the graph]
    g.addNode( sourceNode );
    g.addNode( mapSource );
    g.addNode( transformNode );
    g.addNode( reduceNode );
    g.addNode( sinkNode );
    g.addNode( scalarSinkNode );
    //! [Adding Nodes to the graph]

    //! [Creating links between Nodes]
    g.addLink( sourceNode, "to", transformNode, "in" );
    g.addLink( transformNode, "out", sinkNode, "from" );
    g.addLink( transformNode, "out", reduceNode, "in" );
    g.addLink( reduceNode, "out", scalarSinkNode, "from" );
    //! [Creating links between Nodes]

    //! [Verifing the graph can be compiled]
    if ( !g.compile() ) {
        std::cout << " compilation failed";
        return 1;
    }
    //! [Verifing the graph can be compiled]

    ///! [Configure the interface ports (input and output of the graph)
    auto input = g.getDataSetter( "Source_to" );
    std::vector<Scalar> test;
    input->setData( &test );

    auto opSetter = g.getDataSetter( "MapOperator_f" );
    // can't use auto here, must explicitely define the type
    TransformOperatorSource::function_type doubleMe = []( const Scalar& i ) -> Scalar {
        return 2_ra * i;
    };
    opSetter->setData( &doubleMe );

    auto output  = g.getDataGetter( "Sink_from" );
    auto minimum = g.getDataGetter( "ScalarSink_from" );
    // The reference to the result will not be available before the first run
    // auto& result = output->getData<std::vector<Scalar>>();
    ///! [Configure the interface ports (input and output of the graph)

    //! [Initializing input variable to test the graph]
    test.reserve( 10 );
    std::mt19937 gen( 0 );
    std::uniform_real_distribution<> dis( 0.0, 1.0 );
    // Fill the vector with random numbers between 0 and 1
    for ( int n = 0; n < test.capacity(); ++n ) {
        test.push_back( dis( gen ) );
    }

    std::cout << "Input values : \n\t";
    for ( auto ord : test ) {
        std::cout << ord << ' ';
    }
    std::cout << '\n';
    //! [Initializing input variable to test the graph]

    //! [Execute the graph]
    g.execute();
    // The reference to the result is now available
    auto& result   = output->getData<std::vector<Scalar>>();
    auto& minValue = minimum->getData<Scalar>();
    //! [Execute the graph]

    //! [Print the output result]
    std::cout << "Output values (oneMinusMe): " << result.size() << "\n\t";
    for ( auto ord : result ) {
        std::cout << ord << ' ';
    }
    std::cout << '\n';
    std::cout << "Min value is : " << minValue << "\n";
    //! [Print the output result]

    //! [Verify the result]
    std::cout << "Verifying the result : \n\t";

    bool execOK { true };
    for ( int i = 0; i < 10; ++i ) {
        if ( result[i] != oneMinusMe( test[i] ) ) {
            std::cout << "Error at index " << i << " : " << result[i] << " should be "
                      << oneMinusMe( test[i] ) << "\n\t";
            execOK = false;
        }
    }
    if ( execOK ) { std::cout << "OK\n"; }
    else { std::cout << "NOK\n"; }
    //! [Verify the result]

    //! [Modifying the graph to add link to map operator]
    if ( !g.addLink( mapSource, "f", transformNode, "f" ) ) {
        std::cout << "Unable to link port f ("
                  << ") from " << mapSource->getInstanceName() << " to port f("
                  << ") of " << transformNode->getInstanceName() << "\n";
        return 1;
    }
    //! [Modifying the graph to add link to map operator]

    //! [Execute and test the result]
    std::cout << "Rerun the graph with different operator : \n";
    g.execute();
    std::cout << "Output values (doubleMe): " << result.size() << "\n\t";
    for ( auto ord : result ) {
        std::cout << ord << ' ';
    }
    std::cout << '\n';
    std::cout << "Min value is : " << minValue << "\n";
    execOK = true;
    for ( int i = 0; i < 10; ++i ) {
        if ( result[i] != doubleMe( test[i] ) ) {
            std::cout << "Error at index " << i << " : " << result[i] << " should be "
                      << doubleMe( test[i] ) << "\n\t";
            execOK = false;
        }
    }
    if ( execOK ) { std::cout << "OK\n"; }
    else { std::cout << "NOK\n"; }
    //! [Execute and test the result]

    return 0;
}
