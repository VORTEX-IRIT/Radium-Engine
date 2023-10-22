set(ra_dataflowrendering_target DataflowRendering)
list(APPEND CMAKE_MESSAGE_INDENT "[${ra_dataflowrendering_target}] ")

project(${ra_dataflowrendering_target} LANGUAGES CXX VERSION ${Radium_VERSION})

include(filelist.cmake)

# configure library
add_library(
    ${ra_dataflowrendering_target} SHARED ${dataflow_rendering_sources}
                                          ${dataflow_rendering_headers}
)

# This one should be extracted directly from parent project properties.
target_compile_definitions(${ra_dataflowrendering_target} PRIVATE RA_DATAFLOW_EXPORTS)

target_compile_options(${ra_dataflowrendering_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

add_dependencies(${ra_dataflowrendering_target} DataflowCore Engine)
target_link_libraries(${ra_dataflowrendering_target} PUBLIC DataflowCore Engine)

message(STATUS "Configuring library ${ra_dataflowrendering_target} with standard settings")
configure_radium_target(${ra_dataflowrendering_target})
# configure the library only. The package is a sub-package and should be configured independently
configure_radium_library(
    TARGET ${ra_dataflowrendering_target} COMPONENT TARGET_DIR "Dataflow/Rendering"
    FILES "${dataflow_rendering_headers}"
)
# Generate cmake package
configure_radium_package(
    NAME ${ra_dataflowrendering_target} PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    PACKAGE_DIR "lib/cmake/Radium/${ra_dataflowrendering_target}" NAME_PREFIX "Radium"
)

set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_dataflowrendering_target} PARENT_SCOPE)

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_dataflowrendering_target} PRIVATE pch.hpp)
endif()

list(REMOVE_AT CMAKE_MESSAGE_INDENT -1)
