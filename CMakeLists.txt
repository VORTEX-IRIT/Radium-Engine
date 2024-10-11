cmake_minimum_required(VERSION 3.18 FATAL_ERROR)

# ------------------------------------------------------------------------------
# Policies and global parameters for CMake
cmake_policy(SET CMP0077 NEW)
cmake_policy(SET CMP0092 NEW) # do not set /W3 for msvc

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)
include(${CMAKE_SOURCE_DIR}/cmake/cmake-message-helpers/MessageHelpers.cmake)
message_prefix_push("Main")
# Project setup, versioning stuff here, change when changing the version
# ~~~
# Note: keep the project name lower case only for easy linux packaging support
# ~~~
file(STRINGS "${CMAKE_CURRENT_SOURCE_DIR}/VERSION" INPUT_Radium_VERSION)
project(radiumproject VERSION ${INPUT_Radium_VERSION})

site_name(VERSION_HOST) # read hostname to VERSION_HOST
set(VERSION_HOST "${VERSION_HOST}" CACHE STRING "host of build" FORCE)

# Sets the Radium_VERSION* variables such that all components built in the Radium-buildtree are able
# to access them. Client applications, libraries and plugins will access these variable after their
# find_package(Radium) ...
set(Radium_VERSION ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH})
set(Radium_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(Radium_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(Radium_VERSION_PATCH ${PROJECT_VERSION_PATCH})

include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerVersion.cmake)
include(${CMAKE_CURRENT_SOURCE_DIR}/cmake/CompilerOptions.cmake)

option(
    RADIUM_ENABLE_TESTING
    "Enable testing. Tests are automatically built with target all, run with target check or test."
    ON
)
include(CMakeDependentOption)
cmake_dependent_option(
    RADIUM_ENABLE_GL_TESTING
    "Enable testing of OpenGL functionalities. Option only available if RADIUM_ENABLE_TESTING is ON."
    OFF
    "RADIUM_ENABLE_TESTING"
    OFF
)
option(
    RADIUM_ENABLE_EXAMPLES
    "Enable examples app build. To install examples, build explicitly the target Install_RadiumExamples."
    OFF
)
option(RADIUM_ENABLE_COVERAGE "Enable coverage, gcc only. Experimental, need ENABLE_TESTING" OFF)
option(RADIUM_ENABLE_PCH "Enable precompiled headers." OFF)
option(RADIUM_USE_DOUBLE "Use double precision for Scalar." OFF)
option(RADIUM_GENERATE_LIB_CORE "Include Radium::Core in CMake project." ON)
option(RADIUM_GENERATE_LIB_IO "Include Radium::IO in CMake project." ON)
option(RADIUM_GENERATE_LIB_ENGINE "Include Radium::Engine in CMake project." ON)
option(RADIUM_GENERATE_LIB_GUI "Include Radium::Gui in CMake project." ON)
option(RADIUM_GENERATE_LIB_PLUGINBASE "Include Radium::PluginBase in CMake project." ON)
option(RADIUM_GENERATE_LIB_HEADLESS "Include Radium::Headless in CMake project." ON)
option(
    RADIUM_UPDATE_VERSION
    "Update version file each time the project is compiled (update compilation time in version.cpp)."
    ON
)
option(
    RADIUM_INSTALL_DOC
    "Install documentation. If RadiumDoc is compiled, install documentation to bundle directory for install target."
    ON
)

set(DISPLAY_WIDTH 80)

if(RADIUM_USE_DOUBLE)
    add_definitions(-DCORE_USE_DOUBLE)
endif()
# Changing the default value for CMAKE_BUILD_PARALLEL_LEVEL
if(NOT DEFINED ENV{CMAKE_BUILD_PARALLEL_LEVEL})
    include(ProcessorCount)
    ProcessorCount(N)
    if(NOT N EQUAL 0)
        set(CTEST_BUILD_FLAGS -j${N})
        set(ctest_test_args ${ctest_test_args} PARALLEL_LEVEL ${N})
        set(ENV{CMAKE_BUILD_PARALLEL_LEVEL} ${N})
    endif()
endif()

# We can use include() and find_package() for our scripts in there
list(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

# ------------------------------------------------------------------------------
# General settings
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "Release")
endif()

# Be nice to visual studio
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

# Be nice and export compile commands by default, this is handy for clang-tidy and for other tools.
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Configure RPath, see cmake/RPath.cmake
include(RPath)

# Use gold linker to speed up linking time, see cmake/UseGoldLinker.cmake
include(UseGoldLinker)

# Helpful option enable build profiling to identify slowly compiling files
option(MEASURE_ALL "When enabled all commands will be passed through time command" OFF)
if(MEASURE_ALL)
    set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "time")
