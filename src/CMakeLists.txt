# Adds convenience methods, see cmake/CleanCppExtensions.cmake
include(CleanCppExtensions)

# Creates options to turn on Sanitizers, see cmake/Sanitizers.cmake
include(Sanitizers)

# ------------------------------------------------------------------------------
# Compilation option
if(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    add_compile_options(/W4)
    if(CMAKE_BUILD_TYPE STREQUAL "Debug")
        add_compile_options(/bigobj)
    endif()
else()
    add_compile_options(-Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -pedantic)
endif()

# ------------------------------------------------------------------------------
# Libraries dependencies
find_package(OpenMP QUIET)

# -----------------------------------------------------------------------------------
function(cat in_file out_file)
    file(READ ${in_file} CONTENTS)
    file(APPEND ${out_file} "${CONTENTS}")
endfunction()

# Prepare cmake package
include(CMakePackageConfigHelpers)
set(CONFIG_PACKAGE_LOCATION lib/cmake/Radium)

# -----------------------------------------------------------------------------------
# Include sources and declare components
foreach(lib Core Engine IO PluginBase Gui Headless Dataflow)
    string(TOUPPER ${lib} upcase_lib)
    if(RADIUM_GENERATE_LIB_${upcase_lib})
        add_subdirectory(${lib})
    endif()
endforeach()

if(QT_DEFAULT_MAJOR_VERSION)
    set(QT_DEFAULT_MAJOR_VERSION ${QT_DEFAULT_MAJOR_VERSION} PARENT_SCOPE)
endif()

add_dependencies(RadiumLibs ${RADIUM_COMPONENTS})
set(RADIUM_COMPONENTS ${RADIUM_COMPONENTS} PARENT_SCOPE)
set(RADIUM_MISSING_COMPONENTS ${RADIUM_MISSING_COMPONENTS} PARENT_SCOPE)

# -----------------------------------------------------------------------------------
# Prepare the RadiumConfigTemplate.in file:
file(WRITE ${CMAKE_CURRENT_BINARY_DIR}/RadiumConfigTemplate.in "")

# Set the general header in RadiumConfigTemplate.in
cat(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/CompilerVersion.cmake
    ${CMAKE_CURRENT_BINARY_DIR}/RadiumConfigTemplate.in
)
cat(${CMAKE_CURRENT_SOURCE_DIR}/Config.cmake.in ${CMAKE_CURRENT_BINARY_DIR}/RadiumConfigTemplate.in)

# Generate cmake package
configure_radium_package(
    NAME Radium PACKAGE_CONFIG ${CMAKE_CURRENT_BINARY_DIR}/RadiumConfigTemplate.in
                               PACKAGE_VERSION ${Radium_VERSION}
)

# install general scripts
install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/RadiumSetupFunctions.cmake"
          "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/QtFunctions.cmake"
          "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FindFilesystem.cmake"
          "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/RPath.cmake"
    DESTINATION ${CONFIG_PACKAGE_LOCATION}
)

# copy scripts in the buildtree to be available when searching package in the buildtree
file(
    COPY "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/RadiumSetupFunctions.cmake"
         "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/QtFunctions.cmake"
         "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/FindFilesystem.cmake"
         "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/RPath.cmake"
    DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
)

if(MSVC OR MSVC_IDE OR MINGW)
    install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Windeployqt.cmake"
            DESTINATION ${CONFIG_PACKAGE_LOCATION}
    )
    file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/../cmake/Windeployqt.cmake"
         DESTINATION ${CMAKE_CURRENT_BINARY_DIR}
    )
endif()

# propagate global to parent
set(LocalDependencies ${LocalDependencies} PARENT_SCOPE)

# -------------------------------------------------------------------------------
# Copy MINGW needed libraries for building on windows

if(MINGW)
    message(STATUS "MinGW detected")
    get_filename_component(GCC_PATH ${CMAKE_C_COMPILER} PATH)
    if(${GCC_PATH} MATCHES "mingw64/bin")
        set(LIBGCC "libgcc_s_seh-1.dll") # 64bit
        message(STATUS "  64bit dlls is building")
    else()
        set(LIBGCC "libgcc_s_dw2-1.dll") # 32bit
        message(STATUS "  32bit dlls is building")
    endif()

    install(FILES ${GCC_PATH}/${LIBGCC} ${GCC_PATH}/libstdc++-6.dll DESTINATION ./bin/)
endif(MINGW)
