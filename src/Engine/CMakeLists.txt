set(ra_engine_target Engine)

message_prefix_push("${ra_engine_target}")

project(${ra_engine_target} LANGUAGES CXX VERSION ${Radium_VERSION})

include(filelist.cmake)
set(engine_shaders_full ${engine_shaders})

if(CMAKE_VERSION VERSION_LESS 3.16)
    string(REGEX REPLACE "([^;]+)" "${RADIUM_SHADERS_DIR}/\\1" engine_shaders_full
                         "${engine_shaders}"
    )
else()
    list(TRANSFORM engine_shaders_full PREPEND ${RADIUM_SHADERS_DIR}/)
endif()

add_library(${ra_engine_target} SHARED ${engine_sources} ${engine_headers})

add_custom_target(RadiumEngineShaders ALL SOURCES ${engine_shaders_full})

find_package(glbinding REQUIRED NO_DEFAULT_PATH)
find_package(globjects REQUIRED NO_DEFAULT_PATH)
find_package(Eigen3 REQUIRED NO_DEFAULT_PATH)
find_package(tinyEXR REQUIRED NO_DEFAULT_PATH)

# This one should be extracted directly from external project properties.
target_compile_definitions(${ra_engine_target} PUBLIC GLOBJECTS_USE_EIGEN)

target_compile_options(${ra_engine_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

add_dependencies(${ra_engine_target} Core RadiumEngineShaders)
target_link_libraries(
    ${ra_engine_target} PUBLIC Core glbinding::glbinding glbinding::glbinding-aux
                               globjects::globjects tinyEXR::tinyEXR
)

target_compile_definitions(${ra_engine_target} PRIVATE "-DRA_ENGINE_EXPORTS")
# stb is header only, no stb target, add its directory to include search path
target_include_directories(${ra_engine_target} PRIVATE ${stb_INCLUDE_DIR})

message(STATUS "Configuring library ${ra_engine_target} with standard settings")
configure_radium_target(${ra_engine_target})
configure_radium_library(
    TARGET ${ra_engine_target} COMPONENT PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    FILES "${engine_headers}"
)
set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_engine_target} PARENT_SCOPE)

message(STATUS "installing resources from ${RADIUM_SHADERS_DIR}")
install_target_resources(
    TARGET ${ra_engine_target} RESOURCES_DIR ${RADIUM_SHADERS_DIR} RESOURCES_INSTALL_DIR "Radium"
    FILES ${engine_shaders}
)
message(STATUS "installing resources from ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Textures")
install_target_resources(
    TARGET ${ra_engine_target} RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Textures
    RESOURCES_INSTALL_DIR "Radium"
)
message(STATUS "installing resources from ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Metadata")
install_target_resources(
    TARGET ${ra_engine_target} RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Resources/Metadata
    RESOURCES_INSTALL_DIR "Radium"
)

target_compile_options(${ra_engine_target} PRIVATE PUBLIC ${DEFAULT_COMPILE_OPTIONS} INTERFACE)

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_engine_target} PRIVATE pch.hpp)
endif()

message_prefix_pop()
