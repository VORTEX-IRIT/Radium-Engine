cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0071 NEW)

if(APPLE)
    cmake_policy(SET CMP0042 NEW)
endif(APPLE)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

project(GraphEditor VERSION 0.0.1)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Set default install location to installed-<Compiler_ID> folder in build dir we do not want to
# install to /usr by default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX
        "${CMAKE_CURRENT_BINARY_DIR}/installed-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}"
        CACHE PATH "Install path prefix, prepended onto install directories." FORCE
    )
    message(STATUS "Set install prefix to ${CMAKE_INSTALL_PREFIX}")
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ///////////////////////////////
find_package(Radium REQUIRED COMPONENTS DataflowQtGui)

# Qt utility functions
include(QtFunctions)

# Find Qt packages
find_qt_package(COMPONENTS Core Widgets REQUIRED)
set(QT_DEFAULT_MAJOR_VERSION ${QT_DEFAULT_MAJOR_VERSION} PARENT_SCOPE)
set(Qt_LIBRARIES Qt::Core Qt::Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
qt_wrap_ui(gui_uis ${gui_uis})

# --------------------------------------------------------------------------------------------------
set(app_sources main.cpp)

# set(app_headers)

add_executable(${PROJECT_NAME} ${app_sources})

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)

target_link_libraries(${PROJECT_NAME} PUBLIC ${Qt_LIBRARIES} Radium::DataflowQtGui)

configure_radium_app(NAME ${PROJECT_NAME})
