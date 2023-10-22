set(ra_dataflowqtgui_target DataflowQtGui)
list(APPEND CMAKE_MESSAGE_INDENT "[${ra_dataflowqtgui_target}] ")

project(${ra_dataflowqtgui_target} LANGUAGES CXX VERSION ${Radium_VERSION})

find_package(RadiumNodeEditor REQUIRED NO_DEFAULT_PATH)

include(filelist.cmake)

# Qt utility functions
include(QtFunctions)

# Find Qt packages
find_qt_package(COMPONENTS Core Widgets OpenGL Xml REQUIRED)
set(QT_DEFAULT_MAJOR_VERSION ${QT_DEFAULT_MAJOR_VERSION} PARENT_SCOPE)
set(Qt_LIBRARIES Qt::Core Qt::Widgets Qt::OpenGL Qt::Xml)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
qt_wrap_ui(gui_uis ${gui_uis})

# configure library
find_package(PowerSlider REQUIRED)

add_library(
    ${ra_dataflowqtgui_target} SHARED ${dataflow_qtgui_sources} ${dataflow_qtgui_headers}
                                      ${dataflow_qtgui_resources}
)

# This one should be extracted directly from parent project properties.
target_compile_definitions(${ra_dataflowqtgui_target} PRIVATE RA_DATAFLOW_EXPORTS)

target_compile_options(${ra_dataflowqtgui_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

target_include_directories(${ra_dataflowqtgui_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

add_dependencies(${ra_dataflowqtgui_target} DataflowCore Gui)

target_link_libraries(
    ${ra_dataflowqtgui_target} PUBLIC ${Qt_LIBRARIES} DataflowCore Gui
    PUBLIC RadiumNodeEditor::RadiumNodeEditor PRIVATE PowerSlider::PowerSlider
)

message(STATUS "Configuring library ${ra_dataflowqtgui_target} with standard settings")
configure_radium_target(${ra_dataflowqtgui_target})
# configure the library only. The package is a sub-package and should be configured independently
configure_radium_library(
    TARGET ${ra_dataflowqtgui_target} COMPONENT TARGET_DIR "Dataflow/QtGui"
    FILES "${dataflow_qtgui_headers}"
)
# Generate cmake package
configure_radium_package(
    NAME ${ra_dataflowqtgui_target} PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    PACKAGE_DIR "lib/cmake/Radium/${ra_dataflowqtgui_target}" NAME_PREFIX "Radium"
)

set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_dataflowqtgui_target} PARENT_SCOPE)

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_dataflowqtgui_target} PRIVATE pch.hpp)
endif()

list(REMOVE_AT CMAKE_MESSAGE_INDENT -1)
