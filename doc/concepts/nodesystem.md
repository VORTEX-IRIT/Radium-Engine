\page nodeSystem Radium node system
[TOC]

# Radium node system

Radium-Engine embed a node system allowing to develop computation graph using an adaptation of dataflow programming.
This documentation explain the concepts used in the node system and how to develop computation graph using the Core
node system and how to extend the Core node system to be used in specific Radium-Engine application or library.

## Structure and usage of the Radium::Dataflow component

When building the Radium-Engine libraries, the node system is available from the Radium::Dataflow component.
The availability of this component in the set of built/installed libraries is managed using the
`RADIUM_GENERATE_LIB_DATAFLOW` cmake option (set to `ON` by default) of the main Radium-Engine CMakeLists.txt.

The Radium::Dataflow component is a header-only library with is linked against three sub-components :

- **DataflowCore** library defining the Core node system and a set Core Nodes allowing to develop several computation
 graph.
- **DataflowQtGui** library defining Qt based Gui elements to edit and interact with a computation graph.
- **DataflowRendering** library defining specific nodes to be used by a Graph-based renderer allowing to easily define
 custom renderers.

When defining the CMakeLists.txt configuration file for an application/library that will build upon the node system,
The RadiumDataflow component might be requested in several way :

- `find_package(Radium REQUIRED COMPONENTS Dataflow)`. This will define the imported target `Radium::Dataflow` that
gives access to all the available sub-components at once but also through imported targets `Radium::DataflowCore`,
`Radium::DataflowQtGui` and `Radium::DataflowRendering`.
- `find_package(Radium REQUIRED COMPONENTS DataflowCore)`. This will define the imported target `Radium::DataflowCore`
only.
- `find_package(Radium REQUIRED COMPONENTS DataflowQtGui)`. This will define the imported target `Radium::DataflowQtGui`
only, with transitive dependencies on `Radium::DataflowCore`.
- `find_package(Radium REQUIRED COMPONENTS DataflowRendering)`. This will define the imported target
`Radium::DataflowRendering` only, with transitive dependencies on `Radium::DataflowCore`.

The targets that depends on a Dataflow components should then be configured as any target and linked against the
requested dataflow component, by, e.g, adding the line
`target_link_libraries(target_name PUBLIC Radium::Dataflow)` (or the only needed subcomponent).

## Concepts of the node system

The node system allow to build computation graph that takes its input from some _data source_ and store their results
in some _data sink_ after applying several _functions_ on the data.

Computation graphs can be serialized and un-serialized in json format. The serialization process is nevertheless limited
to serializable data stored on the node and it is of the responsibility of the application to manage non serializable
data such as, e.g. anonymous functions (lambdas, functors, ...) dynamically defined by the application.

The Radium node system relies on the following concepts

- _Node_ (see Ra::Dataflow::Core::Node for reference manual) : a node represent a function that will be executed on
several strongly typed input data, defining the definition domain of the function, to produce some strongly typed
output data, defining the definition co-domain of the function.

  The input and output data are accessed through _ports_  allowing to connect nodes together to form a graph.

  The node profile is implicitly defined by its domain and co-domain.

  A node can be specialized to be a _data source_ node (empty domain) or a _data sink_ node (empty co-domain).
  These specific nodes define the input and output of a complex _computation graph_

- _Port_ (see Ra::Dataflow::Core::PortBase for reference manual) : a port represent an element of the node
profile and allow to build the computation graph by linking ports together, implicitly defining _links_.

  A port gives access to a strongly typed data and, while implementing the general Ra::Dataflow::Core::PortBase
interface should be specialized to be either an input port (element of the definition domain of a node) through the
instancing of the template Ra::Dataflow::Core::PortIn or to an output port (element of the definition
co-domain of a node) through the instancing of the template Ra::Dataflow::Core::PortOut.

  When a node executes its function, it takes its parameter from its input ports and set the result on the output port.

  An output port can be connected to an input port of the same _DataType_ to build the computation graph.
- _Graph_ (see Ra::Dataflow::Core::DataflowGraph for reference manual) : a graph is a set of node connected through
their ports so that they define a direct acyclic graph (DAG) representing a complex function. The DAG represents
connections from some _data source_ nodes to some _data sink_ nodes through

  Once built by adding nodes and links, a graph should be _compiled_ so that the system verify its validity
(DAG, types, connections, ...).

  Once compiled, a graph can be _executed_.
  The input data of the computation graph should be set on the available _data sources_ of the graph and the results
fetched from the _data sinks_.

  As a graph can be used as a node (a sub graph) in any other graph. When doing this, all _data sources_ and
_data sinks_ nodes are associated with _interface ports_ and these interface ports are added as _input_ or _output_
ports on the graph so that links can be defined using these ports.

  _input_ and _output_ ports of a graph can also be accessed directly from the application using _data setters_ and
_data getters_ fetched from the graph. These _data setters_ and _data getters_ allows to use any graph without the
need to know explicitly their _data sources_ and _data sinks_ nor defining ports to be linked with the _input_ and
_output_ ports of the graph.

