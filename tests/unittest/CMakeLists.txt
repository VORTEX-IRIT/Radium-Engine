#------------------------------------------------------------------------------
# Unit tests via Catch framework
#
# For testing on the function/class level.

# -----------------------------------------------------------------------------
set(test_src
    Core/algebra.cpp
    Core/animation.cpp
    Core/attribmanager.cpp
    Core/bijectiveassociation.cpp
    Core/camera.cpp
    Core/color.cpp
    Core/containers.cpp
    Core/demangle.cpp
    Core/distance.cpp
    Core/enumconverter.cpp
    Core/geometryData.cpp
    Core/indexmap.cpp
    Core/indexview.cpp
    Core/mapiterators.cpp
    Core/obb.cpp
    Core/observer.cpp
    Core/polyline.cpp
    Core/raycast.cpp
    Core/resources.cpp
    Core/string.cpp
    Core/singleton.cpp
    Core/taskqueue.cpp
    Core/topomesh.cpp
    Core/variableset.cpp
    Core/vectorarray.cpp
    Engine/environmentmap.cpp
    Engine/renderparameters.cpp
    Engine/signalmanager.cpp
    Gui/keymapping.cpp
    unittest.cpp
    unittestUtils.hpp
)

get_target_property(HAS_VOLUMES IO RADIUM_IO_HAS_VOLUMES)
if(${HAS_VOLUMES})
    message(STATUS "Compiling Volume loader unit test")
    list(APPEND test_src IO/volumeloader.cpp)
endif()

if(RADIUM_ENABLE_GL_TESTING)
    message(STATUS "Add gl related unit tests (will use EGL on Linux, glfw on macos and windows")
    list(APPEND test_src Engine/materials.cpp Engine/texture.cpp)
endif()

add_executable(unittests ${test_src})
target_include_directories(unittests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_options(unittests PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})
target_compile_definitions(unittests PRIVATE UNIT_TESTS) # add -DUNIT_TESTS define

target_link_libraries(unittests PRIVATE Catch2::Catch2 Core Engine Gui)
add_dependencies(unittests Catch2 Core Engine Gui)

find_package(Filesystem COMPONENTS Final Experimental REQUIRED)
target_compile_definitions(
    unittests
    PRIVATE -DCXX_FILESYSTEM_HAVE_FS
            -DCXX_FILESYSTEM_IS_EXPERIMENTAL=$<BOOL:${CXX_FILESYSTEM_IS_EXPERIMENTAL}>
            -DCXX_FILESYSTEM_NAMESPACE=${CXX_FILESYSTEM_NAMESPACE}
)
target_link_libraries(unittests PRIVATE std::filesystem)

find_qt_package(COMPONENTS Core Widgets OpenGL Xml REQUIRED)
set(Qt_LIBRARIES Qt::Core Qt::Widgets Qt::OpenGL Qt::Xml)
target_link_libraries(unittests PRIVATE ${Qt_LIBRARIES})

if(RADIUM_ENABLE_GL_TESTING)
    target_link_libraries(unittests PRIVATE Headless)
    add_dependencies(unittests Headless)
endif()

include(../external/Catch2/src/Catch2_download/contrib/Catch.cmake)

# adds catch tests to ctest
catch_discover_tests(unittests WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# convenience target for running only the unit tests
add_custom_target(
    run_unittests # this way we can use fake data from /tests/unittest/ dir (if we have any):
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} COMMAND $<TARGET_FILE:unittests>
    DEPENDS unittests
)

# Verbose printing of results
add_custom_target(
    run_unittests_verbose WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND $<TARGET_FILE:unittests> -s -a -r=compact DEPENDS unittests
)