endif()

# Append library and executable names with d in debug mode
set(CMAKE_DEBUG_POSTFIX d)

# -------------------------------------------------------------------------------
# Set default paths for Radium
if(CMAKE_BUILD_TYPE STREQUAL "Release")
    set(RADIUM_BUNDLE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/Bundle-${CMAKE_CXX_COMPILER_ID})
else()
    set(RADIUM_BUNDLE_DIRECTORY
        ${CMAKE_CURRENT_SOURCE_DIR}/Bundle-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}
    )
endif()

set(RADIUM_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")
set(RADIUM_SHADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Shaders")

# -------------------------------------------------------------------------------
# Set default install location to RADIUM_BUNDLE_DIRECTORY we do not want to install to /usr by
# default
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${RADIUM_BUNDLE_DIRECTORY}"
        CACHE PATH "Install path prefix, prepended onto install directories." FORCE
    )
    # prevent subsequent modification of CMAKE_INSTALL_PREFIX based on
    # CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT value
    unset(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
endif()

# ------------------------------------------------------------------------------
# Custom Install target
add_custom_target(
    Install_Radium COMMAND "${CMAKE_COMMAND}" --build ${CMAKE_BINARY_DIR} --target install
    WORKING_DIRECTORY "${CMAKE_BINARY_DIR}"
)

# ------------------------------------------------------------------------------
# get changeset id
find_package(Git QUIET)

if(GIT_FOUND AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/.git")
    execute_process(
        COMMAND ${GIT_EXECUTABLE} rev-parse HEAD WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_CHANGESET
    )
    if(GIT_CHANGESET)
        # remove new line sometime appearing in git changeset
        string(REGEX REPLACE "\n$" "" GIT_CHANGESET "${GIT_CHANGESET}")
    endif()
else()
    set(GIT_CHANGESET "")
endif()

if(RADIUM_ENABLE_COVERAGE)
    set(RADIUM_ENABLE_TESTING "ON")
    include(CodeCoverage)
    append_coverage_compiler_flags()
endif(RADIUM_ENABLE_COVERAGE)

# ------------------------------------------------------------------------------
# Installation utilities
include(RadiumSetupFunctions)

message_prefix_pop()
# Documentation build
add_subdirectory(doc)

# This var gather local dependencies in subdirectories
set(LocalDependencies)

# Images, databases and other data which needs to be installed for project add_subdirectory(data)

# Add this target so that some IDE could propose to build all radium libs at once
add_custom_target(RadiumLibs)

# Source code
set(RADIUM_COMPONENTS "")
set(RADIUM_MISSING_COMPONENTS "")
add_subdirectory(src)

# Enable find_package(RadiumEngine) for the current build.
list(APPEND CMAKE_PREFIX_PATH ${CMAKE_CURRENT_BINARY_DIR}/src)

# examples
if(RADIUM_ENABLE_EXAMPLES)
    # add examples subdirectory
    add_subdirectory(examples EXCLUDE_FROM_ALL)
    add_dependencies(RadiumExamples RadiumLibs)
    # compile the examples within the all target (but do not install them within the install target)
    add_custom_target(BuildExamples ALL)
    add_dependencies(BuildExamples RadiumExamples)
    add_dependencies(Install_RadiumExamples Install_Radium)
endif()

# Testing
if(RADIUM_ENABLE_TESTING)
    enable_testing()
    include(CTest)
    # prepare integration tests
    if(NOT RADIUM_ENABLE_EXAMPLES)
        add_subdirectory(examples/CoreExample EXCLUDE_FROM_ALL)
        list(FIND RADIUM_COMPONENTS "Headless" res)
        if(res GREATER "-1")
            add_subdirectory(examples/HeadlessExample EXCLUDE_FROM_ALL)
        endif()
    endif()
    add_subdirectory(tests)
endif()

if(RADIUM_ENABLE_COVERAGE)
    append_coverage_compiler_flags()
    setup_target_for_coverage_lcov(
        NAME
        coverage
        EXECUTABLE
        ${CMAKE_CTEST_COMMAND}
        --output-on-failure
        -C
        ${CMAKE_BUILD_TYPE}
        -j
        $ENV{CMAKE_BUILD_PARALLEL_LEVEL}
        BASE_DIRECTORY
        "/"
        DEPENDENCIES
        RadiumLibs
        unittests
        integration
        EXCLUDE
        "/*external*/"
        "/*/glbinding/*"
        "/*/globjects/*"
        "/*/OpenMesh/*"
        "/*/assimp/*"
        "/*/eigen3/*"
        "/*/nlohmann/*"
        "/*/stb/*"
        "/usr/*"
        "${CMAKE_SOURCE_DIR}/examples*"
        "${CMAKE_SOURCE_DIR}/*/Catch2/*"
        "${CMAKE_BINARY_DIR}/*_autogen/*"
        "${CMAKE_BINARY_DIR}/*mocs*"
    )

    # Fastcov is not supported with gcov llvm: disabling for MacOS Source:
    # https://github.com/RPGillespie6/fastcov/issues/36
    if(UNIX AND NOT APPLE AND FASTCOV_PATH)
        setup_target_for_coverage_fastcov(
            NAME
            coverage_fastcov
            EXECUTABLE
            ${CMAKE_CTEST_COMMAND}
            --output-on-failure
            -C
            ${CMAKE_BUILD_TYPE}
            -j
            $ENV{CMAKE_BUILD_PARALLEL_LEVEL}
            DEPENDENCIES
            RadiumLibs
            unittests
            integration
            BASE_DIRECTORY
            "/"
            EXCLUDE
            "/usr/"
            "examples"
            "glbinding"
            "globjects"
            "OpenMesh"
            "assimp"
            "eigen3"
            "nlohmann"
            "stb"
            "external"
            "install"
            "/Catch2/"
            "_autogen/"
            "build"
            "mocs"
        )
    endif()
endif()

# Packaging stuff (deb, rpm, windows installer) add_subdirectory(packaging)

install(FILES LICENSE README.md TYPE DATA)

# -------------------------------------------------------------------------------
# Wrap up of settings printed on build

message(NOTICE "")
message_title(" Final overview for ${PROJECT_NAME} ")
message_info(" ")
message_info(
    "Version:               ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} @ ${VERSION_HOST}"
)
message_info("Git Changeset: ${GIT_CHANGESET}")
message_info(" ")
message_info("Install prefix: ")
message_info("  ${CMAKE_INSTALL_PREFIX}")
message_info(" ")
message_info(
    "Compiler: ${CMAKE_CXX_COMPILER} - ${CMAKE_CXX_COMPILER_ID} in version ${CMAKE_CXX_COMPILER_VERSION}."
)
message_setting("CMAKE_BUILD_TYPE")
message_info("  possible options: Debug Release RelWithDebInfo MinSizeRel")
message_info("  set with ` cmake -DCMAKE_BUILD_TYPE=Debug .. `")
message_info(" ")
message_setting("RADIUM_ENABLE_EXAMPLES")
message_setting("RADIUM_ENABLE_TESTING")
message_setting("RADIUM_ENABLE_GL_TESTING")
message_setting("RADIUM_ENABLE_COVERAGE")
message_setting("RADIUM_ENABLE_PCH")
message_setting("RADIUM_USE_DOUBLE")
message_setting("RADIUM_GENERATE_LIB_CORE")
message_setting("RADIUM_GENERATE_LIB_ENGINE")
message_setting("RADIUM_GENERATE_LIB_GUI")
message_setting("RADIUM_GENERATE_LIB_HEADLESS")
message_setting("RADIUM_GENERATE_LIB_IO")
message_setting("RADIUM_GENERATE_LIB_PLUGINBASE")
string(REPLACE ";" " " COMPONENTS_LIST "${RADIUM_COMPONENTS}")
message_info(" -- Configured components: ${COMPONENTS_LIST}")
if(NOT ${RADIUM_MISSING_COMPONENTS} STREQUAL "")
    string(REPLACE ";" " " COMPONENTS_LIST "${RADIUM_MISSING_COMPONENTS}")
    message_info(" -- Missing components: ${COMPONENTS_LIST} (see log to find why)")
endif()
message_setting("RADIUM_IO_ASSIMP")
message_setting("RADIUM_IO_TINYPLY")
message_setting("RADIUM_IO_VOLUMES")
message_setting("RADIUM_IO_DEPRECATED")
message_setting("RADIUM_INSTALL_DOC")
message_setting("RADIUM_UPDATE_VERSION")
message_setting("RADIUM_QUIET")
message_setting("USE_GOLD_LINKER")
if(QT_DEFAULT_MAJOR_VERSION)
    message_setting(QT_DEFAULT_MAJOR_VERSION)
endif()

message_end()
message(NOTICE "")

set(CACHED_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}" CACHE INTERNAL
                                                          "Previous value of CMAKE_INSTALL_PREFIX"
)