- _Factories_ (see Ra::Dataflow::Core::NodeFactoriesManager for reference manual) : the serialization of a graph output
a set of json object describing the graph. If serialization is always possible, care must be taken for the system to
manage un-serilization of any nodes.

  When serializing a graph, the json representing a node contains the type (the name of the concrete C++ class) of the
node and several other properties of the node system. When un-serializing a graph, nodes will be automatically
instanced from their type.
The instantiation of a node is made using services offered by node factories and associated to the node type. So, in
order to be un-serializable, each node must register its type to a factory and each graph must refer to the factories used
to instantiate its node.

## HelloGraph : your first program that uses the node system

The example application examples/HelloGraph shows how to define a computation graph to apply filtering on a collection.
In this example, whose code is detailed below, the following graph is built and executed using different input data.

![HelloGraph computation graph](images/HelloGraph.png)

This graphs has two inputs, corresponding to the two **Source< ... >** nodes. These input will deliver to the
computation graph :

- from a Ra::Dataflow::Core::Sources::SingleDataSourceNode, a vector of scalars, whose container type is
Ra::Core::VectorArray (abridged here as a RaVector) and value type is _Scalar_ (float in the default Radium-Engine
configuration),
- from a Ra::Dataflow::Core::Sources::FunctionSourceNode a predicate whose type is _std::function<bool(float const&)>_
which returns _true_ if its parameter is valid according to some decision process.

These two _sources_ are linked to the input of a Ra::Dataflow::Core::Functionals::FilterNode, here represented by the
**Filter< ... >** node. This node select from its **in** input only the values validated by the predicate **f** and
built its output **out** with these values.

The result of this filtering is linked to the graph output, corresponding to the Ra::Dataflow::Core::Sinks::SinkNode
**Sink< ... >**.

Once the graph is built and compile, the HelloGraph application sent different input to the graph and print the result
of the computation.

To develop such an application, the following should be done

### 1. Building and inspecting the graph

First, an object of type Ra::Dataflow::Core::DataflowGraph is instanced :
\snippet examples/DataflowExamples/HelloGraph/main.cpp Creating an empty graph

Then, the nodes are instanced
\snippet examples/DataflowExamples/HelloGraph/main.cpp Creating Nodes

and added to the graph
\snippet examples/DataflowExamples/HelloGraph/main.cpp Adding Nodes to the graph

Links between ports are added to the graph, and if an error is detected, due to e.g. port type incompatiblitiy, it is
reported
\snippet examples/DataflowExamples/HelloGraph/main.cpp Creating links between Nodes

Once the graph is built, it can be inspected using dedicated methods.
\snippet examples/DataflowExamples/HelloGraph/main.cpp Inspect the graph interface : inputs and outputs port

For the graph constructed above, this prints on stdout

```text
Input ports (2) are :
 "Selector_f" accepting type function<bool (float const&)>
 "Source_to" accepting type RaVector<float>
Output ports (1) are :
 "Sink_from" generating type RaVector<float>
```

### 2. Compiling the graph and getting input/output accessors

In order to use the graph as a function acting on its input, it should be first compiled by
\snippet examples/DataflowExamples/HelloGraph/main.cpp Verifying the graph can be compiled

If the compilation success the accessors for the input data and the output result might be fetched from the graph
\snippet examples/DataflowExamples/HelloGraph/main.cpp Configure the interface ports (input and output of the graph)

Here, the accessor `input` allows to set the pointer on the `RaVector` to be processed while the accessor `selector`
allows to set the predicate to evaluate when filtering the collection. This predicates select values les than `0.5`

The accessor `output` will allow, once the graph is executed, to get a reference to or to copy the resulting values.

### 3. Executing the graph

Once the input data are available (in this example, the `test` vector is filled with 10 random values between 0 and 1),
the graph can be executed and a reference to the resulting vector can be fetched using
\snippet examples/DataflowExamples/HelloGraph/main.cpp Execute the graph

### 4. Multiple run of the graph on different input

As accessors use pointers and references on the data sent to / fetched from the graph, the HelloGraph example shows how
to change the input using different available means so that every run of the graph process different values.
See the file examples/DataflowExamples/HelloGraph/main.cpp for all the details.

## Examples of graphs and of programming custom nodes

The unittests developed alongside the Radium::Dataflow component, and located in the directory
`tests/unittest/Dataflow/` of the Radium-Engine source tree, can be used to learn the following :

- sourcesandsinks.cpp : demonstrate the default supported types for sources and sinks node.
- nodes.cpp : demonstrate the development of a more complex graph implementing transform/reduce
on several collections using different reduction operators.
- graphinspect.cpp : demonstrate the way a graph can be inspected to discover its structure.
- serialization.cpp : demonstrate how to save/load a graph from a json file and use it in an
application.
- customnodes.cpp : demonstrate how it is simple to develop your own node type (in C++) and
use your nodes alongside standard nodes.
  \snippet unittest/Dataflow/customnodes.cpp Develop a custom node
