set(ra_gui_target Gui)

message_prefix_push("${ra_gui_target}")

project(${ra_gui_target} LANGUAGES CXX VERSION ${Radium_VERSION})

include(filelist.cmake)

# Qt utility functions
include(QtFunctions)

# Find packages
find_qt_package(COMPONENTS Core Widgets OpenGL Xml REQUIRED)
set(QT_DEFAULT_MAJOR_VERSION ${QT_DEFAULT_MAJOR_VERSION} PARENT_SCOPE)

find_package(OpenGL REQUIRED)
find_package(PowerSlider REQUIRED)

# Qt
set(Qt_LIBRARIES Qt::Core Qt::Widgets Qt::OpenGL Qt::Xml)
set(CMAKE_AUTOMOC_PATH_PREFIX ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_AUTOUIC_SEARCH_PATHS "${CMAKE_CURRENT_SOURCE_DIR}/")
set(CMAKE_AUTOUIC ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
include_directories(${CMAKE_CURRENT_BINARY_DIR} # Moc
)

add_library(${ra_gui_target} SHARED ${gui_sources} ${gui_headers} ${gui_uis} ${gui_resources})

# Dependencies
add_dependencies(${ra_gui_target} Core Engine PluginBase IO)
target_link_libraries(${ra_gui_target} PUBLIC Core Engine PluginBase IO)
target_link_libraries(${ra_gui_target} PRIVATE ${Qt_LIBRARIES} OpenGL::GL PowerSlider::PowerSlider)
target_include_directories(${ra_gui_target} PRIVATE ${stb_INCLUDE_DIR})

# Compile options
target_compile_options(${ra_gui_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

if(CMAKE_BUILD_TYPE MATCHES Debug)
    target_compile_definitions(${ra_gui_target} PUBLIC GUI_IS_COMPILED_WITH_DEBUG_INFO)
endif()

# Ask RadiumIO for supported loaders
get_target_property(USE_ASSIMP IO IO_HAS_ASSIMP)
if(${USE_ASSIMP})
    target_compile_definitions(${ra_gui_target} PRIVATE "-DIO_HAS_ASSIMP")
endif()
get_target_property(USE_TINYPLY IO IO_HAS_TINYPLY)
if(${USE_TINYPLY})
    target_compile_definitions(${ra_gui_target} PRIVATE "-DIO_HAS_TINYPLY")
endif()
get_target_property(HAS_VOLUMES IO IO_HAS_VOLUMES)
if(${HAS_VOLUMES})
    target_compile_definitions(${ra_gui_target} PRIVATE "-DIO_HAS_VOLUMES")
endif()

target_compile_definitions(${ra_gui_target} PRIVATE "-DRA_GUI_EXPORTS")

message(STATUS "Configuring library ${ra_gui_target} with standard settings")
configure_radium_target(${ra_gui_target})
configure_radium_library(
    TARGET ${ra_gui_target} COMPONENT PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    FILES "${gui_headers}"
)
set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_gui_target} PARENT_SCOPE)

message(STATUS "installing resources from Configs")
install_target_resources(
    TARGET ${ra_gui_target} RESOURCES_DIR ${CMAKE_CURRENT_SOURCE_DIR}/Configs
    RESOURCES_INSTALL_DIR "Radium"
)

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_gui_target} PRIVATE pch.hpp)
endif()

message_prefix_pop()
