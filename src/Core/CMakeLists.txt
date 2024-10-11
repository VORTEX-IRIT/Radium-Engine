set(ra_core_target Core)

message_prefix_push("${ra_core_target}")

project(${ra_core_target} LANGUAGES CXX VERSION ${Radium_VERSION})

option(RADIUM_QUIET "Disable Radium Log messages" OFF)

set(RA_VERSION_CPP "${CMAKE_CURRENT_BINARY_DIR}/Version.cpp")
configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Utils/Version.cpp.in" "${RA_VERSION_CPP}")
# If you want to have date and time of the build, your targets has to depend on this. This will
# force recompilation of version.o and thus forcing gcc to update __DATE__ macro.
if(RADIUM_UPDATE_VERSION)
    add_custom_target(
        versionFileTouchForRebuild COMMAND ${CMAKE_COMMAND} -E touch "${RA_VERSION_CPP}"
    )
else()
    add_custom_target(versionFileTouchForRebuild)
endif()

find_package(Threads REQUIRED)
find_package(Filesystem COMPONENTS Final Experimental REQUIRED)
find_package(Backtrace)

include(filelist.cmake)

add_library(${ra_core_target} SHARED ${core_sources} ${core_headers} ${RA_VERSION_CPP})

find_package(Eigen3 3.3 REQUIRED NO_DEFAULT_PATH)
find_package(OpenMesh REQUIRED COMPONENTS Core Tools NO_DEFAULT_PATH)
find_package(cpplocate REQUIRED NO_DEFAULT_PATH)
find_package(nlohmann_json REQUIRED NO_DEFAULT_PATH)

add_dependencies(${ra_core_target} versionFileTouchForRebuild) # We want precise time of build in
                                                               # version

target_compile_options(${ra_core_target} PUBLIC ${RA_DEFAULT_COMPILE_OPTIONS})

target_link_libraries(
    ${ra_core_target} PUBLIC OpenMeshCore OpenMeshTools Threads::Threads Eigen3::Eigen
                             cpplocate::cpplocate nlohmann_json::nlohmann_json std::filesystem
)

target_compile_definitions(${ra_core_target} PRIVATE RA_CORE_EXPORTS)
target_compile_definitions(
    ${ra_core_target}
    PUBLIC -DCXX_FILESYSTEM_HAVE_FS
           -DCXX_FILESYSTEM_IS_EXPERIMENTAL=$<BOOL:${CXX_FILESYSTEM_IS_EXPERIMENTAL}>
           -DCXX_FILESYSTEM_NAMESPACE=${CXX_FILESYSTEM_NAMESPACE}
)

configure_file(radium_backtrace.h.in radium_backtrace.h)
target_include_directories(${ra_core_target} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})
if(Backtrace_FOUND)
    target_include_directories(${ra_core_target} PRIVATE ${Backtrace_INCLUDE_DIRS})
    if(DEFINED Backtrace_LIBRARIES)
        target_link_libraries(${ra_core_target} PRIVATE ${Backtrace_LIBRARIES})
    endif()
endif()

if(WIN_32)
    target_compile_definitions(${ra_core_target} PUBLIC _USE_MATH_DEFINES) # OpenMesh
endif()
if(${RADIUM_QUIET})
    target_compile_definitions(${ra_core_target} PUBLIC RA_NO_LOG)
    message(STATUS "${PROJECT_NAME} : Radium Logs disabled")
endif()

message(STATUS "Configuring library ${ra_core_target} with standard settings")
configure_radium_target(${ra_core_target})
configure_radium_library(
    TARGET ${ra_core_target} COMPONENT PACKAGE_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in
    FILES "${core_headers}"
)
set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} ${ra_core_target} PARENT_SCOPE)

if(RADIUM_ENABLE_PCH)
    target_precompile_headers(${ra_core_target} PRIVATE pch.hpp)
endif()

message_prefix_pop()
