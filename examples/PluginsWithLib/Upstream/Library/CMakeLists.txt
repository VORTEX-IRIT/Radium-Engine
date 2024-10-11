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
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
    message("Set default build type to ${CMAKE_BUILD_TYPE}")
endif()

project(UpstreamLibrary VERSION 1.0.0)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_BINARY_DIR}/installed-${CMAKE_CXX_COMPILER_ID}"
        CACHE PATH "Install path prefix, prepended onto install directories." FORCE
    )
    message(STATUS "Set install prefix to ${CMAKE_INSTALL_PREFIX}")
    set(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT False)
endif()

message(STATUS "[Plugins sharing lib] : configure library ${PROJECT_NAME}")

# Radium and Qt stuff
find_package(Radium COMPONENTS Engine REQUIRED)

set(markdowns README.md)
set(sources ExampleLibrary.cpp)
set(public_headers ExampleLibrary.hpp)
set(headers PrivateHeader.hpp)
set(resources LibraryResources/welcome.txt)
set(config Config.cmake.in)

# Our library project uses these sources and headers.
add_library(
    ${PROJECT_NAME} SHARED ${sources} ${headers} ${public_headers} ${markdowns} ${resources}
                           ${config}
)
target_link_libraries(${PROJECT_NAME} PUBLIC Radium::Engine)

# Client libraries source code organisation is not restricted to the Radium internal configuration.
# If the directory structure is similar to the Radium directory structure, all is managed
# automatically by the function configure_radium_library. If the directory structure is different,
# client libraries should manage their INCLUDE_DIRECTORIES property according to the directory
# structures in their source tree (BUILD_INTERFACE) or in their install tree (INSTALL_INTERFACE)
# Here, source code organisation is flat, so include_directories in the build tree is
# CMAKE_CURRENT_SOURCE_DIR, and the installation is done in PROJECT_NAME subdir (see TARGET_DIR
# parameter of configure_radium_library), so INSTALL_INTERFACE include/${PROJECT_NAME}
target_include_directories(
    ${PROJECT_NAME} PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                           $<INSTALL_INTERFACE:include/${PROJECT_NAME}>
)

# -----------------------------------------------------------------------------------
# Radium library configuration : manage package and installation

# Package is configured outside of the library to prevent redundant package prefix
configure_radium_library(
    TARGET ${PROJECT_NAME}
    TARGET_DIR ${PROJECT_NAME}
    PACKAGE_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake
    NAMESPACE RadiumExamples
    FILES "${public_headers}"
)

configure_radium_package(
    NAME ${PROJECT_NAME} PACKAGE_DIR ${CMAKE_INSTALL_PREFIX}/lib/cmake
    PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in PACKAGE_VERSION ${PROJECT_VERSION}
)

install_target_resources(
    TARGET ${PROJECT_NAME} RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/LibraryResources
)
