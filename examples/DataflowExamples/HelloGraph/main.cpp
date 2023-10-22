#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Nodes/Functionals/FilterNode.hpp>
#include <Dataflow/Core/Nodes/Sinks/SinkNode.hpp>
#include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>

using namespace Ra::Dataflow::Core;

/* ----------------------------------------------------------------------------------- */
/**
 * \brief Demonstrate how to use a graph to filter a collection
 */
int main( int argc, char* argv[] ) {
    using RaVector = Ra::Core::VectorArray<Scalar>;

    //! [Creating an empty graph]
    DataflowGraph g { "helloGraph" };
    //! [Creating an empty graph]

    //! [Creating Nodes]
    auto sourceNode    = g.addNode<Sources::SingleDataSourceNode<RaVector>>( "Source" );
    auto predicateNode = g.addNode<Sources::ScalarUnaryPredicateSource>( "Selector" );
    auto filterNode    = g.addNode<Functionals::FilterNode<RaVector>>( "Filter" );
    auto sinkNode      = g.addNode<Sinks::SinkNode<RaVector>>( "Sink" );
    //! [Creating Nodes]

    //! [Creating links between Nodes]
    // link using nodes port, with compile time type check
    // node belongship to the graph is checked at runtime
    if ( !g.addLink( sourceNode->getOuputPort(), filterNode->getInPort() ) ) { std::abort(); }

    // link with port names, all runtime check
    if ( !g.addLink( predicateNode, "f", filterNode, "f" ) ) { std::abort(); }
    // one can also link using node index, it depends on node init, so be sure to have the right
    // index
    // Functional Port "out" as index 0, and Sink "from" is 0 also
    if ( !g.addLink( filterNode, Node::PortIndex { 0 }, sinkNode, Node::PortIndex { 0 } ) ) {
        std::abort();
    }
    //! [Creating links between Nodes]

    //! [Verifying the graph can be compiled]
    if ( !g.compile() ) {
        std::cout << " compilation failed";
        return 1;
    }
    //! [Verifying the graph can be compiled]

    //! [Configure nodes]
    RaVector test( 10 );
    std::mt19937 gen( 0 );
    std::uniform_real_distribution<> dis( 0.0, 1.0 );
    // Fill the vector with random numbers between 0 and 1
    std::generate( test.begin(), test.end(), [&dis, &gen]() { return dis( gen ); } );
    sourceNode->setData( test );

    Sources::ScalarUnaryPredicateSource::function_type pred = []( Scalar x ) { return x < 0.5; };
    predicateNode->setData( &pred );
    //! [Configure nodes]

    std::cout << "Input values : \n\t";
    std::copy( test.begin(), test.end(), std::ostream_iterator<Scalar>( std::cout, " " ) );
    std::cout << '\n';

    //! [Execute the graph]
    g.execute();
    //! [Execute the graph]

    //! [Print the output result]
    auto& result = sinkNode->getDataByRef();
    std::cout << "Output values (reference): " << result.size() << "\n\t";
    std::copy( result.begin(), result.end(), std::ostream_iterator<Scalar>( std::cout, " " ) );
    std::cout << '\n';
    //! [Print the output result]

    //! [Modify input and rerun the graph]
    Sources::ScalarUnaryPredicateSource::function_type predbig = []( Scalar x ) { return x > 0.5; };
    predicateNode->setData( &predbig );

    g.execute();
    // since result is a ref to node's output, no need to get it again
    std::cout << "Output values after second execution: " << result.size() << "\n\t";
    std::copy( result.begin(), result.end(), std::ostream_iterator<Scalar>( std::cout, " " ) );
    std::cout << '\n';
    //! [Modify input and rerun the graph]

    return 0;
}
