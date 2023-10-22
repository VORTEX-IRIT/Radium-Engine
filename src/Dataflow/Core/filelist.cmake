# ----------------------------------------------------
# This file can be generated from a script:
# To do so, run "./generateFilelistForModule.sh Dataflow/Core"
# from ./scripts directory
# ----------------------------------------------------

set(dataflow_core_sources DataflowGraph.cpp Node.cpp NodeFactory.cpp Nodes/CoreBuiltInsNodes.cpp
                          TypeDemangler.cpp
)

set(dataflow_core_headers
    DataflowGraph.hpp
    EditableParameter.hpp
    Enumerator.hpp
    Node.hpp
    NodeFactory.hpp
    Nodes/CoreBuiltInsNodes.hpp
    Nodes/Functionals/BinaryOpNode.hpp
    Nodes/Functionals/CoreDataFunctionals.hpp
    Nodes/Functionals/FilterNode.hpp
    Nodes/Functionals/ReduceNode.hpp
    Nodes/Functionals/TransformNode.hpp
    Nodes/Sinks/CoreDataSinks.hpp
    Nodes/Sinks/SinkNode.hpp
    Nodes/Sources/CoreDataSources.hpp
    Nodes/Sources/FunctionSource.hpp
    Nodes/Sources/SingleDataSourceNode.hpp
    Port.hpp
    TypeDemangler.hpp
)

set(dataflow_core_private
    Nodes/Private/FunctionalsNodeFactory.hpp Nodes/Private/FunctionalsNodeFactory.cpp
    Nodes/Private/SinksNodeFactory.hpp Nodes/Private/SinksNodeFactory.cpp
    Nodes/Private/SourcesNodeFactory.hpp Nodes/Private/SourcesNodeFactory.cpp
)
