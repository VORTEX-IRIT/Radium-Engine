cmake_minimum_required(VERSION 3.18)
cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0042 NEW)

project(SimpleSimulation VERSION 1.0.0)

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
find_package(Radium REQUIRED Core Engine Gui)

find_qt_package(COMPONENTS Core Widgets REQUIRED)
set(Qt_LIBRARIES Qt::Core Qt::Widgets)
set(CMAKE_AUTOMOC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# ------------------------------------------------------------------------------

set(app_sources main.cpp)
set(app_headers)

set(app_uis)
qt_wrap_ui(app_uis_moc ${app_uis})

set(app_resources)

# to install the app as a redistribuable bundle on macos, add MACOSX_BUNDLE when calling
# add_executable
add_executable(${PROJECT_NAME} ${app_sources} ${app_headers} ${app_uis} ${app_resources})

if("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    target_compile_options(
        ${PROJECT_NAME}
        PRIVATE /MP
                /W4
                /wd4251
                /wd4592
                /wd4127
                /Zm200
                $<$<CONFIG:Release>:
                /Gw
                /GS-
                /GL
                /GF
                >
        PUBLIC
    )
endif()

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(
    ${PROJECT_NAME} PRIVATE ${RADIUM_INCLUDE_DIRS} ${CMAKE_CURRENT_BINARY_DIR} # Moc
                            ${CMAKE_CURRENT_SOURCE_DIR}
)

target_link_libraries(
    ${PROJECT_NAME} PUBLIC Radium::Core Radium::Engine Radium::Gui ${Qt_LIBRARIES}
)

# configure the application
configure_radium_app(NAME ${PROJECT_NAME})
