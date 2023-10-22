#include <catch2/catch.hpp>

#include <iostream>

#include <Dataflow/Core/DataflowGraph.hpp>
#include <Dataflow/Core/Nodes/Sinks/CoreDataSinks.hpp>
#include <Dataflow/Core/Nodes/Sources/CoreDataSources.hpp>

using namespace Ra::Dataflow::Core;

void inspectGraph( const DataflowGraph& g ) {
    // Factories used by the graph
    auto factories = g.getNodeFactories();
    std::cout << "Used factories by the graph \"" << g.getInstanceName() << "\" with type \""
              << g.getTypeName() << "\" :\n";
    for ( const auto& f : *( factories.get() ) ) {
        std::cout << "\t" << f.first << "\n";
    }

    // Nodes of the graph
    const auto& nodes = g.getNodes();
    std::cout << "Nodes of the graph " << g.getInstanceName() << " (" << nodes.size() << ") :\n";
    for ( const auto& n : nodes ) {
        std::cout << "\t\"" << n->getInstanceName() << "\" of type \"" << n->getTypeName()
                  << "\"\n";
        // Inspect input, output and interfaces of the node
        std::cout << "\t\tInput ports :\n";
        for ( const auto& p : n->getInputs() ) {
            std::cout << "\t\t\t\"" << p->getName() << "\" with type " << p->getTypeName() << "\n";
        }
        std::cout << "\t\tOutput ports :\n";
        for ( const auto& p : n->getOutputs() ) {
            std::cout << "\t\t\t\"" << p->getName() << "\" with type " << p->getTypeName() << "\n";
        }
        std::cout << "\t\tInterface ports :\n";
        for ( const auto& p : n->getInterfaces() ) {
            std::cout << "\t\t\t\"" << p->getName() << "\" with type " << p->getTypeName() << "\n";
        }
    }

    // Nodes by level after the compilation
    if ( g.isCompiled() ) {
        auto& cn = g.getNodesByLevel();
        std::cout << "Nodes of the graph, sorted by level after compiling the graph :\n";
        for ( size_t i = 0; i < cn.size(); ++i ) {
            std::cout << "\tLevel " << i << " :\n";
            for ( const auto n : cn[i] ) {
                std::cout << "\t\t\"" << n->getInstanceName() << "\"\n";
            }
        }
    }

    // describe the graph interface : inputs and outputs port of the whole graph (not of the
    // nodes)
    std::cout << "Inputs and output ports of the graph " << g.getInstanceName() << " :\n";
    const auto& inputs = g.getInputs();
    std::cout << "\tInput ports (" << inputs.size() << ") are :\n";
    for ( const auto& inp : inputs ) {
        std::cout << "\t\t\"" << inp->getName() << "\" accepting type \"" << inp->getTypeName()
                  << "\"\n";
    }
    const auto& outputs = g.getOutputs();
    std::cout << "\tOutput ports (" << outputs.size() << ") are :\n";
    for ( const auto& outp : outputs ) {
        std::cout << "\t\t\"" << outp->getName() << "\" accepting type \"" << outp->getTypeName()
                  << "\"\n";
    }

    std::cout << "DataSetters and DataGetters port of the graph " << g.getInstanceName() << " :\n";
    auto setters = g.getAllDataSetters();
    std::cout << "\tSetters ports (" << setters.size() << ") are :\n";
    for ( auto& [ptrPort, portName, portType] : setters ) {
        std::cout << "\t\t\"" << portName << "\" accepting type \"" << portType << "\"\n";
    }
    auto getters = g.getAllDataGetters();
    std::cout << "\tGetters ports (" << getters.size() << ") are :\n";
    for ( auto& [ptrPort, portName, portType] : getters ) {
        std::cout << "\t\t\"" << portName << "\" generating type \"" << portType << "\"\n";
    }
}

