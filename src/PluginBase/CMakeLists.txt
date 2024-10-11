set(ra_pluginbase_target PluginBase)

message_prefix_push("${ra_pluginbase_target}")

project(${ra_pluginbase_target} LANGUAGES CXX VERSION ${Radium_VERSION})

include(filelist.cmake)

# Qt utility functions
include(QtFunctions)

# Find packages
find_qt_package(COMPONENTS Core REQUIRED)

# Qt
set(Qt_LIBRARIES Qt::Core)
set(CMAKE_AUTOMOC_PATH_PREFIX ON)
set(CMAKE_AUTOMOC ON)

add_library(
    ${ra_pluginbase_target} SHARED ${pluginbase_sources} ${pluginbase_headers} ${pluginbase_json}
)

add_dependencies(${ra_pluginbase_target} Core Engine)
target_link_libraries(${ra_pluginbase_target} PUBLIC Core Engine)
target_link_libraries(${ra_pluginbase_target} PRIVATE Qt::Core)

target_compile_options(${ra_pluginbase_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

target_compile_definitions(${ra_pluginbase_target} PRIVATE "-DRA_PLUGINBASE_EXPORTS")

message(STATUS "Configuring library ${ra_pluginbase_target} with standard settings")
configure_radium_target(${ra_pluginbase_target})
configure_radium_library(
    TARGET ${ra_pluginbase_target} COMPONENT
    PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in FILES "${pluginbase_headers}"
)
set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_pluginbase_target} PARENT_SCOPE)

# install json configuration files
install(FILES ${pluginbase_json} DESTINATION ${CONFIG_PACKAGE_LOCATION}/${ra_pluginbase_target})
file(COPY ${pluginbase_json} DESTINATION ${CMAKE_CURRENT_BINARY_DIR})

message_prefix_pop()
