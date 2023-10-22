cmake_minimum_required(VERSION 3.6)

project(radiumdataflow-external VERSION 1.0.0)

include(ExternalProject)
include(ExternalInclude)

list(APPEND CMAKE_MESSAGE_INDENT "[Dataflow] ")
string(REPLACE ";" "" indent_string "${CMAKE_MESSAGE_INDENT}")
set(indent_string "${indent_string}--")

# force installing by default all the external projects
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS install)

# Add fPIC for all dependencies
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_custom_target(DataflowExternals ALL)

if(NOT DEFINED RadiumNodeEditor_DIR)
    check_externals_prerequisite()
    status_message("" "RadiumNodeEditor" "remote git")
    ExternalProject_Add(
        RadiumNodeEditor
        GIT_REPOSITORY https://github.com/MathiasPaulin/RadiumQtNodeEditor.git
        GIT_TAG main
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                   "-DCMAKE_MESSAGE_INDENT=${indent_string}\;"
    )
    set_external_dir(RadiumNodeEditor "lib/cmake/RadiumNodeEditor/")
    add_dependencies(DataflowExternals RadiumNodeEditor)
else()
    status_message("" "RadiumNodeEditor" ${RadiumNodeEditor_DIR})
endif()
