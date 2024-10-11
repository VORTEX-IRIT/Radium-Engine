cmake_minimum_required(VERSION 3.18)
# ------------------------------------------------------------------------------
# Policies and global parameters for CMake
if(POLICY CMP0077)
    # allow to define options cache variable before the option is declared
    # https://cmake.org/cmake/help/latest/policy/CMP0077.html
    cmake_policy(SET CMP0077 NEW)
endif()
if(APPLE)
    # MACOSX_RPATH is enabled by default. https://cmake.org/cmake/help/latest/policy/CMP0042.html
    cmake_policy(SET CMP0042 NEW)
endif(APPLE)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("Set default build type to ${CMAKE_BUILD_TYPE}")
endif()

project(PluginsWithLib VERSION 1.0.0)

# Set default install location to installed-<Compiler_ID> folder in build dir we do not want to
# install to /usr by default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_CURRENT_BINARY_DIR}/installed-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}"
        CACHE PATH "Install path prefix, prepended onto install directories." FORCE
    )
    message("Set install prefix to ${CMAKE_INSTALL_PREFIX}")
endif()

# configure the demo
add_custom_target(${PROJECT_NAME})

# Upstream library and plugin
add_subdirectory(Upstream)
add_dependencies(${PROJECT_NAME} UpstreamLibrary UpstreamPlugin)

# Downstream plugin that search for the package UpstreamLibrary allow to find_package in the build
# tree
set(CMAKE_PREFIX_PATH "${CMAKE_PREFIX_PATH};${CMAKE_CURRENT_BINARY_DIR}/Upstream/Library")
add_subdirectory(Downstream)
add_dependencies(${PROJECT_NAME} DownstreamPlugin)

# documentation (to be updated)
add_custom_target(${PROJECT_NAME}-Doc SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/README.md)

# install
add_custom_target(
    Install_${PROJECT_NAME} COMMAND "${CMAKE_COMMAND}" -P
                                    ${CMAKE_CURRENT_BINARY_DIR}/cmake_install.cmake
)
add_dependencies(Install_${PROJECT_NAME} ${PROJECT_NAME})
