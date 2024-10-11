# https://cmake.org/cmake/help/latest/manual/ctest.1.html
# https://cmake.org/Wiki/CMake/Testing_With_CTest
# https://cmake.org/cmake/help/latest/command/add_test.html
message_prefix_push("Tests")

# Adds convenience methods, see cmake/CleanCppExtensions.cmake
include(CleanCppExtensions)

# Creates options to turn on Sanitizers, see cmake/Sanitizers.cmake
include(Sanitizers)

# Enable helpfull warnings and C++17 for all files
if(MSVC)
    add_compile_options(/std:c++17 /W4)
else()
    add_compile_options(-std=c++17 -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -pedantic)
endif()

# ------------------------------------------------------------------------------
# ---- CTest test suite --------------------------------------------------------
# use add_test() to add a new test to the test suit. When executed test fails (returns not 0), test
# failed. There are more options, check CMake documentation.
#
# the add_test() are done in subfolder's CMakeLists.txt

add_subdirectory(external)

# unittest use catch2 to define unittests on low level functions
add_subdirectory(unittest)

# integration run whole program with parameters, check if it will crash, produce correct results,
# etc.
add_subdirectory(integration)

# Convenience targets for fast testing, they depends on binaries (so the build is triggered, when
# sources were changed).
add_custom_target(
    check
    COMMAND echo [----] Running tests
    USES_TERMINAL
    COMMAND ${CMAKE_CTEST_COMMAND} -j $ENV{CMAKE_BUILD_PARALLEL_LEVEL} --output-on-failure -C
            ${CMAKE_BUILD_TYPE}
    DEPENDS unittests integration
)

add_custom_target(
    check_verbose
    COMMAND echo [----] Running tests
    USES_TERMINAL
    COMMAND ${CMAKE_CTEST_COMMAND} -j $ENV{CMAKE_BUILD_PARALLEL_LEVEL} -V --output-on-failure -C
            ${CMAKE_BUILD_TYPE}
    DEPENDS unittests integration
)

message_prefix_pop()
