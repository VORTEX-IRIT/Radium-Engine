set(ra_dataflowcore_target DataflowCore)
list(APPEND CMAKE_MESSAGE_INDENT "[${ra_dataflowcore_target}] ")

project(${ra_dataflowcore_target} LANGUAGES CXX VERSION ${Radium_VERSION})

include(filelist.cmake)

add_library(
    ${ra_dataflowcore_target} SHARED ${dataflow_core_sources} ${dataflow_core_headers}
                                     ${dataflow_core_private}
)

# This one should be extracted directly from parent project properties.
target_compile_definitions(${ra_dataflowcore_target} PRIVATE RA_DATAFLOW_EXPORTS)

target_compile_options(${ra_dataflowcore_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    target_compile_options(${ra_dataflowcore_target} PRIVATE /bigobj)
endif()

add_dependencies(${ra_dataflowcore_target} Core)
target_link_libraries(${ra_dataflowcore_target} PUBLIC Core)

message(STATUS "Configuring library ${ra_dataflowcore_target} with standard settings")
configure_radium_target(${ra_dataflowcore_target})
# configure the library only. The package is a sub-package and should be configured independently
configure_radium_library(
    TARGET ${ra_dataflowcore_target} COMPONENT TARGET_DIR "Dataflow/Core"
    FILES "${dataflow_core_headers}"
)
# Generate cmake package
configure_radium_package(
    NAME ${ra_dataflowcore_target} PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    PACKAGE_DIR "lib/cmake/Radium/${ra_dataflowcore_target}" NAME_PREFIX "Radium"
)

set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_dataflowcore_target} PARENT_SCOPE)

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_dataflowcore_target} PRIVATE pch.hpp)
endif()

list(REMOVE_AT CMAKE_MESSAGE_INDENT -1)
