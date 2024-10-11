cmake_minimum_required(VERSION 3.18)

if(APPLE)
    cmake_policy(SET CMP0042 NEW)
endif(APPLE)

set(ra_headless_target Headless)

message_prefix_push("${ra_headless_target}")

project(${ra_headless_target} LANGUAGES CXX VERSION ${Radium_VERSION})
# -----------------------------------------------------------------------------------
# lib headless configuration
# -----------------------------------------------------------------------------------
# find required dependencies
find_package(glfw3 3.3 QUIET)
find_package(OpenGL COMPONENTS EGL QUIET)

if(NOT glfw3_FOUND AND NOT OpenGL_EGL_FOUND)
    # do not configure Radium headless if dependency is not found
    message(WARNING "glfw3 and EGL packages not found. Headless component is disabled")
    set(RADIUM_MISSING_COMPONENTS ${RADIUM_MISSING_COMPONENTS} ${ra_headless_target} PARENT_SCOPE)
else()
    # configure the headless component for build and install as component of the Radium package
    message(STATUS "Headless component is enabled")
    # set source files

    include(filelist.cmake)

    add_library(${ra_headless_target} SHARED ${headless_sources} ${headless_headers})
    add_dependencies(${ra_headless_target} Core Engine IO)
    target_link_libraries(${ra_headless_target} PUBLIC Radium::Core Radium::Engine Radium::IO)
    target_compile_options(${ra_headless_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})
    target_compile_definitions(
        ${ra_headless_target} PRIVATE ${ra_headless_target}_EXPORTS GLFW_INCLUDE_NONE
    )
    if(glfw3_FOUND)
        message(STATUS "Headless component has glfw3 support")
        target_compile_definitions(
            ${ra_headless_target} PUBLIC HEADLESS_HAS_GLFW PRIVATE GLFW_INCLUDE_NONE
        )
        target_link_libraries(${ra_headless_target} PRIVATE glfw)
        set(HEADLESS_HAS_GLFW TRUE)
        set_target_properties(
            ${ra_headless_target} PROPERTIES HEADLESS_HAS_GLFW ${HEADLESS_HAS_GLFW}
        )
    else()
        message(STATUS "Headless component does not support glfw3")
    endif()
    if(OpenGL_EGL_FOUND)
        message(STATUS "Headless component has EGL support")
        target_compile_definitions(${ra_headless_target} PUBLIC HEADLESS_HAS_EGL)
        target_link_libraries(${ra_headless_target} PRIVATE OpenGL::EGL)
        set(HEADLESS_HAS_EGL TRUE)
        set_target_properties(${ra_headless_target} PROPERTIES HEADLESS_HAS_EGL ${HEADLESS_HAS_EGL})
    else()
        message(STATUS "Headless component does not support EGL")
    endif()

    configure_radium_target(${ra_headless_target})

    # configure the component "Headless" of packages "Radium"
    set(installed_headers "${headless_headers}")
    configure_radium_library(
        TARGET ${ra_headless_target} COMPONENT
        PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in FILES "${installed_headers}"
    )
    set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_headless_target} PARENT_SCOPE)
endif()

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_headless_target} PRIVATE pch.hpp)
endif()

message_prefix_pop()
