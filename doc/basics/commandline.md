\page basicsCompileCommand Commandline Compilation Instructions (Linux, macos...)

# Building and installing Radium dependencies once for all {#builddep}

We provide a standalone cmake project (`Radium-Engine/external/CMakeLists.txt`) to compile and install the Radium dependencies at any location.

## Configuration and compilation of the dependencies

For command line, tested on Linux and mac OS, **for windows might need more configuration options**.

External dependencies have to be installed outside Radium-Engine source tree.

~~~{.bash}
# from wherever you want outside radium source tree
# in release
cmake -S Radium-Engine/external -B builds/radium-external-build-r -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installs/radium-external-r
cmake --build builds/radium-external-build-r --config Release --parallel

# in debug
cmake -S Radium-Engine/external -B builds/radium-external-build-d -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=installs/radium-external-d
cmake --build builds/radium-external-build-d --config Debug --parallel
~~~

If not given on the command line, the installation directory is set by default to `Radium-Engine/{CMAKE_CURRENT_BINARY_DIR}/Bundle-${CMAKE_CXX_COMPILER_ID}` for `CMAKE_BUILD_TYPE=Release`, and  `Radium-Engine/{CMAKE_CURRENT_BINARY_DIR}/Bundle-${CMAKE_CXX_COMPILER_ID}-${CMAKE_BUILD_TYPE}` for any other `CMAKE_BUILD_TYPE`.

## Command line instructions (linux, macos, windows)

For command line, tested on Linux and mac OS, **for windows might need more configuration options**.

Follow the usual sequence, assuming you have build dependencies as explained [here](\ref builddep):

~~~{.bash}
cmake -S Radium-Engine -B builds/radium-build-r -DCMAKE_BUILD_TYPE=Release -C installs/radium-external-r/radium-options.cmake
cmake --build builds/radium-build-r --config Release --parallel -DQt6_DIR=path/to/qt6
cmake --install builds/radium-build-r
~~~

### Qt configuration {#commandline_qt_configuration}

If Qt is installed system wide (likely on linux), `-DQt6_DIR` is not needed.

To ease maintenance accross Qt versions, you should also configure the path to Qt cmake package using `-DCMAKE_PREFIX_PATH=path/to/qt6`.

\note Running `cmake --install` is recommended as it will copy all the radium related library in the same place,
generate the cmake packages and bundle applications with their dependencies (on macos and windows).
