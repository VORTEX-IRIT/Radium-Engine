cmake_minimum_required(VERSION 3.18)

project(radiumgui-external VERSION 1.0.0)

include(ExternalProject)
include(ExternalInclude)

list(APPEND CMAKE_MESSAGE_INDENT "[Gui] ")
string(REPLACE ";" "" indent_string "${CMAKE_MESSAGE_INDENT}")
set(indent_string "${indent_string}--")

macro(add_dependency_if_compiled target dep)
    if(TARGET ${dep})
        add_dependencies(${target} ${dep})
    endif()
endmacro()

# force installing by default all the external projects
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS install)

# Add fPIC for all dependencies
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

string(REPLACE ";" "" indent_string "${CMAKE_MESSAGE_INDENT}")
set(indent_string "${indent_string}--")

add_custom_target(GuiExternals ALL)

# ------------------------------------------------------------------------------

# Force the use of Qt6, assume dependencies uses either Qt6_DIR or QT_DEFAULT_MAJOR_VERSION
set(Qt_CONFIGURATION "")
if(DEFINED Qt6_DIR)
    set(Qt_CONFIGURATION "-DQt6_DIR=${Qt6_DIR}")
endif()

set(QT_DEFAULT_MAJOUR_VERSION 6)
set(Qt_CONFIGURATION "${Qt_CONFIGURATION};-DQT_DEFAULT_MAJOR_VERSION=${QT_DEFAULT_MAJOR_VERSION}")

# configure RPATH as expected to be used under Radium
list(FIND CMAKE_PLATFORM_IMPLICIT_LINK_DIRECTORIES "${CMAKE_INSTALL_PREFIX}/lib" isSystemDir)
if("${isSystemDir}" STREQUAL "-1")
    if(APPLE)
        set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib;@executable_path/../lib")
    else()
        set(CMAKE_INSTALL_RPATH "\$ORIGIN:\$ORIGIN/../lib")
    endif()
endif("${isSystemDir}" STREQUAL "-1")

set(RPATH_CONFIG
    "-DCMAKE_SKIP_BUILD_RPATH=FALSE -DCMAKE_BUILD_WITH_INSTALL_RPATH=FALSE -DCMAKE_INSTALL_RPATH_USE_LINK_PATH=TRUE -DCMAKE_INSTALL_RPATH=\"${CMAKE_INSTALL_RPATH}\""
)

# set(RPATH_CONFIG "")

if(NOT DEFINED PowerSlider_DIR)
    check_externals_prerequisite()
    status_message("" "PowerSlider" "remote git")
    ExternalProject_Add(
        PowerSlider
        GIT_REPOSITORY https://github.com/dlyr/PowerSlider.git
        GIT_TAG origin/master
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} ${Qt_CONFIGURATION} -DBUILD_DESIGNER_PLUGIN=OFF
                   ${RPATH_CONFIG} -DBUILD_EXAMPLE_APP=OFF
                   "-DCMAKE_MESSAGE_INDENT=${indent_string}\;"
    )

    set_external_dir(PowerSlider "lib/cmake/PowerSlider/")

    add_dependencies(GuiExternals PowerSlider)
else()
    status_message("" "PowerSlider" "${PowerSlider_DIR}")
endif()
