#include "Core/Containers/MakeShared.hpp"
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

    auto gAsNode = Ra::Core::make_shared<DataflowGraph>( "graphAsNode" );

    auto predicateNode = gAsNode->addNode<Sources::ScalarUnaryPredicateSource>( "Selector" );
    auto filterNode    = gAsNode->addNode<Functionals::FilterNode<RaVector>>( "Filter" );

    Sources::ScalarUnaryPredicateSource::function_type pred = []( Scalar x ) { return x < 0.5; };
    predicateNode->setData( &pred );

    gAsNode->addLink( predicateNode->getFunctionPort(), filterNode->getPredicatePort() );

    auto inputIdx  = gAsNode->addInput( filterNode->getInPort() );
    auto outputIdx = gAsNode->addOutput( filterNode->getOutPort() );

    DataflowGraph g { "mainGraph" };
    auto sourceNode = g.addNode<Sources::SingleDataSourceNode<RaVector>>( "Source" );
    auto sinkNode   = g.addNode<Sinks::SinkNode<RaVector>>( "Sink" );
    auto graphNode  = g.addNode( gAsNode );
    g.addLink( sourceNode->getOuputPort().get(), gAsNode->getInputByIndex<RaVector>( inputIdx ) );
    g.addLink( gAsNode->getOutputByIndex<RaVector>( outputIdx ), sinkNode->getInPort().get() );

    if ( !g.compile() ) {
        std::cout << " compilation failed";
        return 1;
    }

    //! [Verifying the graph can be compiled]
    g.saveToJson( "illustrateDoc.json" );

    ///! [Configure the interface ports (input and output of the graph)]
    RaVector test( 10 );
    std::mt19937 gen( 0 );
    std::uniform_real_distribution<> dis( 0.0, 1.0 );
    // Fill the vector with random numbers between 0 and 1
    std::generate( test.begin(), test.end(), [&dis, &gen]() { return dis( gen ); } );

    sourceNode->setData( test );

    //! [Execute the graph]
    g.execute();
    auto& result = sinkNode->getDataByRef();
    std::cout << "Output values (reference): " << result.size() << "\n\t";
    std::copy( result.begin(), result.end(), std::ostream_iterator<Scalar>( std::cout, " " ) );
    std::cout << '\n';

    return 0;
}