TEST_CASE( "Dataflow/Core/Graph", "[Dataflow][Core][Graph]" ) {
    SECTION( "Creation of a graph" ) {
        DataflowGraph g( "Test Graph" );
        // Test not a json error detection
        auto result = g.loadFromJson( "data/Dataflow/NotAJsonFile.json" );
        REQUIRE( !result );
        // Test loading empty graph
        nlohmann::json emptyJson = {};
        result                   = g.fromJson( emptyJson );
        REQUIRE( result );

        // missing identification of the graph (either id or instance)
        nlohmann::json noId = { { "model", { "name", "Core DataflowGraph" } } };
        result              = g.fromJson( noId );
        REQUIRE( !result );
        g.destroy();

        // missing model of the graph
        nlohmann::json noModel = { { "instance", "No model in this node" } };
        result                 = g.fromJson( noModel );
        REQUIRE( !result );
        g.destroy();

        // missing instance data  --> loads an empty graph
        nlohmann::json noGraph = { { "instance", "Missing instance data for model" },
                                   { "model", { "name", "Core DataflowGraph" } } };
        result                 = g.fromJson( noGraph );
        REQUIRE( result );
        g.destroy();

        // Requesting unknown factory
        nlohmann::json wrongFactory = {
            { "instance", "unknown factory" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph", { { "factories", { "NotAFactory" } } } } } } };
        result = g.fromJson( wrongFactory );
        REQUIRE( !result );
        g.destroy();

        // trying to instance an unknown node type
        nlohmann::json NotANode = {
            { "instance", "graph with unknown node" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "NotANode" },
                          { "model", { { "name", "NotANode" } } } } } } } } } } };
        result = g.fromJson( NotANode );
        REQUIRE( !result );
        g.destroy();

        // trying to instance an unknown node type
        nlohmann::json NoModelName = {
            { "instance", "graph with missing node model information" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "Unknown model" },
                          { "model", { { "extra", "NotaTypeName" } } } } } } } } } } };
        result = g.fromJson( NoModelName );
        REQUIRE( !result );
        g.destroy();

        // trying to instance an unknown node type
        nlohmann::json noInstanceIdentification = {
            { "instance", "graph with missing node model information" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes", { { { "model", { { "name", "Source<float>" } } } } } } } } } } };
        result = g.fromJson( noInstanceIdentification );
        REQUIRE( !result );
        g.destroy();

        // errors in the connection description
        nlohmann::json reusingNodeIdentification = {
            { "instance", "graph with wrong connection" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "Source" }, { "model", { { "name", "Source<float>" } } } },
                        { { "instance", "Source" }, { "model", { { "name", "Sink<int>" } } } } } },
                    { "connections", { { { "out_node", "wrongId" } } } } } } } } };
        result = g.fromJson( reusingNodeIdentification );
        REQUIRE( !result );
        g.destroy();
        reusingNodeIdentification = {
            { "instance", "graph with wrong connection" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "model", { { "name", "Source<float>" } } } },
                        { { "model", { { "name", "Sink<int>" } } } } } },
                    { "connections", { { { "out_node", "wrongId" } } } } } } } } };
        result = g.fromJson( reusingNodeIdentification );
        REQUIRE( !result );
        g.destroy();

        // errors in the connection description
        nlohmann::json wrongConnection = {
            { "instance", "graph with wrong connection" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "SourceFloat" },
                          { "model", { { "name", "Source<float>" } } } },
                        { { "instance", "SinkInt" }, { "model", { { "name", "Sink<int>" } } } } } },
                    { "connections", { { { "out_node", "wrongId" } } } } } } } } };
        result = g.fromJson( wrongConnection );
        REQUIRE( !result );
        g.destroy();

        wrongConnection = {
            { "instance", "Test Graph Inline" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "SourceFloat" },
                          { "model", { { "name", "Source<float>" } } } },
                        { { "instance", "SinkInt" }, { "model", { { "name", "Sink<int>" } } } } } },
                    { "connections",
                      { { { "out_node", "SourceFloat" }, { "out_index", 2 } } } } } } } } };
        result = g.fromJson( wrongConnection );
        REQUIRE( !result );
        g.destroy();

        wrongConnection = {
            { "instance", "Test Graph Inline" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "SourceFloat" },
                          { "model", { { "name", "Source<float>" } } } },
                        { { "instance", "SinkInt" }, { "model", { { "name", "Sink<int>" } } } } } },
                    { "connections",
                      { { { "out_node", "SourceFloat" },
                          { "out_index", 0 },
                          { "in_node", "Sink" },
                          { "in_port", "from" } } } } } } } } };
        result = g.fromJson( wrongConnection );
        REQUIRE( !result );
        g.destroy();

        wrongConnection = {
            { "instance", "Test Graph Inline" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "SourceFloat" },
                          { "model", { { "name", "Source<float>" } } } },
                        { { "instance", "SinkInt" }, { "model", { { "name", "Sink<int>" } } } } } },
                    { "connections",
                      { { { "out_node", "SourceFloat" },
                          { "out_index", 0 },
                          { "in_node", "SinkInt" },
                          { "in_port", "from" } } } } } } } } };
        result = g.fromJson( wrongConnection );
        REQUIRE( !result );
        g.destroy();

        // Constructed a correct graph
        nlohmann::json goodSimpleGraph = {
            { "instance", "Test Graph Inline" },
            { "model",
              { { "name", "Core DataflowGraph" },
                { "graph",
                  { { "factories", {} },
                    { "nodes",
                      { { { "instance", "SourceFloat" },
                          { "model", { { "name", "Source<float>" } } } },
                        { { "instance", "SinkFloat" },
                          { "model", { { "name", "Sink<float>" } } } } } },
                    { "connections",
                      { { { "out_node", "SourceFloat" },
                          { "out_port", "to" },
                          { "in_node", "SinkFloat" },
                          { "in_index", 0 } } } } } } } } };
        result = g.fromJson( goodSimpleGraph );
        REQUIRE( result );

        // trying to add a duplicated node
        auto duplicatedNodeName =
            std::make_shared<Sources::SingleDataSourceNode<float>>( "SourceFloat" );
        auto r = g.addNode( duplicatedNodeName );
        REQUIRE( !r );

        auto sinkFloatNode = g.getNode( "Sink" );
        REQUIRE( sinkFloatNode == nullptr );
        sinkFloatNode = g.getNode( "SinkFloat" );
        REQUIRE( sinkFloatNode != nullptr );
        auto sourceFloatNode = g.getNode( "SourceFloat" );
        REQUIRE( sourceFloatNode != nullptr );

        auto sourceIntNode = std::make_shared<Sources::IntSource>( "SourceInt" );
        auto sinkIntNode   = std::make_shared<Sinks::IntSink>( "SinkInt" );
        // node not found
        REQUIRE( !g.removeLink( sinkIntNode, "from" ) );

        // "from" node not found
        REQUIRE( !g.addLink( sourceIntNode, "out", sinkIntNode, "in" ) );

        REQUIRE( g.addNode( sourceIntNode ) );
        // "to" node not found
        REQUIRE( !g.addLink( sourceIntNode, "out", sinkIntNode, "in" ) );

        REQUIRE( g.addNode( sinkIntNode ) );
        // output port of "from" node not found
        // output port of "from" node not found
        REQUIRE( !g.addLink( sourceIntNode, "out", sinkIntNode, "in" ) );

        // input port of "to" node not found
        result = g.addLink( sourceIntNode, "to", sinkIntNode, "in" );
        REQUIRE( !result );

        // link OK
        result = g.addLink( sourceIntNode, "to", sinkIntNode, "from" );
        REQUIRE( result );

        // from port of "to" node already linked
        result = g.addLink( sourceIntNode, "to", sinkIntNode, "from" );
        REQUIRE( !result );

        // type mismatch
        result = g.addLink( sourceIntNode, "to", sinkFloatNode, "from" );
        REQUIRE( !result );

        // protect the graph to prevent link removal
        g.setNodesAndLinksProtection( true );
        REQUIRE( g.getNodesAndLinksProtection() );
        // unable to remove links from protected graph ...
        result = g.removeLink( sinkIntNode, "from" );
        REQUIRE( !result );
        g.setNodesAndLinksProtection( false );
        REQUIRE( !g.getNodesAndLinksProtection() );
        // remove link OK
        result = g.removeLink( sinkIntNode, "from" );
        REQUIRE( result );

        // input port not found to remove its link
        result = g.removeLink( sinkIntNode, "in" );
        REQUIRE( !result );

        // compile the graph
        result = g.compile();
        REQUIRE( result );
        REQUIRE( g.isCompiled() );

        // clear the graph
        g.clearNodes();

        // Nodes can't be found
        auto nullNode = g.getNode( "SourceInt" );
        REQUIRE( nullNode == nullptr );
        nullNode = g.getNode( "SinkInt" );
        REQUIRE( nullNode == nullptr );
        // Nodes can't be found
        nullNode = g.getNode( "SourceFloat" );
        REQUIRE( nullNode == nullptr );
        nullNode = g.getNode( "SinkFloat" );
        REQUIRE( nullNode == nullptr );

        // destroy everything
        g.destroy();
    }

    SECTION( "Inspection of a graph" ) {
        std::cout << "Loading graph data/Dataflow/ExampleGraph.json\n";
        auto g = DataflowGraph::loadGraphFromJsonFile( "data/Dataflow/ExampleGraph.json" );

        // Factories used by the graph
        auto factories = g->getNodeFactories();
        REQUIRE( factories != nullptr );
        const auto& nodes = g->getNodes();
        REQUIRE( nodes.size() == g->getNodesCount() );
        auto c = g->compile();
        REQUIRE( c == true );
        REQUIRE( g->isCompiled() );
        // Prints the graph content
        inspectGraph( *g );
        g->needsRecompile();
        REQUIRE( !g->isCompiled() );

        // removing the boolean sink from the graph
        auto n        = g->getNode( "validation value" );
        auto useCount = n.use_count();
        REQUIRE( n->getInstanceName() == "validation value" );
        c = g->removeNode( n );
        REQUIRE( c == true );
        REQUIRE( n );
        REQUIRE( n.use_count() == useCount - 1 );
        c = g->compile();
        REQUIRE( c == true );

        // Simplified graph after compilation
        auto& cn = g->getNodesByLevel();
        // the source "Validator" is no more in level 0 as it is not reachable from a sink in the
        // graph.
        auto found = std::find_if( cn[0].begin(), cn[0].end(), []( const auto& nn ) {
            return nn->getInstanceName() == "Validator";
        } );
        REQUIRE( found == cn[0].end() );

        // removing the source "Validator"
        n = g->getNode( "Validator" );
        REQUIRE( n->getInstanceName() == "Validator" );
        // protect the graph to prevent node removal
        g->setNodesAndLinksProtection( true );
        c = g->removeNode( n );
        REQUIRE( !c );
        g->setNodesAndLinksProtection( false );
        c = g->removeNode( n );
        REQUIRE( c );

        std::cout << "####### Graph after sink and source removal\n";
        inspectGraph( *g );
    }
}
