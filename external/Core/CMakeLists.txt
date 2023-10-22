cmake_minimum_required(VERSION 3.18)

project(radiumcore-external VERSION 1.0.0)

include(ExternalProject)
include(ExternalInclude)

list(APPEND CMAKE_MESSAGE_INDENT "[Core] ")
string(REPLACE ";" "" indent_string "${CMAKE_MESSAGE_INDENT}")
set(indent_string "${indent_string}--")

# force installing by default all the external projects
set_property(DIRECTORY PROPERTY EP_STEP_TARGETS install)

# Add fPIC for all dependencies
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

add_custom_target(CoreExternals ALL)

if(NOT DEFINED Eigen3_DIR)
    check_externals_prerequisite()
    status_message("[CoreExternal]" "eigen3" "remote git")

    ExternalProject_Add(
        Eigen3
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG tags/3.4.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        PATCH_COMMAND git reset --hard && git apply -v --ignore-whitespace
                      "${CMAKE_CURRENT_LIST_DIR}/patches/eigen.patch"
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} -DEIGEN_TEST_CXX11=OFF -DBUILD_TESTING=OFF
                   -DEIGEN_BUILD_DOC=OFF "-DCMAKE_MESSAGE_INDENT=${indent_string}\;"
    )
    add_dependencies(CoreExternals Eigen3)
    set_external_dir(Eigen3 "share/eigen3/cmake/")
else()
    status_message("" "eigen3" ${Eigen3_DIR})
endif()

if(NOT DEFINED OpenMesh_DIR)
    check_externals_prerequisite()
    status_message("[CoreExternal]" "OpenMesh" "remote git")
    # I found some problems when generating xcode project for OpenMesh: the libXXX.dylib link to
    # libXXX.Major.minor.dylib (eg libOpenMesh.2.1.dylib) is not generated and the script failed.
    # Need to generate this link manually. TODO, find why only OpenMesh is problematic
    ExternalProject_Add(
        OpenMesh
        GIT_REPOSITORY https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh.git
        GIT_TAG tags/OpenMesh-8.1
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} -DBUILD_APPS=OFF
                   "-DCMAKE_MESSAGE_INDENT=${indent_string}\;"
    )
    add_dependencies(CoreExternals OpenMesh)
    set_external_dir(OpenMesh "share/OpenMesh/cmake/")
else()
    status_message("" "OpenMesh" ${OpenMesh_DIR})
endif()

if(NOT DEFINED cpplocate_DIR OR NOT cpplocate_DIR)
    check_externals_prerequisite()
    status_message("[CoreExternal]" "cpplocate" "remote git")
    ExternalProject_Add(
        cpplocate
        GIT_REPOSITORY https://github.com/cginternals/cpplocate.git
        GIT_TAG tags/v2.2.0
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/cpplocate"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} -DOPTION_BUILD_TESTS=OFF
                   -DOPTION_BUILD_DOCS=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>
                   "-DCMAKE_MESSAGE_INDENT=${indent_string}\;"
    )
    add_dependencies(CoreExternals cpplocate)
    set_external_dir(cpplocate "cpplocate/")
else()
    add_custom_target(cpplocate)
    status_message("" "cpplocate" ${cpplocate_DIR})
endif()

if(NOT DEFINED nlohmann_json_DIR)
    check_externals_prerequisite()
    status_message("" "nlohmann_json" "remote git")
    ExternalProject_Add(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG tags/v3.10.5
        GIT_SHALLOW TRUE
        GIT_PROGRESS TRUE
        INSTALL_DIR "${CMAKE_INSTALL_PREFIX}"
        CMAKE_ARGS ${RADIUM_EXTERNAL_CMAKE_OPTIONS} -DJSON_Install=ON -DJSON_BuildTests=OFF
                   "-DCMAKE_MESSAGE_INDENT=${indent_string}\;"
    )
    add_dependencies(CoreExternals nlohmann_json)
    set_external_dir(nlohmann_json "lib/cmake/nlohmann_json/")
else()
    status_message("" "nlohmann_json" ${nlohmann_json_DIR})
endif()
