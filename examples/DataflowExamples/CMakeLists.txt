cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0042 NEW)

project(DataflowExamples VERSION 1.0.0)

add_custom_target(${PROJECT_NAME})
add_custom_target(Install_${PROJECT_NAME})
foreach(APP FunctionalsGraph GraphAsNode GraphEditor GraphSerialization HelloGraph)
    add_subdirectory(${APP})
    add_dependencies(${PROJECT_NAME} ${APP})
    add_dependencies(Install_${PROJECT_NAME} Install_${APP})
endforeach()
