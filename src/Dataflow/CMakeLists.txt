set(ra_dataflow_target Dataflow)
list(APPEND CMAKE_MESSAGE_INDENT "[${ra_dataflow_target}] ")

project(${ra_dataflow_target} LANGUAGES CXX VERSION ${Radium_VERSION})

set(dataflow_headers RaDataflow.hpp)
add_library(${ra_dataflow_target} INTERFACE)

if(RADIUM_GENERATE_LIB_CORE)
    add_subdirectory(Core)
    add_dependencies(${ra_dataflow_target} DataflowCore)
    target_link_libraries(${ra_dataflow_target} INTERFACE DataflowCore)
endif()

# if(RADIUM_GENERATE_LIB_ENGINE) add_subdirectory(Rendering) add_dependencies(${ra_dataflow_target}
# DataflowRendering) target_link_libraries(${ra_dataflow_target} INTERFACE DataflowRendering)
# endif()

if(RADIUM_GENERATE_LIB_GUI)
    add_subdirectory(QtGui)
    add_dependencies(${ra_dataflow_target} DataflowQtGui)
    target_link_libraries(${ra_dataflow_target} INTERFACE DataflowQtGui)
endif()

message(STATUS "Configuring library ${ra_dataflow_target} with standard settings")
target_include_directories(
    ${ra_dataflow_target} INTERFACE $<BUILD_INTERFACE:${RADIUM_SRC_DIR}>
                                    $<INSTALL_INTERFACE:include>
)
configure_radium_library(
    TARGET ${ra_dataflow_target} COMPONENT
    PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in FILES "${dataflow_headers}"
)
set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_dataflow_target} PARENT_SCOPE)
set(RADIUM_MISSING_COMPONENTS ${RADIUM_MISSING_COMPONENTS} PARENT_SCOPE)
