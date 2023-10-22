#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Nodes/Functionals/FilterNode.hpp>
#include <Dataflow/Core/Nodes/Sinks/SinkNode.hpp>
#include <Dataflow/Core/Nodes/Sources/SingleDataSourceNode.hpp>

#include <Core/Containers/VectorArray.hpp>

#include <random>

using namespace Ra::Dataflow::Core;

/* ----------------------------------------------------------------------------------- */
/**
 * \brief Demonstrate how serialize a graph with custom nodes.
 */
int main( int argc, char* argv[] ) {

    //! [Creating the factory for the custom nodes and add it to the nodes system]
    using VectorType = std::vector<Scalar>;
    // using VectorType = Ra::Core::VectorArray<Scalar>;
    // custom node type are either specialization of templated nodes or user-define nodes class

    // create the custom node factory
    auto customFactory = NodeFactoriesManager::createFactory( "ExampleCustomFactory" );
    // add node creators to the factory
    customFactory->registerNodeCreator<Sources::SingleDataSourceNode<VectorType>>(
        Sources::SingleDataSourceNode<VectorType>::getTypename() + "_", "Custom" );
    customFactory->registerNodeCreator<Functionals::FilterNode<VectorType>>(
        Functionals::FilterNode<VectorType>::getTypename() + "_", "Custom" );
    customFactory->registerNodeCreator<Sinks::SinkNode<VectorType>>(
        Sinks::SinkNode<VectorType>::getTypename() + "_", "Custom" );

    //! [Creating the factory for the custom node types and add it to the node system]

    {
        //! [Creating an empty graph using the custom nodes factory]
        DataflowGraph g { "Serialization example" };
        // Add to the graph the custom factory (built-in nodes are automatically managed)
        g.addFactory( NodeFactoriesManager::getFactory( "ExampleCustomFactory" ) );
        //! [Creating an empty graph using the custom nodes factory]

        //! [Creating Nodes]
        auto sourceNode = std::make_shared<Sources::SingleDataSourceNode<VectorType>>( "Source" );
        // non serializable node using a custom filter
        auto filterNode = std::make_shared<Functionals::FilterNode<VectorType>>(
            "Filter", []( const Scalar& x ) { return x > 0.5_ra; } );
        auto sinkNode = std::make_shared<Sinks::SinkNode<VectorType>>( "Sink" );
        //! [Creating Nodes]

        //! [Adding Nodes to the graph]
        g.addNode( sourceNode );
        g.addNode( filterNode );
        g.addNode( sinkNode );
        //! [Adding Nodes to the graph]

        //! [Creating links between Nodes]
        g.addLink( sourceNode, "to", filterNode, "in" );
        g.addLink( filterNode, "out", sinkNode, "from" );
        //! [Creating links between Nodes]

        //! [Inspect the graph interface : inputs and outputs port]
        auto inputs = g.getAllDataSetters();
        std::cout << "Input ports (" << inputs.size() << ") are :\n";
        for ( auto& [ptrPort, portName, portType] : inputs ) {
            std::cout << "\t\"" << portName << "\" accepting type " << portType << "\n";
        }
        auto outputs = g.getAllDataGetters();
        std::cout << "Output ports (" << outputs.size() << ") are :\n";
        for ( auto& [ptrPort, portName, portType] : outputs ) {
            std::cout << "\t\"" << portName << "\" generating type " << portType << "\n";
        }
        //! [Inspect the graph interface : inputs and outputs port]

        //! [Verifing the graph can be compiled]
        if ( !g.compile() ) {
            std::cout << " compilation failed";
            return 1;
        }
        //! [Verifing the graph can be compiled]

        //! [Serializing the graph]
        g.saveToJson( "GraphSerializeExample.json" );
        //! [Serializing the graph]
    }

    std::cout << "\t==**==**==*==**==*==**==*==**==*==**==*==**==*==**==\n";
    std::cout << "\t\tLoading and using the graph ...\n";
    std::cout << "\t==**==**==*==**==*==**==*==**==*==**==*==**==*==**==\n";

    //! [Creating an empty graph and load it from a file]
    DataflowGraph g1 { "" };
    g1.loadFromJson( "GraphSerializeExample.json" );
    //! [Creating an empty graph and load it from a file]

    //! [Inspect the graph interface : inputs and outputs port]
    auto inputs_g1 = g1.getAllDataSetters();
    std::cout << "Input ports (" << inputs_g1.size() << ") are :\n";
    for ( auto& [ptrPort, portName, portType] : inputs_g1 ) {
        std::cout << "\t\"" << portName << "\" accepting type " << portType << "\n";
    }
    auto outputs_g1 = g1.getAllDataGetters();
    std::cout << "Output ports (" << outputs_g1.size() << ") are :\n";
    for ( auto& [ptrPort, portName, portType] : outputs_g1 ) {
        std::cout << "\t\"" << portName << "\" generating type " << portType << "\n";
    }
    //! [Inspect the graph interface : inputs and outputs port]

    //! [Verifing the graph can be compiled]
    if ( !g1.compile() ) {
        std::cout << "Compilation failed for the loaded graph";
        return 2;
    }
    //! [Verifing the graph can be compiled]

    //! [Creating input variable to test the graph]
    VectorType test;
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
    //! [Creating input variable to test the graph]

    //! [setting the values processed by the graph]
    auto input = g1.getDataSetter( "Source_to" );
    input->setData( &test );
    //! [setting the values processed by the graph]

    //! [Execute the graph]
    std::cout << "Executing the loaded graph ...\n";
    g1.execute();
    //! [Execute the graph]

    //! [Print the output result]
    auto output = g1.getDataGetter( "Sink_from" );
    VectorType result;
    output->getData( result );
    std::cout << "Output values : \n\t";
    for ( auto ord : result ) {
        std::cout << ord << ' ';
    }
    std::cout << '\n';
    //! [Print the output result]

    //! [Set the correct filter on the filter node]
    auto filter =
        std::dynamic_pointer_cast<Functionals::FilterNode<VectorType>>( g1.getNode( "Filter" ) );
    if ( !filter ) {
        std::cerr << "Unable to cast the filter to the right type\n";
        return 3;
    }
    filter->setFilterFunction( []( const Scalar& f ) { return f > 0.5_ra; } );
    //! [Set the correct filter on the filter node]

    //! [Execute the graph]
    std::cout << "Executing the re-parameterized graph ...\n";
    g1.execute();
    output->getData( result );
    std::cout << "Output values : \n\t";
    for ( auto ord : result ) {
        std::cout << ord << ' ';
    }
    std::cout << '\n';
    //! [Execute the graph]

    return 0;
}
