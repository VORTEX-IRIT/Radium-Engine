cmake_minimum_required(VERSION 3.18)

# Find Qt6 packages Parameters: COMPONENTS <component_list>: optional parameter listing the Qt
# packages (e.g. Core, Widgets REQUIRED: optional parameter propagated to find_package
#
# Usage: find_qt_package(COMPONENTS Core Widgets OpenGL Xml REQUIRED) which is equivalent to:
# find_package(Qt6 COMPONENTS Core Widgets OpenGL Xml REQUIRED)
#
# https://doc.qt.io/qt-6/cmake-qt5-and-qt6-compatibility.html#versionless-targets

macro(find_qt_package)
    set(options REQUIRED)
    set(oneValueArgs "")
    set(multiValueArgs COMPONENTS)

    cmake_parse_arguments(MY_OPTIONS "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT MY_OPTIONS_COMPONENTS) # User didn't enter any component
        set(MY_OPTIONS_COMPONENTS "")
    endif()

    if(${MY_OPTIONS_REQUIRED})
        set(QT_SEARCH_MODE REQUIRED)
    else()
        set(QT_SEARCH_MODE QUIET)
    endif()

    find_package(Qt6 COMPONENTS ${MY_OPTIONS_COMPONENTS} ${QT_SEARCH_MODE})

endmacro()

# see find_qt_package
macro(find_qt_dependency)
    find_qt_package(${ARGN})
endmacro()

macro(check_and_set_qt_version RADIUM_QT_DEFAULT_MAJOR_VERSION)
    if(NOT RADIUM_QT_DEFAULT_MAJOR_VERSION STREQUAL "")
        if(QT_DEFAULT_MAJOR_VERSION AND NOT QT_DEFAULT_MAJOR_VERSION STREQUAL
                                        "${RADIUM_QT_DEFAULT_MAJOR_VERSION}"
        )
            message(
                FATAL_ERROR
                    " Radium was compiled with Qt${RADIUM_QT_DEFAULT_MAJOR_VERSION}"
                    " while current project defines Qt${QT_DEFAULT_MAJOR_VERSION}\n"
                    " set correct version with -DQT_DEFAULT_MAJOR_VERSION=${RADIUM_QT_DEFAULT_MAJOR_VERSION}"
            )
        endif()
        set(QT_DEFAULT_MAJOR_VERSION ${RADIUM_QT_DEFAULT_MAJOR_VERSION})
    endif()
endmacro()
