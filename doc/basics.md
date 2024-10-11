\page basicsmanual Radium Basics
[TOC]

This part of the documentation describes how to compile Radium and use it in your own project.

## Supported system and compilers

The following platforms and tool chains have been tested and should work :

* CMake 3.18+
* *Windows* : IDEs: Visual Studio 2019, 2022 (2017 is not supported due to embedded cmake version), QtCreator. Command Line: cmake+ninja+MSVC(2017, 2019, 2022) .
* *Mac OSX* : gcc 10 or higher, Apple clang, llvm clang 11 or higher
* *Linux* : gcc 8  or higher, clang

See also our Continuous Integration system at <https://github.com/STORM-IRIT/Radium-Engine/actions>.

## System wide dependencies

Radium expects the following dependencies to be available during compilation.

* [Engine]
  * OpenGL 4.1+ / GLSL 410+
* [Gui]
  * Qt Core/Widgets/OpenGL 5.15 or 6.2+
* [Headless] One, or both of
  * EGL
  * glfw 3.3

## External dependencies (included with radium)

These dependencies are shipped with radium, fetching external git sources.

<!--  (generated running ../scripts/list_dep.py from Radium-Engine/external directory) -->
* [IO]
  * assimp: https://github.com/assimp/assimp.git, [tags/v5.0.1],
    * with options `-DASSIMP_BUILD_ASSIMP_TOOLS=False -DASSIMP_BUILD_SAMPLES=False -DASSIMP_BUILD_TESTS=False -DIGNORE_GIT_HASH=True -DASSIMP_NO_EXPORT=True`
  * tinyply: https://github.com/ddiakopoulos/tinyply.git, [tags/2.3.2],
    * with options `-DSHARED_LIB=TRUE`
* [Gui]
  * PowerSlider: https://github.com/dlyr/PowerSlider.git, [origin/master],
    * with options `-DBUILD_DESIGNER_PLUGIN=OFF -DBUILD_EXAMPLE_APP=OFF`
* [Engine]
  * glm: https://github.com/g-truc/glm.git, [cc98465e3508535ba8c7f6208df934c156a018dc],
    * with options `-DGLM_TEST_ENABLE=OFF -DBUILD_STATIC_LIBS=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>`
  * glbinding: https://github.com/cginternals/glbinding.git, [758d33b4d4f1815d46d91fcf4ca4914d837267fa],
    * with options `-DOPTION_BUILD_TESTS=OFF -DOPTION_BUILD_DOCS=OFF -DOPTION_BUILD_TOOLS=OFF -DOPTION_BUILD_EXAMPLES=OFF -DOPTION_BUILD_OWN_KHR_HEADERS=ON -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>`
  * globjects: https://github.com/cginternals/globjects.git, [4363356ae2ef5c936ab078fba48d1ea507c295c4],
    * with options `-DOPTION_BUILD_DOCS=OFF -DOPTION_BUILD_EXAMPLES=OFF -DOPTION_USE_EIGEN=ON -Dglbinding_DIR=${glbinding_DIR} -Dglm_DIR=${glm_DIR} -DEigen3_DIR=${Eigen3_DIR} -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>`
  * stb: https://github.com/nothings/stb.git, [1034f5e5c4809ea0a7f4387e0cd37c5184de3cdd],
    * with options `None`
  * tinyEXR: https://github.com/MathiasPaulin/tinyexr.git, [origin/radium],
    * with options `-DBUILD_INSTALLABLE_LIB=ON`
* [Core]
  * Eigen3: https://gitlab.com/libeigen/eigen.git, [tags/3.4.0],
    * with options `-DEIGEN_TEST_CXX11=OFF -DBUILD_TESTING=OFF -DEIGEN_BUILD_DOC=OFF`
  * OpenMesh: https://www.graphics.rwth-aachen.de:9000/OpenMesh/OpenMesh.git, [tags/OpenMesh-8.1],
    * with options `-DBUILD_APPS=OFF`
  * cpplocate: https://github.com/cginternals/cpplocate.git, [tags/v2.2.0],
    * with options `-DOPTION_BUILD_TESTS=OFF -DOPTION_BUILD_DOCS=OFF -DCMAKE_INSTALL_PREFIX=<INSTALL_DIR>`
  * nlohmann_json: https://github.com/nlohmann/json.git, [tags/v3.10.5],
    * with options `-DJSON_Install=ON -DJSON_BuildTests=OFF`
<!--  (end script copy) -->

## TL;DR; command line version

```bash
git clone --recurse-submodules https://github.com/STORM-IRIT/Radium-Engine.git

# optionally switch to a specific branch or tag, e.g. release-candidate or v1.2.0
# git switch release-candidate # use git checkout for tags
# git submodule update --init --recursive # in case submodules have changed wrt master

# configure and build (install automatically) external, outside Radium-Engine directory
cmake -S Radium-Engine/external -B builds/radium-external-build-r -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=installs/radium-external-r
cmake --build builds/radium-external-build-r --config Release --parallel

#configure build and install radium-engine
cmake -S Radium-Engine -B builds/radium-build-r -DCMAKE_BUILD_TYPE=Release -C installs/radium-external-r/radium-options.cmake
cmake --build builds/radium-build-r --config Release --parallel
cmake --install builds/radium-build-r
```

default Radium-Engine install prefix is `Radium-Engine/Bundle-${CMAKE_CXX_COMPILER_ID}`.

\note These commands might need some custom search directory for cmake config, e.g. for Qt, notably on windows. Please check detailed instructions below. Also several options are available, e.g. to compile examples or use double instead of float as scalar type.

## Detailed instructions

More details and other systems information (including visual studio setup) are in the following pages, to be read in order.

* \subpage basicsCompileVs
* \subpage basicsCompileCommand
* \subpage basicsRadiumSubmodule : Link to Radium using cmake package
* \subpage basicsCompileRadiumDetails
* \subpage basicsTroubleshooting : Handle usual errors
