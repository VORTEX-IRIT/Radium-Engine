cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0042 NEW)

project(MaterialEditing VERSION 0.0.1)

# ------------------------------------------------------------------------------
# set wanted application defaults for cmake settings
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
    message("Set install prefix to ${CMAKE_INSTALL_PREFIX}")
endif()
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

# ------------------------------------------------------------------------------
find_package(Radium REQUIRED COMPONENTS Gui)

find_qt_package(COMPONENTS Core Widgets REQUIRED)
set(Qt_LIBRARIES Qt::Core Qt::Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# --------------------------------------------------------------------------------------------------

set(app_sources main.cpp)

set(app_headers)

set(app_uis)
qt_wrap_ui(app_uis_moc ${app_uis})

set(app_resources)

add_executable(${PROJECT_NAME} ${app_sources} ${app_headers} ${app_uis} ${app_resources})

target_link_libraries(${PROJECT_NAME} PUBLIC Radium::Gui ${Qt_LIBRARIES})

# ------------------------------------------------------------------------------
# RadiumGlTF is available here https://gitlab.irit.fr/storm/repos/radium/libgltf.git (branch
# Material_Edition_#950). Compile and install RadiumGlTF first, e.g. into
# "path/to/RadiumGlTFinstall/", then configure using cmake  [your configure args]
# -DRadiumGlTF_DIR="path/to/RadiumGlTFinstall/lib/cmake" -DUSE_RADIUMGLTF=ON to use it for this
# example.
option(USE_RADIUMGLTF "Enable loading/saving files with RadiumGltf extension" OFF)
if(USE_RADIUMGLTF)
    message(STATUS "${PROJECT_NAME} uses RadiumGltf extension")
    # TODO : find why this find_package is needed (at least on MacOs whe ).
    find_package(OpenMP QUIET)
    find_package(RadiumGlTF REQUIRED)
    target_compile_definitions(${PROJECT_NAME} PUBLIC USE_RADIUMGLTF)
    target_link_libraries(${PROJECT_NAME} PUBLIC RadiumGlTF::RadiumGlTF)
endif()

configure_radium_app(NAME ${PROJECT_NAME})
